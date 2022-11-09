#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFLEN 100

int main(int argc, char **argv)
{
    // test begin
    char *ip = "127.0.0.1";
    int port = 3000;

    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t addr_len;
    int numbytes;
    char buf[MAXBUFLEN];

    // socket create
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    addr_len = sizeof servaddr;

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(1);
    }
    else
        printf("connected to the server..\n");

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                             (struct sockaddr *)&servaddr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    printf("Received message from server: ");
    printf(buf);

    if (send(sockfd, "Hi Rama! Nice to meet you.\n", MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }

    printf("Reply sent\n");

    exit(0);
    // test end

    bool isClientOn = true;
    char command[50];

    while (isClientOn == true)
    {
        scanf("%s", &command);

        if (strcmp(command, "/login") == 0)
        {
            printf("you are logged in\n");
        }
        else if (strcmp(command, "/logout") == 0)
        {
            printf("you are logged out\n");
        }
        else if (strcmp(command, "/joinsession") == 0)
        {
            printf("you have joined the session\n");
        }
        else if (strcmp(command, "/leavesession") == 0)
        {
            printf("you have left the session\n");
        }
        else if (strcmp(command, "/createsession") == 0)
        {
            printf("you have created a session\n");
        }
        else if (strcmp(command, "/list") == 0)
        {
            printf("here is a list of all connected clients and available sessions:\n");
        }
        else if (strcmp(command, "/quit") == 0)
        {
            printf("terminating program\n");
            isClientOn = false;
        }
        else
        {
            printf("seinding message...\n");
        }
    }
}