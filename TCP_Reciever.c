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
#define SIZE_OF_FILE 2097152

char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
    if (size == 0)
        return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    if (buffer == NULL)
        return NULL;
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
        *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}

int main() {
    char *data = util_generate_random_data(SIZE_OF_FILE);
    int tcp_socket;
    struct sockaddr_in server_addr;

    // Create socket
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Specify the address and port of the server to connect
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SENDER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Assuming localhost

    // Connect to server
    if (connect(tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(tcp_socket);
        return -1;
    }

    // Send data
    int bytes_sent = 0;
    int total_bytes_sent = 0;
    int len = SIZE_OF_FILE;

    while (total_bytes_sent < len) {
        bytes_sent = send(tcp_socket, data + total_bytes_sent, BUFFER_SIZE, 0);
        if (bytes_sent < 0) {
            perror("Sending data failed");
            close(tcp_socket);
            return -1;
        }
        total_bytes_sent += bytes_sent;
    }

    // Close the TCP connection
    close(tcp_socket);

    // Free allocated memory
    free(data);

    // Exit
    return 0;
}