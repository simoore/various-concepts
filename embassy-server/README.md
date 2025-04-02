# msgserver

Add the target needed for stm32 cross compilation.

```bash
rustup target add thumbv7em-none-eabihf
```

Follow example to figure out initial setup and dependancies,
<https://github.com/embassy-rs/embassy/tree/main/examples/stm32h7>

And the embassy book is the go to reference,
https://embassy.dev/book/

## Features

* TCP Echo Server with default static IP and DHCP
* Debug logs via UART
* NTP client
* Networked bootloader

## Deployment

This is a rust alternative to OpenOCD

<https://probe.rs/docs/tools/cargo-flash/>

```bash
# Install probe-rs
curl --proto '=https' --tlsv1.2 -LsSf https://github.com/probe-rs/probe-rs/releases/latest/download/probe-rs-tools-installer.sh | sh

cargo flash --release --chip STM32H743ZI
```