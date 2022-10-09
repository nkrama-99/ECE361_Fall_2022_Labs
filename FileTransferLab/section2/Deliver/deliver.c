/* Some code is borrowed from Section 2.4 of the Berkeley API */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <dirent.h>
#include <math.h>
#include <time.h>
#include<stdlib.h>

//next steps
//make this work for a binary file
//not just a text file
#define SERVER_UDP_PORT 5000
#define MAXLEN 4096 /* maximum data length */
#define DEFLEN 64   /* default length */

bool file_exists(char *file_name);
char *get_user_input();
long measure_delay(struct timeval t1, struct timeval t2);

int get_file_size(char *filename);
char *get_file(char *filename, int file_size);
char *read_file(char *filename, char *file, int amount, int packet_num, int total_packets);

int get_message_len(char *message);
char *copy_string(char *message, int len);
int max_size = 1000;

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

char *read_file(char *filename, char *file, int amount, int packet_num, int total_packets)
{

    char *message = malloc((amount + 50) * sizeof(char));
    // char * l = malloc(1);
    //convert int to string
    int space = (int)((ceil(log10(total_packets)) + 1) * sizeof(char));
    char total_frag[space];
    sprintf(total_frag, "%d", total_packets);
    //convert int to string
    space = (int)((ceil(log10(total_packets)) + 1) * sizeof(char));
    char frag_num[space];
    sprintf(frag_num, "%d", packet_num);
    //convert int to string
    space = (int)((ceil(log10(total_packets)) + 1) * sizeof(char));
    char size[space];
    sprintf(size, "%d", amount);
    //concat the string
    strcpy(message, total_frag);
    strcat(message, ":");
    strcat(message, frag_num);
    strcat(message, ":");
    strcat(message, size);
    strcat(message, ":");
    strcat(message, filename);
    strcat(message, ":");

    //printf("message: %s\n",message);
    //printf("message len: %d\n", strlen(message));
    // char * l = malloc(1);


    // // make this a for loop (dont use string functions)
    // int index = (packet_num - 1) * max_size;
    // int i = strlen(message);
    // strncat(message, file + index, amount);
    // // message[strlen(message)] = EOF;

    // int pos = sizeof(message) / sizeof(message[0]);
    int pos = strlen(message);
    int index = (packet_num - 1) * max_size;
    for (int i = 0; i<amount; i++) {
        message[pos] = file[i+index];
        pos = pos + 1; 
    }

    // // testing by printing message
    // printf(message);

    // strcat(message, EOF);
    // char * l = malloc(1);
    
    // printf("\n>>>Begin\n");
    // printf(message);
    // printf("\n>>>End\n");

    // printf("len: %d\n", strlen(message));
    return message;
}

// int get_file_size(char *filename)
// {

//     FILE *file_pointer;
//     file_pointer = fopen(filename, "r"); //reset the pointer
//     char c = getc(file_pointer);
//     int count = 0;
//     while (c != EOF)
//     {
//         c = getc(file_pointer);
//         count += 1;
//     }
//     return count;
// }

int get_file_size(char *filename)
{
    // opening the file in read mode
    FILE *fp = fopen(filename, "rb");

    fseek(fp, 0L, SEEK_END);

    // calculating the size of the file
    int res = ftell(fp);

    // closing the file
    fclose(fp);

    return res;
}

// char *get_file(char *filename, int file_size)
// {
//     FILE *fptr;
//     FILE *file_pointer;
//     char *message = malloc((file_size + 1) * sizeof(char));

//     int i = 0;
//     file_pointer = fopen(filename, "r"); //reset the pointer
//     char c = getc(file_pointer);
//     while (c != EOF)
//     {
//         message[i] = c;
//         c = getc(file_pointer);
//         i += 1;
//     }
//     message[i] = EOF;
//     fclose(file_pointer);
//     return message;
// }
// char *bs(){
//     struct stat info;
//     const char *filename = "temp.bin";
//     if (stat(filename, &info) != 0) {
//         /* error handling */
//         printf("balls\n");
//     }   
//     printf("FILE SIZE: %lu\n", (unsigned long)info.st_size);

//     char *content = malloc(info.st_size);
//     if (content == NULL) {
//         /* error handling */
//     }   
//     FILE *fp = fopen(filename, "rb");
//     if (fp == NULL) {
//         /* error handling */
//     }
//     /* Try to read a single block of info.st_size bytes */
//     size_t blocks_read = fread(content, info.st_size, 1, fp);
//     if (blocks_read != 1) {
//         /* error handling */
//     }
//     fclose(fp);

//     /*
//     * If nothing went wrong, content now contains the
//     * data read from the file.
//     */

//     printf("DATA LENGTH: %lu\n", (unsigned long)info.st_size);
//     printf(content);
//     printf("\n");
//     return content;
// }

char *get_file(char *filename, int file_size)
{
    FILE *fptr;
    FILE *file_pointer;
    char *message = malloc((file_size) * sizeof(char));

    int i = 0;
    file_pointer = fopen(filename, "rb"); //reset the pointer
    
    // char c = getc(file_pointer);
    // while (i != file_size-1)
    // {
    //     message[i] = c;
    //     c = getc(file_pointer);
    //     i += 1;
    // }

    fread(message, 1, file_size, file_pointer);

    fclose(file_pointer);
    return message;
}

int get_message_len(char *message)
{
    int index = 0;
    int len = 0;
    while (message[index] != EOF)
    {
        len += 1;
        index += 1;
    }
    return len;
}

char *copy_string(char *message, int len)
{
    int index = 0;
    char *copy = malloc(sizeof(char) * (len + 1));
    while (index < len)
    {
        copy[index] = message[index];
        index += 1;
    }
    copy[len] = EOF;
    return copy;
}

int main(int argc, char **argv)
{
    //variables borrowed from Berkely API 2.4
    // int data_size = DEFLEN; 
    int data_size = MAXLEN; 
    int port = SERVER_UDP_PORT;
    int i, j, sd, server_len;
    char *pname, *host, rbuf[MAXLEN], sbuf[MAXLEN];
    struct hostent *hp;
    struct sockaddr_in server, client;
    struct timeval start, end;
    pname = argv[0];

    //check if executeable file is called deliver
    // printf(pname);
    // if (strcmp(pname, "deliver") != 0)
    // {
    //     printf("Executable must be called deliver! Usage: deliver <server address> <server port number>\n");
    //     exit(1);
    // }
    argc--;
    argv++;

    //the main structure of argument checking is borrowed from the Berkley API
    //but their logic is replaced my own

    if (argc > 0)
    {
        //get the server address
        host = *argv;
        if (--argc > 0)
        {
            //grab the port number
            //convert it to an int
            port = atoi(*++argv);
            argc -= 1;
            //if any arguments are left
            //there are too many args
            if (argc > 0)
            {
                printf("Too many arguments! Usage: deliver <server address> <server port number>\n");
                exit(1);
            }
        }
        //port number is missing
        else
        {
            printf("Too few arguments! Usage: deliver <server address> <server port number>\n");
            exit(1);
        }
    }
    //server address and port number are missing
    else
    {
        printf("Too few arguments! Usage: deliver <server address> <server port number>\n");
        exit(1);
    }

    //the following is borrowed from the Berkley API
    //establishes socket,
    //--------------------------------------------------------------
    /* Create a datagram socket */
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        fprintf(stderr, "Can’t create a socket\n");
        exit(1);
    }
    /* Store server’s information */
    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    //take domain name and return pointer to info about server
    if ((hp = gethostbyname(host)) == NULL)
    {
        fprintf(stderr, "Can’t get server’s IP address\n");
        exit(1);
    }
    bcopy(hp->h_addr, (char *)&server.sin_addr,
          hp->h_length);
    /*Bind to local address of socket*/

    //empty memory at client's address
    bzero((char *)&client, sizeof(client));
    client.sin_family = AF_INET;
    //use any available port on the machine
    client.sin_port = htons(0);
    //use any IP client has available
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *)&client, sizeof(client)) == -1)
    {
        fprintf(stderr, "Can't bind client to socket\n");
        exit(1);
    }

    // // I removed this because I didn't know what the purpose was
    // if (data_size > MAXLEN)
    // {
    //     fprintf(stderr, "Data is too big\n");
    //     exit(1);
    // }
    //-------------------------------------------------------------------

    //get user input: ftp <file name>
    char *file_name = get_user_input();
    if (file_name)
    {
        //check if file exists in the directory
        bool exists = file_exists(file_name);
        if (exists)
        {
            // //read file and input it
            // char *name = read_file(file_name);
            // //printf("%s\n",name);
            // int len = get_message_len(name);
            // //printf("Len: %d\n", len);
            // char * buf = copy_string(name, len);
            // bs();
            int file_size = get_file_size(file_name);
            char *file = get_file(file_name, file_size+1);
            // printf("%s\n", file);

            int packets_needed = (file_size / max_size) + 1;
            int left_over = file_size % max_size;

            printf("File Size is: %d - Total packets needed: %d \n", file_size, packets_needed);

            int index = 0;

            char *buf;
            gettimeofday(&start, NULL); /* start measure_delay measure */

            while (index < packets_needed)
            {
                printf(" - Processing Packet #%d \n", index);
                int temp_size = 0;
                if (index == packets_needed - 1)
                {
                    printf(" -- Processing Last packet\n");
                    buf = read_file(file_name, file, left_over, index + 1, packets_needed);
                    temp_size = left_over+50;
                }
                else
                {
                    buf = read_file(file_name, file, max_size, index + 1, packets_needed);
                    temp_size = 1050;
                }

                // testing last packet for a specific case
                if (index == 2167) {
                    printf("\n");
                    printf(buf);
                    printf("\n");
                }

                // printf("PRINTING BUF TO BE SENT BY UDP FUNCTION\n");
                // printf(buf);
                // printf("\n End of buf \n");

                // printf("Yo: %s\n",sbuf);
                //borrowed from Berkley API to transmit the data
                //----------------------------------------------------------------------------------
                server_len = sizeof(server);
                //send data to server
                //sendto returns number of bytes it sent
                if (sendto(sd, buf, MAXLEN, 0, (struct sockaddr *)&server, server_len) == -1)
                {
                    fprintf(stderr, "sendto error\n");
                    exit(1);
                }
                /* receive data */
                if (recvfrom(sd, rbuf, MAXLEN, 0, (struct sockaddr *)&server, &server_len) < 0)
                {
                    fprintf(stderr, "recvfrom error\n");
                    exit(1);
                }
                //-----------------------------------------------------------------------------------

                //if the response we received is 'yes', then we can transfer the file
                if (strcmp(rbuf, "yes") == 0)
                {
                    printf("File has begun transferring\n");
                    index += 1;
                }
                //response is no
                else
                {
                    printf("A file has not transferred\n");
                }

                // delay(200); // creating a 300ms delay

                free(buf);
                bzero(buf,temp_size);
                // memset(buf,0,temp_size);
            }

            free(file);
            gettimeofday(&end, NULL); /* end measure_delay measure */
            printf("Round-trip measure_delay = %ld ms.\n", measure_delay(start, end));

            //terminate socket
            close(sd);
        }
        else
        {
            printf("File name: %s does not exist in this dir\n", file_name);
            close(sd);
        }
    }
    else
    {
        close(sd);
    }

    return (0);
}

long measure_delay(struct timeval t1, struct timeval t2)
{
    long d;
    // printf("t1: %lf, t2: %lf", t1.tv_usec, t2.tv_usec);
    d = (t2.tv_sec - t1.tv_sec) * 1000;
    d += ((t2.tv_usec - t1.tv_usec + 500) / 1000);
    return (d);
}

char *get_user_input()
{
    bool right_input = false;
    char *last_arg;

    while (!right_input)
    {
        char str[MAXLEN];
        //get input from user
        scanf("%[^\n]%*c", str);
        //tokenize the string to get the indiviudal words
        char delim[] = " ";
        char *ptr = strtok(str, delim);
        //immediately check if the first word is ftp
        if (strcmp(ptr, "ftp") == 0)
        {
            int args = 0;
            while (ptr != NULL)
            {
                ptr = strtok(NULL, delim);
                //assign last_arg to second word
                if (args == 0)
                {
                    last_arg = ptr;
                }
                args += 1;
            }
            if (args == 2)
            {
                //2 arguments given
                break;
            }
            else
            {
                //too many arguments
                printf("Usage: ftp <file name>\n");
            }
        }
        //incorrect usage, first word was not ftp
        else
        {
            printf("Usage: ftp <file name>\n");
        }
    }
    //return the second arg: the file name
    return last_arg;
}

bool file_exists(char *file_name)
{
    //directory pointer
    DIR *exists;
    struct dirent *dir;
    //open the current directory
    exists = opendir(".");
    //if not null, the read files in this directory
    if (exists)
    {
        while ((dir = readdir(exists)) != NULL)
        {
            //compare entered file_name to files in directory
            //if match, close the directory and return that the file exists (True)
            if (strcmp(file_name, dir->d_name) == 0)
            {
                closedir(exists);
                return true;
            }
        }
        //at this point, nothing was found, so close the directory
        closedir(exists);
    }
    //return that the file was not found (False)
    return false;
}