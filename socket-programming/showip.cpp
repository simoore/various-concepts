#include <iostream>
#include <assert.h>
#include <sys/types.h>    
#include <sys/socket.h>
#include <netdb.h>          // POSIX.1 standard header, part of glibc
#include <arpa/inet.h>
#include <netinet/in.h>

using AddrInfo = struct addrinfo;
using SockAddrIn = struct sockaddr_in;      // IPv4 socket addr
using SockAddrIn6 = struct sockaddr_in6;    // IPv6 socket addr

int main() {

    // Default initialization, this calls default constructor on fields with no initial value defined.
    AddrInfo hints = {};

    assert(hints.ai_flags == 0);
    assert(hints.ai_family == 0);
    assert(hints.ai_socktype == 0);
    assert(hints.ai_protocol == 0);
    assert(hints.ai_addrlen == 0);
    assert(hints.ai_canonname == nullptr);
    assert(hints.ai_addr == nullptr);
    assert(hints.ai_next == nullptr);

    hints.ai_family = AF_UNSPEC;        // We don't know if we are using IPv4 or IPv6.
    hints.ai_socktype = SOCK_STREAM;    // TCP socket.

    std::cout << "The IP addresses of 'www.example.net' are:" << std::endl;

    AddrInfo *result;
    int status = getaddrinfo("www.example.net", nullptr, &hints, &result);
    if (status != 0) {
        std::cerr << "getaddeinfor: " << gai_strerror(status) << std::endl;
        return EXIT_FAILURE;
    }

    const void *addr;
    const char *ipver;
    for (AddrInfo *node = result; node != nullptr; node = node->ai_next) {
        if (node->ai_family == AF_INET) {
            SockAddrIn *ipv4 = reinterpret_cast<SockAddrIn *>(node->ai_addr);
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // AF_INET6
            SockAddrIn6 *ipv6 = reinterpret_cast<SockAddrIn6 *>(node->ai_addr);
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        char ipstr[INET6_ADDRSTRLEN];
        inet_ntop(node->ai_family, addr, ipstr, sizeof(ipstr));
        std::cout << "  " << ipver << ": " << ipstr << std::endl;
    }


    freeaddrinfo(result);
    return EXIT_SUCCESS;
}
