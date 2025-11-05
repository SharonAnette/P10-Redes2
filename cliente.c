#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib") // Vincula automáticamente la biblioteca WinSock

short numPaq;
unsigned char numPaqHex[2];
int udp_socket, tamEnviado, tamTotal = 0, tamRecivido, tamData;
unsigned char data[516], estrPeticionLectura[516], estrACK[4], mensajeRecivido[516], nomArch[30];
struct sockaddr_in remota, cliente;
FILE *fw, *fr;

// Declaración de funciones
void EnviarACK();
int PeticionLectura();
int PeticionEscritura();
int EstructuraACK(unsigned char *paq);
int EstructuraPeticionLectura(unsigned char *paq, unsigned char *nomArch);
int EstructuraPeticionEscritura(unsigned char *paq, unsigned char *nomArch);

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

    remota.sin_family = AF_INET;
    remota.sin_port = htons(69);
    remota.sin_addr.s_addr = inet_addr("127.0.0.1");  // IP local del servidor

    int opcion = 0;
    while (opcion != 3) {
        printf("\n | 1-Solicitud Lectura | 2-Solicitud Escritura | 3-Salir |\n");
        printf("Seleccione una opcion: ");
        scanf("%i", &opcion);
        getchar();  // Consumir el salto de línea

        switch (opcion) {
            case 1:
                printf("Archivo a leer: ");
                fgets(nomArch, sizeof(nomArch), stdin);
                strtok(nomArch, "\n");
                if (PeticionLectura() == 3) {
                    printf("Archivo recibido correctamente.\n");
                } else {
                    printf("Error al recibir el archivo.\n");
                }
                break;

            case 2:
                printf("Archivo a escribir: ");
                fgets(nomArch, sizeof(nomArch), stdin);
                strtok(nomArch, "\n");
                if (PeticionEscritura() == 4) {
                    printf("Archivo enviado correctamente.\n");
                } else {
                    printf("Error al enviar el archivo.\n");
                }
                break;

            case 3:
                closesocket(udp_socket);
                printf("Cliente cerrado.\n");
                break;

            default:
                printf("Opción no válida.\n");
        }
    }

    WSACleanup();
    return 0;
}

// Función para enviar un ACK
void EnviarACK() {
    int tamACK = EstructuraACK(estrACK);
    sendto(udp_socket, (char *)estrACK, tamACK, 0, (struct sockaddr *)&cliente, sizeof(cliente));
}

// Función para manejar la petición de lectura
int PeticionLectura() {
    int tamData = EstructuraPeticionLectura(estrPeticionLectura, nomArch);
    sendto(udp_socket, (char *)estrPeticionLectura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));

    tamRecivido = recvfrom(udp_socket, (char *)mensajeRecivido, sizeof(mensajeRecivido), 0, NULL, NULL);
    if (mensajeRecivido[1] == 0x03) {
        fw = fopen(nomArch, "wb");
        fwrite(mensajeRecivido + 4, 1, tamRecivido - 4, fw);
        fclose(fw);
        return 3;
    }
    return -1;
}

// Función para manejar la petición de escritura
int PeticionEscritura() {
    fr = fopen(nomArch, "rb");
    if (!fr) {
        printf("No se pudo abrir el archivo.\n");
        return -1;
    }

    int tamData = EstructuraPeticionEscritura(estrPeticionLectura, nomArch);
    sendto(udp_socket, (char *)estrPeticionLectura, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));

    while (!feof(fr)) {
        tamData = fread(data, 1, 512, fr);
        sendto(udp_socket, (char *)data, tamData, 0, (struct sockaddr *)&remota, sizeof(remota));
    }

    fclose(fr);
    return 4;
}

// Función para estructurar un ACK
int EstructuraACK(unsigned char *paq) {
    unsigned char codOP[2] = {0x00, 0x04};
    memcpy(paq, codOP, 2);
    return 4;
}

// Función para estructurar una petición de lectura
int EstructuraPeticionLectura(unsigned char *paq, unsigned char *nomArch) {
    unsigned char opLec[2] = {0x00, 0x01};
    unsigned char modo[] = "octet";
    memcpy(paq, opLec, 2);
    memcpy(paq + 2, nomArch, strlen(nomArch));
    memcpy(paq + strlen(nomArch) + 3, modo, strlen(modo) + 1);
    return strlen(nomArch) + 3 + strlen(modo) + 1;
}

// Función para estructurar una petición de escritura
int EstructuraPeticionEscritura(unsigned char *paq, unsigned char *nomArch) {
    unsigned char opEscritura[2] = {0x00, 0x02};
    unsigned char modo[] = "octet";
    memcpy(paq, opEscritura, 2);
    memcpy(paq + 2, nomArch, strlen(nomArch));
    memcpy(paq + strlen(nomArch) + 3, modo, strlen(modo) + 1);
    return strlen(nomArch) + 3 + strlen(modo) + 1;
}



