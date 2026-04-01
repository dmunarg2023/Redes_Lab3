#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8081
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {

    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("Subscriber QUIC iniciado...\n");

    char *register_msg = "subscriber conectado";

    sendto(sock, register_msg, strlen(register_msg), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));

    while (1) {

        socklen_t addr_len = sizeof(server_addr);

        int bytes = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                             (struct sockaddr*)&server_addr, &addr_len);

        buffer[bytes] = '\0';

        printf("Actualizacion recibida: %s\n", buffer);
    }

    close(sock);

    return 0;
}