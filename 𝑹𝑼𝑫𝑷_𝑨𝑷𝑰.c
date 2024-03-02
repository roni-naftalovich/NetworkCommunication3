#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

// Define constants
#define MAX_PACKET_SIZE 1024  // Maximum size of each RUDP packet
#define MAX_RETRIES 5          // Maximum number of retransmission attempts

// Define the structure for RUDP packet
struct rudp_packet {
    int seq_num;            // Sequence number of the packet
    size_t data_len;        // Length of the data in the packet
    char data[MAX_PACKET_SIZE];  // Data payload of the packet
};

/*
 * @brief Performs handshake with the receiver.
 * 
 * @param sockfd The socket file descriptor.
 * @param receiver_addr The receiver's address information.
 * @param addr_len The length of the receiver's address.
 * @return 0 on success, -1 on failure.
 */
int perform_handshake(int sockfd, struct sockaddr_in *receiver_addr, socklen_t addr_len) {
    // Send a handshake message to the receiver
    char handshake_msg[] = "RUDP Handshake";
    if (sendto(sockfd, handshake_msg, sizeof(handshake_msg), 0, 
               (struct sockaddr *) receiver_addr, addr_len) < 0) {
        perror("Error sending handshake message");
        return -1;
    }

    // Receive a response from the receiver
    char response_msg[256]; // Assuming a maximum message size of 256 bytes
    if (recvfrom(sockfd, response_msg, sizeof(response_msg), 0, 
                 (struct sockaddr *) receiver_addr, &addr_len) < 0) {
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

/*
 * @brief Sends a RUDP packet to the receiver.
 * 
 * @param sockfd The socket file descriptor.
 * @param packet Pointer to the RUDP packet to be sent.
 * @param receiver_addr The receiver's address information.
 * @param addr_len The length of the receiver's address.
 * @return Number of bytes sent on success, -1 on failure.
 */
int rudp_send(int sockfd, struct rudp_packet *packet, struct sockaddr_in *receiver_addr, socklen_t addr_len) {
    // Send the packet to the receiver
    ssize_t bytes_sent = sendto(sockfd, packet, sizeof(*packet), 0, 
                                (struct sockaddr *) receiver_addr, addr_len);
    if (bytes_sent < 0) {
        perror("Error sending RUDP packet");
        return -1;
    }

    return bytes_sent; // Return the number of bytes sent
}

/*
 * @brief Closes the RUDP connection.
 * 
 * @param sockfd The socket file descriptor.
 * @return 0 on success, -1 on failure.
 */
int rudp_close(int sockfd) {
    // Close the socket
    if (close(sockfd) < 0) {
        perror("Error closing RUDP socket");
        return -1;
    }
    return 0;
}