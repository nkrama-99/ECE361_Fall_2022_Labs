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

// if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
//                          (struct sockaddr *)&servaddr, &addr_len)) == -1)
// {
//     perror("recvfrom");
//     exit(1);
// }

// printf("Received message from server: ");
// printf(buf);

// if (send(sockfd, "Hi Rama! Nice to meet you.\n", MAXBUFLEN, 0) == -1)
// {
//     perror("send");
// }

bool insession = false; // whether user is in a session
int sockfd;
struct sockaddr_in servaddr;
socklen_t addr_len;
int numbytes;

void login(char *client_id, char *password, char *server_ip, char *server_port)
{
    char buf[MAXBUFLEN];

    printf("%s\n", client_id);
    printf("%s\n", password);
    printf("%s\n", server_ip);
    printf("%s\n", server_port);

    
	char message[100] = "";
    char type[] = "LOGIN";
    char size[64];
    sprintf(size, "%d", strlen(password));

    sprintf(message, "%s:%s:%s:%s", type, size, client_id, password);
	printf("%s\n", message);

    bzero(&servaddr, sizeof(servaddr));
    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(1);
    }
    else
    {
        printf("Socket successfully created..\n");
    }

    // configure server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_ip);
    servaddr.sin_port = htons(atoi(server_port));
    addr_len = sizeof servaddr;

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(1);
    }
    else
    {
        printf("connected to the server, attempting to log in\n");
    }

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                             (struct sockaddr *)&servaddr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    printf("%s\n", buf);

    // now attempt to login
    char * sizeStr;
    sprintf(sizeStr, "%d", strlen(password));
    
    char *message = "LOGIN";
    // strcat(message, sizeStr);
    // strcat(message, ":");
    // strcat(message, client_id);
    // strcat(message, ":");
    // strcat(message, password);

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }
}

void logout()
{
    close(sockfd);
}

int main(int argc, char **argv)
{
    bool isClientOn = true;

    while (isClientOn == true)
    {
        char *cmd;
        char buf[100];
        fgets(buf, 100, stdin);
        buf[strcspn(buf, "\n")] = 0;
        cmd = strtok(buf, " ");

        if (strcmp(cmd, "/login") == 0)
        {
            char *client_id = strtok(NULL, " ");
            char *password = strtok(NULL, " ");
            char *server_ip = strtok(NULL, " ");
            char *server_port = strtok(NULL, " ");

            login(client_id, password, server_ip, server_port);
        }
        else if (strcmp(cmd, "/logout") == 0)
        {
            logout();
            printf("you have logged out\n");
        }
        else if (strcmp(cmd, "/joinsession") == 0)
        {
            printf("you have joined the session\n");
            char *session_id = strtok(NULL, " ");
            printf("%s\n", session_id);
        }
        else if (strcmp(cmd, "/leavesession") == 0)
        {
            printf("you have left the session\n");
        }
        else if (strcmp(cmd, "/createsession") == 0)
        {
            printf("you have created a session\n");
            char *session_id = strtok(NULL, " ");
            printf("%s\n", session_id);
        }
        else if (strcmp(cmd, "/list") == 0)
        {
            printf("here is a list of active clients and sessions:\n");
        }
        else if (strcmp(cmd, "/quit") == 0)
        {
            printf("terminating session\n");
            break;
        }
        else
        {
            printf("invalid command");
        }
        memset(buf, '0', 100);
    }
    fprintf(stdout, "You have quit successfully.\n");
    return 0;
}