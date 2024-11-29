#include "headers.h"

//Funções
void carregar_dados(FILE * ficheiro) { 
}

void validacao_menus(short * valido, char opcao, const char limInf, const char limSup) { //Lims const pois não devem ser alterados
    if (*valido != 1) {
        validacao_input_menu();
    }
    else if (opcao < limInf || opcao > limSup) { 
        valido = 0; //Scanf leu corretamente e retornou 1, mas como não queremos esses números, voltamos a definir a zero para dizer que é inválido
        validacao_numero_menu(); 
    }
}


//Limpar o terminal consoante o sistema operativo
void limpar_terminal() {
    #ifndef _WIN32 //É automaticamente definido pelo windows
        system("cls"); //Sistemas windows
    #else
        system("clear"); //Sistemas linux, macOs, etc
    #endif
}

void validacao_input_menu() {
	printf("Entrada inválida!\n");
	while (getchar() != '\n');  // Limpar o buffer de entrada (o que foi escrito incorretamente)
	printf("Pressione Enter para continuar.\n");
	getchar();  // Esperar pelo Enter do user
	limpar_terminal();
}

void validacao_numero_menu() {
	printf("Por favor, escolha um número do menu.\n");
	printf("Pressione Enter para continuar.\n");
	getchar(); //Ler o enter
	limpar_terminal(); 
}

char menu_principal() {
	short valido = 0; //Usamos apenas short devido ao facto do scanf poder retornar -1 (ainda que improvavel), acontece no EOF ou Ctrl Z
	char opcao = '0';
		do {
			//Sleep(1000) espera 1 segundo antes de avançar para tornar o programa mais suave | SÓ FUNCIONA EM WINDOWS
			limpar_terminal(); //Limpar terminal apenas se voltarmos a escrever o menu (ou seja, já não estão a ser necessárias as informações anteriores)
			printf("\t\tMENU");
			printf("\n\n");
			printf("1 - Gerir estudantes\n");
			printf("2 - Consultar dados\n");
			printf("3 - Estatísticas\n");
			printf("4 - Extras\n");
			// printf("5 - Opções\n"); talvez possamos implementar um modo de nacionalidade (apresentar o programa em inglês, etc)
			printf("0 - Sair do programa\n");
			printf("\n\n\tOpção: ");
            //rever
			valido = scanf("%c", &opcao); //scanf retorna 1 se conseguir ler de acordo com o esperado
			
			validacao_menus(&valido,opcao,'0','4');
			
			if ((opcao == '0')&&(valido == 1)) { //Verifica-se também valido == 1 no caso de ter sido introduzida uma entrada invalida e 2ªopcao nao ter sofrido alteraçoes, ficando a 0
				return 0; 
			}
		} while (valido == 0);
}

//Nota: poderíamos ter optado por fazer várias funções apenas com o output do menu, mas visto que teríamos ainda de fazer diferentes funções
//devido a ter que gerir as opções e os números, seria adicionar complexidade desnecessária, do nosso ponto de vista
void menu_gerir_estudantes() { 
	char opcao = '0';
	short valido = 0;
	do {
		system("cls");
		printf("\t\tGERIR ESTUDANTES");
		printf("\n\n");
		printf("1 - Inserir estudante\n");
		printf("2 - Eliminar estudante\n");
		printf("3 - Atualizar dados do estudante\n");
		printf("0 - Voltar ao menu anterior\n");
		printf("\n\n\tOpção: ");
		valido = scanf("%c", &opcao);
		
		validacao_menus(&valido,opcao,'0','3');
		
		if ((opcao == '0')&&(valido == 1)) {
			break; //Sai do loop(volta ao menu principal)
		}
	} while (valido == 0);	 
}


void menu_consultar_dados() {
	char opcao = '0';
	short valido = 0;
	do {
		system("cls");
		printf("\t\tCONSULTAR DADOS");
		printf("\n\n");
		printf("1 - Procurar estudante por nome\n");
		printf("2 - Listar estudantes por intervalo de datas de nascimento\n");
		printf("3 - Listar estudantes por nacionalidade\n");
		printf("4 - Listar estudantes por ordem alfabética de apelido\n");
		printf("0 - Voltar ao menu anterior\n");
		printf("\n\n\tOpção: ");
		valido = scanf("%c", &opcao);
		
		if (valido != 1) {
	        validacao_input_menu();
		}
        
		if (opcao < '0' || opcao > '4') {
			valido = 0;
			validacao_numero_menu();
		}
		
		if ((opcao == '0')&&(valido == 1)) {
			*sair = '1';
			break;
		}
	} while (valido == 0);	 
}

void menu_estatisticas() {
	char opcao = '0';
	short valido = 0;
	do {
		system("cls");
		printf("\t\tESTATÍSTICAS");
		printf("\n\n");
		printf("1 - Contar estudantes por escalão de média atual\n");
		printf("2 - Calcular número médio de matrículas(geral e por nacionalidade)\n");
		printf("3 - Determinar número de finalistas\n");
		printf("4 - Calcular média de idades por nacionalidade e ano\n");
		printf("5 - Listar estudantes em risco de prescrição\n");
		printf("0 - Voltar ao menu anterior\n");
		printf("\n\n\tOpção: ");
		valido = scanf("%c", &opcao);
		
		if (valido != 1) {
	        validacao_input_menu();
		}
        
		if (opcao < '0' || opcao > '5') {
			valido = 0;
			validacao_numero_menu();
		}
		
		if ((opcao == '0')&&(valido == 1)) {
			*sair = '1';
			break;
		}
	} while (valido == 0);	 
}

void menu_extras() {
	char opcao = '0';
	short valido = 0;
	do {
		system("cls");
		printf("\t\tEXTRAS");
		printf("\n\n");
		printf("1 - Listar estudantes nascidos em dias específicos da semana\n");
		printf("2 - Listar os estudantes cujo aniversário num determinado ano é ao domingo\n");
		printf("3 - Relacionar o ano de inscrição com intervalos das classificações\n");
		printf("0 - Voltar ao menu anterior\n");
		printf("\n\n\tOpção: ");
		valido = scanf("%c", &opcao);
		
		if (valido != 1) {
	        validacao_input_menu();
		}
        
		if (opcao < '0' || opcao > '3') {
			valido = 0;
			validacao_numero_menu();
		}
		
		if ((opcao == '0')&&(valido == 1)) {
			*sair = '1';
			break;
		}
	} while (valido == 0);	 
}


void escolha_menus() {
	
}

//TODO
void ler_data(Estudante * aluno, Data * date) {
	Data nascimento;
	char data[11]; //Vamos usar o formato DD/MM/AAAA (10 caracteres + \0)

	while(1) {
		printf("Data de nascimento (DD/MM/AAAA): ");
		if(fgets(data, sizeof(data), stdin) !=NULL); //stdin porque é daí que lemos os dados; //fgets retorna um ponteiro para "data", logo verificamos se não é NULL
	}
}