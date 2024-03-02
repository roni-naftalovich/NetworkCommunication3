#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/time.h>

#define RECEIVER_PORT 9998  // Port of the receiver
#define MAX_PACKET_SIZE 1024      // Maximum size of each RUDP packet

// Structure to hold RUDP packet
struct rudp_packet {
    int seq_num;                // Sequence number of the packet
    size_t data_len;            // Length of the data in the packet
    char data[MAX_PACKET_SIZE];// Data payload of the packet
};

void receive_data(int udp_socket) {
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    // Buffer to store received data
    char buffer[MAX_PACKET_SIZE];

    // File pointer to save the received data
    FILE *file = fopen("received_file.txt", "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // Variables for measuring time and calculating bandwidth
    struct timeval start_time, end_time;
    double elapsed_time_ms, total_time_ms = 0.0;
    int num_received = 0;

    // Flag to track if the file has been received at least once
    bool file_received = false;

    while (true) {
        // Start measuring time
        gettimeofday(&start_time, NULL);

        // Receive data until the sender sends an exit message
        while (true) {
            // Receive a packet from the sender
            ssize_t bytes_received = recvfrom(udp_socket, buffer, sizeof(buffer), 0, 
                                              (struct sockaddr *)&sender_addr, &addr_len);
            if (bytes_received < 0) {
                perror("Error receiving data");
                fclose(file);
                return;
            } else if (bytes_received == 0) {
                // No more data to receive
                break;
            }

            // Write the received data to the file
            size_t bytes_written = fwrite(buffer, 1, bytes_received, file);
            if (bytes_written != bytes_received) {
                perror("Error writing data to file");
                fclose(file);
                return;
            }

            // Set the file_received flag to true
            file_received = true;
        }

        // End measuring time
        gettimeofday(&end_time, NULL);

        // Calculate elapsed time in milliseconds
        elapsed_time_ms = (end_time.tv_sec - start_time.tv_sec) * 1000.0; // Convert seconds to milliseconds
        elapsed_time_ms += (end_time.tv_usec - start_time.tv_usec) / 1000.0; // Add microseconds converted to milliseconds

        // If the file has been received at least once, print the time taken to receive the file
        if (file_received) {
            printf("Time taken to receive the file: %.2f milliseconds\n", elapsed_time_ms);
            total_time_ms += elapsed_time_ms;
            num_received++;
            file_received = false; // Reset the flag for the next iteration
        }

        // Wait for a response from the sender
        char response[MAX_PACKET_SIZE];
        ssize_t bytes_received = recvfrom(udp_socket, response, sizeof(response), 0, 
                                          (struct sockaddr *)&sender_addr, &addr_len);
        if (bytes_received < 0) {
            perror("Error receiving response from sender");
            fclose(file);
            return;
        } else if (bytes_received > 0) {
            // Handle the response from the sender
            if (strcmp(response, "Resend") == 0) {
                // Sender wants to resend the file, continue receiving
                continue;
            } else if (strcmp(response, "Exit") == 0) {
                // Sender sends an exit message, proceed to next step
                break;
            }
        }
    }

    // Calculate the average time
    double avg_time_ms = total_time_ms / num_received;
    printf("Average time to receive the file: %.2f milliseconds\n", avg_time_ms);

    // Calculate and print the average bandwidth
    // For simplicity, assume the file size is fixed and known
    double file_size_MB = 2.0; // 2MB
    double total_bandwidth_MBps = (file_size_MB * num_received) / (total_time_ms / 1000.0);
    double avg_bandwidth_MBps = total_bandwidth_MBps / num_received;
    printf("Average bandwidth: %.2f MB/s\n", avg_bandwidth_MBps);

    // Close the file
    fclose(file);
}

int main() {
    // Create a UDP socket
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure the receiver address
    struct sockaddr_in receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(RECEIVER_PORT);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the receiver address
    if (bind(udp_socket, (const struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0) {
        perror("Binding failed");
        close(udp_socket);
        exit(EXIT_FAILURE);
    }

    printf("UDP connection established\n");

    // Call the function to receive data using RUDP protocol
    receive_data(udp_socket);

    // Close the socket
    close(udp_socket);

    return 0;
}