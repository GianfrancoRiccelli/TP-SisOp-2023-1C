#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

typedef enum {
	INSTRUCCIONES_CONSOLA, MENSAJE, TERMINAR_CONSOLA
} op_code;


typedef struct {
	int size;
	void *stream;
} t_buffer;

typedef struct {
	int codigo_operacion;
	t_buffer *buffer;
} t_paquete;

struct addrinfo *addrinfo_servidor(char *ip, char *puerto);
int crear_socket(struct addrinfo *server_info);

// servidor

int iniciar_servidor(char *puerto);
int esperar_cliente(int);
void *recibir_buffer(int *, int);
int recibir_operacion(int);
void *recibir_mensaje(int);

// cliente

int crear_conexion(char *ip, char *puerto);
t_buffer *crear_buffer();
t_paquete *crear_paquete(int codigo_operacion);
void agregar_a_paquete(t_paquete *paquete, void *valor, int bytes);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
void enviar_mensaje(void *mensaje, int bytes, int socket_cliente);
void liberar_conexion(int socket_cliente);
void *sacar_de_buffer(t_buffer *buffer, int tam_dato);

#endif /* SOCKETS_H_ */
