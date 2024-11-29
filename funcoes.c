#include "headers.h"

//Funções
void carregar_dados(FILE * ficheiro) { 

}

void validacao_menus(short * valido, char opcao, const char limInf, const char limSup) { //Lims const pois não devem ser alterados
    if (*valido != 1) {
        validacao_input_menu();
    }
    else if (opcao < limInf || opcao > limSup) { 
        *valido = 0; //Scanf leu corretamente e retornou 1, mas como não queremos esses números, voltamos a definir a zero para dizer que é inválido
        validacao_numero_menu(); 
    }
}


//Limpar o terminal consoante o sistema operativo
void limpar_terminal() {
    #ifdef _WIN32 //É automaticamente definido pelo windows
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
        
        if (valido == 1) { //Se for valido, então retornamos o valor escolhido
            return opcao;
        }
    } while (valido == 0);
}

//Nota: poderíamos ter optado por fazer várias funções apenas com o output do menu, mas visto que teríamos ainda de fazer diferentes funções
//devido a ter que gerir as opções e os números, estariamos a adicionar complexidade desnecessária, do nosso ponto de vista
char menu_gerir_estudantes() { 
	char opcao = '0';
	short valido = 0;
	do {
		limpar_terminal();
		printf("\t\tGERIR ESTUDANTES");
		printf("\n\n");
		printf("1 - Inserir estudante\n");
		printf("2 - Eliminar estudante\n");
		printf("3 - Atualizar dados do estudante\n");
		printf("0 - Voltar ao menu anterior\n");
		printf("\n\n\tOpção: ");
		valido = scanf("%c", &opcao);
		
		validacao_menus(&valido,opcao,'0','3');
		
		if (valido == 1) {
            return opcao;
		}
	} while (valido == 0);	 
}

char menu_consultar_dados() {
	char opcao = '0';
	short valido = 0;
	do {
        limpar_terminal();
		printf("\t\tCONSULTAR DADOS");
		printf("\n\n");
		printf("1 - Procurar estudante por nome\n");
		printf("2 - Listar estudantes por intervalo de datas de nascimento\n");
		printf("3 - Listar estudantes por nacionalidade\n");
		printf("4 - Listar estudantes por ordem alfabética de apelido\n");
		printf("0 - Voltar ao menu anterior\n");
		printf("\n\n\tOpção: ");
		valido = scanf("%c", &opcao);
		
		validacao_menus(&valido,opcao,'0','4');
		
		if (valido == 1) {
			return opcao;
		}
	} while (valido == 0);	 
}

char menu_estatisticas() {
	char opcao = '0';
	short valido = 0;
	do {
        limpar_terminal();
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
		
		validacao_menus(&valido,opcao,'0','5');
		
		if (valido == 1) {
			return opcao;
		}
	} while (valido == 0);	 
}

char menu_extras() {
	char opcao = '0';
	short valido = 0;
	do {
        limpar_terminal();
		printf("\t\tEXTRAS");
		printf("\n\n");
		printf("1 - Listar estudantes nascidos em dias específicos da semana\n");
		printf("2 - Listar os estudantes cujo aniversário num determinado ano é ao domingo\n");
		printf("3 - Relacionar o ano de inscrição com intervalos das classificações\n");
		printf("0 - Voltar ao menu anterior\n");
		printf("\n\n\tOpção: ");
		valido = scanf("%c", &opcao);
		
		validacao_menus(&valido,opcao,'0','3');
        
		if (valido == 1) {
			return opcao;
		}
	} while (valido == 0);	 
}

void processar_gerir_estudantes(Escolha opcao, Escolha * n_menu) {
    do {
        opcao = menu_gerir_estudantes();
        switch(opcao) {
            case '0':
                *n_menu = 'P';
                break;
            case '1':
                //inserir estudante
                break;
            case '2':
                //eliminar estudante
                break;
            case '3':
                //atualizar estudante
                break;
            default:
                printf("Erro!!"); //Aprofundar
                break;
    }
    } while (*n_menu == 'G');
}

void processar_consultar(Escolha opcao, Escolha * n_menu) {
    do {
        opcao = menu_consultar_dados();
        switch(opcao) {
            case '0':
                *n_menu = 'P';
                break;
            case '1':
                //Procurar estudante por nome
                break;
            case '2':
                //Listar estudantes por intervalo de datas de nascimento
                break;
            case '3':
                //Listar estudantes por nacionalidade
                break;
            case '4':
                //Listar estudantes por ordem alfabética de apelido
                break;
            default:
                printf("Erro!!"); //Aprofundar
                break;
    }
    } while (*n_menu == 'C')
}

void processar_estatisticas(Escolha opcao, Escolha * n_menu) {
    do {
        opcao = menu_estatisticas();
        switch(opcao) {
            case '0':
                *n_menu = 'P';
                break;
            case '1':
                //Contar estudantes por escalão de média atual
                break;
            case '2':
                //Calcular número médio de matrículas(geral e por nacionalidade)
                break;
            case '3':
                //Determinar número de finalistas
                break;
            case '4':
                //Calcular média de idades por nacionalidade e ano
                break;
            case '5':
                //Listar estudantes em risco de prescrição
                break;
            default:
                printf("Erro!!"); //Aprofundar
                break;
    }
    } while (*n_menu == 'E')
}

void processar_extras(Escolha opcao, Escolha * n_menu) {
    do {
        opcao = menu_extras();
        switch(opcao) {
            case '0':
                *n_menu = 'P';
                break;
            case '1':
                //Listar estudantes nascidos em dias específicos da semana
                break;
            case '2':
                //Listar os estudantes cujo aniversário num determinado ano é ao domingo
                break;
            case 3:
                //Relacionar o ano de inscrição com intervalos das classificações
                break;
            default:
                printf("Erro!!"); //Aprofundar
                break;
    }
    } while (*n_menu == 'X');
}

Escolha escolha_menus() {
	Escolha escolha; //C é case sensitive (pode parecer má prática mas estava difícil de encontrar outro nome igualmente expressivo :))

    escolha.menu_atual = 'P';
    escolha.opcao_principal = '0';
    escolha.opcao_submenu = '0'; //Esta variavel nunca é alterada fora das funcoes, ver isto

    do {
        limpar_terminal();
        escolha.opcao = menu_principal();

        switch(escolha.opcao) {
            case '1':
                escolha.menu_atual = 'G';
                processar_gerir_estudantes(escolha.opcao_submenu, &escolha.menu_atual); //Passamos menu atual por referência no caso da opção escolhida ser voltar atrás
                break;
            case '2':
                escolha.menu_atual = 'C';
                processar_consultar_dados(escolha.opcao_submenu, &escolha.menu_atual);
                break;
            case '3':
                escolha.menu_atual = 'E';
                processar_estatisticas(escolha.opcao_submenu, &escolha.menu_atual);
                break;
            case '4':
                escolha.menu_atual = 'X';
                processar_extras(escolha.opcao_submenu, &escolha.menu_atual);
                break;
            case '0':
                escolha.menu_atual = 'S'; //Indicar que estamos a sair, para o loop
                break;
            default:
                printf("Erro\n"); //Aprofundar no erro
                break;
        }
    } while (escolha.menu_atual == 'P'); 
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