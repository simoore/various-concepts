use std::fs::File;
use std::io::BufReader;
use std::path::Path;

use rustls::server::WebPkiClientVerifier;
use rustls::{ClientConfig, RootCertStore, ServerConfig};
use rustls_pki_types::{CertificateDer, PrivateKeyDer};

#[derive(Debug, thiserror::Error)]
pub enum MtlsError {
    #[error("CertificateNotFound {0}")]
    CertificateNotFound(String),
    #[error("Hyper Error")]
    Hyper(#[from] hyper::Error),
    #[error("HyperHttp Error")]
    HyperHttp(#[from] hyper::http::Error),
    #[error("InvalidDnsName Error")]
    InvalidDnsName(#[from] rustls::pki_types::InvalidDnsNameError),
    #[error("IO Error: {0}")]
    Io(#[from] std::io::Error),
    #[error("Key Error")]
    KeyError,
    #[error("No certs found")]
    NoCertsFound,
    #[error("Rustls Error")]
    RustlsError(#[from] rustls::Error),
    #[error("VerifierBuilderError")]
    VerifierBuilderError(#[from] rustls::server::VerifierBuilderError),
}

/// Load public certificate from file.
///
/// @param cert_path
///     Path to the file containing the certificates.
/// @return
///     A list of certificates.
pub fn load_pem_certs(cert_path: &Path) -> Result<Vec<CertificateDer<'static>>, MtlsError> {
    if !cert_path.exists() {
       return Err(MtlsError::CertificateNotFound(cert_path.to_str().unwrap_or("Unknown").to_string()));
    }
    println!("Loading cert file {:?}", cert_path.canonicalize()?);
    let cert_file = File::open(cert_path)?;
    let mut cert_reader = BufReader::new(cert_file);
    let certs: Vec<CertificateDer> = rustls_pemfile::certs(&mut cert_reader)
        .filter_map(|x| x.ok())
        .collect();
    if certs.is_empty() {
        return Err(MtlsError::NoCertsFound);
    }
    Ok(certs)
}

/// Load private key from file.
///
/// @param key_path
///     Path to the file containing the key.
/// @return
///     The key.
pub fn load_private_key(key_path: &Path) -> Result<PrivateKeyDer<'static>, MtlsError> {
    println!("Loading cert file {:?}", key_path);
    let key_file = File::open(key_path)?;
    let mut key_reader = BufReader::new(key_file);
    let key_io = rustls_pemfile::private_key(&mut key_reader)?;
    key_io.ok_or(MtlsError::KeyError)
}

/// Creates the rustls server config for this application.
///
/// @param certs
///     The loaded server certificates.
/// @param key
///     The loaded server private key.
/// @param ca_certs
///     The certificate authority certificates.
/// @return
///     A server config that can be used to establish a TLS connection.
pub fn build_server_config(
    certs: Vec<CertificateDer<'static>>,
    key: PrivateKeyDer<'static>,
    ca_certs: Vec<CertificateDer<'static>>,
) -> Result<ServerConfig, MtlsError> {
    let mut root_cert_store = RootCertStore::empty();
    root_cert_store.add_parsable_certificates(ca_certs);
    let client_verifier = WebPkiClientVerifier::builder(root_cert_store.into()).build()?;
    let server_config = ServerConfig::builder()
        .with_client_cert_verifier(client_verifier)
        .with_single_cert(certs, key)?;
    Ok(server_config)
}

/// Creates the rustls client config for this application.
///
/// @param certs
///     The loaded client certificates.
/// @param key
///     The loaded client private key.
/// @param ca_certs
///     The certificate authority certificates.
/// @return
///     A client config that can be used to establish a TLS connection.
pub fn build_client_config(
    certs: Vec<CertificateDer<'static>>,
    key: PrivateKeyDer<'static>,
    ca_certs: Vec<CertificateDer<'static>>,
) -> Result<ClientConfig, MtlsError> {
    let mut root_cert_store = RootCertStore::empty();
    root_cert_store.add_parsable_certificates(ca_certs);
    let client_config = ClientConfig::builder()
        .with_root_certificates(root_cert_store)
        .with_client_auth_cert(certs, key)?;
    Ok(client_config)
}
