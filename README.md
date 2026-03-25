# Proyecto1_Lenguajes
Este es un programa creado en C++.
El propósito de este programa es hacer uan gestión de órdenes y de productos, donde un cliente contacte con un servidor. 
Se cuenta con dos funcionalidades: 
Cliente(Mesero): 
-Registra nueva orden: mesa,producto y cantidad
-Modifica ordenes existentes
-Ver ordenes activas
-Ver productos disponibles
Administrador (Servidor):
-Ve todas las ordenes
-Ve ordenes pendientes
-Marca orden completa
-CRUD de productos
-Configura el numero de mesas
El sistema usa un modelo Cliente-Servidor con sockets TCP. El servidor lanza un proceso hijo con fork() por cada cliente que se conecta, permitiendo atender carios clientes  simultáneamente sin bloquear las demás conexiones. Un proceso hijo adicional corre el menú interactivo del administrador.
