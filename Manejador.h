/*
 * manejador.h
 * Solo declaramos las funciones que atienden las conexiones de clientes
 * y el menú interactivo del servidor.
 */

#ifndef MANEJADOR_H
#define MANEJADOR_H

 // Atiende a un cliente conectado: lee mensajes y responde según la operación
void atender_cliente(int fd_cliente);

// Menú interactivo que corre en el proceso hijo del servidor
void menu_servidor();

#endif

