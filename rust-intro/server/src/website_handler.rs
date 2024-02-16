use crate::http::{Method, Request, Response, StatusCode};
use std::fs;
use super::server::Handler;

pub struct WebsiteHandler {
    public_path: String
}

impl WebsiteHandler {
    pub fn new(public_path: String) -> Self {
        Self { public_path }    
    }

    fn read_file(&self, file_path: &str) -> Option<String> {
        let path = format!("{}\\{}", self.public_path, file_path);
        let public_path_canonical = fs::canonicalize(&self.public_path).unwrap();
        match fs::canonicalize(path) {
            Ok(path) => {
                if path.to_string_lossy().starts_with(&public_path_canonical.to_string_lossy()[..]) {
                    fs::read_to_string(path).ok()
                } else {
                    println!("Directory Traversal Attack Attempted: {}", path.to_string_lossy());
                    None
                }
            }
            Err(_) => None,
        }
    }
}

impl Handler for WebsiteHandler {
    fn handle_request(&mut self, request: &Request) -> Response {
        match request.method() {
            Method::Get => match request.path() {
                "/" => Response::new(StatusCode::Ok, self.read_file("index.html")),
                "/hello" => Response::new(StatusCode::Ok, self.read_file("hello.html")),
                path => match self.read_file(path) {
                    Some(contents) => Response::new(StatusCode::Ok, Some(contents)),
                    None => Response::new(StatusCode::NotFound, None),
                }
            }
            _ => Response::new(StatusCode::NotFound, None),
        }
    }
}