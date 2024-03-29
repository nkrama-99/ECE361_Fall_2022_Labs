/* Some code is borrowed from Section 2.4 of the Berkeley API */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define SERVER_UDP_PORT 5000 /* well-known port */
#define MAXLEN 4096          /* maximum data length */

bool write_to_file(char *received);

void delay(int number_of_seconds) 
{ 
    // Converting time into milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Storing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds) {
        // printf("waiting\n");
    };

} 

bool write_to_file(char *received)
{    
    char *total_frag = (char*)malloc(MAXLEN * sizeof(char));
    char *packet_no = (char*)malloc(MAXLEN * sizeof(char));
    char *size = (char*)malloc(MAXLEN * sizeof(char));
    char *file_name = (char*)malloc(MAXLEN * sizeof(char));   
    
    char *content_ptr;

    // printf("Processing total_frag\n");
    int index = 0;
    int counter = 0;

    bool ftp = true;

    while (*(received + index) != ':') {
        // printf("Testing with: %c\n", *(received + index));
        *(total_frag + counter) = *(received + index);
        counter++;
        index++;
    };
    *(total_frag + counter) = '\0';
    index++;
    
    // printf("Processing packet_no\n");
    counter = 0;
    while (*(received + index) != ':') {
        // printf("Testing with: %c\n", *(received + index));
        *(packet_no + counter) = *(received + index);
        counter++;
        index++;
    };
    *(packet_no + counter) = '\0';
    index++;

    // printf("Processing size\n");
    counter = 0;
    while (*(received + index) != ':') {
        // printf("Testing with: %c\n", *(received + index));
        *(size + counter) = *(received + index);
        counter++;
        index++;
    };
    *(size + counter) = '\0';
    index++;

    // printf("Processing file_name\n");
    counter = 0;
    while (*(received + index) != ':') {
        // printf("Testing with: %c\n", *(received + index));
        *(file_name + counter) = *(received + index);
        counter++;
        index++;
    };
    *(file_name + counter) = '\0';
    index++;

    content_ptr = received + index;

    int inc_limit = atoi(size);
    char *content = (char*)malloc(inc_limit * sizeof(char));
    
    for (int i=0; i<inc_limit; i++) {
        content[i] = *(content_ptr + i);
    }

    // printf("Parsing the message received:\n");
    // printf(" - total_frag : %s \n", total_frag);
    // printf(" - packet_no : %s \n", packet_no);
    // printf(" - size : %s \n", size);
    // printf(" - file_name : %s \n", file_name);
    // // printf(" - content : %s \n", content);

    printf("Writing to file | total_frag = %s | packet_no = %s | size = %s | file_name = %s \n", total_frag , packet_no, size, file_name);

    // testing last packet for a specific case
    // if (atoi(packet_no) == 2168) {
    //     printf("\n");
    //     printf(content);
    //     printf("\n");
    // }

    if (atoi(packet_no) == atoi(total_frag)) {
        printf("FTP Complete!\n");
        ftp = false;
    }
    
    FILE *fptr;

    if (atoi(packet_no) == 1)
    {
        fptr = fopen(file_name, "w+b"); // "w" defines "writing mode"
    }
    else
    {
        fptr = fopen(file_name, "ab"); // "w" defines "writing mode"
    }
    fflush(fptr);

    char c = *content;
    int inc = 0;

    fwrite(content, 1, atoi(size), fptr);

    fclose(fptr);

    // free up all memory allocated
    free(total_frag);
    free(packet_no);
    free(size);
    free(file_name);
    free(content);

    return ftp;
}

int main(int argc, char **argv)
{
    //variables borrowed from Berkely API 2.4
    int sd, client_len, port, n;
    char buf[MAXLEN];
    struct sockaddr_in server, client;
    char expected_message[] = "ftp ";

    char yes[] = "yes";
    char no[] = "no";  
    char* reply;

    switch (argc)
    {
    case 2:
        port = atoi(argv[1]);
        break;
    default:
        fprintf(stderr, "Usage: %s <UDP listen port>\n", argv[0]);
        exit(1);
    }

    printf("Chosen port: %d \n", port);

    //the following is borrowed from the Berkley API
    //establishes socket and binds to it

    //AF_INET family: internet communication using TCP/IP
    //SOCK_DRAM type: delivers data in blocks of bytes
    //0 protocol: default protocol for family, type pair
    //the default protocol for AF_INET,SOCK_DGRAM is UDP
    //sd is the socket descriptor returned

    //----------------------------------------------------------------
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        fprintf(stderr, "Can’t create a socket\n");
        exit(1);
    }
    /* Bind an address to the socket */
    //erase data @ address of server
    bzero((char *)&server, sizeof(server));

    server.sin_family = AF_INET;
    //convert host byte order to network byte order (Internet)
    //to use a common data format
    server.sin_port = htons(port);
    //use any ip address available for host
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    //communication will occur via the specified port number
    if (bind(sd, (struct sockaddr *)&server,
             sizeof(server)) == -1)
    {
        fprintf(stderr, "Can’t bind name to socket\n");
        exit(1);
    }

    bool ftp = false;

    //-----------------------------------------------------------------------
    //wait for an incoming message
    while (1)
    {
        client_len = sizeof(client);
        //receive message from client into buf
        //returns number of bytes received
        if ((n = recvfrom(sd, buf, MAXLEN, 0,
                          (struct sockaddr *)&client, &client_len)) < 0)
        {
            fprintf(stderr, "Can’t receive datagram\n");
            exit(1);
        }

        // delay(500);
        // printf("PRINTING BUF RECEIVED BY UDP FUNCTION\n");
        // printf(buf);
        // printf("\n End of buf \n");
        
        if (ftp == true) {
            ftp = write_to_file(buf);
            reply = yes;
        } else {
            if (strcmp(buf, expected_message)) {
                printf("FTP!\n");
                ftp = true;
                reply = yes;
            } else {
                reply = no;
            }
        }

        //define reply and reply_len
        printf("Response: ");
        printf(reply);
        printf("\n");

        if (sendto(sd, reply, strlen(reply), 0,
                   (struct sockaddr *)&client, client_len) != strlen(reply))
        {
            fprintf(stderr, "Can’t send datagram\n");
            exit(1);
        }

        strcpy(buf, "");
    }

    //close the socket
    close(sd);

    free(reply);
    
    return (0);
}