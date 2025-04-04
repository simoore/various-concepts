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

* TCP Echo Server with default static IP
* Debug logs

## Deployment

This is a rust alternative to OpenOCD, download the binaries from github, and add it to your path

<https://probe.rs/docs/tools/cargo-flash/>

Then the `.cargo/config` has bee setup so you just need to use `cargo run` to flash and execute.

It seems that cargo doesn't read `.cargo/config.toml` for environmental variables on windows. Instead use this
for setting log level,

```batch
set DEFMT_LOG=debug
```
