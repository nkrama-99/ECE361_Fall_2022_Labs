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
        printf("%s\n", command);

        if (strcmp(command, "/login") == 0)
        {
            printf("you are logged in\n");
        }
        else if (strcmp(command, "/logout") == 0)
        {
            printf("you are logged out\n");
            isClientOn = false;
        }
        else
        {
            printf("unknown command\n");
        }
    }
}