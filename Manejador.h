/*
 * manejador.h
 * Solo declaramos las funciones que atienden las conexiones de clientes
 * y el menú interactivo del servidor.
 */

#ifndef MANEJADOR_H
#define MANEJADOR_H

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

 // Atiende a un cliente conectado: lee mensajes y responde según la operación
void atender_cliente(SOCKET fd_cliente);

// Menú interactivo que corre en el proceso hijo del servidor
void menu_servidor();

#endif
