all: server client

server: servidor.c
	gcc servidor.c -o -pthread

client: cliente.c
	gcc cliente.c -o -pthread
