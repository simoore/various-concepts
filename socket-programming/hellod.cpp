/// Instructions
/// ------------
/// Execute this server. From another machine run: `telnet <IPAddr> 4490` to receive the hello message.
/// You can use 127.0.0.1 if using the same machine.
///
/// Reference
/// ---------
/// https://beej.us/guide/bgnet/html/#client-server-background

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

/// This server sends the string "Hello, World!" on any new connection, then closes the connection.
class HelloServer {
public:

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC CONSTANTS
    ///////////////////////////////////////////////////////////////////////////

    /// The port of this server.
    static constexpr const char *sPort = "4490";

    /// The number of queued connections supported by this server.
    static constexpr int sBacklog = 10;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////
    
    using AddrInfo = struct addrinfo;
    using SockAddr = struct sockaddr;
    using SockAddrStorage = struct sockaddr_storage;
    using SockAddrIn = struct sockaddr_in;
    using SockAddrIn6 = struct sockaddr_in6;
    using SigAction = struct sigaction;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    /// The constructor creates a listening socket for this server.
    HelloServer() {
        AddrInfo *serverInfo = getAddrInfo();
        mListeningSocket = bindSocket(serverInfo);
        freeaddrinfo(serverInfo);
    }

    /// Creates the listening socket which enables the server to accept connections.
    void startListening() {
        int status = listen(mListeningSocket, sBacklog);
        if (status == -1) {
            perror("Listening error");
            exit(1);
        }

        // Signals are software interrupts from other processes - usually the kernal. Here we are handling the SIGCHILD
        // signal from the kernal which appears when a child process exits. Note we can ignore this signal and make
        // the kernal clean up child processes with the call signal(SIGCHILD, SIG_IGN);
        SigAction sa;
        sa.sa_handler = sigchildHandler;

        // sa_mask can be used to prevent other signals from interrupting this signal when executing.
        sigemptyset(&sa.sa_mask);

        // Automatically restarts system calls interrupted by this signal.
        sa.sa_flags = SA_RESTART;

        // When a child process exits, the kernal sends a SIGCHILD signal to the parent. The child process remains a
        // zombie process on the process table so the parent process can read its exit code.
        status = sigaction(SIGCHLD, &sa, nullptr);
        if (status == -1) {
            perror("sigaction error");
            exit(1);
        }

        std::cout << "server: waiting for connections..." << std::endl;
    }

    void service() {
        // When sockaddr was originally defined, it could hold the IPv4, but not IPv6. sockaddr_storage is larger and
        // was included to hold both.
        socklen_t sinSize = sizeof(SockAddrStorage);
        SockAddrStorage theirAddr;
        SockAddr *theirAddrPtr = reinterpret_cast<SockAddr *>(&theirAddr);

        // Accept will take a queued connection waiting to be accepted and return a brand new socket file descriptor.
        // Accept will block if no connections waiting. Blocking on unix operating systems puts the thread to sleep
        // until a connection arrives.
        int fd = accept(mListeningSocket, theirAddrPtr, &sinSize);
        if (fd == -1) {
            perror("Accept connection error");
            return;
        }

        // Converts the IP address of the connection to string from display.
        char s[INET6_ADDRSTRLEN];
        inet_ntop(theirAddr.ss_family, getInAddr(theirAddrPtr), s, sizeof(s));
        std::cout << "server: connection from " << s << std::endl;

        // Fork creates a new process by duplicating this process. This sounds very heavy. If this is the parent
        // process, fork returns the PID of the child process. For the child process, 0 is returned. -1 is returned
        // on failure to fork. The fork copies the file descriptors (the sockets) from the previous process so closing
        // the socket in one process will not affect the socket in the other process.
        if (!fork()) {

            // If we are a child process, we close our copy of the listening socket, then we send a messages, then we
            // close our copy of the connected socket before exiting the process.
            close(mListeningSocket);
            int status = send(fd, "Hello, World!", 13, 0);
            if (status == -1) {
                perror("Send error");
            }
            close(fd);
            exit(0);
        }

        // The child process has its own socket file descriptor to send data with. We can close ours.
        close(fd);
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
        hints.ai_family = AF_INET;          // IPv4 or IPv6 - AF_UNSPEC should be used to allow both IPv4 and IPv6.
        hints.ai_socktype = SOCK_STREAM;    // TCP protocol
        hints.ai_flags = AI_PASSIVE;        // Use local host

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
            int listeningSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (listeningSocket == -1) {
                perror("Socket creation error");
                continue;
            }

            // SO_REUSEADDR, allows bind to reuse local address if supported by the protocol. This is important if a
            // child process still has an open connection and you recreate and bind the listening socket. Without this
            // option, the bind will fail because it is in use. "Unix Network Programming by Richard Stevens" saids
            // ALL TCP servers should use this option.
            const int yes = 1;
            int status = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
            if (status == -1) {
                perror("Set socket option error");
                exit(1);
            }

            // Binds the address to the socket. The sockaddr contains a field for address family and the 14 byte
            // space for address information. IPv4 will store the port than the IP address in this field. This makes
            // address length 6 bytes.
            status = bind(listeningSocket, p->ai_addr, p->ai_addrlen);
            if (status == -1) {
                close(listeningSocket);
                perror("Socket binding error");
                continue;
            }
            return listeningSocket;
        }
        std::cerr << "server: failed to bind" << std::endl;
        exit(1);
        return -1;
    }

    // Called when one of the child processes exits. waitpid reads the exit status of the terminated child process then
    // the kernal removes it from the process table. I suspect there are modern ways to do this now.
    static void sigchildHandler(int s) {
        int saved_errno = errno;
        while (waitpid(-1, nullptr, WNOHANG) > 0);
        errno = saved_errno;
    }

    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE VARIABLES
    ///////////////////////////////////////////////////////////////////////////

    /// This is the file descriptor of the listening socket.
    int mListeningSocket{0};
};


int main() {
    static HelloServer server;
    server.startListening();
    while (true) {
        server.service();
    }
    return 0;
}
