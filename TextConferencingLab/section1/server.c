#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define MAXBUFLEN 100
#define PORT 3000
#define MAX_CLIENTS 30
#define MAX_SESSIONS 5
#define MAX_CLIENTS_PER_SESSION 5

struct Client
{
    char *id;
    char *password;
    int sockfd;
    // I don't think these are needed
    // struct sockaddr_storage addr;
    // socklen_t addr_len;
};

struct Session
{
    char *id;
    int sockfds[MAX_CLIENTS_PER_SESSION];
};

struct Session sessions[MAX_SESSIONS];
struct Client client[MAX_CLIENTS];

bool createSession(char *sessionId, char *password)
{
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        if (strlen(sessions[i].id) == 0)
        {
            // this space is available in sessions
            sessions[i].id = sessionId;
            for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
            {
                sessions[i].sockfds[j] = -1;
            }
            return true;
        }
    }

    // no available space in sessions
    return false;
}

int joinSession(int sockfd, char *sessionId)
{
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        if (strcmp(sessionId, sessions[i].id) == 0)
        {
            // found session
            for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
            {
                if (sessions[i].sockfds[j] == -1)
                {
                    // this place available in sockfds
                    sessions[i].sockfds[j] = sockfd;
                    return 0; // success
                }
            }

            return 1; // failed - no space for client in session
        }
    }

    return 2; // failed - session not found
}

bool leaveSession(int sockfd)
{
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
        {
            if (sessions[i].sockfds[j] == sockfd)
            {
                // client is found in this session, will be removed
                sessions[i].sockfds[j] = -1;
                return true;
            }
        }
    }

    return true;
}

bool message(int sockfd, char *message)
{
    int sessionIndex = -1;

    // find session
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
        {
            if (sessions[i].sockfds[j] == sockfd)
            {
                sessionIndex = i;
            }
        }
    }

    if (sessionIndex == -1)
    {
        // shouldn't reach here
        printf("something went wrong!\n");
        return false;
    }

    // send message to everyone in the session
    for (int i = 0; i < MAX_CLIENTS_PER_SESSION; i++)
    {
        if (sessions[sessionIndex].sockfds[i] != -1)
        {
            // this is a client
            if (send(sockfd, message, MAXBUFLEN, 0) == -1)
            {
                perror("send");
                return false;
            }
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    int opt = 1;
    int master_socket, addrlen, new_socket, client_socket[MAX_CLIENTS], activity, valread;
    struct sockaddr_in address;

    char buffer[1025]; // data buffer of 1K

    // socket descriptor set
    fd_set readfds;

    // client sockets
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_socket[i] = 0;
    }

    // master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set master socket to allow multiple connections ,
    // this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // socket config
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind master socket
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Initiating Text Conferencing Server on %d\n", PORT);

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // accept the incoming connection
    addrlen = sizeof(address);
    printf("Waiting for connection...\n");

    while (1)
    {
        int sd;
        int max_sd;

        // clear set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        // add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        // wait for an activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        // If master socket, new connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("> New connection, socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            char *message = "Login successful!";
            if (send(new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }

            printf("> Welcome message sent successfully\n");

            // add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                // if position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("> Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // else other sockets
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {
                if ((valread = read(sd, buffer, 1024)) == 0)
                {
                    // Check if it was for closing
                    getpeername(sd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen);
                    printf("A client disconnected , ip %s , port %d \n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                }
                else
                {
                    // TODO: This is where we process the code we received
                    printf("received reply from client: ");
                    printf("%s", buffer);
                    printf("\n");
                }
            }
        }
    }

    exit(0);
}