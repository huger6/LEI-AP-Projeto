#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <locale.h>

//TODO LIST
/*
-Perguntar à professora da funcionalidade 1: está-nos a ser pedido para carregar todos os dados de uma só vez para o programa? Se sim, isso não seria
...pouco eficiente e não seria melhor carregar os dados apenas quando nos é solicitado algum tipo de dado do ficheiro, onde poderíamos implementar algum
...tipo de merge sort (isto deve fazer parte do menu???)
-Perguntar se podemos pedir a data de nascimento separada ou como uma string
-Perguntar o que querem dizer com mecanismo de avanço página a página
-Os dados devem ficar no ecra? É que estava a pensar em limpar o terminal em cada ida ao menu
*/

//Notas
/*
short - hd
unsigned short - hu
*/

//Structs

typedef struct data_nascimento {
    unsigned int dia, mes, ano; //TODO
}Data;

//Vamos dividir os dados enter uma struct Estudante, para dados pessoais, e outra para dados escolares ()
typedef struct estudante {
    unsigned int codigo; //int para prevenir, caso o código tenha, imagine-se, 
    Data nascimento; 
    char nacionalidade[5]; //Temos 5 nacionalidades. Dentro de cada posição do array colocamos um array de chars com a nacionalidade.
    unsigned short matriculas; //unsigned porque matriculas sempre > 0 e short porque usa apenas 2 bytes em x dos 4 de um int
    unsigned short ects; //Mesma lógica das matriculas
}Estudante;

typedef struct dados_escolares {
    unsigned int codigo;
    unsigned char prescrever;

}Escola;

typedef struct estatisticas {
	float medias_matriculas;
	int finalistas;
}Estatisticas;


//Protótipos das funções
void menu();




//Main
int main() {
    //Colocar a consola em PT-PT (caracteres UTF8)
    SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Portuguese");
}







//Funções

void validacao_input_menu() {
	printf("Entrada inválida!\n");
	while (getchar() != '\n');  // Limpar o buffer de entrada (o que foi escrito incorretamente)
	printf("Pressione Enter para continuar.\n");
	getchar();  // Esperar pelo Enter do utilizador
	system("cls");
}

void validacao_numero_menu() {
	printf("Por favor, escolha um número do menu.\n");
	printf("Pressione Enter para continuar.\n");
	getchar(); //Ler o enter
	system("cls"); 
}

/*
void menu() {
    unsigned short opcao = 0; 
    short valido = 0; //Usamos apenas short devido ao facto do scanf poder retornar -1 (ainda que improvavel), acontece no EOF ou Ctrl Z
		do {
			system("cls"); //Limpar terminal
			printf("\t\tMENU");
			printf("\n\n");
			printf("0 - Sair do programa\n");
			printf("1 - \n");
			printf("2 - Séries\n");
			printf("3 - Estatísticas\n");
			printf("4 - Exportar dados\n");
			printf("\n\nOpção: ");
			valido = scanf("%hu", opcao); //scanf retorna 1 se conseguir ler de acordo com o esperado
			
			if (valido != 1) {
		        validacao_input_menu();
			}
	         
			else if (opcao < 0 || opcao > 4) {
				valido = 0;
				printf("Por favor, escolha um número do menu.\n");
				printf("Pressione Enter para continuar.\n");
	            getchar();  
	            system("cls");  
			}
			
			if ((*primeira_opcao == 0)&&(valido == 1)) {
				exit(0);
			}
		} while (valido == 0);
}
*/


void menu_principal(char * primeira_opcao) {
	short valido = 0; //Usamos apenas short devido ao facto do scanf poder retornar -1 (ainda que improvavel), acontece no EOF ou Ctrl Z
		do {
			//Sleep(1000) espera 1 segundo antes de avançar para tornar o programa mais suave
			system("cls"); //Limpar terminal apenas se voltarmos a escrever o menu (ou seja, já não estão a ser necessárias as informações anteriores)
			printf("\t\tMENU");
			printf("\n\n");
			printf("1 - Gerir estudantes\n");
			printf("2 - Consultar dados\n");
			printf("3 - Listagens\n");
			printf("4 - Estatísticas\n");
			printf("5 - Ficheiros\n");
			printf("6 - Opções\n");
			printf("0 - Sair do programa\n");
			printf("\n\n\tOpção: ");
			valido = scanf("%c", primeira_opcao); //scanf retorna 1 se conseguir ler de acordo com o esperado
			
			if (valido != 1) {
		        validacao_input_menu();
			}
	        
			else if (*primeira_opcao < '0' || *primeira_opcao > '6') { 
				valido = 0; //Scanf leu corretamente e retornou 1, mas como não queremos esses números, voltamos a definir a zero para dizer que é inválido
				validacao_numero_menu(); 
			}
			
			if ((*primeira_opcao == '0')&&(valido == 1)) { //Verifica-se também valido ==1 no caso de ter sido introduzida uma entrada invalida e 2ªopcao nao ter sofrido alteraçoes, ficando a 0
				exit(0); //Averiguar melhor o que fazer aqui
			}
		} while (valido == 0);
}

void menu_gerir_estudantes(char * sair) {
	char segunda_opcao = '0';
	short valido = 0;
	*sair = '0';
	do {
		system("cls");
		printf("\t\tGERIR ESTUDANTES");
		printf("\n\n");
		printf("1 - Inserir estudante\n");
		printf("2 - Eliminar estudante\n");
		printf("3 - Atualizar dados do estudante\n");
		printf("0 - Voltar ao menu anterior\n");
		printf("\n\n\tOpção: ");
		valido = scanf("%c", segunda_opcao);
		
		if (valido != 1) {
	        validacao_input_menu();
		}
        
		if (*segunda_opcao < 0 || *segunda_opcao > 3) {
			valido = 0;
			validacao_numero_menu();
		}
		
		if ((*segunda_opcao == '0')&&(valido == 1)) {
			*sair = '1';
			break;
		}
	} while (valido == 0);	 
}


void menu_consultar_dados(char * sair) {
	char segunda_opcao = '0';
	short valido = 0;
	*sair = '0';
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
		valido = scanf("%c", segunda_opcao);
		
		if (valido != 1) {
	        validacao_input_menu();
		}
        
		if (segunda_opcao < '0' || segunda_opcao > '4') {
			valido = 0;
			validacao_numero_menu();
		}
		
		if ((segunda_opcao == '0')&&(valido == 1)) {
			*sair = '1';
			break;
		}
	} while (valido == 0);	 
}



void escolha_menus() {
	
}
