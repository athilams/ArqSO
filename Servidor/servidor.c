#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define K 1024
#define TAM_MSG 1024
#define PORT 5006


typedef struct
{
	char nome[50];
	int inode;

}filenames;

typedef struct
{	
	char nome[50];
	int inode;
	filenames files[];

}dirnames;

typedef struct inode
{
	int inode; // numero do inode
	int size; // tamanho em blocos do arquivo
	char creation[26]; // armazena a data da criação
	char modification[26]; // armazena a data de modificação
	int ind; // este inteiro indica qual "nível" de ponteiro indireto.
	struct inode *indirect_pointer; // Se ind = 0, aponta para NULL. Se maior que 0, aponta para o ponteiro indireto;
	long int block_pointer[]; // vetor para armazenar posição dos blocos

}inode;

void *t_connection(void *arg);

pthread_mutex_t mutex;

char comando[TAM_MSG];

int i = 0; 

int main()
{
	pthread_mutex_init(&mutex, NULL);

	// ############################### PARTE 2 INICIO ###############################

	int tam_bloco // tamanho do bloco em KB
		, qte_bloco // numero de blocos livres
		, qte_bloco_i // quantidade de blocos referenciados por inode
		, max_i; // quantidade de inodes disponíveis
	
long int fs_r_size // tamanho da região livre em Bytes	
		, i_start // primeiro byte da região de inodes
		, i_size // tamanho em bytes da região de inodes 
		, n_start // primeiro byte da região de nomes
		, n_size // tamanho em bytes da região de nomes
		, d_start // primeito byte da regiãõ de nomes de diretorios
		, d_size; // tamanho em bytes da regiãõ de nomes de diretorios

	filenames ffile;
	dirnames dir;
	inode inodes;

	FILE *FS;

	printf("Criando Sistema de Arquivos ...");
	if ((FS = fopen("filesystem.bin", "wb")) == NULL)
	{
		printf("ERRO AO CRIAR/ABRIR SISTEMA DE ARQUIVOS");
		exit(1);
	}
	else
	{
		printf("Sistema de Arquivos criado com sucesso!\n");
		printf("Tamanho de blocos (em KB): ");
		fscanf(stdin, "%d", &tam_bloco);
		tam_bloco = tam_bloco * K;
		printf("Quantidade de blocos a serem criados: ");
		fscanf(stdin, "%d", &qte_bloco);

		printf("Alocando região de dados ...");
		if (truncate("filesystem.bin", tam_bloco * qte_bloco * 4))
		{
			printf("ERRO AO ALOCAR ESPAÇO!");
			exit(1);
		}

		fseek(FS, 0, SEEK_END);
		fs_r_size =  ftell(FS);
		printf("Bytes alocados: %li Bytes totais: %li", fs_r_size, ftell(FS));
	
		printf("\nQuantidade máxima de inodes (divisor da quantidade de blocos): ");
		fscanf(stdin, "%d", &max_i);
		qte_bloco_i = qte_bloco/max_i;
		
		i_start = fs_r_size + 1;
		printf("Alocando região de inode ...");
		if (truncate("filesystem.bin", max_i * sizeof(struct inode) + qte_bloco_i * tam_bloco + ftell(FS)))
		{
			printf("ERRO AO ALOCAR ESPAÇO!");
			exit(1);
		}

		fseek(FS, 0, SEEK_END);
		i_size = ftell(FS) - i_start;
		printf("Bytes alocados: %li Bytes totais: %li\n", i_size, ftell(FS));

		printf("Alocando região de nomes (1/2) ...");
		if (truncate("filesystem.bin", qte_bloco * sizeof(filenames) + ftell(FS)))
		{
			printf("ERRO AO ALOCAR ESPAÇO!");
			exit(1);
		}

		n_start = ftell(FS) + 1;
		fseek(FS, 0, SEEK_END);
		n_size = ftell(FS) - n_start;
		printf("Bytes alocados: %li Bytes totais: %li\n", n_size, ftell(FS));

		int qte_dir = qte_bloco;
		printf("Alocando região de nomes (2/2) ...");
		if (truncate("filesystem.bin", qte_dir * sizeof(dirnames) + ftell(FS)))
		{
			printf("ERRO AO ALOCAR ESPAÇO!");
			exit(1);
		}

		d_start = ftell(FS) + 1;
		fseek(FS, 0, SEEK_END);
		d_size = ftell(FS) - d_start;
		printf("Bytes alocados: %li Bytes totais: %li\n", d_size, ftell(FS));

		printf("Criando índice ...");

		int blocos_livres[qte_bloco];
		memset(blocos_livres, 0, sizeof(blocos_livres));
		ptr[1] = &blocos_livres[0];

		int inode_list[max_i];
		memset(inode_list, 0, sizeof(inode_list));
		ptr[2] = &inode_list[0];

		int name_list[qte_bloco];
		memset(name_list, 0, sizeof(name_list));
		ptr[3] = &name_list[0];
		

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
		ptr[0] = &socketCliente;
		if (socketCliente >=0)
		{
			
			printf("CLIENTE CONECTADO! ID: %d\n", socketCliente);
			pthread_create(&connection, NULL, t_connection(), &socketCliente);	
			//pthread_join(connection, NULL);			
		}else
			printf("ERRO AO CRIAR SOCKET\n");
			
	}
	return 0;
}

int search (int *bl, int *in, int *nl)
{

}

void *t_connection(void *arg)
{
	
	int keepreading;
	int socketCliente = *((int*)arg[0]);

	FILE *output, *FS;
	
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

				if (!strncmp(mensagemrecebida, "delete", 7))
				{
					pthread_mutex_lock(&mutex);
					
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

					// pthread_mutex_lock(&mutex);
					// 	system(mensagemrecebida);
					// pthread_mutex_unlock(&mutex);

					int m=0;
					while (arg[2][m] != 0)
					{
						m++;
					}		

					int n=0;
					while (arg[3][n] != 0)
					{
						n++;
					}


					argv[2][m] = 1;

					inodes.inode = m+1;
					inodes.size = 0;
					time_t clk = time(NULL);
			  		strncpy(inodes.creation, ctime(&clk), 26);
			  		strncpy(inodes.modification, ctime(&clk), 26);
			  		inodes.ind = 0;
			  		inodes.indirect_pointer = NULL;

			  		pthread_mutex_lock(&mutex);
			  		FS = fopen("filesystem.bin", "r+b");
			  		fseek(FS, i_start + m * sizeof(inode), SEEK_SET);
					fwrite(&inodes.inode, sizeof(int), 1, FS);
					fwrite(&inodes.size, sizeof(int), 1, FS);
					fwrite(inodes.creation, sizeof(char), 26, FS);
					fwrite(inodes.modification, sizeof(char), 26, FS);
					fwrite(&inodes.ind, sizeof(int), 1, FS);
					fwrite(inodes.indirect_pointer, sizeof(void*), 1, FS);
					
			  		strncpy(ffile.nome, buff, 50);
					ffile.inode = m+1;

					fseek(FS, n_start + n * sizeof(filenames), SEEK_SET);
					fwrite(ffile.nome, sizeof(char), 50, FS);
					fwrite(&ffile.inode, sizeof(int), 1, FS);

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
						system(mensagemrecebida);
					pthread_mutex_unlock(&mutex);
				}

				if (!strncmp(mensagemrecebida, "cat", 3))
				{
					printf("LER ARQUIVO\n");

					// char *buff;	

					// strtok_r(mensagemrecebida, " ", &buff);
					// buff[strlen(buff)-1] = '\0';
					// printf("%s", buff);
					// output = f(buff, "r");
					
					// while(keepreading = getline(&sendOut, &len, output)!= -1)
					// {
					// 	//printf("%s\n", sendOut);
					// 	send(socketCliente, sendOut, TAM_MSG, 0);
					// }

					// memset(&mensagemenviar, '\0', sizeof(mensagemenviar));
					// strncpy(mensagemenviar, "fim", 3);
					// send(socketCliente, mensagemenviar , sizeof(mensagemenviar), 0);
					// fclose(output);					
					// free(sendOut);

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
