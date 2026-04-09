#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define PORT 8081
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024
#define TIMEOUT 2

int main() {

    int sock;
    struct sockaddr_in server_addr;
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    int seq = 1;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("Publisher QUIC iniciado...\n");

    while (1) {

        char input[BUFFER_SIZE];

        printf("Ingrese mensaje: ");
        fgets(input, sizeof(input), stdin);

        sprintf(message, "SEQ:%d|%s", seq, input);

        sendto(sock, message, strlen(message), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr));

        printf("Mensaje enviado con SEQ %d\n", seq);

        socklen_t addr_len = sizeof(server_addr);

        int n = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                         (struct sockaddr*)&server_addr, &addr_len);

        if (n < 0) {

            printf("ACK no recibido, retransmitiendo...\n");

            sendto(sock, message, strlen(message), 0,
                   (struct sockaddr*)&server_addr, sizeof(server_addr));

        } else {

            buffer[n] = '\0';

            if (strncmp(buffer, "ACK:", 4) == 0) {

                int ack;

                sscanf(buffer, "ACK:%d", &ack);

                if (ack == seq) {

                    printf("ACK recibido para SEQ %d\n", seq);

                    seq++;
                }
            }
        }
    }

    close(sock);

    return 0;
}

