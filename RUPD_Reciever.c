#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
//#include "RUDP_API.h"
#include <time.h>

#define DEFAULT_PORT 9998
#define DEFAULT_RECEIVER_IP "127.0.0.1"
#define BUFFER_SIZE 2000000
#define SIZE_OF_FILE 2000000
#define MAX_RETRIES 500

float receive_file(int server_sock) {
    FILE *file;
    char buffer[BUFFER_SIZE];
    time_t start, end;
    float time_taken;
    int bytesReceived;

    file = fopen("received_file.txt", "w");
    if (file == NULL) {
        perror("Error creating file");
        return 0;
    }

    start = clock();

    while ((bytesReceived = recv(server_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesReceived, file);
        if (bytesReceived < BUFFER_SIZE)
            break;
    }
    fclose(file);

    end = clock();
    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC * 1000;

    printf("File transfer completed.\n");
    return time_taken;
}

int main(int argc, char *argv[]) {
    printf("Starting Receiver...\n");
    int port= DEFAULT_PORT;
    char file[SIZE_OF_FILE] = {0};
    int numoftry=1;
    time_t start_time, end_time;

     if (argc != 3)
    {
        printf("invalid command\n");
        return -1;
    }
     port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in receiver_addr;
    sockfd = rudp_socket();

    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    receiver_addr.sin_port = htons(port);

    //bind
    if (bind(sockfd, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Reciever waiting for RUDP connection\n");

    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);
    memset(&sender_addr, 0, sender_addr_len);
    int flag;

     while (numoftry< MAX_RETRIES) {
        // Wait for handshake
        start_time = clock();
        int seqnum = rudp_connect(sockfd, &sender_addr, sizeof(sender_addr), 1);
        if (seqnum != 0) {
            perror("connection failed");
            close(sockfd);
            return -1;
        }
        if (start_time>10)
        {
           break;
        }
        
        printf("Connected successfully. \n");
        int flag = rudp_rcv_file(file, sockfd, &receiver_addr, sizeof(receiver_addr));

        if (flag < 0) {
            perror("receiving failed");
            return -1;
        }
        end_time = clock();
        
        printf("Finished receiving\n");
        printf("Statistic for run number %d :\n" , numoftry);
        // Calculate the time taken to receive the file
        double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("Time taken to receive the file: %lf seconds\n", time_taken);
        // Calculate average bandwidth
        double average_bandwidth = (SIZE_OF_FILE / time_taken) / 1024; // in KB/s
        printf("Average bandwidth: %lf KB/s\n", average_bandwidth);
       // }

        rudp_close(sockfd) ;
         
       numoftry++;
    }
    }

    