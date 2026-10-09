#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#define PORT 8081
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024
#define TIMEOUT 2
#define MAX_RETRIES 5

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    int seq = 1;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Error creando socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Error con la IP del servidor");
        close(sock);
        return 1;
    }

    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error configurando timeout");
        close(sock);
        return 1;
    }

    printf("Publisher QUIC iniciado...\n");

    while (1) {
        printf("Ingrese mensaje: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Fin de entrada.\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            printf("No puede enviar un mensaje vacío.\n");
            continue;
        }

        snprintf(message, BUFFER_SIZE, "SEQ:%d|%.1000s", seq, input);

        int ack_recibido = 0;
        int intentos = 0;

        while (!ack_recibido && intentos < MAX_RETRIES) {
            if (sendto(sock, message, strlen(message), 0,
                       (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Error enviando mensaje");
                break;
            }

            printf("Mensaje enviado con SEQ %d: %s\n", seq, input);

            socklen_t addr_len = sizeof(server_addr);
            int n = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr *)&server_addr, &addr_len);

            if (n < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    intentos++;
                    printf("ACK no recibido para SEQ %d. Retransmitiendo... (intento %d)\n",
                           seq, intentos);
                    continue;
                } else {
                    perror("Error recibiendo ACK");
                    break;
                }
            }

            buffer[n] = '\0';

            if (strncmp(buffer, "ACK:", 4) == 0) {
                int ack;
                if (sscanf(buffer, "ACK:%d", &ack) == 1) {
                    if (ack == seq) {
                        printf("ACK recibido correctamente para SEQ %d\n", seq);
                        ack_recibido = 1;
                        seq++;
                    } else {
                        printf("ACK recibido (%d), pero no corresponde al SEQ actual (%d)\n",
                               ack, seq);
                    }
                }
            } else {
                printf("Mensaje inesperado recibido: %s\n", buffer);
            }
        }

        if (!ack_recibido) {
            printf("No se recibió ACK para SEQ %d después de %d intentos.\n",
                   seq, MAX_RETRIES);
        }
    }

    close(sock);
    return 0;
}