#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10  // how many pending connections queue will hold
#define PORT "3490" // the port users will be connecting to
#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        printf("usage: ./server {port_number}\n");
        exit(1);
    }

    int server_port_number = atoi(argv[1]);
    printf("Initiating Text Conferencing Server on %d\n", server_port_number);

    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    int rv;
    int yes = 1;
    struct addrinfo hints, *servinfo, *p;
    socklen_t sin_size;
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    while (1)
    {
        int numbytes;
        char buf[MAXBUFLEN];

        printf("server: waiting for connections...\n");

        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);

        printf("server: got connection from %s\n", s);

        if (send(new_fd, "Hello, world! Rama was here.\n", MAXBUFLEN, 0) == -1)
        {
            perror("send");
        }

        if ((numbytes = recvfrom(new_fd, buf, MAXBUFLEN - 1, 0,
                                 (struct sockaddr *)&their_addr, &sin_size)) == -1)
        {
            perror("recvfrom");
            exit(1);
        }

        printf("Received message from client: ");
        printf(buf);
        printf("TERMINATE connection");

        close(new_fd);
    }

    exit(0);
}