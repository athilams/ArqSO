#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define FREESPACESIZE 1024
#define setBit(A, k) ( A[k/32] |= (1 << (k%32)) )
#define clearBit(A, k) ( A[(k/32)] &= ~(1 << (k%32)) )
#define testBit(A, k) ( A[(k/32)] & (1 << (k%32)) )
#define INODESNUMBER 16

typedef struct
{
	int inode; // numero do inode
	char name[50]; // nome do arquivo
	int size; // tamanho em bytes do arquivo
	char creation[26]; // armazena a data da criação
	char modification[26]; // armazena a data de modificação
	int block_number; // armazena posição do primeiro bloco

}inode;

int main ()
{
	FILE *FS;

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

	while (1)
	{

		inode read; 

		fopen("filesystem.bin", "r+b");

		fseek(FS, FREESPACESIZE + sizeof(inode) * INODESNUMBER, SEEK_SET);
		fread(&bitMapF, sizeof(bitMapF), 1, FS);
		fread(&bitMapI, sizeof(bitMapI), 1, FS);

		printf("\nNome do arquivo: ");
		memset(read.name, '\0', sizeof(read.name));
		fgets(read.name, 50, stdin);
		time_t clk = time(NULL);
		strncpy(read.creation, ctime(&clk), 26);
	  	strncpy(read.modification, ctime(&clk), 26);
	  	printf("\nConteudo do arquivo: ");
	  	char buffer[50], *dbuffer = malloc(sizeof(char));
	  	memset(buffer, '\0', sizeof(buffer));
	  	fgets(buffer, 50, stdin);

	  	int z = 0;

	  	while(buffer[z] != '\n' && buffer[z]!= EOF)
	  		z++;

	  	dbuffer = realloc(dbuffer, z);

	  	strncpy(dbuffer, buffer, z-1);

	  	read.size = sizeof(dbuffer);

	  	int i = 0;
	  	int j = 0;

	  	while (testBit(bitMapF, i))
	  	{
	  		i++;
	  	}

	  	while (testBit(bitMapI, j))
	  	{
	  		j++;
	  	}

	  	fseek(FS, FREESPACESIZE + sizeof(inode) * j, SEEK_SET);
	  	
	  	printf("\nEscrevendo dados inode...");

		read.block_number = i;

		if (fwrite(read.name, sizeof(read.name), 1, FS) &&
			fwrite(&read.size, sizeof(read.size), 1, FS) &&
			fwrite(read.creation, sizeof(read.creation), 1, FS) &&
			fwrite(read.modification, sizeof(read.modification), 1, FS) &&
			fwrite(&read.block_number, sizeof(read.block_number), 1, FS))
		{
			sleep(1);
			printf(" OK!");
		}
		else
		 	printf(" ERRO AO ESCREVER DADOS INODE");

		fseek(FS, -sizeof(int), SEEK_CUR);
		int t;
		fread(&t, sizeof(int), 1, FS);
		printf(" %d\n", t);

		fseek(FS, i, SEEK_SET);

		printf("\nEscrevendo dados arquivo...");

		if(fwrite(buffer, sizeof(buffer), 1, FS))
			printf(" OK!");
		else 
			printf(" ERRO AO ESCREVER DADOS ARQUIVO");
			
		for(int k = i; k < i + z; k++)
		{
			setBit(bitMapF, k);
		}

		setBit(bitMapI, j);

		fseek(FS, FREESPACESIZE + INODESNUMBER * sizeof(inode), SEEK_SET);
		
		fwrite(bitMapF, sizeof(bitMapF), 1, FS);
		fwrite(bitMapI, sizeof(bitMapI), 1, FS);

		printf("\nDados salvos a partir do byte %d!", i);

		fclose(FS);

	}

	return 0;
}