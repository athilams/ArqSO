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

int main()
{

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

		int inode_list[max_i];
		memset(inode_list, 0, sizeof(inode_list));
		
		int m=0;
		while (inode_list[m] != 0)
		{
			m++;
		}

		int name_list[qte_bloco];
		memset(name_list, 0, sizeof(name_list));

		int n=0;
		while (name_list[n] != 0)
		{
			n++;
		}

		inode_list[m] = 1;

		inodes.inode = m+1;
		inodes.size = 0;
		time_t clk = time(NULL);
  		strncpy(inodes.creation, ctime(&clk), 26);
  		strncpy(inodes.modification, ctime(&clk), 26);
  		inodes.ind = 0;
  		inodes.indirect_pointer = NULL;

  		fseek(FS, i_start + m * sizeof(inode), SEEK_SET);
		fwrite(&inodes.inode, sizeof(int), 1, FS);
		fwrite(&inodes.size, sizeof(int), 1, FS);
		fwrite(inodes.creation, sizeof(char), 26, FS);
		fwrite(inodes.modification, sizeof(char), 26, FS);
		fwrite(&inodes.ind, sizeof(int), 1, FS);
		fwrite(inodes.indirect_pointer, sizeof(void*), 1, FS);
		
  		strncpy(ffile.nome, mensagemrecebida, 50);
		ffile.inode = m+1;

		fseek(FS, n_start + n * sizeof(filenames), SEEK_SET);
		fwrite(ffile.nome, sizeof(char), 50, FS);
		fwrite(&ffile.inode, sizeof(int), 1, FS);

		fclose(FS);


		//inode *inode_list = malloc (sizeof(inode) + qte_bloco/qte_bloco_i * tam_bloco);

	}
   
	return 0;
}




		 