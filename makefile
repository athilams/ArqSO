all: server.o client.o

server.o: servidor.c
	gcc servidor.c -pthread

client.o: cliente.c
	gcc cliente.c -pthread