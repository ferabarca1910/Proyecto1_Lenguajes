/*
 * servidor.cpp
 * Punto de entrada del servidor. Abre el socket, acepta conexiones
 * y lanza un hilo (CreateThread) por cada cliente que se conecta.
 *
 * Librerías del sistema usadas:
 *   winsock2.h    -> socket(), bind(), listen(), accept()
 *   windows.h     -> Se utiliza CreateThread() ya que no usamos fork() en Windows
 *   cstring       -> memset()
 */

#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
#include <cstring>
#include "Protocolo.h"
#include "Manejador.h"
#include "Datos.h"

#define PUERTO 8080
#define MAX_CONEXIONES 10

// Hilo que atiende a un cliente; recibe el SOCKET casteado a puntero
DWORD WINAPI hilo_cliente(LPVOID arg) {
    SOCKET clientSocket = (SOCKET)arg;
    atender_cliente(clientSocket);
    return 0;
}

// Hilo del menu interactivo del servidor
DWORD WINAPI hilo_menu(LPVOID arg) {
    (void)arg;
    menu_servidor();
    return 0;
}

int main() {
    // Inicializar Winsock (sockets para windows)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket, clientSocket;
    struct sockaddr_in direccion_servidor, direccion_cliente;
    int tam_cliente = sizeof(direccion_cliente);

    // Crear el socket TCP 
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error al crear el socket\n";
        return 1;
    }

    // Permite que el puerto no sea bloqueado si se reincia el servidor
    int opcion = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opcion, sizeof(opcion));

    // Configurar la dirección: escuchar en cualquier IP, en el puerto definido
    memset(&direccion_servidor, 0, sizeof(direccion_servidor));
    direccion_servidor.sin_family      = AF_INET;
    direccion_servidor.sin_addr.s_addr = INADDR_ANY;
    direccion_servidor.sin_port        = htons(PUERTO);

    // Asociar el socket al puerto
    if (bind(serverSocket, (struct sockaddr*)&direccion_servidor, sizeof(direccion_servidor)) == SOCKET_ERROR) {
        std::cerr << "Error en bind\n";
        return 1;
    }

    // Poner el socket en modo escucha 
    listen(serverSocket, MAX_CONEXIONES);

    std::cout << "=== Servidor del restaurante iniciado en el puerto " << PUERTO << " ===\n";

    // Lanzar el menú del servidor en un thread separado del thread principal
    // El thread principal queda en el bucle de accept
    CreateThread(NULL, 0, hilo_menu, NULL, 0, NULL);

    // Bucle que espera conexiones entrantes
    while (true) {
        clientSocket = accept(serverSocket,
                            (struct sockaddr*)&direccion_cliente,
                            &tam_cliente);
        if (clientSocket == INVALID_SOCKET) {
            // accept puede fallar transitoriamente; se continúa
            continue;
        }

        // Crear un thread para atender a este cliente
        CreateThread(NULL, 0, hilo_cliente, (LPVOID)clientSocket, 0, NULL);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
