#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define TAM_MSG 1024
#define PORT 5006

void *t_connection(void *arg);

pthread_mutex_t mutex;

char comando[TAM_MSG];

int i = 0; 

int main()
{
	pthread_mutex_init(&mutex, NULL);

	//criar o socket do servidor
	int socketServidor, socketCliente;
	socketServidor = socket (AF_INET, SOCK_STREAM, 0);

	socklen_t addr_size;
	struct sockaddr_storage serverStorage;
	addr_size = sizeof serverStorage;
	//especificação do endereço do socket 
	struct sockaddr_in endereco_servidor;
	endereco_servidor.sin_family = AF_INET;
	endereco_servidor.sin_port = htons(PORT);
	endereco_servidor.sin_addr.s_addr = INADDR_ANY;

	//associar o socket com endereço e porta
	bind(socketServidor, (struct sockaddr*)&endereco_servidor, sizeof(endereco_servidor));

	listen(socketServidor, 10);

	printf("SERVIDOR INICIADO NA PORTA %d\nAGUARDANDO CONEXAO...\n", PORT);

	pthread_t connection;
	int i = 0;

	while(1)
	{
		
		socketCliente = accept(socketServidor, (struct sockaddr*)&serverStorage , &addr_size);
		if (socketCliente >=0)
		{
			
			printf("CLIENTE CONECTADO! ID: %d\n", socketCliente);
			pthread_create(&connection, NULL, t_connection, &socketCliente);	
			//pthread_join(connection, NULL);			
		}else
			printf("ERRO AO CRIAR SOCKET\n");
			
	}
	return 0;
}

void *t_connection(void *arg)
{
	int keepreading;
	int socketCliente = *((int*)arg);

	FILE *output;

	char mensagemenviar[TAM_MSG], mensagemrecebida[TAM_MSG], confirm[1];
	memset(&mensagemrecebida, '\0', sizeof(mensagemrecebida));

	char *sendOut = NULL;
	size_t len = 0;

	while(1)
	{
		if(socketCliente  ==  -1)
			{
				printf("ERRO DE CONEXÃO\n");
			}else
			if(socketCliente > 0)
			{	
				
				//recebe comando do cliente
				if (recv(socketCliente, &mensagemrecebida, TAM_MSG, 0)<0)
					printf("ERRO AO RECEBER MENSAGEM\n");

				if (!strncmp(mensagemrecebida, "rm -rf", 6))
				{
					printf("DELETAR PASTA\n");

					system(mensagemrecebida);
				}else
				{
					if (!strncmp(mensagemrecebida, "rm", 2))
					{
						printf("DELETAR ARQUIVO\n");

						system(mensagemrecebida);
					}
				}

				if (!strncmp(mensagemrecebida, "mkdir", 5))
				{
					printf("CRIAR PASTA\n");

					pthread_mutex_lock(&mutex);
						system(mensagemrecebida);
					pthread_mutex_unlock(&mutex);
				}

				if (!strncmp(mensagemrecebida, "touch", 5))
				{
					printf("CRIAR ARQUIVO\n");

					pthread_mutex_lock(&mutex);
						system(mensagemrecebida);
					pthread_mutex_unlock(&mutex);
				}

				if (!strncmp(mensagemrecebida, "close", 5))
				{
					printf("ENCERRAR CONEXAO\n");
					shutdown(socketCliente, 2);
					pthread_cancel(pthread_self());
				}

				if (!strncmp(mensagemrecebida, "echo", 4))
				{
					printf("ESCREVER\n");

					pthread_mutex_lock(&mutex);
						system(mensagemrecebida);
					pthread_mutex_unlock(&mutex);
				}

				if (!strncmp(mensagemrecebida, "cat", 3))
				{
					printf("LER ARQUIVO\n");

					char *buff;	

					strtok_r(mensagemrecebida, " ", &buff);
					buff[strlen(buff)-1] = '\0';
					printf("%s", buff);
					output = fopen(buff, "r");
					
					while(keepreading = getline(&sendOut, &len, output)!= -1)
					{
						//printf("%s\n", sendOut);
						send(socketCliente, sendOut, TAM_MSG, 0);
					}

					memset(&mensagemenviar, '\0', sizeof(mensagemenviar));
					strncpy(mensagemenviar, "fim", 3);
					send(socketCliente, mensagemenviar , sizeof(mensagemenviar), 0);
					fclose(output);					
					free(sendOut);
				}


				if (!strncmp(mensagemrecebida, "cd", 2))
				{
					printf("ABRIR PASTA\n");
					
					char *buff;
					strtok_r(mensagemrecebida, " ", &buff);
					buff[strlen(buff)-1] = '\0';
					printf("%s", buff);
					chdir(buff);
					send(socketCliente, buff, TAM_MSG, 0);
					
				}

				
				if (!strncmp(mensagemrecebida, "ls", 2))
				{
					printf("LISTAR CONTEUDO\n");
				
					system("ls > .output.txt");
					output = fopen(".output.txt", "r");
					
					while(keepreading = getline(&sendOut, &len, output)!= -1)
					{
						send(socketCliente, sendOut, TAM_MSG, 0);
					}

					memset(&mensagemenviar, '\0', sizeof(mensagemenviar));
					strncpy(mensagemenviar, "fim", 3);
					send(socketCliente, mensagemenviar , sizeof(mensagemenviar), 0);
					fclose(output);					
					free(sendOut);
				}			

			}
	}
}
