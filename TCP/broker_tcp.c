#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    int client_sockets[MAX_CLIENTS] = {0};
    int client_count = 0;

    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }

    printf("Broker TCP escuchando en el puerto %d...\n", PORT);

    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;
        for (int i = 0; i < client_count; i++) {
            FD_SET(client_sockets[i], &readfds);
            if (client_sockets[i] > max_sd)
                max_sd = client_sockets[i];
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Error en select");
        }

        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (new_socket < 0) {
                perror("Error en accept");
                exit(EXIT_FAILURE);
            }

            printf("Nuevo cliente conectado\n");

            if (client_count < MAX_CLIENTS) {
                client_sockets[client_count++] = new_socket;
            } else {
                printf("Máximo de clientes alcanzado\n");
                close(new_socket);
            }
        }

        for (int i = 0; i < client_count; i++) {
            int sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE);

                if (valread == 0) {
                    printf("Cliente desconectado\n");
                    close(sd);

                    for (int j = i; j < client_count - 1; j++) {
                        client_sockets[j] = client_sockets[j + 1];
                    }
                    client_count--;
                    i--;
                } else {
                    buffer[valread] = '\0';
                    printf("Mensaje recibido: %s", buffer);

                    for (int j = 0; j < client_count; j++) {
                        if (client_sockets[j] != sd) {
                            send(client_sockets[j], buffer, strlen(buffer), 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}