use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};

async fn process(mut socket: TcpStream) {
    log::info!("Processing open connection");

    let mut buf = vec![0; 1024];
    loop {
        let n = socket.read(&mut buf).await.expect("Read socket error");
        if n == 0 {
            break;
        }
        log::info!("Received {}", std::str::from_utf8(&buf[0..n]).unwrap());
        socket.write_all(&buf[0..n]).await.expect("Write socket error");
    }
    log::info!("Socket is closing");
}

async fn listen() {
    log::info!("Opening listening socket");
    let listener = TcpListener::bind("127.0.0.1:6379").await.unwrap();
    loop {
        let (socket, _) = listener.accept().await.unwrap();
        tokio::spawn(async move { process(socket).await; });
    }
}

#[tokio::main]
async fn main() {
    env_logger::builder().filter_level(log::LevelFilter::Info).init();
    listen().await;
}