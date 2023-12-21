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

// Send file to TFTP server
void sendFile(int udpSocket, struct sockaddr_storage *serverAddr, socklen_t *addrLen, const char *data, int totalSize) {
    char buffer[BUFFER_SIZE];
    int sent, blockNum = 1;
    
    for (sent = 0; sent < totalSize; sent += 512, blockNum++) {
        int dataSize = (totalSize - sent > 512) ? 512 : totalSize - sent;
        buffer[1] = 3; // Opcode for DAT
        *(short *)(buffer + 2) = htons(blockNum);
        memcpy(buffer + 4, data + sent, dataSize);
        sendto(udpSocket, buffer, dataSize + 4, 0, (struct sockaddr *)serverAddr, *addrLen);
        
        recvfrom(udpSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)serverAddr, addrLen);
        // Check for ACK of the current block
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: puttftp serverName port fileName\n");
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
    
    struct sockaddr_storage serverAddr;
    socklen_t addrLen = sizeof(serverAddr);
    if (serverInfo->ai_addrlen > sizeof(serverAddr)) {
        fprintf(stderr, "Server address size is too large\n");
        freeaddrinfo(serverInfo);
        close(udpSocket);
        return EXIT_FAILURE;
    }
    memcpy(&serverAddr, serverInfo->ai_addr, serverInfo->ai_addrlen);

    
    sendRequest(udpSocket, serverInfo, argv[3], 2); // Opcode 2 for WRQ
    const char *data = "File data to send"; // Example data
    int dataSize = strlen(data);
    sendFile(udpSocket, &serverAddr, &addrLen, data, dataSize);
    
    freeaddrinfo(serverInfo);
    close(udpSocket);
    return EXIT_SUCCESS;
}


