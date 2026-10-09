#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 20

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr, client_addr;
    struct sockaddr_in clients[MAX_CLIENTS];

    socklen_t addr_len;
    int client_count = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creando socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(clients, 0, sizeof(clients));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(sockfd);
        return 1;
    }

    printf("Broker QUIC escuchando en puerto %d...\n", PORT);

    while (1) {
        addr_len = sizeof(client_addr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&client_addr, &addr_len);

        if (n < 0) {
            perror("Error en recvfrom");
            continue;
        }

        buffer[n] = '\0';
        printf("Mensaje recibido de %s:%d -> %s\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port),
               buffer);

        int exists = 0;
        for (int i = 0; i < client_count; i++) {
            if (clients[i].sin_addr.s_addr == client_addr.sin_addr.s_addr &&
                clients[i].sin_port == client_addr.sin_port) {
                exists = 1;
                break;
            }
        }

        if (!exists && client_count < MAX_CLIENTS) {
            clients[client_count] = client_addr;
            client_count++;
            printf("Nuevo cliente registrado: %s:%d\n",
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));
        }

        for (int i = 0; i < client_count; i++) {
            if (clients[i].sin_addr.s_addr == client_addr.sin_addr.s_addr &&
                clients[i].sin_port == client_addr.sin_port) {
                continue;
            }

            if (sendto(sockfd, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&clients[i], sizeof(clients[i])) < 0) {
                perror("Error reenviando mensaje");
            }
        }
    }

    close(sockfd);
    return 0;
}



