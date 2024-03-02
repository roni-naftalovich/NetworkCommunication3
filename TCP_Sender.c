#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define SENDER_PORT 9997
#define RECEIVER_IP "127.0.0.1"
#define RECEIVER_PORT 9998
#define BUFFER_SIZE 2048
#define SIZE_OF_FILE 2000000
#define EXIT_MESSAGE "EXIT"






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

int main() {
    //char* dataBuffer = NULL //this is a buffer area for saving the file's data that we can read from
    char *data = util_generate_random_data(SIZE_OF_FILE);

    //Let's start with reading the file content and save it into the dataBuffer


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


    struct sockaddr_in sender_addr;
    memset(&sender_addr,0,sizeof(sender_addr));
    sender_addr.sin_family = AF_INET;
    sender_addr.sin_port = htons(RECEIVER_PORT); 
    //sender_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Assuming localhost
    socklen_t adresslen = sizeof(sender_addr);

    if (inet_pton(AF_INET, RECEIVER_IP, &sender_addr.sin_addr) <= 0)
    {
        printf("inet_pton() problem");
        close(tcp_socket);
        return -1;
    }

    

    // Connect to the receiver
    if (connect(tcp_socket, (struct sockaddr *)&sender_addr, adresslen) < 0) {
        printf("Connection failed");
        exit(EXIT_FAILURE);
    }

    int sending = 1;

    while(sending){

    // Send the file
    int bytes_sent = send(tcp_socket, data, SIZE_OF_FILE, 0);
    if (bytes_sent < 0) {
        printf("Sending data failed");
        return -1;
    }
    else if (bytes_sent >= 0 && bytes_sent< SIZE_OF_FILE){
       printf("Sent only part of the data");
    }
    else{
        printf("data sent");
     }
     printf("press 1 to send again or 0 to exit:\n");
     scanf("%d", &sending);
    }

    // Send exit message
    send(tcp_socket, EXIT_MESSAGE, strlen(EXIT_MESSAGE), 0);

    // Close the connection
    close(tcp_socket);

    // Free allocated memory
    free(data);

    return 0;
}