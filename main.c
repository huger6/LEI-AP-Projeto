#include "headers.h"
#include "funcoes.c"

//TODO PRO ANO: usar a funcao nova de limpar_buffer para validar inputs de forma mais controlada.
//As funções de procura com strings não suportam acentos ou ç!!
//NOTA: erros.txt está atualmente em modo w para facilitar debugging, alterar quando já não for necessário
int main() {
	//Limpar quaisquer resíduos de iterações anteriores
	limpar_terminal();
	//Copia a data atual para uma variável global.
	data_atual();
    //Colocar a consola em PT-PT (caracteres UTF8)
	colocar_terminal_utf8();

	//Criamos um array de cada struct para armzenar TAMANHO_INICIAL_ARRAYS alunos
	Uni bd; //Pode ser fadcilmente alterado para guardar várias universidades

	/*
				FASE DE INSTALAÇÃO DO PROGRAMA

	TEMOS DE:
	-Verificar se é a primeira abertura do programa. Se for, carregar .txt, se não carregar .bat
	-Dar opções ao user de guardar os dados automaticamente (autosave) - opcional
	-autosave seria basicamente guardar sempre que entrarmos em cada menu
	-Oferecer a possibilidade de guardar os dados em binário
	-Talvez seja benéfico mudar as funções de carregar para int para devolver algum código de erro e efetuar validações
	*/

	if (fase_instalacao(CONFIG_TXT, '0') == 1) {
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
			exit(EXIT_FAILURE);
		}

		inicializar_aluno(&bd, bd.tamanho_aluno);
		inicializar_escolares(&bd, bd.tamanho_escolares);
		inicializar_estatisticas(&bd.stats);
		if (!carregar_dados_txt(DADOS_TXT, SITUACAO_ESCOLAR_TXT, &bd)) {
			if (!carregar_dados_txt(DADOS_BACKUP_TXT, SITUACAO_ESCOLAR_BACKUP_TXT, &bd)) {
				print_falha_carregar_dados();

				free_tudo(&bd);
				exit(EXIT_FAILURE);
			}
			print_uso_backup();
		}
		else {
			//Se os dados foram bem carregados, vamos guardar um backup
			printf("Informação sobre os backups efetuados:\n");
			guardar_dados_txt(DADOS_BACKUP_TXT, SITUACAO_ESCOLAR_BACKUP_TXT, &bd);
		}
		//Abrir o ficheiro flag (não é aberto antes para evitar ter de o fechar, em caso de erro)
		(void) fase_instalacao(CONFIG_TXT, '1'); //void para o compilador não reclamar
		guardar_dados_bin(LOGS_BIN, &bd, '0'); //Guardar os dados em binário, caso o utilizador decida sair do programa forçadamente
	}	
	else {
		//A memória é toda alocada em carregar_dados_bin
		if(!carregar_dados_bin(LOGS_BIN, &bd)) {
			if (!carregar_dados_bin(LOGS_BACKUP_BIN, &bd)) { //tentar usar o backup
				print_falha_carregar_dados();

				free_tudo(&bd); //checksum errado (outros dá free(NULL), o que não é crítico)
				exit(EXIT_FAILURE);
			}
			guardar_dados_bin(LOGS_BIN, &bd, '0'); //Guardar para termos sempre 2 dados
			print_uso_backup();
		}
	}

	//"Cérebro" do programa.
	the_architect(&bd);

	//Apenas se guarda se o autosave estiver desligado 
	//Caso contrário, já foi guardado ao entrar no menu principal
	if (autosaveON == '0') {
		guardar_dados_bin(LOGS_BIN, &bd, '0');
	}
	//O backup no entanto só se guarda aqui
	guardar_dados_bin(LOGS_BACKUP_BIN, &bd, '0');

	//Libertar a memória alocada a sair do programa.
	free_tudo(&bd);
	exit(EXIT_SUCCESS);
}