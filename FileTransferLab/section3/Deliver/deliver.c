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
#include <time.h>
#include <stdbool.h>

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

    clock_t start;
    clock_t end;
    double rtt;
    double dev_rtt = 0;

    start = clock();

    if (access(fileName, F_OK) == 0)
    {
        if (sendto(sockfd, ftp, sizeof(ftp), 0, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
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

    int recv = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)servinfo, &serversize);
    if (recv < 0)
    {
        printf("error: recvfrom\n");
        exit(1);
    }

    buff[recv] = '\0';

    if (strcmp(buff, "yes") == 0)
    {
        end = clock();
        rtt = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("A file transfer can start.\n");
        printf("Round-trip time from client to server is %f seconds.\n", rtt);
    }
    else if (strcmp(buff, "no") == 0)
    {
        printf("A file transfer cannot start.\n");
        exit(1);
    }
    else
    {
        printf("Invalid response\n");
    }

    FILE *file;
    char fileBuff[1000];
    char packBuff[4096];

    file = fopen(fileName, "r");

    if (file == NULL)
    {
        printf("error: empty file\n");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    int totalFrag = (ftell(file) / 1000) + 1;
    rewind(file);

    struct timeval timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = (int) rtt * 1000000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        printf("setsockopt error\n");
    }

    double alpha = 0.125;
    double beta = 0.125;

    for (int fragNum = 1; fragNum <= totalFrag; fragNum++)
    {
        int size = fread(fileBuff, sizeof(char), 1000, file);
        int header = sprintf(packBuff, "%d:%d:%d:%s:", totalFrag, fragNum, size, fileName);
        memcpy(packBuff + header, fileBuff, size);

        bool isRetransmit = true;

        while (isRetransmit)
        {
            start = clock();
            if (sendto(sockfd, packBuff, header + size, 0, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
            {
                printf("error: sendto\n");
                exit(1);
            }

            int rec = recvfrom(sockfd, buff, 256, 0, (struct sockaddr *)servinfo, &serversize);
            end = clock();

            double sample_rtt = (double)(end - start) / CLOCKS_PER_SEC;
            dev_rtt = ((1-beta) * dev_rtt + (beta) * abs(rtt - sample_rtt)) * 1000000;
            rtt = ((1-alpha) * rtt + alpha * sample_rtt) * 1000000;

            timeout.tv_usec = rtt + 4 * dev_rtt;

            printf("Timeout: %d\n", timeout.tv_usec);

            if (rec > 0)
            {
                isRetransmit = false;
                if (strcmp(buff, "yes") == 0)
                {
                    printf("ACK\n");
                }
            } else {
                printf("RETRANSMIT\n");
            }
        }
    }

    return 0;
}