// we used the beej textbook for reference
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv)
{
    char ftp[100];
    char fileName[100];

    if (argc != 3)
    {
        printf("error: bad args\n");
        exit(1);
    }

    printf("Please input message as follows: ftp <filename>\n");
    scanf("%s%s", ftp, fileName);

    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    status = getaddrinfo(argv[1], argv[2], &hints, &servinfo);

    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd < 0)
    {
        printf("error: socket");
        exit(1);
    }

    if (access(fileName, F_OK) == 0)
    {
        if (sendto(sockfd, ftp, sizeof(ftp), 0, (const struct sockaddr *)servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            printf("error: sendto\n");
            exit(1);
        }
        printf("Sending to server...\n");
    }
    else
    {
        printf("error: access\n");
        exit(1);
    }

    char buff[100];
    socklen_t serversize = sizeof(servinfo);

    int recv = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&servinfo, &serversize);
    if (recv < 0)
    {
        printf("error: recvfrom\n");
        exit(1);
    }

    buff[recv] = '\0';

    if (strcmp(buff, "yes") == 0)
    {
        printf("A file transfer can start.\n");
    }
    else if (strcmp(buff, "no") == 0)
    {
        printf("A file transfer cannot start.\n");
        exit(1);
    }
    else
    {
        printf("Other, invalid response\n");
    }

    return 0;
}
