#include "headers.h"
#include "funcoes.c"
//NOTA IMPORTANTE: para dar run temos que abrir o projeto em terminal integrado e depois de estar na main, dar compile run.
//": Para os ficheiros abrirem corretamente têm que estar na pasta output

//TODO AMANHA: Função inserir_estudante; fazer o inicializar_structs funcionar de forma generalizada, para poder inicializar os dados sempre que se faz um realloc
//Na função carregar dados, não são inicializadas algumas casas do array  (VER COMENTÁRIO NO CÓDIGO)
int main() {
	int tamanho_aluno, tamanho_escolares, tamanho_stats;
    //Colocar a consola em PT-PT (caracteres UTF8)
	colocar_terminal_pt();
	
	//Criamos um array de cada struct para armzenar TAMANHO_INICIAL_ALUNO alunos
	Estudante * aluno = (Estudante *) malloc(TAMANHO_INICIAL_ALUNO * sizeof(Estudante));
	Dados * escolares = (Dados *) malloc(TAMANHO_INICIAL_ALUNO * sizeof(Dados));
	Estatisticas * stats = (Estatisticas *) malloc(TAMANHO_INICIAL_ALUNO * sizeof(Estatisticas));

	if (!aluno || !escolares || !stats) {
		printf("Ocorreu um erro ao alocar memória para os alunos.\n");
		printf("A encerrar o programa.\n");
		return 1; //Erro
	}

	inicializar_structs(aluno, escolares, stats, TAMANHO_INICIAL_ALUNO);
	//Servirá para verificar se o tamanho atual de alunos excede ou não o alocado
	tamanho_aluno = TAMANHO_INICIAL_ALUNO;
	tamanho_escolares = TAMANHO_INICIAL_ALUNO;
	tamanho_stats = TAMANHO_INICIAL_ALUNO;
	//Até aqui está tudo correto
	carregar_dados(DADOS_TXT, SITUACAO_ESCOLAR_TXT, &aluno, &tamanho_aluno, &escolares, &tamanho_escolares);

	escolha_menus();

	free(aluno);
	free(escolares);
	free(stats);
	
	
	//escolha_menus();
	return 0;

}