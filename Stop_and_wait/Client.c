#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Define Packet Structure
typedef struct {
    char data[1024];
} Packet;

// Define Frame Structure
typedef struct {
    int frame_kind; // ACK: 0, SEQ: 1, FIN: 2
    int sq_no;
    int ack;
    Packet packet;
} Frame;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    char buffer[1024];

    int frame_id = 0;
    Frame frame_send, frame_recv;
    int ack_recv = 1; // Flag to track ACK reception

    // Create UDP Socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("[-]Socket creation failed");
        exit(1);
    }
    printf("[+]UDP Socket Created.\n");

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    addr_size = sizeof(serverAddr);

    while (1) {
        if (ack_recv == 1) {
            // Prepare Frame
            frame_send.sq_no = frame_id;
            frame_send.frame_kind = 1; // SEQ Frame
            frame_send.ack = 0;

            printf("Enter Data: ");
            scanf("%s", buffer);
            strcpy(frame_send.packet.data, buffer);

            // Send Frame
            sendto(sockfd, &frame_send, sizeof(Frame), 0, 
                   (struct sockaddr*)&serverAddr, addr_size);
            printf("[+]Frame Sent\n");

            ack_recv = 0; // Reset flag until ACK is received
        }

        // Receive ACK
        int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, 
                                   (struct sockaddr*)&serverAddr, &addr_size);
        
        if (f_recv_size > 0 && frame_recv.frame_kind == 0 && frame_recv.ack == frame_id + 1) {
            printf("[+]ACK Received\n");
            ack_recv = 1; // Set flag to send next frame
            frame_id++;   // Move to next frame
        } else {
            printf("[-]ACK Not Received, Resending Frame...\n");
        }
    }

    close(sockfd);
    return 0;
}
