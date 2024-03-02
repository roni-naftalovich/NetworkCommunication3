#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>

// Define constants
#define RECEIVER_IP "127.0.0.1"  // IP address of the receiver
#define RECEIVER_PORT 9998        // Port of the receiver
#define SIZE_OF_FILE 2097152      // 2MB file size
#define BUFFER_SIZE 2048
#define MAX_PACKET_SIZE 1024      // Maximum size of each RUDP packet
#define MAX_RETRIES 5             // Maximum number of retransmission attempts

// Define RUDP API functions

int rudp_socket() {
    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating RUDP socket");
        exit(EXIT_FAILURE);
    }
    
    return sockfd;
}

// Define the structure for RUDP packet
struct rudp_packet {
    int seq_num;                // Sequence number of the packet
    size_t data_len;            // Length of the data in the packet
    char data[MAX_PACKET_SIZE]; // Data payload of the packet
};

// Global variables to store receiver address and address length
struct sockaddr_in g_receiver_addr;
socklen_t g_addr_len;

int perform_handshake(int sockfd) {
    // Send a handshake message to the receiver
    char *msg = "RUDP Handshake";
    int len = strlen(msg);
    if (sendto(sockfd, msg, len, 0, 
               (struct sockaddr *) &g_receiver_addr, g_addr_len) < 0) {
        perror("Error sending handshake message");
        return -1;
    }

    // Receive a response from the receiver
    char response_msg[256]; // Assuming a maximum message size of 256 bytes
    if (recvfrom(sockfd, response_msg, sizeof(response_msg), 0, 
                 (struct sockaddr *) &g_receiver_addr, &g_addr_len) < 0) {
        perror("Error receiving handshake response");
        return -1;
    }

    // Check if the received response is valid
    if (strcmp(response_msg, "RUDP Handshake Ack") != 0) {
        printf("Error: Invalid handshake response\n");
        return -1;
    }

    printf("Handshake successful!\n");
    return 0;
}


int rudp_send(int sockfd, const char *data, size_t len, struct sockaddr_in *receiver_addr, socklen_t addr_len) {
    int last_seq_num_sent = 0; // Initialize sequence number
    size_t total_bytes_sent = 0;

    // Segment the data into packets and send them
    while (total_bytes_sent < len) {
        // Create a RUDP packet
        struct rudp_packet packet;
        memset(&packet, 0, sizeof(packet));

        // Copy a chunk of data into the packet
        size_t chunk_size = len - total_bytes_sent;
        if (chunk_size > MAX_PACKET_SIZE) {
            chunk_size = MAX_PACKET_SIZE;
        }
        memcpy(packet.data, data + total_bytes_sent, chunk_size);
        packet.data_len = chunk_size;
        packet.seq_num = last_seq_num_sent++; // Assign sequence number to the packet

        // Send the packet
        int retries = 0;
        while (retries < MAX_RETRIES) {
            // Send the packet to the receiver
            ssize_t bytes_sent = sendto(sockfd, &packet, sizeof(packet), 0, 
                                        (struct sockaddr *) receiver_addr, addr_len);
            if (bytes_sent < 0) {
                perror("Error sending RUDP packet");
                return -1;
            }

            // Wait for ACK from the receiver (timeout handling not included in this example)
            char ack_msg[256]; // Assuming a maximum message size of 256 bytes
            ssize_t bytes_received = recvfrom(sockfd, ack_msg, sizeof(ack_msg), 0, NULL, NULL);
            if (bytes_received >= 0 && strcmp(ack_msg, "ACK") == 0) {
                // ACK received, move to the next packet
                break;
            }

            // Retry sending the packet
            retries++;
        }

        if (retries == MAX_RETRIES) {
            // Maximum retries exceeded, return error
            printf("Error: Maximum retries exceeded. Unable to send packet.\n");
            return -1;
        }

        // Update total bytes sent
        total_bytes_sent += chunk_size;
    }

    return total_bytes_sent; // Return the total bytes sent
}

int rudp_close(int rudp_socket) {
    // Close the RUDP connection
    int result = close(rudp_socket);
    if (result == -1) {
        perror("Error closing RUDP socket");
        exit(EXIT_FAILURE);
    }
    return 0;
}

char *util_generate_random_data(unsigned int size) {
    // Generate random data to be sent
    char *buffer = (char *)malloc(size * sizeof(char));
    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    for (unsigned int i = 0; i < size; i++) {
        buffer[i] = rand() % 256; // Random byte value between 0 and 255
    }
    return buffer;
}

int main() {
    // Create a RUDP socket
    int rudp_sock = rudp_socket();

    // Generate random data to be sent
    char *file_content = util_generate_random_data(SIZE_OF_FILE);

    // Perform handshake with the receiver
    if (perform_handshake(rudp_sock) < 0) {
        perror("Handshake failed");
        rudp_close(rudp_sock);
        free(file_content);
        exit(EXIT_FAILURE);
    }

    // Connect to the receiver using RUDP protocol
    struct sockaddr_in receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(RECEIVER_PORT);
    receiver_addr.sin_addr.s_addr = inet_addr(RECEIVER_IP);

    // Send the file content over RUDP
    size_t total_bytes_sent = 0;
    while (total_bytes_sent < SIZE_OF_FILE) { 
        size_t remaining_bytes = SIZE_OF_FILE - total_bytes_sent;
        size_t chunk_size = remaining_bytes > BUFFER_SIZE ? BUFFER_SIZE : remaining_bytes;
        int sent_bytes = rudp_send(rudp_sock, file_content + total_bytes_sent, chunk_size, &receiver_addr, sizeof(receiver_addr));
        if (sent_bytes < 0) {
            perror("Sending data over RUDP failed");
            rudp_close(rudp_sock);
            free(file_content);
            exit(EXIT_FAILURE);
        }
        total_bytes_sent += sent_bytes;
    }

    // Close the RUDP connection
    rudp_close(rudp_sock);

    // Free allocated memory
    free(file_content);

    printf("File sent successfully\n");

    return 0;
}