#ifndef HEADERS_H //Se HEADERS_H não foi definida, o código entre o #ifndef e o #endif será processado, caso contrário, será ignorado.
//Ou seja, evita o processamento dos headers várias vezes
#define HEADERS_H //define caso seja a 1x

//Definir os nomes dos ficheiros como constantes(de modo a que não sejam alterados)
#define DADOS_TXT "dados.txt"
#define SITUACAO_ESCOLAR_TXT "situacao_Escolar_Estudantes.txt"
#define TAMANHO_INICIAL_ALUNO 1000
#define TAMANHO_INICIAL_BUFFER 100
#define SEPARADOR '\t'
#define MAX_PARAMETROS 4 //De acordo com os dados atuais, são 4 parametros por linha, caso se aumente, este valor deve aumentar também.
#define PARAMETROS_ESTUDANTE 4
#define PARAMETROS_DADOS_ESCOLARES 5 //parametros a serem lidos, não os na struct
#define MAX_NACIONALIDADES 206 //Número máximo de países
#define MAX_STRING_NACIONALIDADE 100 //Definimos o número máximo de chars que uma nacionalidade pode ter
#define TAMANHO_INICIAL_NOME 50 


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

//Struct para tratar todos os dados relativos aos estudantes
typedef struct estudante {
    int codigo; //int para prevenir, caso o código tenha, imagine-se, 6 digitos
    char * nome; //Declaramos um ponteiro para posteriormente alocar memória dinamicamente consoante o tamanho do nome
    Data nascimento; 
    char * nacionalidade; //Criamos um array do tipo nacionalidade, que irá conter todas as nacionalidades
}Estudante;

typedef struct dados_escolares {
    int codigo;
    short matriculas; 
    short ects; 
    short ano_atual;
    float media_atual;
    char prescrever;
}Dados;

//Struct para todos os dados estatísticos
typedef struct estatisticas {
	float medias_matriculas;
    float media;
	int finalistas;
    float media_idade_nacionalidade;
    float media_idade_ano;
    int risco_prescrever;
}Estatisticas;

typedef struct {
    char menu_atual;   // Identificar o menu (P-Principal, G-Gerir estudantes,C-Consultar Dados, E-Estatísticas, X-Extras, S-Sair)
    char opcao_principal;  // Opção selecionada dentro do menu principal
    char opcao_submenu; //Opção dos submenus
} Escolha;


//Protótipos das funções
void remover_espacos(char * str);
void separar_parametros(const char * linha, char ** parametros, int * num_parametros);
char * ler_linha(FILE * ficheiro, int * n_linhas);
void guardar_dados(const char * nome_ficheiro, Estudante * aluno, Estatisticas * stats);
void inicializar_structs(Estudante * aluno, Dados * escolares, Estatisticas * stats, int n_alunos);

void carregar_dados(const char * nome_ficheiro_dados,const char * nome_ficheiro_escolar, Estudante ** aluno, int * tamanho_alunos, Dados ** escolares, int * tamanho_escolares);
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
int validar_data(short dia, short mes, short ano, const char modo);
void ler_data(Estudante * aluno, char * str, const char modo);

#endif //Termina a condição