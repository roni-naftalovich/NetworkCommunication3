#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <time.h>

#define SENDER_PORT 9998
#define RECEIVER_IP "127.0.0.1"
#define RECEIVER_PORT 9997
#define BUFFER_SIZE 2000000
#define DEFAULT_ALGO "reno"
#define SIZE_OF_FILE 2000000
#define EXIT_MESSAGE "EXIT"


//void util_read_file(const char *filename, unsigned int *size) {
   // FILE *file;
    //file = fopen(filename, "r");
   // if (file== NULL) {
  //      printf("Error opening file %s\n", filename);
  //  }
//}



char *util_generate_random_data(unsigned int size) {
    char *buffer = (char *)malloc(size);
    if (buffer == NULL) {
        printf("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
        buffer[i] = rand() % 256;
    return buffer;
}

int main(int argsc, char **argsv) {
    printf("Starting Sender...\n");
    //char* dataBuffer = NULL //this is a buffer area for saving the file's data that we can read from
    unsigned int data_size = SIZE_OF_FILE;
    char *data = util_generate_random_data(data_size);
    //util_read_file(data, &data_size);
  
    
    char *tcp_algo = DEFAULT_ALGO;
    char *ip_address = RECEIVER_IP;
    int port_Address = SENDER_PORT;

    if (argsc <= 1)
    {
        // no arguments
    }
    else
    {
        int i = 1;
        while (i < argsc)
        {
            char *arg = argsv[i];
            if (strcmp(arg, "-p") == 0)
            {
                i++;
                if (i == argsc)
                {
                    // error empty arg
                }
                else
                {
                    port_Address = atoi(argsv[i]);
                }
            }
            else if (strcmp(arg, "-ip") == 0)
            {
                i++;
                if (i == argsc)
                {
                    // error empty arg
                }
                else
                {
                    ip_address = argsv[i];
                }
            }
            else if (strcmp(arg, "-algo") == 0)
            {
                i++;
                if (i == argsc)
                {
                    // error empty arg
                }
                else
                {
                    // check if value is reno or cubic
                    tcp_algo = argsv[i];
                }
            }
            i++;
        }
    }

    // Create TCP socket
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        printf("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int yes = 1;
     if(setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
     printf("setsockopt");
     close(tcp_socket);
     return -1;
}

if (setsockopt(tcp_socket, IPPROTO_TCP, TCP_CONGESTION, tcp_algo, strlen(tcp_algo)) != 0) {
    printf("Failed to set TCP congestion control algorithm\n");
    close(tcp_socket);
    free(data);
    return -1;
}


    struct sockaddr_in sender_addr;
    memset(&sender_addr,0,sizeof(sender_addr));
    sender_addr.sin_family = AF_INET;
    sender_addr.sin_port = htons(port_Address); 
    //sender_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Assuming localhost
    socklen_t adresslen = sizeof(sender_addr);

    if (inet_pton(AF_INET, ip_address, &sender_addr.sin_addr) <= 0)
    {
        printf("inet_pton() problem");
        close(tcp_socket);
        return -1;
    }

    

    // Connect to the receiver
    if (connect(tcp_socket, (struct sockaddr *)&sender_addr, adresslen) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    int sending = 1;
    long bytes_sent;
    while(sending==1){

    //Send the file
     bytes_sent = send(tcp_socket, data, SIZE_OF_FILE, 0);
     if (bytes_sent < 0) {
         printf("Sending data failed");
         return -1;
     }
     else if (bytes_sent >= 0 && bytes_sent< SIZE_OF_FILE){
        printf("Sent only part of the data\n");
     }
     else{
         printf("data sent\n");
     }
     int input= -1;
     while(input){
     printf("Press 1 to resend the data or 0 to exit\n");
     scanf("%d", &input);

     if (input ==0)
     {
        send(tcp_socket, EXIT_MESSAGE, strlen(EXIT_MESSAGE), 0);
        printf("exit message sent to receiver\n");
        break;
     }
     else if(input==1){
        printf("Resending data\n");
        bytes_sent = send(tcp_socket, data, SIZE_OF_FILE, 0);
        if (bytes_sent < 0) {
        printf("Sending data failed");
        return -1;
        }else{printf("data resent succesfully\n");
        }
     }else{
        continue;
     }
     }}

    
    // Close the connection
    close(tcp_socket);

    // Free allocated memory
    free(data);
    printf("Sender ends\n");

    return 0;
}