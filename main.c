#include "headers.h"
#include "funcoes.c"
//NOTA IMPORTANTE: para dar run temos que abrir o projeto em terminal integrado e depois de estar na main, dar compile run.
//": Para os ficheiros abrirem corretamente têm que estar na pasta output
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

	for(int i = 0; i < 10; i++) {
		printf("O código do aluno é %d!\n", aluno[i].codigo);
		printf("O nome do aluno é %s!\n", aluno[i].nome);
		printf("A data de nascimento do aluno é %hd-%hd-%hd!\n", aluno[i].nascimento.dia, aluno[i].nascimento.mes, aluno[i].nascimento.ano);
		printf("A nacionalidade do aluno é %s!\n", aluno[i].nacionalidade);
	}

	for(int i = 0; i < 10; i++) {
		printf("O código do aluno é %d!\n", escolares[i].codigo);
		printf("O número de matrículas do aluno é %hd!\n", escolares[i].matriculas);
		printf("O número de ECTS do aluno é %hd!\n", escolares[i].ects);
		printf("O ano atual do aluno é %hd!\n", escolares[i].ano_atual);
		printf("A média atual do aluno é %.2f!\n", escolares[i].media_atual);
		printf("O aluno está em risco de prescrição? %c\n", escolares[i].prescrever);
	}

	free(aluno);
	free(escolares);
	free(stats);
	
	
	//escolha_menus();
	return 0;

}