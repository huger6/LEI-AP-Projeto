#include "headers.h"
#include "funcoes.c"

int main() {
    //Colocar a consola em PT-PT (caracteres UTF8) Averiguar para sistemas não win
    SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
    // setlocale(LC_ALL, "Portuguese"); Esta linha faz com que os caracteres fiquem destorcidos
	
	//Lidar com os ficheiros
	FILE * dados;
	FILE * situacao_escolar;

	dados = fopen(DADOS_TXT, "w");
	if (dados == NULL) { 
		printf("Ocorreu um erro a abrir o ficheiro %s. Por favor, tente novamente.\n", DADOS_TXT);
		return -1; //Erro
	}
	
	situacao_escolar = fopen(SITUACAO_ESCOLAR_TXT, "w");
	if (situacao_escolar == NULL) {
		printf("Ocorreu um erro a abrir o ficheiro %s. Por favor, tente novamente.\n", SITUACAO_ESCOLAR_TXT);
		return -1; 
	}

	escolha_menus();





	//Se quisermos sair do programa... if {...
	fclose(dados);
	fclose(situacao_escolar);
}
