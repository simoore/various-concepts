#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <tuple>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


/// Sends a message to a UDP server on port 4950.
class Talker {
public:

    static constexpr const char *sServerPort = "4950";
    using AddrInfo = struct addrinfo;

    Talker(const std::string hostname, const std::string message) {
        
        AddrInfo *serverInfo = getAddrInfo(hostname);
        auto [sockfd, addr, addrlen] = createSocket(serverInfo);
        freeaddrinfo(serverInfo);

        int numbytes = sendto(sockfd, message.c_str(), message.size(), 0, addr, addrlen);
        if (numbytes == -1) {
            perror("talker: sendto error");
            exit(1);
        }
        std::cout << "talker: sent " << numbytes << " to " << hostname << std::endl;
        close(sockfd);
    }

    /// Loops through all returned addresses to find an appropriate socket to talk to.
    std::tuple<int, const sockaddr *, socklen_t> createSocket(const AddrInfo *serverinfo) const {
        for (const AddrInfo *p = serverinfo; p != nullptr; p = p->ai_next) {
            int sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sockfd == -1) {
                perror("talker: socket creation failed");
                continue;
            }
            return std::make_tuple(sockfd, p->ai_addr, p->ai_addrlen);
        }
        std::cerr << "talker: failed to create socket" << std::endl;
        exit(2);
        return std::make_tuple(-1, nullptr, 0);
    }

    /// @return A linked-list of suitable addresses that can be used by this server. Pass this linked list to
    /// bindSocket() to bind the address to a listening socket.
    AddrInfo *getAddrInfo(const std::string hostname) const {

        AddrInfo hints = {};
        hints.ai_family = AF_INET6;     // IPv4 or IPv6 - AF_UNSPEC should be used to allow both IPv4 and IPv6.
        hints.ai_socktype = SOCK_DGRAM; // UDP protocol

        AddrInfo *serverInfo = nullptr;

        // The first parameter is the host name (domain name or IP address)
        // If the node parameter is null, and ai_flags in AI_PASSIVE, then the returned addresses will be suitable
        // for binding a socket the accepts connections.
        int status = getaddrinfo(hostname.c_str(), sServerPort, &hints, &serverInfo);
        if (status != 0) {
            std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
            exit(1);
        }

        return serverInfo;
    }
};


int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "usage: talker hostname message" << std::endl;
        exit(1);
    }
    std::string hostname = argv[1];
    std::string message = argv[2];
    Talker talker(hostname, message);
    return 0;
}