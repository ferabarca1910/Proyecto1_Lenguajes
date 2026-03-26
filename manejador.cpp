/*
 * manejador.cpp
 * Implementamos  logic del servidor:
 *   - atender_cliente(): proceso hijo que lee mensajes de un socket y hace la accion
 *   - menu_servidor():   menu interactivo para administrar el restaurante
 *
 * Librerías:
 *   cstring     -> memset(), strncpy()
 *   cstdio      -> snprintf()
 */

#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <cstring>
#include <cstdio>
#include "Protocolo.h"
#include "Datos.h"
#include "Manejador.h"

 // Envía bytes al socket hasta completar el tamanno requerido
static int enviar(SOCKET fd, const void* buffer, int tam) {
    return send(fd, (const char*)buffer, tam, 0);
}

// Lee bytes del socket hasta completar el tamano requerido
static int recibir(SOCKET fd, void* buffer, int tam) {
    return recv(fd, (char*)buffer, tam, 0);
}

/*
 * atender_cliente()
 * Corre dentro del proceso hijo creado por un  fork().
 * Lee mensajes del cliente en un bucle y ejecuta la operación solicitada.
 * Cuando el cliente manda OP_SALIR quiere cerra la conexion, terminamos proceso.
 */
void atender_cliente(SOCKET fd_cliente) {
    Mensaje msg;
    Respuesta resp;

    while (true) {
        memset(&msg, 0, sizeof(msg));
        memset(&resp, 0, sizeof(resp));

        // Lee el mensaje c del socket
        int bytes = recibir(fd_cliente, &msg, sizeof(msg));
        if (bytes <= 0) {
            // El cliente se fue
            break;
        }

        if (msg.operacion == OP_SALIR) break;

        // ---------- Registrar una nueva orden ------------------------
        if (msg.operacion == OP_REGISTRAR_ORDEN) {
            int num_mesas = cargar_mesas();

            // Validar la mesa existe 
            if (num_mesas > 0 && (msg.numero_mesa < 1 || msg.numero_mesa > num_mesas)) {
                resp.exito = 0;
                snprintf(resp.texto, sizeof(resp.texto),
                    "Mesa %d no existe. El restaurante tiene %d mesas.",
                    msg.numero_mesa, num_mesas);
                enviar(fd_cliente, &resp, sizeof(resp));
                continue;
            }

            //-------- Cargar lista actual, agregar la orden y guardar----------------
            NodoOrden* ordenes = cargar_ordenes();
            int nuevo_id = siguiente_id_orden(ordenes);
            ordenes = agregar_orden(ordenes, nuevo_id,
                msg.numero_mesa, msg.producto, msg.cantidad);
            guardar_ordenes(ordenes);
            liberar_ordenes(ordenes);

            resp.exito = 1;
            snprintf(resp.texto, sizeof(resp.texto),
                "Su orden #%d fue registrada .", nuevo_id);
            enviar(fd_cliente, &resp, sizeof(resp));
        }

        // ------------- Modificar  orden existente ----------------
        else if (msg.operacion == OP_MODIFICAR_ORDEN) {
            NodoOrden* ordenes = cargar_ordenes();
            NodoOrden* actual = ordenes;
            int encontrado = 0;

            // Recorrer la lista buscando el id de la orden a modificar
            while (actual != nullptr) {
                if (actual->id == msg.id) {
                    actual->numero_mesa = msg.numero_mesa;
                    strncpy(actual->producto, msg.producto, TAM_PRODUCTO - 1);
                    actual->cantidad = msg.cantidad;
                    encontrado = 1;
                    break;
                }
                actual = actual->siguiente;
            }

            if (encontrado) {
                guardar_ordenes(ordenes);
                resp.exito = 1;
                snprintf(resp.texto, sizeof(resp.texto),
                    "Orden #%d modificada.", msg.id);
            }
            else {
                resp.exito = 0;
                snprintf(resp.texto, sizeof(resp.texto),
                    "No existe la orden #%d.", msg.id);
            }
            liberar_ordenes(ordenes);
            enviar(fd_cliente, &resp, sizeof(resp));
        }

        // --- ------Listar todas las ordenes -----------------
        else if (msg.operacion == OP_LISTAR_ORDENES) {
            NodoOrden* ordenes = cargar_ordenes();
            NodoOrden* actual = ordenes;

            // Enviamos cada orden como un Mensaje de respuesta
            // El cliente sabe que terminó cuando recibe exito=0 con  un texto vacío
            while (actual != nullptr) {
                Mensaje linea;
                memset(&linea, 0, sizeof(linea));
                linea.id = actual->id;
                linea.numero_mesa = actual->numero_mesa;
                strncpy(linea.producto, actual->producto, TAM_PRODUCTO - 1);
                linea.cantidad = actual->cantidad;
                linea.estado = actual->estado;
                enviar(fd_cliente, &linea, sizeof(linea));
                actual = actual->siguiente;
            }

            // Marcador de fin de lista
            Mensaje fin;
            memset(&fin, 0, sizeof(fin));
            fin.operacion = -1;
            enviar(fd_cliente, &fin, sizeof(fin));
            liberar_ordenes(ordenes);
        }

        // --- Marcar una orden como completada ---
        else if (msg.operacion == OP_COMPLETAR_ORDEN) {
            NodoOrden* ordenes = cargar_ordenes();
            NodoOrden* actual = ordenes;
            int encontrado = 0;

            while (actual != nullptr) {
                if (actual->id == msg.id) {
                    actual->estado = ESTADO_COMPLETA;
                    encontrado = 1;
                    break;
                }
                actual = actual->siguiente;
            }

            if (encontrado) {
                guardar_ordenes(ordenes);
                resp.exito = 1;
                snprintf(resp.texto, sizeof(resp.texto),
                    "Orden #%d marcada como completada.", msg.id);
            }
            else {
                resp.exito = 0;
                snprintf(resp.texto, sizeof(resp.texto),
                    "No se encontró la orden #%d.", msg.id);
            }
            liberar_ordenes(ordenes);
            enviar(fd_cliente, &resp, sizeof(resp));
        }

        // ------- Crear  nuevo producto en el menú -------------------
        else if (msg.operacion == OP_CREAR_PRODUCTO) {
            NodoProducto* productos = cargar_productos();
            int nuevo_id = siguiente_id_producto(productos);
            productos = agregar_producto(productos, nuevo_id,
                msg.producto, msg.precio);
            guardar_productos(productos);
            liberar_productos(productos);

            resp.exito = 1;
            snprintf(resp.texto, sizeof(resp.texto),
                "Producto '%s' creado con ID %d.", msg.producto, nuevo_id);
            enviar(fd_cliente, &resp, sizeof(resp));
        }

        // ------- Listar productos del menú -----------
        else if (msg.operacion == OP_LISTAR_PRODUCTOS) {
            NodoProducto* productos = cargar_productos();
            NodoProducto* actual = productos;

            while (actual != nullptr) {
                Mensaje linea;
                memset(&linea, 0, sizeof(linea));
                linea.id = actual->id;
                strncpy(linea.producto, actual->nombre, TAM_PRODUCTO - 1);
                linea.precio = actual->precio;
                enviar(fd_cliente, &linea, sizeof(linea));
                actual = actual->siguiente;
            }

            // Marcador de fin
            Mensaje fin;
            memset(&fin, 0, sizeof(fin));
            fin.operacion = -1;
            enviar(fd_cliente, &fin, sizeof(fin));
            liberar_productos(productos);
        }

        // -------- Modificar  producto  ---------
        else if (msg.operacion == OP_MODIFICAR_PRODUCTO) {
            NodoProducto* productos = cargar_productos();
            NodoProducto* actual = productos;
            int encontrado = 0;

            while (actual != nullptr) {
                if (actual->id == msg.id) {
                    strncpy(actual->nombre, msg.producto, TAM_PRODUCTO - 1);
                    actual->precio = msg.precio;
                    encontrado = 1;
                    break;
                }
                actual = actual->siguiente;
            }

            if (encontrado) {
                guardar_productos(productos);
                resp.exito = 1;
                snprintf(resp.texto, sizeof(resp.texto),
                    "Producto #%d actualizado.", msg.id);
            }
            else {
                resp.exito = 0;
                snprintf(resp.texto, sizeof(resp.texto),
                    "No se encontró el producto #%d.", msg.id);
            }
            liberar_productos(productos);
            enviar(fd_cliente, &resp, sizeof(resp));
        }

        // -------Eliminar un producto del menú ---------
        else if (msg.operacion == OP_ELIMINAR_PRODUCTO) {
            NodoProducto* productos = cargar_productos();
            NodoProducto* actual = productos;
            NodoProducto* anterior = nullptr;
            int encontrado = 0;

            // Recorre la lista buscando el nodo a eliminar
            while (actual != nullptr) {
                if (actual->id == msg.id) {
                    // Si tiene nodo anterior, saltar el nodo actual
                    if (anterior != nullptr)
                        anterior->siguiente = actual->siguiente;
                    else
                        productos = actual->siguiente; // era el primero
                    delete actual;
                    encontrado = 1;
                    break;
                }
                anterior = actual;
                actual = actual->siguiente;
            }

            if (encontrado) {
                guardar_productos(productos);
                resp.exito = 1;
                snprintf(resp.texto, sizeof(resp.texto),
                    "Producto #%d eliminado.", msg.id);
            }
            else {
                resp.exito = 0;
                snprintf(resp.texto, sizeof(resp.texto),
                    "No se encontró el producto #%d.", msg.id);
            }
            liberar_productos(productos);
            enviar(fd_cliente, &resp, sizeof(resp));
        }

        // --- Configurar el número de mesas ---
        else if (msg.operacion == OP_CONFIG_MESAS) {
            guardar_mesas(msg.num_mesas);
            resp.exito = 1;
            snprintf(resp.texto, sizeof(resp.texto),
                "Número de mesas configurado: %d.", msg.num_mesas);
            enviar(fd_cliente, &resp, sizeof(resp));
        }
    }
}


//----------------------- Menú interactivo del servidor--------------------

// Muestra en pantalla todas las órdenes almacenadas
static void mostrar_ordenes() {
    NodoOrden* ordenes = cargar_ordenes();
    NodoOrden* actual = ordenes;

    if (actual == nullptr) {
        std::cout << "  (No hay órdenes registradas)\n";
        liberar_ordenes(ordenes);
        return;
    }

    std::cout << "\n  ID  | Mesa | Producto               | Cant | Estado\n";
    std::cout << "  ----|------|------------------------|------|----------\n";
    while (actual != nullptr) {
        printf("  %-3d | %-4d | %-22s | %-4d | %s\n",
            actual->id,
            actual->numero_mesa,
            actual->producto,
            actual->cantidad,
            actual->estado == ESTADO_PENDIENTE ? "Pendiente" : "Completa");
        actual = actual->siguiente;
    }
    liberar_ordenes(ordenes);
}

// Muestra solo las órdenes pendientes
static void mostrar_pendientes() {
    NodoOrden* ordenes = cargar_ordenes();
    NodoOrden* actual = ordenes;
    int hay_pendientes = 0;

    std::cout << "\n  ID  | Mesa | Producto               | Cant\n";
    std::cout << "  ----|------|------------------------|-----\n";
    while (actual != nullptr) {
        if (actual->estado == ESTADO_PENDIENTE) {
            printf("  %-3d | %-4d | %-22s | %d\n",
                actual->id,
                actual->numero_mesa,
                actual->producto,
                actual->cantidad);
            hay_pendientes = 1;
        }
        actual = actual->siguiente;
    }
    if (!hay_pendientes)
        std::cout << "  (No hay ordenes pendientes)\n";
    liberar_ordenes(ordenes);
}

// Muestra todos los productos del menú
static void mostrar_productos() {
    NodoProducto* productos = cargar_productos();
    NodoProducto* actual = productos;

    if (actual == nullptr) {
        std::cout << "  (No hay productos en el menú)\n";
        liberar_productos(productos);
        return;
    }

    std::cout << "\n  ID  | Nombre                        | Precio\n";
    std::cout << "  ----|-------------------------------|--------\n";
    while (actual != nullptr) {
        printf("  %-3d | %-29s | %.2f\n",
            actual->id, actual->nombre, actual->precio);
        actual = actual->siguiente;
    }
    liberar_productos(productos);
}

/*
 * menu_servidor()
 * Corre en el proceso hijo lanzado desde main().
 * Permite al administrador gestionar el restaurante sin bloquear
 * las conexiones de los clientes (éstas corren en otros procesos).
 */
void menu_servidor() {
    int opcion;

    while (true) {
        std::cout << "\n------------ SERVIDOR RESTAURANTE --------------\n";
        std::cout << "  1. Ver todas las ordenes\n";
        std::cout << "  2. Ver ordenes pendientes\n";
        std::cout << "  3. Marcar orden como completada\n";
        std::cout << "  4. Ver menu (productos)\n";
        std::cout << "  5. Agregar producto al menu\n";
        std::cout << "  6. Modificar producto del menu\n";
        std::cout << "  7. Eliminar producto del menu\n";
        std::cout << "  8. Configurar numero de mesas\n";
        std::cout << "  0. Salir\n";
        std::cout << "Opcion: ";
        std::cin >> opcion;

        if (opcion == 0) break;

        if (opcion == 1) {
            mostrar_ordenes();
        }

        else if (opcion == 2) {
            mostrar_pendientes();
        }

        else if (opcion == 3) {
            mostrar_pendientes();
            int id;
            std::cout << "\nIngrese el ID de la orden a completar: ";
            std::cin >> id;

            NodoOrden* ordenes = cargar_ordenes();
            NodoOrden* actual = ordenes;
            int encontrado = 0;
            while (actual != nullptr) {
                if (actual->id == id) {
                    actual->estado = ESTADO_COMPLETA;
                    encontrado = 1;
                    break;
                }
                actual = actual->siguiente;
            }
            if (encontrado) {
                guardar_ordenes(ordenes);
                std::cout << "Orden #" << id << " marcada como completada.\n";
            }
            else {
                std::cout << "No se encontro la orden #" << id << ".\n";
            }
            liberar_ordenes(ordenes);
        }

        else if (opcion == 4) {
            mostrar_productos();
        }

        else if (opcion == 5) {
            char nombre[TAM_PRODUCTO];
            float precio;
            std::cout << "Nombre del producto: ";
            std::cin >> nombre;
            std::cout << "Precio: ";
            std::cin >> precio;

            NodoProducto* productos = cargar_productos();
            int nuevo_id = siguiente_id_producto(productos);
            productos = agregar_producto(productos, nuevo_id, nombre, precio);
            guardar_productos(productos);
            liberar_productos(productos);
            std::cout << "Producto '" << nombre << "' agregado con ID " << nuevo_id << ".\n";
        }

        else if (opcion == 6) {
            mostrar_productos();
            int id;
            char nombre[TAM_PRODUCTO];
            float precio;
            std::cout << "\nID del producto a modificar: ";
            std::cin >> id;
            std::cout << "Nuevo nombre: ";
            std::cin >> nombre;
            std::cout << "Nuevo precio: ";
            std::cin >> precio;

            NodoProducto* productos = cargar_productos();
            NodoProducto* actual = productos;
            int encontrado = 0;
            while (actual != nullptr) {
                if (actual->id == id) {
                    strncpy(actual->nombre, nombre, TAM_PRODUCTO - 1);
                    actual->precio = precio;
                    encontrado = 1;
                    break;
                }
                actual = actual->siguiente;
            }
            if (encontrado) {
                guardar_productos(productos);
                std::cout << "Producto #" << id << " actualizado.\n";
            }
            else {
                std::cout << "No se encontro el producto #" << id << ".\n";
            }
            liberar_productos(productos);
        }

        else if (opcion == 7) {
            mostrar_productos();
            int id;
            std::cout << "\nID del producto a eliminar: ";
            std::cin >> id;

            NodoProducto* productos = cargar_productos();
            NodoProducto* actual = productos;
            NodoProducto* anterior = nullptr;
            int encontrado = 0;
            while (actual != nullptr) {
                if (actual->id == id) {
                    if (anterior != nullptr)
                        anterior->siguiente = actual->siguiente;
                    else
                        productos = actual->siguiente;
                    delete actual;
                    encontrado = 1;
                    break;
                }
                anterior = actual;
                actual = actual->siguiente;
            }
            if (encontrado) {
                guardar_productos(productos);
                std::cout << "Producto #" << id << " eliminado.\n";
            }
            else {
                std::cout << "No existe el producto #" << id << ".\n";
            }
            liberar_productos(productos);
        }

        else if (opcion == 8) {
            int n;
            std::cout << "Numero de mesas del restaurante: ";
            std::cin >> n;
            guardar_mesas(n);
            std::cout << "Configurado: " << n << " mesas.\n";
        }
    }
}
