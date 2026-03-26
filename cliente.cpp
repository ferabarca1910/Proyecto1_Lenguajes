/*
 * cliente.cpp
 * Se conecta al servidor y permite registrar o modificar órdenes.
 *
 * Librerías del sistema usadas:
 *   cstring       -> memset(), strncpy()
 *   cstdio        -> printf(), snprintf()
 */

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <cstring>
#include <cstdio>
#include "Protocolo.h"

#define PUERTO     8080
#define IP_SERVIDOR "127.0.0.1"

// Envía un Mensaje al servidor y espera la Respuesta
static int comunicacion(SOCKET socketCliente, Mensaje& msg, Respuesta& resp) {
    if (send(socketCliente, (const char*)&msg, sizeof(msg), 0) == SOCKET_ERROR) {
        std::cerr << "Error al enviar mensaje al servidor\n";
        return 0;
    }
    if (recv(socketCliente, (char*)&resp, sizeof(resp), 0) == SOCKET_ERROR) {
        std::cerr << "Error al recibir respuesta del servidor\n";
        return 0;
    }
    return 1;
}

// Permite ver en pantalla todas las ordenes registradas
static void ver_ordenes(SOCKET socketCliente) {
    Mensaje msg;
    memset(&msg, 0, sizeof(msg));
    msg.operacion = OP_LISTAR_ORDENES;
    send(socketCliente, (const char*)&msg, sizeof(msg), 0);

    std::cout << "\n  ID  | Mesa | Producto               | Cant | Estado\n";
    std::cout << "  ----|------|------------------------|------|----------\n";

    // Recibir nodos uno a uno
    Mensaje linea;
    while (true) {
        recv(socketCliente, (char*)&linea, sizeof(linea), 0);
        if (linea.operacion == -1) break;
        printf("  %-3d | %-4d | %-22s | %-4d | %s\n",
               linea.id,
               linea.numero_mesa,
               linea.producto,
               linea.cantidad,
               linea.estado == ESTADO_PENDIENTE ? "Pendiente" : "Completa");
    }
}

// Permite ver en pantalla todos los productos 
static void ver_productos(SOCKET socketCliente) {
    Mensaje msg;
    memset(&msg, 0, sizeof(msg));
    msg.operacion = OP_LISTAR_PRODUCTOS;
    send(socketCliente, (const char*)&msg, sizeof(msg), 0);

    std::cout << "\n  ID  | Producto                      | Precio\n";
    std::cout << "  ----|-------------------------------|--------\n";

    Mensaje linea;
    while (true) {
        recv(socketCliente, (char*)&linea, sizeof(linea), 0);
        if (linea.operacion == -1) break;
        printf("  %-3d | %-29s | %.2f\n",
               linea.id, linea.producto, linea.precio);
    }
}

int main() {
    SOCKET socketCliente;
    struct sockaddr_in direccion_servidor;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Crear el socket TCP
    socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socketCliente < 0) {
        std::cerr << "Error al crear socket\n";
        return 1;
    }

    // Preparar la dirección del servidor al que se va a conectar
    memset(&direccion_servidor, 0, sizeof(direccion_servidor));
    direccion_servidor.sin_family = AF_INET;
    direccion_servidor.sin_port   = htons(PUERTO);

    // Convertir la IP en texto a formato binario de red
    if (inet_pton(AF_INET, IP_SERVIDOR, &direccion_servidor.sin_addr) <= 0) {
        std::cerr << "Dirección IP inválida\n";
        return 1;
    }

    // Intentar conectar con el servidor
    if (connect(socketCliente, (struct sockaddr*)&direccion_servidor,
                sizeof(direccion_servidor)) < 0) {
        std::cerr << "No se pudo conectar al servidor.\n";
        return 1;
    }

    std::cout << "=== Cliente Mesero - Conectado al servidor ===\n";

    int opcion;
    while (true) {
        std::cout << "\n--- MENU MESERO ---\n";
        std::cout << "  1. Registrar nueva orden\n";
        std::cout << "  2. Modificar orden existente\n";
        std::cout << "  3. Ver todas las ordenes\n";
        std::cout << "  4. Ver menu del restaurante\n";
        std::cout << "  0. Salir\n";
        std::cout << "Opcion: ";
        std::cin >> opcion;

        if (opcion == 0) break;

        Mensaje msg;
        Respuesta resp;
        memset(&msg, 0, sizeof(msg));
        memset(&resp, 0, sizeof(resp));

        if (opcion == 1) {
            // Registrar una nueva orden
            msg.operacion = OP_REGISTRAR_ORDEN;
            std::cout << "Numero de mesa: ";
            std::cin >> msg.numero_mesa;
            std::cout << "Producto: ";
            std::cin >> msg.producto;
            std::cout << "Cantidad: ";
            std::cin >> msg.cantidad;

            if (comunicacion(socketCliente, msg, resp))
                std::cout << (resp.exito ? "[OK] " : "[Error] ") << resp.texto << "\n";
        }

        else if (opcion == 2) {
            // Modificar una orden; primero mostrar la lista para que el mesero elija
            ver_ordenes(socketCliente);
            memset(&msg, 0, sizeof(msg));

            msg.operacion = OP_MODIFICAR_ORDEN;
            std::cout << "\nID de la orden a modificar: ";
            std::cin >> msg.id;
            std::cout << "Nuevo numero de mesa: ";
            std::cin >> msg.numero_mesa;
            std::cout << "Nuevo producto: ";
            std::cin >> msg.producto;
            std::cout << "Nueva cantidad: ";
            std::cin >> msg.cantidad;

            if (comunicacion(socketCliente, msg, resp))
                std::cout << (resp.exito ? "[OK] " : "[Error] ") << resp.texto << "\n";
        }

        else if (opcion == 3) {
            ver_ordenes(socketCliente);
        }

        else if (opcion == 4) {
            ver_productos(socketCliente);
        }
    }

    // Notificar al servidor que se va a cerrar la conexión
    Mensaje salida;
    memset(&salida, 0, sizeof(salida));
    salida.operacion = OP_SALIR;
    send(socketCliente, (const char*)&salida, sizeof(salida), 0);

    closesocket(socketCliente);
    WSACleanup();
    std::cout << "Sesion cerrada.\n";
    return 0;
}
