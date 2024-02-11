use std::str::FromStr;

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
            "Get" => Ok(Self::Get),
            "Delete" => Ok(Self::Delete),
            "Post" => Ok(Self::Post),
            "Put" => Ok(Self::Put),
            "Head" => Ok(Self::Head),
            "Connect" => Ok(Self::Connect),
            "Options" => Ok(Self::Options),
            "Trace" => Ok(Self::Trace),
            "Patch" => Ok(Self::Patch),
            _ => Err(MethodError),
        }
    }
}

pub struct MethodError;
