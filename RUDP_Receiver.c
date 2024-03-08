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


#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 9998
#define MAX_CLIENTS 1
#define BUFFER_SIZE 1024
#define MAX_RUNS 50
#define SIZE_OF_FILE 2800000
#define FILENAME "stats.txt"


int main(int args, char** argv) {
    printf("Starting RUDP Receiver...\n");
    int port = DEFAULT_PORT;
    if (args != 3) {
        printf("Invalid command, system use default port\n");
    } else if (args ==3) {
        port = atoi(argv[2]);

    }

    int sock;
    struct timeval start, end;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    // Reset the server and client structures to zeros.
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, client_len);
    

    // The variable to store the socket option for reusing the server's address.
    int opt = 1;

    sock = rudp_socket(opt);
    if (sock <0 ) {
        perror("rudp_socket_setup(2)");
        return 1;
    }

    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

     // Try to bind the socket to the server's address.
    if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind(2)");
        close(sock);
        return -1;
        }

    fprintf(stdout, "Listening for incoming connections ...\n");
        


    int client_sock = rudp_accept(sock, (struct sockaddr_in *)&client_addr, &client_len);
    if (client_sock <0){
        perror("rudp_accept(2)");
        close(sock);
        return 1;
    }
    fprintf(stdout, "Handshake succesfuly done, accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    int total_bytes_received = 0;
    int bytes_received=0;
    int numofrun = 0;
    int packages_received = 0;
    double speed_sum = 0.0;
    double time_sum = 0.0;
    char buffer[BUFFER_SIZE];

    FILE *stats_file = fopen(FILENAME, "w");
    if (stats_file == NULL) {
        perror("fopen");
        return 1;
    }

    while(1){
        bytes_received=0;
        numofrun = 0;
        total_bytes_received = 0;
        memset(buffer, 0, BUFFER_SIZE);


        while(total_bytes_received<SIZE_OF_FILE){
            bytes_received=0;
            bytes_received = rudp_recvfrom(sock,(struct sockaddr_in *)&client_addr, &client_len);
            if (bytes_received ==- 1) {
                perror("rudp_recvfrom(2)");
                close(sock);
                return 1;
            }
            if(numofrun == 0){
                gettimeofday(&start, NULL);
            }
            if (bytes_received == -2) {
                close(sock);
                break;
            }
            total_bytes_received += bytes_received;
            numofrun++;
            memset(buffer, 0, sizeof(buffer));


        }

        gettimeofday(&end, NULL);
        if(bytes_received==-2){
            break;
        }


        printf("File transfer complete\n");
        double time_taken = ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_usec - start.tv_usec) / 1000.0);
        double speed_taken = (total_bytes_received / 1024.0 / 1024.0) / (time_taken / 1000.0);
        speed_sum = speed_sum + speed_taken;
        time_sum = time_sum + time_taken;

        packages_received++;

        fprintf(stats_file, "Run number %d :\n Time = %.2f ms; Speed = %.2f MB/s\n", packages_received, time_taken, speed_taken);

        // Print the sent message.
        printf( "waiting for the Senders response...\n");
    }
    fclose(stats_file);

    close(sock);
    printf("closed the client socket\n");
    //Print statistics
    printf("----------------------------------\n");
    printf("-           * Statistics *       -\n");
    stats_file = fopen(FILENAME, "r");
    if (stats_file == NULL) {
        perror("fopen");
        return 1;
    }

    while (fgets(buffer, sizeof(buffer), stats_file) != NULL) {
        printf("%s", buffer);
    }
    printf("\n");
    printf(" Average time: %.2fms\n", time_sum / packages_received);
    printf(" Average bandwidth: %.2fMB/s\n", speed_sum / packages_received);
    printf("----------------------------------\n");
    printf( "Receiver end.\n");

return 0;
}