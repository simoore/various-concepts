# Certificates

This provides a basic example of generating certificates that can be used for TLS.

## Creating a Certificate Authority

The resulting `smooreca.pem` should be distributed to all hosts that need to use the CA for trust. For TLS it 
is needed by the clients to trust they are talking to the correct server. For mTLS, it is needed by both so they
can verify keys from each other.

### Step 1) Generate the private key

```bash
# RSA-4096 is better for compatibility
openssl genpkey -algorithm RSA -out smooreca.key -pkeyopt rsa_keygen_bits:4096

# OR

# ECC secp384r1 is better for security and efficiency
openssl ecparam -name secp384r1 -genkey -noout -out smooreca.key
```

### Step 2) Create the certificate of the CA

```bash
openssl req -x509 -new -nodes -key smooreca.key -sha384 -days 100 -out smooreca.pem \
    -subj "/C=AU/ST=NSW/L=Sydney/O=SMooreOrg/OU=Security/CN=SMooreCA" \
    -addext "basicConstraints=critical,CA:TRUE,pathlen:0" \
    -addext "keyUsage=critical,keyCertSign,cRLSign" \
    -addext "subjectKeyIdentifier=hash"
```

You can also create a certificate signing request which contains some of the common information to make it
easier to generate a new certificate when needed.

### Step 3) Verify root certificate

```bash
openssl x509 -noout -text -in smooreca.pem
```

## Create server certificates 

The resulting private key `server.key` and certifiate `server.crt` need to be accessed by the server.

The following process generates a `smooreca.srl` file. Keep this for signing multiple certificates as it places
a serial number in each certificate it signs and this ensures uniqueness of the serial numbers used by the CA.

### Step 1) Generate the private key

```bash
openssl ecparam -name secp384r1 -genkey -noout -out server.key
```

### Step 2) Create a certificate signing request

```bash
openssl req -new -key server.key -out server.csr \
    -subj "/C=AU/ST=NSW/L=Sydney/O=SMooreOrg/OU=IT/CN=myserver.example.com"
```

### Step 3) Create a certificate configuration file

Create a file called `server.cnf` with the following contents,

```ini
[ server_cert ]
basicConstraints = critical,CA:FALSE
keyUsage = critical, digitalSignature, keyEncipherment
extendedKeyUsage = serverAuth
subjectAltName = @alt_names

[ alt_names ]
DNS.1 = myserver.example.com
DNS.2 = www.myserver.example.com
```

### Step 4) Sign the server certificate

```bash
openssl x509 -req -in server.csr -CA smooreca.pem -CAkey smooreca.key -CAcreateserial \
    -out server.crt -days 100 -sha384 -extfile server.cnf -extensions server_cert
```

### Step 5) Verify the server certificate

```bash
openssl x509 -noout -text -in server.crt
```

## Create client certificates

We present an alternative approach that puts more of the details in the configuration file `client.cnf`

```ini
[ req ]
default_bits       = 2048
default_keyfile    = client.key
distinguished_name = req_distinguished_name
req_extensions     = req_ext
prompt             = no

[ req_distinguished_name ]
C  = AU
ST = NSW
L  = Sydney
O  = SMooreOrg
OU = IT
CN = client.example.com

[ req_ext ]
subjectAltName = @alt_names
extendedKeyUsage = clientAuth

[ alt_names ]
DNS.1 = client.example.com
DNS.2 = client
```

First create a key in the same way you did for the server and call the file `client.key`, and the  generate the request, and finally generate the certificate,

```bash
# Generate the request
openssl req -new -key client.key -out client.csr -config client.cnf

# Generate the certificate
openssl x509 -req -in client.csr -CA smooreca.pem -CAkey smooreca.key -CAcreateserial \
    -out client.crt -days 365 -sha256 -extfile client.cnf -extensions req_ext
```

The resulting `client.key` and `client.crt` need to be accessed by the client application.

## Debugging

Open a client to read certificates from server,

```bash
openssl s_client -connect 127.0.0.1:4433 -crlf -verify 1
```

Check that the root certificate can validate server certificates,

```bash
openssl verify -CAfile smooreca.pem server.crt
```

## References

* <https://arminreiter.com/2022/01/create-your-own-certificate-authority-ca-using-openssl/>
* <https://learn.microsoft.com/en-us/azure/application-gateway/self-signed-certificates>
