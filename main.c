#include "headers.h"
#include "funcoes.c"
//NOTA IMPORTANTE: para dar run temos que abrir o projeto em terminal integrado e depois de estar na main, dar compile run.
//": Para os ficheiros abrirem corretamente têm que estar na pasta output

//NOVA METODOLOGIA:
//Usar ler_linha_txt() para ler qualquer entrada de stdin
//Se necessário usar um scanf, colocar fseek(stdin, 0, SEEK_END); para evitar erros na leitura;

//TODO: 

//As funções de procura com strings não suportam acentos ou ç!!
//NOTA: erros.txt está atualmente em modo w para facilitar debugging, alterar quando já não for necessário
int main() {
	//Copia a data atual para uma variável global.
	data_atual();
    //Colocar a consola em PT-PT (caracteres UTF8)
	colocar_terminal_utf8();

	//Criamos um array de cada struct para armzenar TAMANHO_INICIAL_ARRAYS alunos
	Uni bd; //Pode ser fadcilmente alterado para guardar várias universidades
	bd.aluno = (Estudante *) malloc(TAMANHO_INICIAL_ARRAYS * sizeof(Estudante));
	bd.tamanho_aluno = 0;
	bd.capacidade_aluno = TAMANHO_INICIAL_ARRAYS;

	bd.escolares = (Dados *) malloc(TAMANHO_INICIAL_ARRAYS * sizeof(Dados));
	bd.tamanho_escolares = 0;
	bd.capacidade_escolares = TAMANHO_INICIAL_ARRAYS;

	if (!bd.aluno || !bd.escolares) {
		printf("Ocorreu um erro ao alocar memória para os alunos.\n");
		printf("A encerrar o programa.\n");
		return 1; //Erro
	}

	//Servirá para verificar se o tamanho atual de alunos excede ou não o alocado
	bd.capacidade_aluno = TAMANHO_INICIAL_ARRAYS; //Variável que irá manter o tamanho do array de alunos
	bd.capacidade_escolares = TAMANHO_INICIAL_ARRAYS;

	inicializar_aluno(&bd, bd.tamanho_aluno);
	inicializar_escolares(&bd, bd.tamanho_escolares);
	inicializar_estatisticas(&bd.stats);
	
	carregar_dados(DADOS_TXT, SITUACAO_ESCOLAR_TXT, &bd);
	//Necessário verificar se foram carregados dados, caso não, averiguar o que fazer.

	//"Cérebro" do programa.
	the_architect(&bd);

	//Provavelmente também será necessário dar free em nome e nacionalidade antes
	free(bd.aluno);
	free(bd.escolares);

	
	
	return 0;
 
}