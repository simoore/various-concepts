use bytes::Bytes;
use http_body_util::{BodyExt, Empty};
use hyper::Request;
use hyper_util::rt::TokioIo;
use tokio::io::{self, AsyncWriteExt as _};
use tokio::net::TcpStream;

#[derive(Debug, thiserror::Error)]
enum ClientError {
    #[error("IO Error")]
    IO(#[from] io::Error),
    #[error("Hyper Error")]
    Hyper(#[from] hyper::Error),
    #[error("HyperHttp Error")]
    HyperHttp(#[from] hyper::http::Error),
}

#[tokio::main]
async fn main() {
    let url = "http://www.example.com";
    let url = url.parse::<hyper::Uri>().unwrap();
    if url.scheme_str() != Some("http") {
        println!("This example only works with 'http' URLs.");
        return;
    }

    if let Err(e) = fetch_url(url).await {
        log::error!("Client error {:?}", e);
    }
}

async fn fetch_url(url: hyper::Uri) -> std::result::Result<(), ClientError> {
    let host = url.host().expect("uri has no host");
    let port = url.port_u16().unwrap_or(80);
    let addr = format!("{}:{}", host, port);
    let stream = TcpStream::connect(addr).await?;
    let io = TokioIo::new(stream);

    let (mut sender, conn) = hyper::client::conn::http1::handshake(io).await?;
    tokio::task::spawn(async move {
        if let Err(err) = conn.await {
            println!("Connection failed: {:?}", err);
        }
    });

    let authority = url.authority().unwrap().clone();
    let path = url.path();
    let req = Request::builder()
        .uri(path)
        .header(hyper::header::HOST, authority.as_str())
        .body(Empty::<Bytes>::new())?;

    let mut res = sender.send_request(req).await?;

    println!("Response: {}", res.status());
    println!("Headers: {:#?}\n", res.headers());

    // Stream the body, writing each chunk to stdout as we get it (instead of buffering and printing at the end).
    while let Some(next) = res.frame().await {
        let frame = next?;
        if let Some(chunk) = frame.data_ref() {
            io::stdout().write_all(chunk).await?;
        }
    }
    io::stdout().write_all(b"\nDone!\n").await?;

    Ok(())
}
