#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

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

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in serverAddr, newAddr;
    char buffer[1024];
    socklen_t addr_size;

    int frame_id = 0;
    Frame frame_recv, frame_send;

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

    // Bind Socket to Address
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("[-]Bind failed");
        exit(1);
    }
    printf("[+]Bind to port %d successful.\n", port);

    addr_size = sizeof(newAddr);

    while (1) {
        // Receive Frame
        int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0,
                                   (struct sockaddr*)&newAddr, &addr_size);
        
        if (f_recv_size > 0 && frame_recv.frame_kind == 1 && frame_recv.sq_no == frame_id) {
            printf("[+]Frame Received: %s\n", frame_recv.packet.data);

            // Prepare ACK Frame
            frame_send.sq_no = 0;
            frame_send.frame_kind = 0;
            frame_send.ack = frame_recv.sq_no + 1;

            // Send ACK
            sendto(sockfd, &frame_send, sizeof(Frame), 0, 
                   (struct sockaddr*)&newAddr, addr_size);
            printf("[+]ACK Sent\n");

            frame_id++;
        } else {
            printf("[-]Frame Not Received or Corrupted\n");
        }
    }

    close(sockfd);
    return 0;
}
