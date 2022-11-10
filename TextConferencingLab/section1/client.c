#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char **argv)
{
    bool isClientOn = true;
    char *pch;
    int toklen;
    char buf[100];
    // whether user is in a session
    bool insession = false;
    // supported commands

    while(1)
    {
        char *pch;
        int toklen;

        fgets(buf, 100 - 1, stdin);
        buf[strcspn(buf, "\n")] = 0;
        pch = buf;
        while (*pch == ' ')
            pch++;
        if (*pch == 0)
        {
            continue;
        }
        pch = strtok(buf, " ");
        toklen = strlen(pch);
        if (strcmp(pch, "/login") == 0)
        {
            printf("you have logged in\n");
        }
        else if (strcmp(pch, "/logout") == 0)
        {
            printf("you have logged out\n");
        }
        else if (strcmp(pch, "/joinsession") == 0)
        {
            printf("you have joined the session\n");
        }
        else if (strcmp(pch, "/leavesession") == 0)
        {
            printf("you have left the session\n");
        }
        else if (strcmp(pch, "/createsession") == 0)
        {
            printf("you have created a session\n");
        }
        else if (strcmp(pch, "/list") == 0)
        {
            printf("here is a list of active clients and sessions:\n");
        }
        else if (strcmp(pch, "/quit") == 0)
        {
            printf("terminating session\n");
            break;
        }
        else
        {
            buf[toklen] = ' ';
        }
    }
    fprintf(stdout, "You have quit successfully.\n");
    return 0;
}