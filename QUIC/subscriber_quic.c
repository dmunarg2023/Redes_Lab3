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
    if (sock < 0) {
        perror("Error creando socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Error con la IP del broker");
        close(sock);
        return 1;
    }

    printf("Subscriber QUIC iniciado...\n");

    char *register_msg = "subscriber conectado";
    if (sendto(sock, register_msg, strlen(register_msg), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error registrando subscriber");
        close(sock);
        return 1;
    }

    while (1) {
        socklen_t addr_len = sizeof(server_addr);
        int n = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&server_addr, &addr_len);

        if (n < 0) {
            perror("Error recibiendo mensaje");
            continue;
        }

        buffer[n] = '\0';
        printf("Mensaje recibido: %s\n", buffer);

        int seq;
        char contenido[BUFFER_SIZE];

        if (sscanf(buffer, "SEQ:%d|%[^\n]", &seq, contenido) == 2) {
            printf("Contenido: %s\n", contenido);

            char ack[50];
            snprintf(ack, sizeof(ack), "ACK:%d", seq);

            if (sendto(sock, ack, strlen(ack), 0,
                       (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Error enviando ACK");
            } else {
                printf("ACK enviado: %d\n", seq);
            }
        }
    }

    close(sock);
    return 0;
}








