#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8081
#define SERVER_IP "127.0.0.1"

int main() {

    int sock;
    struct sockaddr_in server_addr;
    char message[1024];

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("Publisher QUIC iniciado...\n");

    while (1) {

        printf("Ingrese mensaje: ");
        fgets(message, sizeof(message), stdin);

        sendto(sock, message, strlen(message), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr));
    }

    close(sock);

    return 0;
}