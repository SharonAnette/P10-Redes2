 #include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib") // Vincula automáticamente la biblioteca WinSock

int udp_socket, lbind, tamRecivido, tamEnviado, tamTotal;
struct sockaddr_in servidor, cliente;
unsigned char mensajeRecivido[516], data[516];
FILE *fw, *fr;

// Declaración de funciones
void ManejarLectura();
void ManejarEscritura();
int EstructuraDatos(unsigned char *paq, int tam, unsigned short bloque);
int EstructuraACK(unsigned char *paq);
int EstructuraError(unsigned char *paq);

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "Error al iniciar WinSock: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == INVALID_SOCKET) {
        fprintf(stderr, "Error al abrir el socket: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(69);
    servidor.sin_addr.s_addr = htonl(INADDR_ANY);

    lbind = bind(udp_socket, (struct sockaddr *)&servidor, sizeof(servidor));
    if (lbind == SOCKET_ERROR) {
        fprintf(stderr, "Error en bind: %d\n", WSAGetLastError());
        closesocket(udp_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Servidor iniciado. Esperando conexiones...\n");

    while (1) {
        socklen_t clienteLen = sizeof(cliente);
        tamRecivido = recvfrom(udp_socket, (char *)mensajeRecivido, sizeof(mensajeRecivido), 0, (struct sockaddr *)&cliente, &clienteLen);
        if (tamRecivido == SOCKET_ERROR) {
            fprintf(stderr, "Error al recibir datos: %d\n", WSAGetLastError());
            continue;
        }

        if (mensajeRecivido[1] == 0x01) { // Opcode 1: Solicitud de lectura
            printf("Solicitud de lectura recibida.\n");
            ManejarLectura();
        } else if (mensajeRecivido[1] == 0x02) { // Opcode 2: Solicitud de escritura
            printf("Solicitud de escritura recibida.\n");
            ManejarEscritura();
        } else {
            printf("Código de operación no reconocido.\n");
        }
    }

    closesocket(udp_socket);
    WSACleanup();
    return 0;
}

// Manejo de solicitudes de lectura
void ManejarLectura() {
    fr = fopen((char *)(mensajeRecivido + 2), "rb");
    if (!fr) {
        printf("Archivo no encontrado: %s\n", mensajeRecivido + 2);
        int tamError = EstructuraError(data);
        sendto(udp_socket, (char *)data, tamError, 0, (struct sockaddr *)&cliente, sizeof(cliente));
        return;
    }

    int tamParcial;
    unsigned short bloque = 1;

    while (!feof(fr)) {
        tamParcial = fread(data + 4, 1, 512, fr);
        int tamData = EstructuraDatos(data, tamParcial, bloque);
        sendto(udp_socket, (char *)data, tamData, 0, (struct sockaddr *)&cliente, sizeof(cliente));
        bloque++;
    }

    fclose(fr);
    printf("Archivo enviado correctamente.\n");
}

// Manejo de solicitudes de escritura
void ManejarEscritura() {
    fw = fopen((char *)(mensajeRecivido + 2), "wb");
    if (!fw) {
        printf("Error al crear archivo: %s\n", mensajeRecivido + 2);
        int tamError = EstructuraError(data);
        sendto(udp_socket, (char *)data, tamError, 0, (struct sockaddr *)&cliente, sizeof(cliente));
        return;
    }

    while (1) {
        socklen_t clienteLen = sizeof(cliente);
        tamRecivido = recvfrom(udp_socket, (char *)mensajeRecivido, sizeof(mensajeRecivido), 0, (struct sockaddr *)&cliente, &clienteLen);
        if (tamRecivido < 4) break;

        fwrite(mensajeRecivido + 4, 1, tamRecivido - 4, fw);
        int tamACK = EstructuraACK(data);
        sendto(udp_socket, (char *)data, tamACK, 0, (struct sockaddr *)&cliente, sizeof(cliente));
    }

    fclose(fw);
    printf("Archivo recibido correctamente.\n");
}

// Estructura de datos para enviar
int EstructuraDatos(unsigned char *paq, int tam, unsigned short bloque) {
    unsigned char opLec[2] = {0x00, 0x03};
    unsigned char bloqueHex[2] = {bloque >> 8, bloque & 0xFF};
    memcpy(paq, opLec, 2);
    memcpy(paq + 2, bloqueHex, 2);
    return tam + 4;
}

// Estructura de un ACK
int EstructuraACK(unsigned char *paq) {
    unsigned char codOP[2] = {0x00, 0x04};
    unsigned char bloqueHex[2] = {mensajeRecivido[2], mensajeRecivido[3]};
    memcpy(paq, codOP, 2);
    memcpy(paq + 2, bloqueHex, 2);
    return 4;
}

// Estructura de un mensaje de error
int EstructuraError(unsigned char *paq) {
    unsigned char codOP[2] = {0x00, 0x05};
    unsigned char codError[2] = {0x00, 0x01};
    char mensaje[] = "Archivo no encontrado";
    memcpy(paq, codOP, 2);
    memcpy(paq + 2, codError, 2);
    memcpy(paq + 4, mensaje, strlen(mensaje) + 1);
    return strlen(mensaje) + 5;
}

