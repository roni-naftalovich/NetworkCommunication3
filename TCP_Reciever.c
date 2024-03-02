#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define SENDER_PORT 9997
#define RECEIVER_PORT 9998
#define BUFFER_SIZE 2048
#define SIZE_OF_FILE 2000000


int main() {
    //char *data = util_generate_random_data(SIZE_OF_FILE);
    int tcp_socket;

    // Create socket
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(SENDER_PORT);
    receiver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    

    // bind
    if (bind(tcp_socket, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0) {
        perror("Reciever Connection failed");
        close(tcp_socket);
        return -1;
    }

     int yes = 1;
     if(setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
     perror("setsockopt");
     close(tcp_socket);
     return -1;
}

   //listen
   if(listen(tcp_socket, 1) <0){
    printf("Listening failed");
    close(tcp_socket);
   }
   printf("Reciever waiting for TCP connection\n");

    
    
   //accept
   // Specify the address and port of the server to connect
   struct sockaddr_in sender_addr;
   memset(&sender_addr,0,sizeof(sender_addr));
   socklen_t sender_addr_len = sizeof(sender_addr);
   int client_socket= accept(tcp_socket,(struct sockaddr *)&sender_addr, ( socklen_t *)&sender_addr_len);
   if(client_socket < 0){
    printf("Accepting failed");
    close(tcp_socket);
    return -1;
   }

   printf("Connection established with the sender.\n");
char infobuffer[BUFFER_SIZE]= {0};
   int bytes_received= 0;
   clock_t start_time = clock(); // Start measuring time
    while (bytes_received < SIZE_OF_FILE) {
        int bytes = recv(client_socket, infobuffer, BUFFER_SIZE, 0);
        if (bytes < 0) {
            printf("Receiving data failed");
            close(client_socket);
            close(tcp_socket);
            return -1;
        }
        bytes_received += bytes;
    }
    clock_t end_time = clock();
   
 // Calculate the time taken to receive the file
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Time taken to receive the file: %.2f seconds\n", time_taken);

    // Calculate average bandwidth
    double average_bandwidth = (SIZE_OF_FILE / time_taken) / 1024; // in KB/s
    printf("Average bandwidth: %.2f KB/s\n", average_bandwidth);

    // Close the sockets
    close(client_socket);
    close(tcp_socket);

    // Free allocated memory
    //free(infobuffer);

    return 0;
}