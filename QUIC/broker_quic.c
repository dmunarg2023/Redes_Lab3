#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main() {

    int sockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    struct sockaddr_in clients[MAX_CLIENTS];
    int client_count = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Broker QUIC escuchando en puerto %d...\n", PORT);

    while (1) {

        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&client_addr, &addr_len);

        buffer[n] = '\0';

        printf("Mensaje recibido: %s\n", buffer);

        int exists = 0;

        for (int i = 0; i < client_count; i++) {
            if (clients[i].sin_addr.s_addr == client_addr.sin_addr.s_addr &&
                clients[i].sin_port == client_addr.sin_port) {
                exists = 1;
                break;
            }
        }

        if (!exists && client_count < MAX_CLIENTS) {
            clients[client_count++] = client_addr;
            printf("Nuevo cliente registrado\n");
        }

        for (int i = 0; i < client_count; i++) {

            sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&clients[i], sizeof(clients[i]));
        }
    }

    close(sockfd);

    return 0;
}
