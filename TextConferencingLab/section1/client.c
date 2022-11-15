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

#define MAXBUFLEN 1000
#define STDIN 0

bool inSession = false; // whether user is in a session
int sockfd = 0;
struct sockaddr_in servaddr;
socklen_t addr_len;
int numbytes;
char set_client_id[MAXBUFLEN];
bool connected = false;
bool isLoggedIn = false;

void login(char *password, char *server_ip, char *server_port)
{
    if (isLoggedIn == true)
    {
        printf("you are already logged in\n");
        return;
    }

    char buf[MAXBUFLEN] = "";
    char message[MAXBUFLEN] = "";
    char type[] = "LOGIN";
    char size[MAXBUFLEN];
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

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }

    // wait for login_ack
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                             (struct sockaddr *)&servaddr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    char *reply_type = strtok(buf, ":");
    char *reply_size = strtok(NULL, ":");
    char *reply_source = strtok(NULL, ":");
    char *reply_data = strtok(NULL, ":");

    if (strcmp(reply_type, "LO_ACK") == 0)
    {
        printf("logged in successfully\n");
        connected = true;
        isLoggedIn = true;
    }
    else if (strcmp(reply_type, "LO_NAK") == 0)
    {
        printf("login unsuccessful\n");
    }
}

void logout()
{
    if (inSession == true)
    {
        printf("please leave session first\n");
        return;
    }

    char message[MAXBUFLEN] = "";
    char type[] = "EXIT";

    sprintf(message, "%s:%s:%s:%s", type, "0", set_client_id, "");

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }
    close(sockfd);
    connected = false;
    printf("logged out and disconnected from server\n");
    isLoggedIn = false;
}

void joinSession(char *session_id)
{
    if (inSession == true)
    {
        printf("you are already in a session\n");
        return;
    }

    char buf[MAXBUFLEN];
    char message[MAXBUFLEN] = "";
    char type[] = "JOIN";
    char size[MAXBUFLEN];
    sprintf(size, "%d", strlen(session_id));
    sprintf(message, "%s:%s:%s:%s", type, size, set_client_id, session_id);

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }

    // wait for login_ack
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                             (struct sockaddr *)&servaddr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    char *reply_type = strtok(buf, ":");
    char *reply_size = strtok(NULL, ":");
    char *source = strtok(NULL, ":");
    char *data = strtok(NULL, ":");

    if (strcmp(reply_type, "JN_ACK") == 0)
    {
        printf("joined session successfully\n");
        connected = true;
        isLoggedIn = true;
        inSession = true;
    }
    else if (strcmp(reply_type, "JN_NAK") == 0)
    {
        printf("%s\n", data);
    }
}

void leaveSession()
{
    if (inSession == false)
    {
        printf("you are not in a session\n");
        return;
    }

    char message[MAXBUFLEN] = "";
    char type[] = "LEAVE_SESS";

    sprintf(message, "%s:%s:%s:%s", type, "0", set_client_id, "");

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }

    inSession = false;
}

void createSession(char *session_id)
{
    char buf[MAXBUFLEN];
    char message[MAXBUFLEN] = "";
    char type[] = "NEW_SESS";
    char size[MAXBUFLEN];
    sprintf(size, "%d", strlen(session_id));

    sprintf(message, "%s:%s:%s:%s", type, size, set_client_id, session_id);

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                             (struct sockaddr *)&servaddr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    char *reply_type = strtok(buf, ":");
    char *reply_size = strtok(NULL, ":");
    char *source = strtok(NULL, ":");
    char *data = strtok(NULL, ":");

    if (strcmp(reply_type, "NS_ACK") == 0)
    {
        printf("created and joined session successfully\n");
        connected = true;
        isLoggedIn = true;
        inSession = true;
    }
    else if (strcmp(reply_type, "NS_NAK") == 0)
    {
        printf("session creation failed\n");
    }
}

void query()
{
    char buf[MAXBUFLEN];
    char message[MAXBUFLEN] = "";
    char type[] = "QUERY";

    sprintf(message, "%s:%s:%s:%s", type, "0", set_client_id, "");

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                             (struct sockaddr *)&servaddr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    // printf("printing buf...\n");
    // printf("%s\n", buf);
    // printf("done...\n");

    char *reply_type = strtok(buf, ":");
    char *reply_size = strtok(NULL, ":");
    char *source = strtok(NULL, ":");
    char *data = strtok(NULL, ":");

    if (strcmp(reply_type, "QU_ACK") == 0)
    {
        printf("%s\n", data);
    }
}

void sendMessage(char *messageContent)
{
    char message[MAXBUFLEN] = "";
    char type[] = "MESSAGE";
    char size[MAXBUFLEN];
    sprintf(size, "%d", strlen(messageContent));

    sprintf(message, "%s:%s:%s:%s", type, size, set_client_id, messageContent);

    if (send(sockfd, message, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }
}

int main(int argc, char **argv)
{
    bool isClientOn = true;
    fd_set read_fds;

    while (isClientOn == true)
    {
        // set file descriptors to socket and io
        FD_ZERO(&read_fds);
        FD_SET(STDIN, &read_fds);

        if (connected == true)
        {
            FD_SET(sockfd, &read_fds);
        }

        if (select(sockfd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
        }

        if (FD_ISSET(STDIN, &read_fds))
        {
            char *cmd;
            char message[MAXBUFLEN];
            char buf[MAXBUFLEN];
            fgets(buf, MAXBUFLEN, stdin);
            buf[strcspn(buf, "\n")] = 0;
            strcpy(message, buf);
            cmd = strtok(buf, " ");

            if (strcmp(cmd, "/login") == 0)
            {

                char *client_id = strtok(NULL, " ");
                char *password = strtok(NULL, " ");
                char *server_ip = strtok(NULL, " ");
                char *server_port = strtok(NULL, " ");

                if (client_id == NULL || password == NULL || server_ip == NULL || server_port == NULL)
                {
                    printf("invalid login commands\n");
                }
                else
                {
                    strcpy(set_client_id, client_id);
                    login(password, server_ip, server_port);
                }
            }
            else if (strcmp(cmd, "/logout") == 0)
            {
                if (isLoggedIn == false)
                {
                    printf("you are not logged in\n");
                }
                else
                {
                    logout();
                }
            }
            else if (strcmp(cmd, "/joinsession") == 0)
            {
                if (isLoggedIn == false)
                {
                    printf("you are not logged in\n");
                }
                else
                {
                    char *session_id = strtok(NULL, " ");
                    if (session_id == NULL)
                    {
                        printf("invalid join session commands\n");
                    }
                    else
                    {
                        joinSession(session_id);
                    }
                }
            }
            else if (strcmp(cmd, "/leavesession") == 0)
            {
                if (isLoggedIn == false)
                {
                    printf("you are not logged in\n");
                }
                else
                {
                    leaveSession();
                }
            }
            else if (strcmp(cmd, "/createsession") == 0)
            {
                if (isLoggedIn == false)
                {
                    printf("you are not logged in\n");
                }
                else
                {
                    char *session_id = strtok(NULL, " ");
                    if (session_id == NULL)
                    {
                        printf("invalid create session commands\n");
                    }
                    else
                    {
                        createSession(session_id);
                    }
                }
            }
            else if (strcmp(cmd, "/list") == 0)
            {
                if (isLoggedIn == false)
                {
                    printf("you are not logged in\n");
                }
                else
                {
                    query();
                }
            }
            else if (strcmp(cmd, "/quit") == 0)
            {
                if (inSession == true)
                {
                    printf("please leave session first\n");
                }
                else if (connected == true)
                {
                    printf("please logout first\n");
                }
                else
                {
                    printf("terminating session\n");
                    break;
                }
            }
            else
            {
                if (inSession == true)
                {
                    sendMessage(message);
                }
                else
                {
                    printf("please join a session first\n");
                }
            }
            memset(buf, '0', MAXBUFLEN);
        }
        else if (FD_ISSET(sockfd, &read_fds))
        {
            char buf[MAXBUFLEN] = "";

            if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                                     (struct sockaddr *)&servaddr, &addr_len)) == -1)
            {
                perror("recvfrom");
                exit(1);
            }

            char *cmd = strtok(buf, ":");
            char *source = strtok(NULL, ":");
            char *content = strtok(NULL, ":");

            if (cmd != NULL && source != NULL && content != NULL && strcmp(cmd, "MESSAGE") == 0)
            {
                printf("%s - %s\n", source, content);
            }
        }
        else
        {
            printf("> shouldn't be coming here\n");
        }
    }

    fprintf(stdout, "You have quit successfully.\n");
    return 0;
}