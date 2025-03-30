use std::path::Path;
use std::sync::Arc;

use http::{Method, Request, Response, StatusCode};
use http_body_util::{BodyExt, Full};
use hyper::body::{Bytes, Incoming};
use hyper_util::rt::TokioIo;
use tokio::net::TcpListener;
use tokio_rustls::TlsAcceptor;

use hypermtls::MtlsError;

// Custom echo service, handling two different routes and a catch-all 404 responder.
async fn echo(req: Request<Incoming>) -> Result<Response<Full<Bytes>>, hyper::Error> {
    let mut response = Response::new(Full::default());
    match (req.method(), req.uri().path()) {
        // Help route.
        (&Method::GET, "/") => {
            *response.body_mut() = Full::from("Try POST /echo\n");
        }
        // Echo service route.
        (&Method::POST, "/echo") => {
            *response.body_mut() = Full::from(req.into_body().collect().await?.to_bytes());
        }
        // Catch-all 404.
        _ => {
            *response.status_mut() = StatusCode::NOT_FOUND;
        }
    };
    Ok(response)
}

/// Initialize the server and listen for connections.
async fn run_server() -> Result<(), MtlsError> {
    println!("Running server");
    let cert_path = Path::new("../certificates/artifacts/server.crt");
    let key_path = Path::new("../certificates/artifacts/server.key");
    let ca_cert_path = Path::new("../certificates/artifacts/smooreca.pem");
    let addr = std::net::SocketAddr::new(std::net::Ipv4Addr::LOCALHOST.into(), 1337);

    println!("Loading certificates");
    let certs = hypermtls::load_pem_certs(cert_path)?;
    let key = hypermtls::load_private_key(key_path)?;
    let ca_certs = hypermtls::load_pem_certs(ca_cert_path)?;
    let server_config = hypermtls::build_server_config(certs, key, ca_certs)?;

    println!("Starting to serve on https://{}", addr);
    let tls_acceptor = TlsAcceptor::from(Arc::new(server_config));
    let service = hyper::service::service_fn(echo);
    let listener = TcpListener::bind(&addr).await?;

    loop {
        let (tcp_stream, _remote_addr) = listener.accept().await?;
        let tls_stream = match tls_acceptor.accept(tcp_stream).await {
            Ok(tls_stream) => tls_stream,
            Err(e) => {
                println!("TLS acceptor error, {:?}", e);
                continue;
            }
        };
        tokio::spawn(async move {
            let http = hyper::server::conn::http1::Builder::new();
            let conn = http.serve_connection(TokioIo::new(tls_stream), service);
            if let Err(e) = conn.await {
                println!("Failed to serve connection: {:#}", e);
            }
        });
    }
}

#[tokio::main]
async fn main() {
    if let Err(e) = run_server().await {
        println!("Server error {}", e);
    }
}
