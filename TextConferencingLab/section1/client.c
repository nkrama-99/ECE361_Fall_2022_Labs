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
char set_client_id[100];

void login(char *password, char *server_ip, char *server_port)
{
    char buf[MAXBUFLEN];
    char message[100] = "";
    char type[] = "LOGIN";
    char size[64];
    sprintf(size, "%d", strlen(password));

    sprintf(message, "%s:%s:%s:%s", type, size, set_client_id, password);

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

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }

    // wait for login_ack
}

void logout()
{
    char message[100] = "";
    char type[] = "EXIT";

    sprintf(message, "%s:%s:%s:%s", type, "0", set_client_id, "");

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }
    close(sockfd);
}

void joinSession(char *session_id)
{
    char message[100] = "";
    char type[] = "JOIN";
    char size[64];
    sprintf(size, "%d", strlen(session_id));
    sprintf(message, "%s:%s:%s:%s", type, size, set_client_id, session_id);

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }
}

void leaveSession()
{
    char message[100] = "";
    char type[] = "LEAVE_SESS";

    sprintf(message, "%s:%s:%s:%s", type, "0", set_client_id, "");

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }
}

void createSession(char *session_id)
{
    char message[100] = "";
    char type[] = "NEW_SESS";
    char size[64];
    sprintf(size, "%d", strlen(session_id));

    sprintf(message, "%s:%s:%s:%s", type, size, set_client_id, session_id);

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }
}

void query()
{
    char message[100] = "";
    char type[] = "QUERY";

    sprintf(message, "%s:%s:%s:%s", type, "0", set_client_id, "");

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }
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
            strcpy(set_client_id, client_id);
            char *password = strtok(NULL, " ");
            char *server_ip = strtok(NULL, " ");
            char *server_port = strtok(NULL, " ");

            login(password, server_ip, server_port);
        }
        else if (strcmp(cmd, "/logout") == 0)
        {
            logout();
        }
        else if (strcmp(cmd, "/joinsession") == 0)
        {
            printf("you have joined the session\n");
            char *session_id = strtok(NULL, " ");
            joinSession(session_id);
        }
        else if (strcmp(cmd, "/leavesession") == 0)
        {
            printf("you have left the session\n");
            leaveSession();
        }
        else if (strcmp(cmd, "/createsession") == 0)
        {
            printf("you have created a session\n");
            char *session_id = strtok(NULL, " ");
            createSession(session_id);
        }
        else if (strcmp(cmd, "/list") == 0)
        {
            query();
        }
        else if (strcmp(cmd, "/quit") == 0)
        {
            printf("terminating session\n");
            break;
        }
        else
        {
            printf("invalid command\n");
            break;
        }
        memset(buf, '0', 100);
    }
    fprintf(stdout, "You have quit successfully.\n");
    return 0;
}