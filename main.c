#include "headers.h"

int main(void) {
	//Limpar quaisquer resíduos de iterações anteriores
	limpar_terminal();
	//Copia a data atual para uma variável global
	data_atual();
    //Colocar a consola em PT-PT (caracteres UTF8)
	colocar_terminal_utf8();
  
	Uni bd; //Pode ser fadcilmente alterado para guardar várias universidades

	//Verificar estado do programa

	//Primeira iteração do programa
	if (fase_instalacao(CONFIG_TXT, '0') == 1) {

		//Alocar memória para a struct de Estudante
		bd.aluno = (Estudante *) malloc(TAMANHO_INICIAL_ARRAYS * sizeof(Estudante));
		bd.tamanho_aluno = 0;
		bd.capacidade_aluno = TAMANHO_INICIAL_ARRAYS;

		if (!bd.aluno) {
			printf("Ocorreu um erro ao alocar memória para os alunos.\n");
			printf("A encerrar o programa.\n");
			//Não há free porque nada foi alocado
			exit(EXIT_FAILURE);
		}

		//Alocar memória para a struct de Dados
		bd.escolares = (Dados *) malloc(TAMANHO_INICIAL_ARRAYS * sizeof(Dados));
		bd.tamanho_escolares = 0;
		bd.capacidade_escolares = TAMANHO_INICIAL_ARRAYS;

		if (!bd.escolares) {
			printf("Ocorreu um erro ao alocar memória para os dados escolares.\n");
			printf("A encerrar o programa.\n");
			free(bd.aluno); //Não há nomes/nac alocados
			exit(EXIT_FAILURE);
		}

		//Inicializar as structs
		inicializar_aluno(&bd, bd.tamanho_aluno);
		inicializar_escolares(&bd, bd.tamanho_escolares);
		inicializar_estatisticas(&bd.stats);

		//Carregar dados dos ficheiros .txt
		if (!carregar_dados_txt(DADOS_TXT, SITUACAO_ESCOLAR_TXT, &bd)) {
			//Tentar usar os backups em caso de erro
			if (!carregar_dados_txt(DADOS_BACKUP_TXT, SITUACAO_ESCOLAR_BACKUP_TXT, &bd)) {
				print_falha_carregar_dados();

				free_tudo(&bd);
				exit(EXIT_FAILURE);
			}
			print_uso_backup();
		}
		else {
			//Se os dados foram bem carregados, guarda-se um backup (automaticamente)
			printf("Foram guardados alguns backups:\n");
			guardar_dados_txt(DADOS_BACKUP_TXT, SITUACAO_ESCOLAR_BACKUP_TXT, &bd);
		}
		//Abrir o ficheiro flag (não é aberto antes para evitar ter de o fechar, em caso de erro)
		(void) fase_instalacao(CONFIG_TXT, '1'); //void para o compilador não reclamar
		guardar_dados_bin(LOGS_BIN, &bd, '0'); //Guardar os dados em binário, caso o utilizador decida sair do programa forçadamente
	}
	else {
		//Carregar os dados de forma binária
		//Nota: A memória é toda alocada em carregar_dados_bin
		if(!carregar_dados_bin(LOGS_BIN, &bd)) {
			//Tentar usar os backups
			if (!carregar_dados_bin(LOGS_BACKUP_BIN, &bd)) { 
				print_falha_carregar_dados();

				//Sair do programa para evitar um softloc
				printf("\nComo medida de segurança, a aplicação será reposta ao seu estado inicial antes de encerrar.\n");
				printf("Por favor siga todas as instruções.\n");
				pressione_enter();
				repor_estado_inicial(&bd);
			} 
			else {
				//Guardar para termos sempre 2 dados
				guardar_dados_bin(LOGS_BIN, &bd, '0'); 
				print_uso_backup();
			}
		}
	}

	//"Cérebro" do programa: apresenta os menus e trata das operações
	the_architect(&bd);

	//Apenas se guarda se o autosave estiver desligado 
	//Caso contrário, já foi guardado ao entrar no menu principal e escusamos guardar 2x
	if (autosaveON == '0') {
		guardar_dados_bin(LOGS_BIN, &bd, '0');
	}
	//O backup no entanto só se guarda aqui (porque não depende de autosave)
	guardar_dados_bin(LOGS_BACKUP_BIN, &bd, '0');

	//Libertar a memória alocada e sair do programa
	free_tudo(&bd);
	exit(EXIT_SUCCESS);
}