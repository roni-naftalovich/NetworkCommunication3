#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <netinet/tcp.h>

#define SENDER_PORT 9998
#define RECEIVER_PORT 9997
#define BUFFER_SIZE 2000000
#define SIZE_OF_FILE 2000000
#define DEFAULT_ALGO "reno"
#define MAX_TRY 100 // 30 times chances to accept msg from sender



int main(int argsc, char **argsv) {
    printf("Starting reciever...\n");
    //char *data = util_generate_random_data(SIZE_OF_FILE);
    int tcp_socket;
    char *tcp_algo = DEFAULT_ALGO;
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

    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(port_Address);
    receiver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Create socket
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }

    //  int yes = 1;
    //  if(setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
    //  perror("setsockopt");
    //  close(tcp_socket);
    //  return -1;
    // }

    if (setsockopt(tcp_socket, IPPROTO_TCP, TCP_CONGESTION, tcp_algo, strlen(tcp_algo)) != 0) {
    printf("Failed to set TCP congestion control algorithm\n");
    close(tcp_socket);
    return -1;
    }
    

    // bind
    if (bind(tcp_socket, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0) {
        perror("Reciever Connection failed");
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
   int numoftry = 1;
   int client_socket;
   ssize_t bytes_received= 0;
   int updatenumoftry = 0;
   clock_t start_time;
   double sumOfTimes= 0;
   double sumOfSpeed=0;
   char infobuffer[SIZE_OF_FILE]= {0};
   char copy[SIZE_OF_FILE]= {0};

   client_socket= accept(tcp_socket,(struct sockaddr *)&sender_addr, ( socklen_t *)&sender_addr_len);
   if(client_socket < 0){
    printf("Accepting failed");
    close(tcp_socket);
    return -1;
   }
    printf("Connection established with the sender.\n");
   
   while(numoftry< MAX_TRY){
   if (numoftry==updatenumoftry)
   {
   start_time = clock(); // Start measuring time
   }

   ssize_t bytes = recv(client_socket, infobuffer, SIZE_OF_FILE, 0);
        if (bytes < 0) {
            printf("Receiving data failed");
            close(client_socket);
            close(tcp_socket);
            return -1;
        }
        bytes_received+= bytes;
        //if (bytes_received == SIZE_OF_FILE)
       // {
        
        


        if (strcmp(infobuffer, "EXIT") == 0) {
        break;
        }
        strcpy(copy, infobuffer);
        memset(infobuffer, '\0', sizeof(infobuffer));

          if (bytes_received < SIZE_OF_FILE) {
            updatenumoftry++;
            continue;
        }

        clock_t end_time = clock();
        printf("----------------------------------\n");
        printf("-           * Statistics *       -\n");
        printf("run number %d :\n", numoftry);

        // Calculate the time taken to receive the file
        double time_taken = (((double)(end_time - start_time)) / CLOCKS_PER_SEC) * 1000;
        sumOfTimes += time_taken;
        sumOfSpeed += (SIZE_OF_FILE / time_taken) / 1000;
        printf("Time taken to receive the file: %lf ms\n", time_taken);

        bytes_received = 0;
        numoftry++;
        updatenumoftry = numoftry;
    }
   
   printf("Exit message sent from sender\n");
   double average_bandwidth = (sumOfSpeed / sumOfTimes)*1000 / 1024 ; // in MB/s
   printf("Average bandwidth: %lf MB/s\n", average_bandwidth);

    // Close the sockets
    close(client_socket);
    close(tcp_socket);
   

    return 0;
}