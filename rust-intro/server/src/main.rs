use http::Method;
use http::Request;
use server::Server;

mod http;
mod server;

fn main() {
    let get = Method::Get;
    let delete = Method::Delete;

    let server = Server::new("127.0.0.1:8080".to_string());
    server.run();
}


