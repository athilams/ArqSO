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

int main()
{
	char mensagemrecebida[TAM_MSG];
	char command[TAM_MSG];

	//criar o socket
	int socketCliente;
	socketCliente = socket(AF_INET, SOCK_STREAM, 0);

	//especificação do endereço do socket
	struct sockaddr_in endereco_servidor;
	endereco_servidor.sin_family = AF_INET;
	endereco_servidor.sin_port = htons(PORT);
	endereco_servidor.sin_addr.s_addr = INADDR_ANY;


	//conecta o socket com o socket do servidor
	int success = connect(socketCliente, (struct sockaddr*)&endereco_servidor, sizeof(endereco_servidor));
	
	//verifica se a conexao com o servidor foi bem sucedida
	if(success < 0)
		printf("ERRO AO CONECTAR AO SERVIDOR!\n");
	else
	{
		printf("CONEXAO BEM SUCEDIDA!\n");
		printf("close -> encerra conexao com o servidor\n");
		printf("shutdown -> desliga o servidor\n");
		
		while(1)
		{
			//envia comando ao servidor
			command[0] = '\0';
			printf("DIGITE O COMANDO: "); fflush(stdout);
			memset(&command, '\0', sizeof(command));
			fgets(command, TAM_MSG, stdin);

			send(socketCliente, command, sizeof(command), 0);

			//sai do loop e encerra o socket
			if (!strncmp(command, "close", 5) || !strncmp(command, "shutdown", 8))
			{
				send(socketCliente, command, sizeof(command), 0);
				shutdown(socketCliente, 2);
				return 0;
			}

			if(!strncmp(command, "ls", 2))
			{
				printf("LISTAR CONTEUDO:\n");
				memset(&mensagemrecebida, '\0', sizeof(mensagemrecebida));
				while(1)
				{
					
					recv(socketCliente, &mensagemrecebida, sizeof(mensagemrecebida), 0);

					if (!strncmp(mensagemrecebida, "fim",3))
			       		break;									
					printf("%s\n", mensagemrecebida);
										
				}
			}

			if(!strncmp(command, "cat", 3))
			{
				printf("LER ARQUIVO:\n");
				memset(&mensagemrecebida, '\0', sizeof(mensagemrecebida));
				while(1)
				{
					
					recv(socketCliente, &mensagemrecebida, sizeof(mensagemrecebida), 0);

					if (!strncmp(mensagemrecebida, "fim",3))
			       		break;									
					printf("%s\n", mensagemrecebida);
										
				}
			}

			//if(!strncmp(command, "cd", 2))
			//{
			//	memset(&mensagemrecebida, '\0', sizeof(mensagemrecebida));
			//	recv(socketCliente, &mensagemrecebida, sizeof(mensagemrecebida), 0);
			//	printf("DIRETORIO ATUAL: "); fflush(stdout);
			//	printf("%s\n", mensagemrecebida);
			//}

		}

		shutdown(socketCliente, 2);
	}

	return 0;
}
