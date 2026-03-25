/*
 * datos.h
 * Contiene las estructuras de datos del restaurante y toda la lógica
 * de operaciones sobre ellas: listas enlazadas, persistencia en archivo,
 * y el CRUD completo de órdenes y productos.
 *
 * Librerías usadas:
 *   fstream   -> ifstream, ofstream para leer y escribir archivos
 *   cstring   -> strncpy, memset
 *   cstdio    -> printf para mostrar tablas formateadas
 *   iostream  -> std::cout
 */

#ifndef DATOS_H
#define DATOS_H

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>
#include "../comun/protocolo.h"

#define ARCHIVO_ORDENES   "ordenes.dat"
#define ARCHIVO_PRODUCTOS "productos.dat"
#define ARCHIVO_MESAS     "mesas.dat"

 // ---------Listas enlazadas----------------------

struct NodoOrden {
    int id;
    int numero_mesa;
    char producto[TAM_PRODUCTO];
    int cantidad;
    int estado;          
    NodoOrden* siguiente;
};

// Nodo que presenta producto

struct NodoProducto {
    int id;
    char nombre[TAM_PRODUCTO];
    float precio;
    NodoProducto* siguiente;
};

// ----------Utilidades de los IDS---------------------

// Recorre la lista y devuelve el mayor ID encontrado + 1
inline int siguiente_id_orden(NodoOrden* cabeza) {
    int max_id = 0;
    NodoOrden* actual = cabeza;
    while (actual != nullptr) {
        if (actual->id > max_id) max_id = actual->id;
        actual = actual->siguiente;
    }
    return max_id + 1;
}

inline int siguiente_id_producto(NodoProducto* cabeza) {
    int max_id = 0;
    NodoProducto* actual = cabeza;
    while (actual != nullptr) {
        if (actual->id > max_id) max_id = actual->id;
        actual = actual->siguiente;
    }
    return max_id + 1;
}

// Guardar y cargar desde archiv d tecto
inline void guardar_ordenes(NodoOrden* cabeza){
    std::ofstream archivo(ARCHIVO_ORDENES);
    NodoOrden* actual = cabeza;
    while (actual != nullptr) {
        archivo << actual->id << " "
            << actual->numero_mesa << " "
            << actual->producto << " "
            << actual->cantidad << " "
            << actual->estado << "\n";
        actual = actual->siguiente;
    }
}

// Lee el archivo de órdenes y reconstruye la lista enlazada
inline NodoOrden* cargar_ordenes() {
    NodoOrden* cabeza = nullptr;
    std::ifstream archivo(ARCHIVO_ORDENES);
    if (!archivo.is_open()) return nullptr;

    int id, mesa, cantidad, estado;
    char producto[TAM_PRODUCTO];
    while (archivo >> id >> mesa >> producto >> cantidad >> estado) {
        NodoOrden* nuevo = new NodoOrden();
        nuevo->id = id;
        nuevo->numero_mesa = mesa;
        strncpy(nuevo->producto, producto, TAM_PRODUCTO - 1);
        nuevo->producto[TAM_PRODUCTO - 1] = '\0';
        nuevo->cantidad = cantidad;
        nuevo->estado = estado;
        nuevo->siguiente = nullptr;

        // Insertar al final de la lista
        if (cabeza == nullptr) {
            cabeza = nuevo;
        }
        else {
            NodoOrden* actual = cabeza;
            while (actual->siguiente != nullptr)
                actual = actual->siguiente;
            actual->siguiente = nuevo;
        }
    }
    return cabeza;
}

// Libera todos los nodos de la lista de órdenes
inline void liberar_ordenes(NodoOrden* cabeza) {
    while (cabeza != nullptr) {
        NodoOrden* temp = cabeza;
        cabeza = cabeza->siguiente;
        delete temp;
    }
}

// Guarda todos los productos del menú en disco
inline void guardar_productos(NodoProducto* cabeza) {
    std::ofstream archivo(ARCHIVO_PRODUCTOS);
    NodoProducto* actual = cabeza;
    while (actual != nullptr) {
        archivo << actual->id << " "
            << actual->nombre << " "
            << actual->precio << "\n";
        actual = actual->siguiente;
    }
}

// Lee el archivo de productos y reconstruye la lista enlazada
inline NodoProducto* cargar_productos() {
    NodoProducto* cabeza = nullptr;
    std::ifstream archivo(ARCHIVO_PRODUCTOS);
    if (!archivo.is_open()) return nullptr;

    int id;
    char nombre[TAM_PRODUCTO];
    float precio;
    while (archivo >> id >> nombre >> precio) {
        NodoProducto* nuevo = new NodoProducto();
        nuevo->id = id;
        strncpy(nuevo->nombre, nombre, TAM_PRODUCTO - 1);
        nuevo->nombre[TAM_PRODUCTO - 1] = '\0';
        nuevo->precio = precio;
        nuevo->siguiente = nullptr;

        if (cabeza == nullptr) {
            cabeza = nuevo;
        }
        else {
            NodoProducto* actual = cabeza;
            while (actual->siguiente != nullptr)
                actual = actual->siguiente;
            actual->siguiente = nuevo;
        }
    }
    return cabeza;
}

// Libera todos los nodos de la lista de productos
inline void liberar_productos(NodoProducto* cabeza) {
    while (cabeza != nullptr) {
        NodoProducto* temp = cabeza;
        cabeza = cabeza->siguiente;
        delete temp;
    }
}

// Guarda el número de mesas configurado
inline void guardar_mesas(int num_mesas) {
    std::ofstream archivo(ARCHIVO_MESAS);
    archivo << num_mesas << "\n";
}

// Lee el número de mesas (0 si no se ha configurado aún)
inline int cargar_mesas() {
    std::ifstream archivo(ARCHIVO_MESAS);
    if (!archivo.is_open()) return 0;
    int n = 0;
    archivo >> n;
    return n;
}

// --------------CRUD ORDENES------------------

//-------------------- Crear ornde---------------------
inline int crear_orden(int numero_mesa, const char* producto, int cantidad) {
    int num_mesas = cargar_mesas();
    if (num_mesas > 0 && (numero_mesa < 1 || numero_mesa > num_mesas))
        return -1;

    NodoOrden* ordenes = cargar_ordenes();
    int nuevo_id = siguiente_id_orden(ordenes);

    // Crear el nodo nuevo e insertarlo al final
    NodoOrden* nuevo = new NodoOrden();
    nuevo->id = nuevo_id;
    nuevo->numero_mesa = numero_mesa;
    strncpy(nuevo->producto, producto, TAM_PRODUCTO - 1);
    nuevo->producto[TAM_PRODUCTO - 1] = '\0';
    nuevo->cantidad = cantidad;
    nuevo->estado = ESTADO_PENDIENTE;
    nuevo->siguiente = nullptr;

    if (ordenes == nullptr) {
        ordenes = nuevo;
    }
    else {
        NodoOrden* actual = ordenes;
        while (actual->siguiente != nullptr)
            actual = actual->siguiente;
        actual->siguiente = nuevo;
    }

    guardar_ordenes(ordenes);
    liberar_ordenes(ordenes);
    return nuevo_id;
}

// ------- modificar_orden() busca orden por ID, actualiza sus campos y guarda.------
inline int modificar_orden(int id, int numero_mesa,
    const char* producto, int cantidad) {
    NodoOrden* ordenes = cargar_ordenes();
    NodoOrden* actual = ordenes;
    int encontrado = 0;

    while (actual != nullptr) {
        if (actual->id == id) {
            actual->numero_mesa = numero_mesa;
            strncpy(actual->producto, producto, TAM_PRODUCTO - 1);
            actual->producto[TAM_PRODUCTO - 1] = '\0';
            actual->cantidad = cantidad;
            encontrado = 1;
            break;
        }
        actual = actual->siguiente;
    }

    if (encontrado) guardar_ordenes(ordenes);
    liberar_ordenes(ordenes);
    return encontrado;
}

// COMPLETAR ORDEN (cambia estados de las ordenes)

inline int completar_orden(int id) {
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

    if (encontrado) guardar_ordenes(ordenes);
    liberar_ordenes(ordenes);
    return encontrado;
}

// Imprime en pantalla todas las ordenes
inline void listar_ordenes() {
    NodoOrden* ordenes = cargar_ordenes();
    NodoOrden* actual = ordenes;

    if (actual == nullptr) {
        std::cout << "  (No hay ordenes registradas)\n";
        liberar_ordenes(ordenes);
        return;
    }

    std::cout << "\n  ID  | Mesa | Producto               | Cant | Estado\n";
    std::cout << "  ----|------|------------------------|------|----------\n";
    while (actual != nullptr) {
        printf("  %-3d | %-4d | %-22s | %-4d | %s\n",
            actual->id, actual->numero_mesa, actual->producto,
            actual->cantidad,
            actual->estado == ESTADO_PENDIENTE ? "Pendiente" : "Completa");
        actual = actual->siguiente;
    }
    liberar_ordenes(ordenes);
}

// Imprime solo  ordenes con estado pendiente
inline void listar_ordenes_pendientes() {
    NodoOrden* ordenes = cargar_ordenes();
    NodoOrden* actual = ordenes;
    int hay = 0;

    std::cout << "\n  ID  | Mesa | Producto               | Cant\n";
    std::cout << "  ----|------|------------------------|-----\n";
    while (actual != nullptr) {
        if (actual->estado == ESTADO_PENDIENTE) {
            printf("  %-3d | %-4d | %-22s | %d\n",
                actual->id, actual->numero_mesa,
                actual->producto, actual->cantidad);
            hay = 1;
        }
        actual = actual->siguiente;
    }
    if (!hay) std::cout << "  (No hay ordenes pendientes)\n";
    liberar_ordenes(ordenes);
}

//------------------------CRUD productos ---------------

// ---------crear_producto--------------

inline int crear_producto(const char* nombre, float precio) {
    NodoProducto* productos = cargar_productos();
    int nuevo_id = siguiente_id_producto(productos);

    NodoProducto* nuevo = new NodoProducto();
    nuevo->id = nuevo_id;
    strncpy(nuevo->nombre, nombre, TAM_PRODUCTO - 1);
    nuevo->nombre[TAM_PRODUCTO - 1] = '\0';
    nuevo->precio = precio;
    nuevo->siguiente = nullptr;

    if (productos == nullptr) {
        productos = nuevo;
    }
    else {
        NodoProducto* actual = productos;
        while (actual->siguiente != nullptr)
            actual = actual->siguiente;
        actual->siguiente = nuevo;
    }

    guardar_productos(productos);
    liberar_productos(productos);
    return nuevo_id;
}

//---------modificar producto----------

inline int modificar_producto(int id, const char* nombre, float precio) {
    NodoProducto* productos = cargar_productos();
    NodoProducto* actual = productos;
    int encontrado = 0;

    while (actual != nullptr) {
        if (actual->id == id) {
            strncpy(actual->nombre, nombre, TAM_PRODUCTO - 1);
            actual->nombre[TAM_PRODUCTO - 1] = '\0';
            actual->precio = precio;
            encontrado = 1;
            break;
        }
        actual = actual->siguiente;
    }

    if (encontrado) guardar_productos(productos);
    liberar_productos(productos);
    return encontrado;
}

//--------eliminar producto -------------

inline int eliminar_producto(int id) {
    NodoProducto* productos = cargar_productos();
    NodoProducto* actual = productos;
    NodoProducto* anterior = nullptr;
    int encontrado = 0;

    while (actual != nullptr) {
        if (actual->id == id) {
            // Saltar el nodo: el anterior apunta al siguiente del eliminado
            if (anterior != nullptr)
                anterior->siguiente = actual->siguiente;
            else
                productos = actual->siguiente; // era el primer nodo
            delete actual;
            encontrado = 1;
            break;
        }
        anterior = actual;
        actual = actual->siguiente;
    }

    if (encontrado) guardar_productos(productos);
    liberar_productos(productos);
    return encontrado;
}

// Imprime en pantalla todos los productos del menú
inline void listar_productos() {
    NodoProducto* productos = cargar_productos();
    NodoProducto* actual = productos;

    if (actual == nullptr) {
        std::cout << "  (No hay productos en el menu)\n";
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

#endif