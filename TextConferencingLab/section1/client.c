#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char **argv)
{
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
            isClientOn = false;
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
            isClientOn = false
        }
        else
        {
            printf("seinding message...\n");
        }
    }
}