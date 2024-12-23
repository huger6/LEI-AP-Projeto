#ifndef HEADERS_H //Se HEADERS_H não foi definida, o código entre o #ifndef e o #endif será processado, caso contrário, será ignorado.
//Ou seja, evita o processamento dos headers várias vezes
#define HEADERS_H 


//Definir os nomes dos ficheiros como constantes(de modo a que não sejam alterados)
#define DADOS_TXT "dados.txt"
#define SITUACAO_ESCOLAR_TXT "situacao_Escolar_Estudantes.txt"
#define ERROS_TXT "erros.txt" //Ficheiro onde serão armazenados todos os erros provenientes da leitura de dados(para evitar a eliminação dos mesmos)
#define TAMANHO_INICIAL_ARRAYS 1000
#define TAMANHO_INICIAL_BUFFER 100
#define SEPARADOR '\t' //Necessário alterar em carregar_dados, nas mensagens de erro, caso seja mudado
#define PARAMETROS_ESTUDANTE 4
#define PARAMETROS_DADOS_ESCOLARES 5 //parametros a serem lidos, não os na struct
#define MAX_NACIONALIDADES 206 //Número máximo de países
#define MAX_STRING_NACIONALIDADE 101 //Definimos o número máximo de chars que uma nacionalidade pode ter
#define TAMANHO_INICIAL_NOME 50 
#define MAX_MATRICULAS 20 //Limite razoável de matrículas(pode ser alterado)
#define MAX_ECTS 400
#define MAX_ANO_ATUAL 8
#define TAMANHO_INICIAL_ERRO 10
#define MAX_INT 2147483647
#define MIN_INT -2147483648
#define MAX_SHORT 32767
#define MIN_SHORT -32768
#define MAX_FORMATO 10 //Ajustar se o formato do ficheiro for superior a 9 caracteres.


//Não usamos o define porque declararia como int, o que derrotaria todo o ponto de usar shorts para poupar memória
extern const short ANO_ATUAL; //definimos o ano atual, ajustar consoante o ano;
extern const short ANO_NASC_LIM_INF; //definimos o limite inferior como o ano de nasc da pessoa mais velha do mundo atualmente

#include <stdio.h>
#include <stdlib.h>
//Para evitar erros em sistemas não Windows, apenas incluímos a biblioteca se estivermos em Windows.
//A variável _WIN32 está presente em todos os sistemas Windows.
#ifdef _WIN32 
    #include <windows.h> 
#endif
#include <locale.h>
#include <string.h>
#include <ctype.h> //Para fazer verificações relativas aos dados introduzidos (isalpha)
#include <locale.h>


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

//Os arrays destas structs DEVEM ser ORDENADOS
typedef struct uni{
    Estudante * aluno;
    int tamanho_aluno; //tamanho atual do array aluno
    int capacidade_aluno; //tamanho alocado do array aluno
    Dados * escolares; 
    int tamanho_escolares;
    int capacidade_escolares;
    Estatisticas stats;
} Uni;

//Protótipos das funções

//Ficheiros e gestão de dados
char * ler_linha_txt(FILE * ficheiro, int * n_linhas);
void carregar_dados(const char * nome_ficheiro_dados, const char * nome_ficheiro_escolar, Uni * bd);
void guardar_dados(const char * nome_ficheiro_dados, const char * nome_ficheiro_escolares, Uni * bd);
FILE * pedir_listagem(char * formato_selecionado);
//Gestão de memória
void inicializar_aluno(Uni * bd, int indice_aluno);
void inicializar_estatisticas(Estatisticas * stats);
int realocar_aluno(Uni * bd, const char modo);
int realocar_escolares(Uni * bd, const char modo);
int realocar_nome(Estudante * aluno, const char modo);
//Procura e validações
int procurar_codigo_aluno(int codigo, Uni * bd);
int procurar_codigo_escolares(int codigo, Uni * bd);
int validar_codigo_ao_inserir(int codigo, Uni * bd);
int validar_codigo_eliminar(int codigo, Uni * bd);
int validar_nome_ficheiro(const char * nome_ficheiro);
FILE * validar_ficheiro_e_abrir(const char * nome);
void verificar_codigos_duplicados(Uni * bd, FILE * erros);
void verificar_codigos_escolares_sem_aluno(Uni * bd, FILE * erros, char * primeiro_erro);
int validar_data(short dia, short mes, short ano, const char modo);
int validar_nome(Estudante * aluno, char * nome, const char modo);
int validar_nacionalidade(char * nacionalidade, const char modo);
void validacao_menus(short * valido, const char opcao, const char limInf, const char limSup);
void validacao_input_menu(const char limInf, const char limSup);
void validacao_numero_menu(const char limInf, const char limSup);
//Ordenação
void merge_aluno(Uni * bd, int inicio, int meio, int fim);
void merge_sort_aluno(Uni * bd, int inicio, int fim);
void merge_escolares(Uni * bd, int inicio, int meio, int fim);
void merge_sort_escolares(Uni * bd, int inicio, int fim);
void ordenar_ao_inserir(int codigo, Uni * bd, int indice_aluno, int indice_escolares);
int ordenar_ao_eliminar(int codigo, Uni * bd);
//Menus
char mostrar_menu(void (*escrever_menu)(), char min_opcao, char max_opcao);
void menu_principal();
void menu_gerir_estudantes();
void menu_consultar_dados();
void menu_estatisticas();
void menu_ficheiros();
void menu_extras();
void menu_dias_da_semana();
void menu_formatos_disponiveis();
void processar_gerir_estudantes(Uni * bd);
void processar_consultar_dados(Uni * bd);
void processar_estatisticas(Uni * bd);
void processar_ficheiros(Uni * bd);
void processar_extras(Uni * bd);
void escolha_menus(Uni * bd);
//Inserção/leitura de dados
void ler_data(Estudante * aluno, char * str, const char modo);
void inserir_estudante(Uni * bd);
void eliminar_estudante(Uni * bd);
//Listagens
void procurar_estudante_por_nome(Uni * bd);
void listar_aniversarios_por_dia(Uni * bd);
void listar_aniversario_ao_domingo(Uni * bd);
//Funções auxiliares
void remover_espacos(char * str);
void separar_parametros(const char * linha, char ** parametros, int * num_parametros);
void limpar_buffer();
void limpar_terminal();
void pressione_enter();
void colocar_terminal_utf8();
void verificar_primeiro_erro(FILE * erros, char * primeiro_erro, const char * nome_ficheiro);
int string_para_int(const char * str, int * resultado);
int string_para_short(const char * str, short * resultado);
int string_para_float(const char * str, float * resultado);
short calcular_dia_da_semana(short dia, int mes, int ano);
int sim_nao();
char obter_separador(FILE * ficheiro, char * formato);

#endif