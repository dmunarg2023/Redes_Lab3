#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "172.24.36.51"
#define BUFFER_SIZE 1024

int main() {

    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creando socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error conectando al broker");
        return 1;
    }
    printf("Subscriber TCP conectado al broker...\n");

    while (1) {

        int bytes = recv(sock, buffer, BUFFER_SIZE, 0);

        if (bytes <= 0) {
            printf("Conexion cerrada\n");
            break;
        }

        buffer[bytes] = '\0';
        printf("Actualizacion recibida: %s", buffer);
    }

    close(sock);

    return 0;
}