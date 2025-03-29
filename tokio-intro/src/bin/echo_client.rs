use tokio::{io::{AsyncReadExt, AsyncWriteExt}, net::TcpStream};

async fn echo() {
    log::info!("Connecting to echo server");
    let mut stream = TcpStream::connect("127.0.0.1:6379").await.unwrap();
    stream.write_all("Hello server".as_bytes()).await.unwrap();
    let mut buf = vec![0; 1024];
    let n = stream.read(&mut buf).await.unwrap();
    log::info!("Received {}", std::str::from_utf8(&buf[0..n]).unwrap());
    stream.shutdown().await.unwrap();
    log::info!("Stream is closed");
}

#[tokio::main]
async fn main() {
    env_logger::builder().filter_level(log::LevelFilter::Info).init();
    echo().await;
}