#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        printf("usage: ./server {port_number}\n");
        exit(1);
    }

    int server_port_number = atoi(argv[1]);
    printf("Initiating Text Conferencing Server on %d\n", server_port_number);

    exit(0);
}
