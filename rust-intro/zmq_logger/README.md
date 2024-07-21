```bash
# Execute this from the workspace folder above
cargo new zmq_logger
cd zmq_logger
cargo build

cargo add zmq
cargo add -F static hdf5-sys
cargo add hdf5

# run tests
cargo test

# run tests and enable println! on successful tests
cargo test -- --nocapture
```

For gtk programming in rust see: [GUI development with Rust and GTK 4](https://gtk-rs.org/gtk4-rs/stable/latest/book/introduction.html)