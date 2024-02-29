
#include <sys/types.h> 
#include <sys/socket.h>
#include <math.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <arpa/inet.h>
#define SENDER_PORT 9997
#define RECEIVER_PORT 9998
#define BUFFER_SIZE 2048
#define SIZE_OF_FILE 2097152
#define EXIT_MESSAGE "EXIT"
/*
* @brief A random data generator function based on srand() and rand().
* @param size The size of the data to generate (up to 2^32 bytes).
* @return A pointer to the buffer.
*/
char *util_generate_random_data(unsigned int size) {
 char *buffer = NULL;
 // Argument check.
 if (size == 0)
 return NULL;
 buffer = (char *)calloc(size, sizeof(char));
 // Error checking.
 if (buffer == NULL)
 return NULL;
 // Randomize the seed of the random number generator.
 srand(time(NULL));
 for (unsigned int i = 0; i < size; i++)
 *(buffer + i) = ((unsigned int)rand() % 256);
 return buffer;
}
int main(){
char* data=util_generate_random_data(SIZE_OF_FILE);
int tcp_socket;
tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
if (tcp_socket < 0) {
        perror("Socket creation failed");
        close(tcp_socket);
        return -1;
    }
// Specify the address and port of the server to connect
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SENDER_PORT); 
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Assuming localhost

int connectSystem = connect(tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
if (connectSystem<0)
{printf("connection creation faild");
    return -1;
}

memset(&server_addr,0,sizeof(server_addr));

int yes = 1;
if(setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
     perror("setsockopt");
     close(tcp_socket);
     return -1;
}
int listenSystem = listen(tcp_socket, 5);
if (listenSystem<0)
{printf("listen faild");
  close(tcp_socket);
    return -1;
}
int acceptSystem = accept(tcp_socket,(struct sockaddr *)&server_addr, sizeof(server_addr));
if (acceptSystem<0)
{printf("acception faild");
    return -1;
}
int len, bytes_sent;
len = SIZE_OF_FILE;
bytes_sent = send(tcp_socket,data,len,0);

while (bytes_sent <len){
    if(bytes_sent <0){
    perror("sending data faild");
    close(tcp_socket);
    return -1}
    else{
        bytes_sent = send(tcp_socket,data,len,0);}
}

bytes_sent = send(tcp_socket, EXIT_MESSAGE, strlen(EXIT_MESSAGE), 0);
    if (bytes_sent < 0) {
        perror("Sending exit message failed");
        close(tcp_socket);
        return -1;
    }
    // Close the TCP connection
    close(tcp_socket);

    // Free allocated memory
    free(data);

    // Exit
    return 0;
}