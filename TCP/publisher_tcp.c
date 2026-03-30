#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // 1. Crear socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }

    // 2. Configurar dirección
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // acepta cualquier IP
    address.sin_port = htons(PORT);

    // 3. Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    // 4. Escuchar
    if (listen(server_fd, 5) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }

    printf("Broker escuchando en el puerto %d...\n", PORT);

    // 5. Loop para aceptar clientes
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Error en accept");
            continue;
        }

        printf("Cliente conectado!\n");

        // 6. Recibir mensajes
        int valread;
        while ((valread = read(new_socket, buffer, BUFFER_SIZE)) > 0) {
            buffer[valread] = '\0';
            printf("Mensaje recibido: %s\n", buffer);
        }

        printf("Cliente desconectado\n");
        close(new_socket);
    }

    return 0;
}