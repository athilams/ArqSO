#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define TAM_MSG 1024
#define PORT 5006

#define FREESPACESIZE 1024
#define setBit(A, k) ( A[k/32] |= (1 << (k%32)) )
#define clearBit(A, k) ( A[(k/32)] &= ~(1 << (k%32)) )
#define testBit(A, k) ( A[(k/32)] & (1 << (k%32)) )
#define INODESNUMBER 32

typedef struct
{
	int inode; // numero do inode
	char name[50]; // nome do arquivo
	int size; // tamanho em bytes do arquivo
	char creation[26]; // armazena a data da criação
	char modification[26]; // armazena a data de modificação
	int block_number; // armazena posição do primeiro bloco

}inode;

void *t_connection(void *arg);

pthread_mutex_t mutex;

char comando[TAM_MSG];

int i = 0; 

int main()
{
	pthread_mutex_init(&mutex, NULL);

	// ############################### PARTE 2 INICIO ###############################

	
		

	// ############################### PARTE 2 FIM ###############################




	//criar o socket do servidor
	int socketServidor;
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
		int socketCliente;
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
	FILE *output, *FS;
	int keepreading;
	int socketCliente = *((int*)arg);
	int bitMapF[FREESPACESIZE/32];
	int bitMapI[INODESNUMBER/32];

	printf("Criando Sistema de Arquivos ...\n");
	sleep(1);
	if ((FS = fopen("filesystem.bin", "wb")) == NULL)
	{
		printf("ERRO AO CRIAR/ABRIR SISTEMA DE ARQUIVOS");
		exit(1);
	}

	if (truncate("filesystem.bin", FREESPACESIZE + INODESNUMBER * sizeof(inode) + sizeof(bitMapF) + sizeof(bitMapI)))
	{
			printf("ERRO AO ALOCAR ESPAÇO!");
			exit(1);
	}

	fseek(FS, FREESPACESIZE, SEEK_SET);

	for (int i = 1; i <= INODESNUMBER; i++)
	{
		fwrite(&i, sizeof(int), 1, FS);
		fseek(FS, sizeof(inode)-sizeof(int), SEEK_CUR);	
	}

	for (int i = 0; i < FREESPACESIZE/32; i++)
	{
		bitMapF[i] = 0;
	}	

	for (int i = 0; i < INODESNUMBER/32; i++)
	{
		bitMapI[i] = 0;
	}	

	fseek(FS, FREESPACESIZE + INODESNUMBER * sizeof(inode), SEEK_SET);

	fwrite(bitMapF, sizeof(bitMapF), 1, FS);
	fwrite(bitMapI, sizeof(bitMapI), 1, FS);

	fclose(FS);
	
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

				if (!strncmp(mensagemrecebida, "shutdown", 8))
				{
					printf("SERVIDOR FINALIZANDO EM 3...\n");
					sleep(1);
					printf("                        2...\n");
					sleep(1);
					printf("                        1...\n");
					sleep(1);
					exit(0);
				}

				if (!strncmp(mensagemrecebida, "delete", 6))
				{
					printf("DELETAR\n");
					pthread_mutex_lock(&mutex);
						FS = fopen("filesystem.bin", "r+b");
						fseek(FS, FREESPACESIZE + INODESNUMBER * sizeof(inode), SEEK_SET);

						fread(bitMapF, sizeof(bitMapF), 1, FS);
						fread(bitMapI, sizeof(bitMapI), 1, FS);


						char *fil, *data, check[50];

						strtok_r(mensagemrecebida, " ", &fil);
						fil = strtok(fil, " ");						

						int x;

						for (x=0; x< INODESNUMBER;x++)
						{
							fseek(FS, FREESPACESIZE + x * sizeof(inode) + sizeof(int), SEEK_SET);
							fread(check, sizeof(char), strlen(fil), FS);
							if(!strcmp(check, fil))
								break;
						}

						if(x == INODESNUMBER)
							printf("Arquivo não encontrado!\n");
						else
						{
							fseek(FS, FREESPACESIZE + sizeof(inode) * (x+1) - sizeof(int) -2 , SEEK_SET);
							int reset, nallc = -1;
							fread(&reset, sizeof(int), 1, FS);
							printf("%d\n", reset);
							for (int k = reset; k < reset+31;k++)
							{
								clearBit(bitMapF, k);
							}

							fseek(FS, -sizeof(int), SEEK_CUR);
							fwrite(&nallc, sizeof(int), 1, FS);
							char noname[50];
							memset(noname, '\0', 50);
							fseek(FS, FREESPACESIZE + sizeof(inode) * x + sizeof(int), SEEK_SET);

							
							clearBit(bitMapI, x);
							fseek(FS, FREESPACESIZE + INODESNUMBER * sizeof(inode), SEEK_SET);

							fwrite(bitMapF, sizeof(bitMapF), 1, FS);
							fwrite(bitMapI, sizeof(bitMapI), 1, FS);
						}
						fclose(FS);
					pthread_mutex_unlock(&mutex);
				}
				// else
				// {
				// 	if (!strncmp(mensagemrecebida, "rm", 2))
				// 	{
				// 		printf("DELETAR ARQUIVO\n");
						
				// 		pthread_mutex_lock(&mutex);
				// 		system(mensagemrecebida);
				// 		pthread_mutex_unlock(&mutex);
				// 	}
				// }

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

					char *buff;

					strtok_r(mensagemrecebida, " ", &buff);
					buff[strlen(buff)-1] = '\0';

					pthread_mutex_lock(&mutex);
						
						inode read;
						int bitMapF[FREESPACESIZE/32];
						int bitMapI[INODESNUMBER/32]; 

						FS = fopen("filesystem.bin", "r+b");

						fseek(FS, FREESPACESIZE + sizeof(inode) * INODESNUMBER + sizeof(bitMapF), SEEK_SET);
						fread(&bitMapI, sizeof(bitMapI), 1, FS);

						memset(read.name, '\0', 50);
						strncpy(read.name, buff, 50);
						time_t clk = time(NULL);
						strncpy(read.creation, ctime(&clk), 26);
					  	strncpy(read.modification, ctime(&clk), 26);
					  	
						int j = 0;
					  	while (testBit(bitMapI, j))
					  	{
					  		j++;
					  	}

					  	fseek(FS, FREESPACESIZE + sizeof(inode) * j + sizeof(int), SEEK_SET);
					  	printf("\n%ld", ftell(FS));
					  	printf("\nEscrevendo dados inode...");
					 
						read.size = 0;
						read.block_number = -1;

						if(fwrite(read.name, 50, sizeof(char), FS) &&
						fwrite(&read.size, 1, sizeof(int), FS) &&
						fwrite(read.creation, 26, sizeof(char), FS) &&
						fwrite(read.modification, 26, sizeof(char), FS) &&						
						fwrite(&read.block_number, 1, sizeof(int), FS))
						{
							printf(" INODE OK!\n");
						}
						else					
							printf(" ERRO AO ESCREVER DADOS INODE\n");
								
						setBit(bitMapI, j);

						fseek(FS, FREESPACESIZE + INODESNUMBER * sizeof(inode), SEEK_SET);
						
						fwrite(bitMapF, sizeof(bitMapF), 1, FS);
						fwrite(bitMapI, sizeof(bitMapI), 1, FS);

						fclose(FS);

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

					FS = fopen("filesystem.bin", "r+b");

					char *fil, *data, check[50];

					strtok_r(mensagemrecebida, " ", &fil);
					fil = strtok_r(fil, " ", &data);
					printf("%s\n%s\n",fil, data);

					int x;

					for (x=0; x< INODESNUMBER;x++)
					{
						fseek(FS, FREESPACESIZE + x * sizeof(inode) + sizeof(int), SEEK_SET);
						fread(check, sizeof(char), strlen(fil), FS);
						if(!strcmp(check, fil))
							break;
					}

					if(x == INODESNUMBER)
						printf("Arquivo não encontrado!\n");
					else
					{
						int blchk;
						int i = 0;
						fseek(FS, FREESPACESIZE + sizeof(inode) * (x+1) - sizeof(int) -2 , SEEK_SET);
						fread(&blchk, sizeof(int), 1, FS);
						printf("TESTE %d\n", blchk);
						if (blchk == -1)
						{							
							fseek(FS, FREESPACESIZE + sizeof(inode) * INODESNUMBER, SEEK_SET);
							fread(&bitMapF, sizeof(bitMapF), 1, FS);
							
							while (testBit(bitMapF, i))
						  	{
						  		i++;
						  	}

						  	fseek(FS, i, SEEK_SET);
							fwrite(data, sizeof(char), strlen(data), FS);

							for (int k = i; k < i+32;k++)
							{
								setBit(bitMapF, k);
							}

						  	fseek(FS, FREESPACESIZE + sizeof(inode) * (x+1) - sizeof(int) -2, SEEK_SET);
						  	fwrite(&i, sizeof(int), 1, FS);

						  	
							fseek(FS, FREESPACESIZE + sizeof(inode) * x + sizeof(int) + sizeof(char)*50, SEEK_SET);
							int dtsz = strlen(data);
							fwrite(&dtsz, sizeof(int), 1, FS);

						}
						else
						{
							i = blchk;
							if(strlen(data)<=32)
							{
								fseek(FS, i, SEEK_SET);
								fwrite(data, sizeof(char), strlen(data), FS);															
							}
							else
							{
								printf("\nNAO IMPLEMENTEI ESSA PARTE!\n");
							}
						}

						fseek(FS, FREESPACESIZE + sizeof(inode) * INODESNUMBER, SEEK_SET);
						fwrite(bitMapF, sizeof(bitMapF), 1, FS);
					}
					
					fclose(FS);

					pthread_mutex_unlock(&mutex);
				}

				if (!strncmp(mensagemrecebida, "cat", 3))
				{
					printf("LER ARQUIVO\n");				
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
