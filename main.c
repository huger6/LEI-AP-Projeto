#include "headers.h"
#include "funcoes.c"
//NOTA IMPORTANTE: para dar run temos que abrir o projeto em terminal integrado e depois de estar na main, dar compile run.

int main() {
    //Colocar a consola em PT-PT (caracteres UTF8)
	colocar_terminal_pt();
	
	//Lidar com os ficheiros

	//DADOS DEVEM SER CARREGADOS/GUARDADOS NAS FUNÇÕES
	char modo_abertura_valido = '0'; //Poderia ser evitada pela criação de uma struct apenas para erros mas dada a simplicidade do program não é necessário
	FILE * dados;
	FILE * situacao_escolar;

	dados = abrir_ficheiro(DADOS_TXT, "r", modo_abertura_valido);
	situacao_escolar = abrir_ficheiro(SITUACAO_ESCOLAR_TXT, "r", modo_abertura_valido);







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
