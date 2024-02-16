use std::str::FromStr;

#[derive(Debug)]
pub enum Method {
    Get,
    Delete,
    Post,
    Put,
    Head,
    Connect,
    Options,
    Trace,
    Patch,
}

impl FromStr for Method {
    type Err = MethodError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "GET" => Ok(Self::Get),
            "DELETE" => Ok(Self::Delete),
            "POST" => Ok(Self::Post),
            "PUT" => Ok(Self::Put),
            "HEAD" => Ok(Self::Head),
            "CONNECT" => Ok(Self::Connect),
            "OPTIONS" => Ok(Self::Options),
            "TRACE" => Ok(Self::Trace),
            "PATCH" => Ok(Self::Patch),
            _ => Err(MethodError),
        }
    }
}

pub struct MethodError;
