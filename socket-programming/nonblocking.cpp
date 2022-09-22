#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

class Echoer {
public:
    
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC CONSTANTS
    ///////////////////////////////////////////////////////////////////////////

    static constexpr const char *sPort = "4950";
    static constexpr int sMaxBufLen = 100;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    using AddrInfo = struct addrinfo;
    using SockAddrStorage = struct sockaddr_storage;
    using SockAddr = struct sockaddr;
    using SockAddrIn = struct sockaddr_in;
    using SockAddrIn6 = struct sockaddr_in6;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    Echoer() {
        AddrInfo *serverInfo = getAddrInfo();
        mSocket = bindSocket(serverInfo);

        // This makes the socket non-blocking. F_SETFL is set file status flags.
        fcntl(mSocket, F_SETFL, O_NONBLOCK);
        freeaddrinfo(serverInfo);
        std::cout << "Waiting to recvfrom..." << std::endl;
    }

    ~Echoer() {
        close(mSocket);
    }

    void service() {
        SockAddrStorage theirAddr;
        SockAddr *theirAddrPtr = reinterpret_cast<SockAddr *>(&theirAddr);
        char buf[sMaxBufLen];
        socklen_t addrlen = sizeof(theirAddr);

        // For non-blocking recvfrom returns -1 if no data is available and the errno can be checked. EAGAIN or 
        // EWOULDBLOCK indicate normal operation for non-blocking read.
        int size = recvfrom(mSocket, buf, sMaxBufLen - 1, 0, theirAddrPtr, &addrlen);
        if (size == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Recvfrom error");
                exit(1);
            } else {
                std::cout << "No data available" << std::endl;
                sleep(1);
                return;
            }
        }
        buf[size] = '\0';

        char s[INET6_ADDRSTRLEN];
        inet_ntop(theirAddr.ss_family, getInAddr(theirAddrPtr), s, sizeof(s));
        std::cout << "server: recvfrom " << s << std::endl;

        std::string msg = "Hello from echoer: " + std::string(buf);
        std::cout << "Sending > " << msg << std::endl;
        size = sendto(mSocket, msg.c_str(), msg.size(), 0, theirAddrPtr, addrlen);
        if (size == -1) {
            perror("Sendto error");
            exit(1);
        }
    }

private:

    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// @brief SockAddr can either be cast to a SockAddrIn for IPv4 or SockAddrIn6 for IPv6.
    /// @param sa The SockAddr
    /// @return 
    const void *getInAddr(SockAddr *sa) const {
        if (sa->sa_family == AF_INET) {
            return &reinterpret_cast<SockAddrIn *>(sa)->sin_addr;
        }
        return &reinterpret_cast<SockAddrIn6 *>(sa)->sin6_addr;
    }

    /// @return A linked-list of suitable addresses that can be used by this server. Pass this linked list to
    /// bindSocket() to bind the address to a listening socket.
    AddrInfo *getAddrInfo() const {
        AddrInfo hints = {};
        hints.ai_family = AF_INET6;        // IPv4 or IPv6 - AF_UNSPEC should be used to allow both IPv4 and IPv6.
        hints.ai_socktype = SOCK_DGRAM;    // UDP protocol
        hints.ai_flags = AI_PASSIVE;       // Use local host

        AddrInfo *serverInfo = nullptr;

        // The first parameter is the host name (domain name or IP address)
        // If the node parameter is null, and ai_flags in AI_PASSIVE, then the returned addresses will be suitable
        // for binding a socket the accepts connections.
        int status = getaddrinfo(nullptr, sPort, &hints, &serverInfo);
        if (status != 0) {
            std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
            exit(1);
        }

        return serverInfo;
    }

    /// Binds the first suitable address in `serverInfo` to a socket.
    /// @return The socket file descriptor. -1 if it was unable to bind a socket.
    int bindSocket(const AddrInfo *serverInfo) const {
        
        for (const AddrInfo *p = serverInfo; p != nullptr; p = p->ai_next) {

            // Creates a socket and returns the file descriptor for the socket.
            // Parameters: 
            // domain: PF_INET for IPv4 or PF_INET6 for IPv6
            // type: SOCK_STREAM or SOCK_DGRAM for connection or connectionless socket (there are other types).
            // protocol: The protocol to use like TCP or UDP. Can set to 0 to let the type determine the protocol.
            int s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (s == -1) {
                perror("Socket creation error");
                continue;
            }

            // Binds the address to the socket. The sockaddr contains a field for address family and the 14 byte
            // space for address information. IPv4 will store the port than the IP address in this field. This makes
            // address length 6 bytes.
            int status = bind(s, p->ai_addr, p->ai_addrlen);
            if (status == -1) {
                close(s);
                perror("Socket binding error");
                continue;
            }
            return s;
        }
        std::cerr << "server: failed to bind" << std::endl;
        exit(1);
        return -1;
    }

    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE VARIABLES
    ///////////////////////////////////////////////////////////////////////////

    int mSocket{0};

};

int main() {
    Echoer echoer;
    while (true) {
        echoer.service();
    }
}