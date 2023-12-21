#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 516

// Obtain server address information
int getServerAddress(const char *serverName, const char *port, struct addrinfo **serverInfo) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;      // Accept both AF_INET and AF_INET6
    hints.ai_socktype = SOCK_DGRAM;   // UDP socket
    
    int status = getaddrinfo(serverName, port, &hints, serverInfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }
    return 0;
}

// Create a UDP socket
int createUDPSocket(struct addrinfo *serverInfo) {
    int udpSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if (udpSocket == -1) {
        perror("Error creating socket");
        return -1;
    }
    return udpSocket;
}

// Send a TFTP request (RRQ or WRQ)
void sendRequest(int udpSocket, struct addrinfo *serverInfo, const char *fileName, int opcode) {
    char buffer[BUFFER_SIZE] = {0};
    buffer[1] = (char)opcode;  // Opcode for RRQ or WRQ
    
    // Add the file name and mode to the request
    strcpy(buffer + 2, fileName);
    strcpy(buffer + 2 + strlen(fileName) + 1, "octet");

    size_t requestLen = 2 + strlen(fileName) + 1 + strlen("octet") + 1;
    sendto(udpSocket, buffer, requestLen, 0, serverInfo->ai_addr, serverInfo->ai_addrlen);
}

// Receive file from TFTP server
void receiveFile(int udpSocket, const char *fileName) {
    char buffer[BUFFER_SIZE];
    int received, blockNum = 1;
    FILE *file = fopen(fileName, "wb");
    
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        received = recvfrom(udpSocket, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (received < 0) {
            perror("Error receiving packet");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        
        // Check if it's a data packet and write data to file
        if (buffer[1] == 3) {  // Data packet
            fwrite(buffer + 4, 1, received - 4, file);
            
            // Send ACK
            buffer[1] = 4; // Opcode for ACK
            *(short *)(buffer + 2) = htons(blockNum++);
            sendto(udpSocket, buffer, 4, 0, NULL, 0);

            if (received < BUFFER_SIZE) {  // Last packet
                break;
            }
        }
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: gettftp serverName port fileName\n");
        return EXIT_FAILURE;
    }

    struct addrinfo *serverInfo;
    if (getServerAddress(argv[1], argv[2], &serverInfo) != 0) {
        return EXIT_FAILURE;
    }
    
    int udpSocket = createUDPSocket(serverInfo);
    if (udpSocket == -1) {
        freeaddrinfo(serverInfo);
        return EXIT_FAILURE;
    }
    
    sendRequest(udpSocket, serverInfo, argv[3], 1); // Opcode 1 for RRQ
    receiveFile(udpSocket, argv[3]);

    freeaddrinfo(serverInfo);
    close(udpSocket);
    return EXIT_SUCCESS;
}
