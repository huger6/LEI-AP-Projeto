#include "headers.h"
#include "funcoes.c"
//NOTA IMPORTANTE: para dar run temos que abrir o projeto em terminal integrado e depois de estar na main, dar compile run.
//": Para os ficheiros abrirem corretamente têm que estar na pasta output
//carregar_dados: ao lidar com erros, só incrementamos indice se nao houver erro

//TODO: linha 978 funcoes.c ; rever inicializar_structs
int main() {
	int tamanho_aluno, tamanho_escolares;
	int indice_atual = 0; //Vai ser útil para gerir mais eficientemente a introdução/eliminação de estudantes
    //Colocar a consola em PT-PT (caracteres UTF8)
	colocar_terminal_utf8();
	
	//Criamos um array de cada struct para armzenar TAMANHO_INICIAL_ARRAYS alunos
	Uni bd;
	(Estudante) bd.aluno = (Estudante *) malloc(TAMANHO_INICIAL_ARRAYS * sizeof(Estudante));
	bd.tamanho_aluno = 0;
	bd.capacidade_aluno = TAMANHO_INICIAL_ARRAYS;
	(Dados) bd.escolares = (Dados *) malloc(TAMANHO_INICIAL_ARRAYS * sizeof(Dados));
	bd.tamanho_escolares = 0;
	bd.capacidade_escolares = TAMANHO_INICIAL_ARRAYS;
	(Estatisticas) bd.stats = (Estatisticas *) malloc(sizeof(Estatisticas)); //Apenas vai haver uma aba de estatisticas, a não ser que entre em jogo outra struct, por exemplo, escola

	if (!bd.aluno || !bd.escolares || !bd.stats) {
		printf("Ocorreu um erro ao alocar memória para os alunos.\n");
		printf("A encerrar o programa.\n");
		return 1; //Erro
	}

	inicializar_structs(bd, indice_atual);
	//Servirá para verificar se o tamanho atual de alunos excede ou não o alocado
	tamanho_aluno = TAMANHO_INICIAL_ARRAYS; //Variável que irá manter o tamanho do array de alunos
	tamanho_escolares = TAMANHO_INICIAL_ARRAYS;
	//Até aqui está tudo correto
	carregar_dados(DADOS_TXT, SITUACAO_ESCOLAR_TXT, &aluno, &escolares, &tamanho_aluno);

	escolha_menus();

	free(aluno);
	free(escolares);
	free(stats);
	
	
	return 0;

}