#include <stdio.h> 
#include <stdlib.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <string.h> 
#include <time.h> // For the time function
#include <sys/time.h> // For the timeval structure
#include <netinet/tcp.h> 
#include "RUDP_API.h"
#define BUFFER_SIZE 1024
#define TIMEOUT_SECONDS 2
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 9998
#define SIZE_OF_FILE 2800000



char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
// Argument check.
    if (size == 0) {
        return NULL;
    }
    buffer = (char *) calloc(size, sizeof(char));
    if (buffer == NULL){
        printf("calloc(3) failed\n");
        return NULL;
    }
// Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++) {
        *(buffer + i) = ((unsigned int) rand() % 256);
    }
    return buffer;
}


int main(int args, char** argv){
    printf("RUDP Sender starting\n");
    char *server_ip = DEFAULT_IP;
    int port = DEFAULT_PORT;

    if (args != 5) {
        printf("Invalid command, system use default port and ip\n");
    } else if (args == 5) {
        server_ip = argv[2];
        port = atoi(argv[4]);
    }

    int sock = -1;
    int opt = 1;
    struct sockaddr_in server;
    char *data = util_generate_random_data(SIZE_OF_FILE);

    memset(&server, 0, sizeof(server));
    sock = rudp_socket(opt);
    if (sock <0){
        perror("rudp_socket(2)");
        return 1;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port); // Todo: set the server's port

     if (inet_pton(AF_INET, server_ip , &server.sin_addr) <= 0) { // setting the SERVER_IP address
        perror("inet_pton(3)");
        close(sock);
        return 1;
    }

    fprintf(stdout, "Waiting for RUDP Connection...\n");
    if(rudp_connect(sock, (struct sockaddr_in *)&server, sizeof(server)) < 0){
//        perror("rudp_connect(2)");
        rudp_close(sock);
        return -1;
    }
    fprintf(stdout, "Handshake succesfuly done, connected to the server!\n"
                    "Sending data to the server\n");
//    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, CC_algo, strlen(CC_algo)) < 0) {
//        perror("setsockopt(2)");
//        close(sock);
//        return 1;
//    }
    int bytes_sent = rudp_sendto(sock, data, SIZE_OF_FILE, (struct sockaddr_in *)&server, sizeof(server));
    if (bytes_sent < 0) {
        perror("rudp_sendto(2)");
        rudp_close(sock);
        return -1;
    }
    fprintf(stdout, "data sent successfully!\n");
    while (1){
        printf("Press 1 to resend the data or 0 to exit\n");
        int choice;
        if(scanf("%d", &choice)<0){
            perror("scanf(2)");
            close(sock);
            break;
        }
        if(choice == 0){
            if(rudp_dissconnect_client(sock, (struct sockaddr_in *)&server, sizeof(server)) < 0){
                perror("rudp_dissconnect_client(2)");
                rudp_close(sock);
                return -1;
            }
            printf("Disconnected from the server\n");
            rudp_close(sock);
            break;
        }
        if(choice == 1){
            bytes_sent= rudp_sendto(sock, data, SIZE_OF_FILE, (struct sockaddr_in *)&server, sizeof(server));
            if (bytes_sent < 0) {
                perror("rudp_sendto(2)");
                rudp_close(sock);
                return -1;
            }
            fprintf(stdout, "data sent successfully!\n");
            bytes_sent=0;
        }
    }
    free(data);
    fprintf(stdout, "Connection closed!\n");








    return 0;
}