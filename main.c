#include "headers.h"
#include "funcoes.c"
//NOTA IMPORTANTE: para dar run temos que abrir o projeto em terminal integrado e depois de estar na main, dar compile run.
//": Para os ficheiros abrirem corretamente têm que estar na pasta output
//carregar_dados: ao lidar com erros, só incrementamos indice se nao houver erro

//Na função carregar dados, não são inicializadas algumas casas do array  (VER COMENTÁRIO NO CÓDIGO)
//Tratar dos casos em que há estudantes em Estudante que não estão em Dados e vice versa
int main() {
	int tamanho_aluno;
	int indice_atual = 0; //Vai ser útil para gerir mais eficientemente a introdução/eliminação de estudantes
    //Colocar a consola em PT-PT (caracteres UTF8)
	colocar_terminal_pt();
	
	//Criamos um array de cada struct para armzenar TAMANHO_INICIAL_ALUNO alunos
	Estudante * aluno = (Estudante *) malloc(TAMANHO_INICIAL_ALUNO * sizeof(Estudante));
	Dados * escolares = (Dados *) malloc(TAMANHO_INICIAL_ALUNO * sizeof(Dados));
	Estatisticas stats = (Estatisticas *) malloc(sizeof(Estatisticas)); //Apenas vai haver uma aba de estatisticas, a não ser que entre em jogo outra struct, por exemplo, escola

	if (!aluno || !escolares || !stats) {
		printf("Ocorreu um erro ao alocar memória para os alunos.\n");
		printf("A encerrar o programa.\n");
		return 1; //Erro
	}

	inicializar_structs(aluno, escolares, stats, indice_atual, TAMANHO_INICIAL_ALUNO);
	//Servirá para verificar se o tamanho atual de alunos excede ou não o alocado
	tamanho_aluno = TAMANHO_INICIAL_ALUNO; //Variável que irá manter o tamanho do array de alunos
	//tamanho_escolares = TAMANHO_INICIAL_ALUNO;
	//tamanho_stats = TAMANHO_INICIAL_ALUNO;
	//Até aqui está tudo correto
	carregar_dados(DADOS_TXT, SITUACAO_ESCOLAR_TXT, &aluno, &escolares, &tamanho_aluno);

	escolha_menus();

	free(aluno);
	free(escolares);
	free(stats);
	
	
	return 0;

}