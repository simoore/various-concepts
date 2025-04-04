#![no_std]
#![no_main]

use embassy_executor::Spawner;
use embassy_net::tcp::TcpSocket;
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_time::Timer;
use {defmt_rtt as _, panic_probe as _};

#[embassy_executor::task]
async fn blink_task(mut led: Output<'static>) -> ! {
    loop {
        led.set_high();
        Timer::after_millis(500).await;
        led.set_low();
        Timer::after_millis(500).await;
    }
}

#[embassy_executor::task(pool_size = 2)]
async fn echo_server_task(stack: embassy_net::Stack<'static>, port: u16) -> ! {
    let mut rx_buffer = [0u8; 1024];
    let mut tx_buffer = [0u8; 1024];
    let mut buf = [0u8; 1024];
    loop {
        let mut socket = TcpSocket::new(stack, &mut rx_buffer, &mut tx_buffer);
        if let Err(e) = socket.accept(port).await {
            defmt::error!("Socket accept error {:?}", e);
            continue;
        }
        loop {
            let n = match socket.read(&mut buf).await {
                Ok(n) => n,
                Err(e) => {
                    defmt::error!("Error reading socket {:?}", e);
                    break;
                }
            };
            if n == 0 {
                defmt::info!("Socket is closing");
                break;
            }
            let Ok(message) = core::str::from_utf8(&buf[..n]) else {
                defmt::error!("Couldn't deserialize message");
                continue;
            };
            defmt::info!("Received {}: {}", port, message);
            if let Err(e) = socket.write(&buf[..n]).await {
                defmt::error!("Write error {:?}", e);
            }
        }
    }
}

#[embassy_executor::main]
async fn main(spawner: Spawner) {

    let p = msgserver::init::init_mcu();
    let stack = msgserver::init::init_network(
        &spawner, p.ETH, p.PA1, p.PA2, p.PC1, p.PA7, p.PC4, p.PC5, p.PG13, p.PB13, p.PG11, 
    )
    .await;

    defmt::info!("Hello World!");

    let led = Output::new(p.PB14, Level::High, Speed::Low);
    spawner.spawn(blink_task(led)).unwrap();
    spawner.spawn(echo_server_task(stack, 2345)).unwrap();
    spawner.spawn(echo_server_task(stack, 6789)).unwrap();

    loop {
        defmt::info!("This is the main loop!");
        Timer::after_millis(10000).await;
    }
}
