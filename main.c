#include "headers.h"
#include "funcoes.c"

//TODO PRO ANO: usar a funcao nova de limpar_buffer para validar inputs de forma mais controlada.
//As funções de procura com strings não suportam acentos ou ç!!
//NOTA: erros.txt está atualmente em modo w para facilitar debugging, alterar quando já não for necessário
int main() {
	//Copia a data atual para uma variável global.
	data_atual();
    //Colocar a consola em PT-PT (caracteres UTF8)
	colocar_terminal_utf8();

	//Criamos um array de cada struct para armzenar TAMANHO_INICIAL_ARRAYS alunos
	Uni bd; //Pode ser fadcilmente alterado para guardar várias universidades

	/*
				FASE DE INSTALAçÃO DO PROGRAMA

	TEMOS DE:
	-Verificar se é a primeira abertura do programa. Se for, carregar .txt, se não carregar .bat
	-Dar opções ao user de guardar os dados automaticamente (autosave) - opcional
	-autosave seria basicamente guardar sempre que entrarmos em cada menu
	-Oferecer a possibilidade de guardar os dados em binário
	-Talvez seja benéfico mudar as funções de carregar para int para devolver algum código de erro e efetuar validações
	*/

	if (fase_instalacao(INSTALACAO_TXT) == 1) {
		//É necessário alocar tudo do ínicio
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
		carregar_dados_txt(DADOS_TXT, SITUACAO_ESCOLAR_TXT, &bd);
	}	
	else {
		//A memória é toda alocada em carregar_dados_bin
		carregar_dados_bin(LOGS_BIN, &bd);
	}


	//Necessário verificar se foram carregados dados, caso não, averiguar o que fazer.

	//"Cérebro" do programa.
	the_architect(&bd);
	guardar_dados_bin(LOGS_BIN, &bd, '1');
	//Provavelmente também será necessário dar free em nome e nacionalidade antes
	free_aluno(&bd);
	free(bd.aluno);
	free(bd.escolares);

	
	
	return 0;
 
}