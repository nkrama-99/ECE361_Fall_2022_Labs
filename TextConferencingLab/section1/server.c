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

#define MAXBUFLEN 1000
#define MAX_CLIENTS 30
#define MAX_USERS 50
#define MAX_SESSIONS 5
#define MAX_CLIENTS_PER_SESSION 5
#define USERS_COUNT 7

int PORT;

struct Client
{
    char id[100];
    char password[100];
    int sockfd;
};

struct Session
{
    char id[100];
    char admin[100];
    int clientIndexes[MAX_CLIENTS_PER_SESSION];
};

struct User
{
    char username[50];
    char password[50];
};

struct Session sessions[MAX_SESSIONS];
struct Client clients[MAX_CLIENTS];
struct User registeredUsers[MAX_USERS];

int findClientIndexFromSockfd(int sockfd)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].sockfd == sockfd)
        {
            return i;
        }
    }

    // this should never happen
    return -1;
}

int findClientIndexFromID(char *client_id)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (strcmp(clients[i].id, client_id) == 0)
        {
            return i;
        }
    }

    // this should never happen
    return -1;
}

void buildQuery(char *buf)
{
    strcat(buf, "Active Clients...\n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].sockfd != -1)
        {
            char temp[MAXBUFLEN] = "";
            sprintf(temp, "- %s\n", clients[i].id);
            strcat(buf, temp);
        }
    }
    strcat(buf, "\n");

    strcat(buf, "Active Sessions...\n");
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        printf("%s\n", sessions[i].admin);
        if (strlen(sessions[i].id) != 0)
        {
            char temp[MAXBUFLEN] = "";
            sprintf(temp, "- %s -> admin = %s\n", sessions[i].id, sessions[i].admin);
            strcat(buf, temp);
            for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
            {
                if (sessions[i].clientIndexes[j] != -1)
                {
                    char temp2[MAXBUFLEN] = "";
                    sprintf(temp2, "  -- %s\n", clients[sessions[i].clientIndexes[j]].id);
                    strcat(buf, temp2);
                }
            }
        }
    }
    strcat(buf, "\n");
}

void printAllClients()
{
    printf("Active Clients:\n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].sockfd != -1)
        {
            printf("- index: %d | id: %s | password: %s | sockfd: %d\n", i, clients[i].id, clients[i].password, clients[i].sockfd);
        }
    }
    printf("\n");
}

void printAllSessions()
{
    printf("Active Sessions:\n");
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        if (strlen(sessions[i].id) != 0)
        {
            printf("- index: %d | id: %s \n", i, sessions[i].id);
            printf("  -- clients:\n");
            for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
            {
                if (sessions[i].clientIndexes[j] != -1)
                {
                    printf("     --- clientIndex: %d | clientId: %s\n", sessions[i].clientIndexes[j], clients[sessions[i].clientIndexes[j]].id);
                }
            }
        }
    }
    printf("\n");
}

bool createClient(int sockfd, char *id, char *password)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].sockfd == -1)
        {
            // this space is available in clients
            clients[i].sockfd = sockfd;
            strcpy(clients[i].password, password);
            strcpy(clients[i].id, id);
            // printf("%s %s\n", clients[i].id, clients[i].password);
            return true;
        }
    }

    return false;
}

bool authenticateUser(char *userId, char *password)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (strcmp(clients[i].id, userId) == 0)
        {
            // already logged in
            return false;
        }
    }

    for (int i = 0; i < MAX_USERS; i++)
    {
        // authenticate user
        if (strcmp(registeredUsers[i].username, userId) == 0)
        {
            if (strcmp(registeredUsers[i].password, password) == 0)
            {
                return true;
            }
            return false;
        }
    }

    return false;
}

bool removeClient(int sockfd)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].sockfd == sockfd)
        {
            clients[i].sockfd = -1;
            strcpy(clients[i].password, "");
            strcpy(clients[i].id, "");
        }
    }
}

bool createSession(char *sessionId, char *source)
{
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        if (strcmp(sessions[i].id, sessionId) == 0)
        {
            return false;
        }
    }

    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        if (strlen(sessions[i].id) == 0)
        {
            strcpy(sessions[i].admin, source);
            // this space is available in sessions
            strcpy(sessions[i].id, sessionId);
            for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
            {
                // init to -1
                sessions[i].clientIndexes[j] = -1;
            }
            return true;
        }
    }

    // no available space in sessions
    return false;
}

int joinSession(int sockfd, char *sessionId)
{

    int clientIndex = findClientIndexFromSockfd(sockfd);

    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        if (strcmp(sessionId, sessions[i].id) == 0)
        {
            // found session
            for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
            {
                if (sessions[i].clientIndexes[j] == -1)
                {
                    // this place available in sockfds, save clientIndex
                    sessions[i].clientIndexes[j] = clientIndex;
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
    int clientIndex = findClientIndexFromSockfd(sockfd);

    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
        {
            if (sessions[i].clientIndexes[j] == clientIndex)
            {
                // client is found in this session, will be removed
                sessions[i].clientIndexes[j] = -1;

                bool clientsInSession = false;
                for (int k = 0; k < MAX_CLIENTS_PER_SESSION; k++)
                {
                    if (sessions[i].clientIndexes[k] != -1)
                    {
                        clientsInSession = true;
                    }
                }

                if (clientsInSession == false)
                {
                    strcpy(sessions[i].id, "");
                }

                printf("%d\n", sessions[i].clientIndexes[j]);
                return true;
            }
        }
    }

    return true;
}

bool kickClient(char *client_id, char *source)
{   
    int sourceIndex = findClientIndexFromID(source);
    int clientIndex = findClientIndexFromID(client_id);
    int sourceSessionIndex = -1;
    int sessionIndex = -1;
    
    if (clientIndex == -1) {
        return false;
    }

    // find session for source
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
        {
            if (sessions[i].clientIndexes[j] == sourceIndex)
            {
                sourceSessionIndex = i;
            }
        }
    }

    // find session for user being kicked
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
        {
            if (sessions[i].clientIndexes[j] == clientIndex)
            {
                sessionIndex = i;
            }
        }
    }

    if (sourceSessionIndex != sessionIndex){
        return false;
    }

    if (sessionIndex == -1)
    {
        // shouldn't reach here
        printf("something went wrong!\n");
        return false;
    }

    char body[MAXBUFLEN] = "";
    sprintf(body, "%s:%s:%s", "KICK", clients[clientIndex].id, "you have been kicked from the session");
    printf("sending to clients: %s\n", body);

    // this is a client
    int toSockfd = clients[sessions[sessionIndex].clientIndexes[clientIndex]].sockfd;

    printf("%d %d %d\n", toSockfd, clientIndex, sessionIndex);

    if (send(toSockfd, body, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }

    return true;
}

bool transferAdmin(char *client_id)
{
    int clientIndex = findClientIndexFromID(client_id);
    int sessionIndex = -1;
    
    if (clientIndex == -1) {
        return false;
    }

    // find session
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
        {
            if (sessions[i].clientIndexes[j] == clientIndex)
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

    strcpy(sessions[sessionIndex].admin, client_id);

    char body[MAXBUFLEN] = "";
    sprintf(body, "%s:%s:%s", "ADMIN", clients[clientIndex].id, "you are now the admin of the session");
    printf("sending to clients: %s\n", body);

    // this is a client
    int toSockfd = clients[sessions[sessionIndex].clientIndexes[clientIndex]].sockfd;

    printf("%d %d %d\n", toSockfd, clientIndex, sessionIndex);

    if (send(toSockfd, body, MAXBUFLEN, 0) == -1)
    {
        perror("send");
    }

    return true;
}

bool message(int sockfd, char *message)
{
    int clientIndex = findClientIndexFromSockfd(sockfd);
    int sessionIndex = -1;

    // find session
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
        {
            if (sessions[i].clientIndexes[j] == clientIndex)
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

    char body[MAXBUFLEN] = "";
    sprintf(body, "%s:%s:%s", "MESSAGE", clients[clientIndex].id, message);
    
    printf("sending to clients: %s\n", body);

    // send message to everyone in the session
    for (int i = 0; i < MAX_CLIENTS_PER_SESSION; i++)
    {
        if (sessions[sessionIndex].clientIndexes[i] != -1)
        {
            // this is a client
            int toSockfd = clients[sessions[sessionIndex].clientIndexes[i]].sockfd;
            if (toSockfd != sockfd)
            {
                if (send(toSockfd, body, MAXBUFLEN, 0) == -1)
                {
                    perror("send");
                    // return false;
                }
            }
        }
    }

    return true;
}

void initSessions()
{
    for (int i = 0; i < MAX_SESSIONS; i++)
    {
        strcpy(sessions[i].id, "");

        for (int j = 0; j < MAX_CLIENTS_PER_SESSION; j++)
        {
            sessions[i].clientIndexes[j] = -1;
        }
    }
}

void initClients()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        strcpy(clients[i].id, "");
        strcpy(clients[i].password, "");
        clients[i].sockfd = -1;
    }
}

void initRegisteredUsers()
{
    for (int i = 0; i < MAX_USERS; i++)
    {
        strcpy(registeredUsers[i].username, "");
        strcpy(registeredUsers[i].password, "");
    }

    char buf[1000] = "";
    char *username;
    char *password;
    FILE *fp;
    int count = 0;

    fp = fopen("registered_users.txt", "r");
    fgets(buf, 1000, fp);
    fclose(fp);
    // printf("%s\n", buf);

    password = strtok(buf, ";");
    while (password != NULL && count < MAX_USERS)
    {
        username = strtok(NULL, ";");
        password = strtok(NULL, ";");
        // printf("username:%s, password:%s\n", username, password);
        if (username != NULL && password != NULL)
        {
            strcpy(registeredUsers[count].username, username);
            strcpy(registeredUsers[count].password, password);
            count++;
        }
    }

    // for (int i = 0; i < MAX_USERS; i++)
    // {
    //     printf("#Registered username:%s, password:%s\n", registeredUsers[i].username, registeredUsers[i].password);
    // }
}

bool registerUser(char *username, char *password)
{
    printf("username:%s | password:%s\n", username, password);

    if (username == NULL || password == NULL || strcmp("", username) == 0 || strcmp("", password) == 0)
    {
        // invalid entry
        return false;
    }

    for (int i = 0; i < MAX_USERS; i++)
    {
        if (strcmp(username, registeredUsers[i].username) == 0)
        {
            // user exists
            return false;
        }
    }

    // save in memory
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (strcmp("", registeredUsers[i].username) == 0)
        {
            // found spot
            strcpy(registeredUsers[i].username, username);
            strcpy(registeredUsers[i].password, password);
            break;
        }
    }

    // save in db
    char buf[1000] = "";

    strcat(buf, "registered_users;");
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (strcmp("", registeredUsers[i].username) != 0)
        {
            strcat(buf, registeredUsers[i].username);
            strcat(buf, ";");
            strcat(buf, registeredUsers[i].password);
            strcat(buf, ";");
        }
    }

    // printf("%s\n", buf);
    FILE *fp;
    fp = fopen("registered_users.txt", "w");
    fprintf(fp, buf);
    fclose(fp);

    return true;
}

int main(int argc, char *argv[])
{
    int PORT = atoi(argv[1]);

    printf("Chosen port: %d \n", PORT);

    initClients();
    initSessions();
    initRegisteredUsers();

    int opt = 1;
    int master_socket, addrlen, new_socket, client_socket[MAX_CLIENTS], activity, valread;
    struct sockaddr_in address;

    char buffer[MAXBUFLEN]; // data buffer of 1K

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
        printf(">>>>>>>>>>>>>>>>>>>\n");
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
        else
        {
            // else other sockets
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                sd = client_socket[i];

                if (FD_ISSET(sd, &readfds))
                {
                    if ((valread = recv(sd, buffer, MAXBUFLEN - 1, 0)) == -1)
                    {
                        printf("errno:%d\n", errno);
                        perror("read");
                    }

                    // This is where we process the code we received
                    printf("received reply from client: ");
                    printf("%s", buffer);
                    printf("\n");

                    char *type = strtok(buffer, ":");
                    char *size = strtok(NULL, ":");
                    char *source = strtok(NULL, ":");
                    char *data = strtok(NULL, ":");

                    printf("type: %s , size: %s , source: %s , data: %s\n", type, size, source, data);

                    if (valread == 0)
                    {
                        printf("unexpected connection lost on sockfd: %d\n", sd);
                        // attempting to gracefully remove them
                        leaveSession(sd);
                        removeClient(sd);
                        close(sd);
                        client_socket[i] = 0;
                    }
                    else if (type == NULL)
                    {
                        // null check
                    }
                    else if (strcmp(type, "REGISTER") == 0)
                    {
                        printf("register\n");
                        bool regSuccess = false;

                        if (registerUser(source, data) == true)
                        {
                            printf("register success\n");
                            regSuccess = createClient(sd, source, data);
                        }
                        else
                        {
                            printf("authenticate failed\n");
                        }

                        if (regSuccess == true)
                        {
                            printf("registration success and logged in\n");
                            char *message = "REG_ACK:0:server:";
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                        else
                        {
                            printf("registration failed\n");
                            
                            char *message = "REG_NAK:0:server:";
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }

                            close(sd);
                            client_socket[i] = 0;
                        }
                    }
                    else if (strcmp(type, "LOGIN") == 0)
                    {
                        printf("login\n");
                        bool loginSuccess = false;

                        if (authenticateUser(source, data) == true)
                        {
                            printf("authenticate success\n");
                            loginSuccess = createClient(sd, source, data);
                        }
                        else
                        {
                            printf("authenticate failed\n");
                        }

                        if (loginSuccess == true)
                        {
                            printf("client creation success\n");
                            char *message = "LO_ACK:0:server:";
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                        else
                        {
                            printf("client creation failed\n");
                            char *message = "LO_NAK:0:server:";
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                    }
                    else if (strcmp(type, "JOIN") == 0)
                    {
                        printf("join session\n");
                        if (joinSession(sd, data) == 0)
                        {
                            char *message[100];
                            sprintf(message, "%s:%s:%s:%s", "JN_ACK", "0", "server", "");
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                        else if (joinSession(sd, data) == 1)
                        {
                            char message[100];
                            char size[64];
                            sprintf(size, "%d", strlen(data) + strlen(" - no space for client"));

                            sprintf(message, "%s:%s:%s:%s%s", "JN_NAK", size, "server", data, " - no space for client");
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                        else if (joinSession(sd, data) == 2)
                        {
                            char message[100];
                            char size[64];
                            sprintf(size, "%d", strlen(data) + strlen(" - session id not found"));

                            sprintf(message, "%s:%s:%s:%s%s", "JN_NAK", size, "server", data, " - session id not found");
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                    }
                    else if (strcmp(type, "LEAVE_SESS") == 0)
                    {
                        if (leaveSession(sd) == true)
                        {
                            printf("leave session\n");
                        }
                    }
                    else if (strcmp(type, "KICK") == 0)
                    {
                        char message[100];
                        if (kickClient(data, source) == true)
                        {
                            sprintf(message, "%s:%s:%s:%s%s", "KK_ACK", size, "server", data, " - client kicked");
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        } else {
                            sprintf(message, "%s:%s:%s:%s", "KK_NAK", size, "server", "");
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                    }
                    else if (strcmp(type, "TRANS_ADMIN") == 0)
                    {
                        char message[100];
                        if (transferAdmin(data) == true)
                        {
                            sprintf(message, "%s:%s:%s:%s%s", "TA_ACK", size, "server", data, " - client is now admin");
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        } else {
                            sprintf(message, "%s:%s:%s:%s", "TA_NAK", size, "server", "");
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                    }
                    else if (strcmp(type, "NEW_SESS") == 0)
                    {
                        printf("new session\n");
                        if (createSession(data, source) == true)
                        {
                            char *message = "NS_ACK:0:server:";
                            if (joinSession(sd, data) == 0)
                            {
                                if (send(sd, message, MAXBUFLEN, 0) == -1)
                                {
                                    perror("send");
                                }
                            }
                        }
                        else
                        {
                            char *message = "NS_NAK:0:server:";
                            if (send(sd, message, MAXBUFLEN, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                    }
                    else if (strcmp(type, "QUERY") == 0)
                    {
                        char query[MAXBUFLEN] = "";
                        buildQuery(query);

                        char message[MAXBUFLEN] = "";
                        char size[MAXBUFLEN];
                        sprintf(size, "%d", strlen(query) + 1);
                        sprintf(message, "%s:%s:%s:%s", "QU_ACK", size, "server", query);
                        printf("message body: %s", message);

                        if (send(sd, message, MAXBUFLEN, 0) == -1)
                        {
                            perror("send");
                        }
                    }
                    else if (strcmp(type, "EXIT") == 0)
                    {
                        printf("client exit\n");
                        // need to remove from sessions?
                        removeClient(sd);
                        close(sd);
                        client_socket[i] = 0;
                    }
                    else if (strcmp(type, "MESSAGE") == 0)
                    {
                        printf("client message\n");
                        message(sd, data);
                    }
                }
            }
        }

        // print all sessions and clients
        printf("\n");
        printAllClients();
        printAllSessions();
        strcpy(buffer, "");
    }

    exit(0);
}