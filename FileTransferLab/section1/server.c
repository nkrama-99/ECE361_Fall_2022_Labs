// we used the beej textbook for reference
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

#define MAXLEN 4096

bool ftpChecker(char *buf)
{
    char ftp_input[] = "ftp";

    if (strcmp(ftp_input, buf) == 0)
    {
        return true;
    }

    return false;
}

int main(int argc, char **argv)
{
    int port = atoi(argv[1]);

    printf("Chosen port: %d \n", port);

    int sd;              //sd is file descriptor of socket
    int clientLength, n; //sd is file descriptor of socket
    char buf[MAXLEN + 1];
    struct sockaddr_in serverAddress, clientAddress;

    // create socket and catch error if socket creation failed
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "Can’t create a socket\n");
        exit(EXIT_FAILURE);
    }

    // Filling up server information
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    // bind and catch error if bind failed
    if (bind(sd, (const struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "bind failed\n");
        exit(EXIT_FAILURE);
    }

    printf("Ready to receive\n");

    while (1)
    {
        clientLength = sizeof(clientAddress);

        //receive message and catch error receive failed
        if ((n = recvfrom(sd, buf, MAXLEN, 0,
                          (struct sockaddr *)&clientAddress, &clientLength)) < 0)
        {
            fprintf(stderr, "Can’t receive datagram\n");
            exit(EXIT_FAILURE);
        }

        // reply if receive "fps"
        char yes[] = "yes";
        char no[] = "no";

        if (ftpChecker(buf))
        {
            printf("yes, received valid response\n");

            // reply message and catch error if reply failed
            if (sendto(sd, yes, n, 0,
                       (struct sockaddr *)&clientAddress, clientLength) != n)
            {
                fprintf(stderr, "Can’t send datagram\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            printf("no, invalid response\n");

            // reply message and catch error if reply failed
            if (sendto(sd, no, n, 0,
                       (struct sockaddr *)&clientAddress, clientLength) != n)
            {
                fprintf(stderr, "Can’t send datagram\n");
                exit(EXIT_FAILURE);
            }
        }

        strcpy(buf, ""); //reset input buffer
    }

    return 0;
}