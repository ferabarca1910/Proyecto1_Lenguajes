
/*
 * protocolo.h
 * Estructuras y constantes compartidas entre cliente y servidor.
 * Todo lo que se envía por el socket tiene que ser de un tamanho fijo
 */

#ifndef PROTOCOLO_H
#define PROTOCOLO_H

 
#define TAM_PRODUCTO 64

// Codigos de operacion
#define OP_REGISTRAR_ORDEN    1
#define OP_MODIFICAR_ORDEN    2
#define OP_LISTAR_ORDENES     3
#define OP_COMPLETAR_ORDEN    4
#define OP_CREAR_PRODUCTO     5
#define OP_LISTAR_PRODUCTOS   6
#define OP_MODIFICAR_PRODUCTO 7
#define OP_ELIMINAR_PRODUCTO  8
#define OP_CONFIG_MESAS       9
#define OP_SALIR              67

// Estados posibles de una orden
#define ESTADO_PENDIENTE 0
#define ESTADO_COMPLETA  1

/*
 * Mensaje generico que viaja por el socket.
 * El canpo 'operacion' indica qué tipo de accion se solicita.
 * Los demas campos se usan según la operación.
 */
struct Mensaje {
    int operacion;
    int id;              // id de orden o producto segun el caso
    int numero_mesa;
    char producto[TAM_PRODUCTO];
    int cantidad;
    float precio;
    int estado;
    int num_mesas;       // usado al configurar la cantidad de mesas
};

/*
 * Respuesta del servidor al cliente.
 * 'exito' indica si la operacion fue exitosa.
 * 'texto' lleva informacion adicional cuando no se lleno un canpo.
 */
struct Respuesta {
    int exito;
    char texto[256];
};

#endif