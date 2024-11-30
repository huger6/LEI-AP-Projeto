#ifndef HEADERS_H //Se HEADERS_H não foi definida, o código entre o #ifndef e o #endif será processado, caso contrário, será ignorado.
//Ou seja, evita o processamento dos headers várias vezes
#define HEADERS_H //define caso seja a 1x

//Definir os nomes dos ficheiros como constantes(de modo a que não sejam alterados)
#define DADOS_TXT "dados.txt"
#define SITUACAO_ESCOLAR_TXT "situacao_Escolar_Estudantes.txt"

//Não usamos o define porque declararia como int, o que derrotaria todo o ponto de usar shorts para poupar memória
extern const short ANO_ATUAL; //definimos o ano atual, ajustar consoante o ano;
extern const short ANO_NASC_LIM_INF; //definimos o limite inferior como o ano de nasc da pessoa mais velha do mundo atualmente

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include <locale.h>
#include <string.h>


//Structs
typedef struct data_nascimento {
    short dia;
    short mes;
    short ano;
}Data;

//Vamos dividir os dados enter uma struct Estudante, para dados pessoais, e outra para dados escolares ()
typedef struct estudante {
    int codigo; //int para prevenir, caso o código tenha, imagine-se, 6 digitos
    char * nome; //Declaramos um ponteiro para posteriormente alocar memória dinamicamente consoante o tamanho do nome
    Data nascimento; 
    char nacionalidade[5]; //Temos 5 nacionalidades. Dentro de cada posição do array colocamos um array de chars com a nacionalidade.
    unsigned short matriculas; //unsigned porque matriculas sempre > 0 e short porque usa apenas 2 bytes em x dos 4 de um int
    unsigned short ects; //Mesma lógica das matriculas
}Estudante;

typedef struct dados_escolares {
    unsigned int codigo;
    unsigned char prescrever;
}Dados;

typedef struct estatisticas {
	float medias_matriculas;
	int finalistas;
}Estatisticas;

typedef struct {
    char menu_atual;   // Identificar o menu (P-Principal, G-Gerir estudantes,C-Consultar Dados, E-Estatísticas, X-Extras, S-Sair)
    char opcao_principal;  // Opção selecionada dentro do menu principal
    char opcao_submenu; //Opção dos submenus
} Escolha;


//Protótipos das funções

void carregar_dados(FILE * ficheiro);
void limpar_buffer();
void validacao_menus(short * valido, const char opcao, const char limInf, const char limSup);
void limpar_terminal();
void validacao_input_menu(const char limInf, const char limSup);
void validacao_numero_menu(const char limInf, const char limSup);
char menu_principal();
char menu_gerir_estudantes();
char menu_consultar_dados();
char menu_estatisticas();
char menu_extras();
void processar_gerir_estudantes(Escolha * escolha);
void processar_consultar_dados(Escolha * escolha);
void processar_estatisticas(Escolha * escolha);
void processar_extras(Escolha * escolha);
Escolha escolha_menus();
char validar_data(short dia, short mes, short ano);
void ler_data(Estudante * aluno);

#endif //Termina a condição