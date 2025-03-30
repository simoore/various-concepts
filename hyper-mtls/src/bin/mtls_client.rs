use std::path::Path;
use std::sync::Arc;

use http_body_util::BodyExt;
use hyper::Request;
use hyper_util::rt::TokioIo;
use tokio::io::AsyncWriteExt;
use tokio::net::TcpStream;
use tokio_rustls::TlsConnector;

use hypermtls::MtlsError;

async fn run_client() -> Result<(), MtlsError> {
    let cert_path = Path::new("../certificates/artifacts/client.crt");
    let key_path = Path::new("../certificates/artifacts/client.key");
    let ca_cert_path = Path::new("../certificates/artifacts/smooreca.pem");

    let server_addr = "127.0.0.1:1337";

    // This value needs to match what is in the certificate.
    let domain = "myserver.example.com".to_string();

    let certs = hypermtls::load_pem_certs(cert_path)?;
    let key = hypermtls::load_private_key(key_path)?;
    let ca_certs = hypermtls::load_pem_certs(ca_cert_path)?;
    let client_config = hypermtls::build_client_config(certs, key, ca_certs)?;

    println!("Starting to serve on https://{}", server_addr);
    let tls_connector = TlsConnector::from(Arc::new(client_config));
    let stream = TcpStream::connect(server_addr).await?;
    let domain_ref = rustls::pki_types::ServerName::try_from(domain)?;
    let tls_stream = tls_connector.connect(domain_ref, stream).await?;
    let io = TokioIo::new(tls_stream);

    let (mut sender, conn) = hyper::client::conn::http1::handshake(io).await?;
    tokio::task::spawn(async move {
        if let Err(err) = conn.await {
            println!("Connection failed: {:?}", err);
        }
    });

    let req = Request::builder()
        .uri("https://127.0.0.1:1337/echo")
        .method("POST")
        .body("Hello from client".to_string())?;
    let mut res = sender.send_request(req).await?;

    println!("Response: {}", res.status());
    println!("Headers: {:#?}\n", res.headers());

    // Stream the body, writing each chunk to stdout as we get it (instead of buffering and printing at the end).
    while let Some(next) = res.frame().await {
        let frame = next?;
        if let Some(chunk) = frame.data_ref() {
            tokio::io::stdout().write_all(chunk).await?;
        }
    }
    tokio::io::stdout().write_all(b"\nDone!\n").await?;

    Ok(())
}

#[tokio::main]
async fn main() {
    if let Err(e) = run_client().await {
        println!("Client error {:?}", e);
    }
}
