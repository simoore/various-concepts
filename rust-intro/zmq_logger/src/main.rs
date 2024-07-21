fn new_subscribe_socket(ctx: &zmq::Context) -> Result<zmq::Socket, zmq::Error> {
    let subscriber = match ctx.socket(zmq::SUB) {
        Ok(subscriber) => subscriber,
        Err(e) => {
            println!("Failed to create socket, trying again in 1s");
            return Err(e);
        }
    };
    if let Err(e) = subscriber.connect("tcp://*:8888") {
        println!("Failed to bind socket, trying again in 1s");
        return Err(e);
    };
    if let Err(e) = subscriber.set_subscribe(b"TestTopic") {
        println!("Failed to set topic, trying again in 1s");
        return Err(e);
    };
    Ok(subscriber)
}

fn main() {
    println!("Hello, world!");

    let ctx = zmq::Context::new();

    loop {
        let Ok(subscriber) = new_subscribe_socket(&ctx) else {
            std::thread::sleep(std::time::Duration::from_millis(1000));
            continue;
        };
        loop {
            match subscriber.recv_bytes(zmq::DONTWAIT) {
                Ok(bytes) => {
                    println!("Received bytes {}", String::from_utf8_lossy(&bytes));
                }
                Err(e) => {
                    println!("Error receiving {}", e);
                    if e != zmq::Error::EAGAIN {
                        println!("Socket in error, trying again receiving {}", e);
                        break;
                    }
                }
            }
            std::thread::sleep(std::time::Duration::from_millis(10));
        }
    }
}
