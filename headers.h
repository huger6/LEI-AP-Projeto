#ifndef HEADERS_H //Se HEADERS_H não foi definida, o código entre o #ifndef e o #endif será processado, caso contrário, será ignorado.
//Ou seja, evita o processamento dos headers várias vezes
#define HEADERS_H 


//Definir os nomes dos ficheiros como constantes(de modo a que não sejam alterados)
#define DADOS_TXT "dados.txt"
#define DADOS_BACKUP_TXT "dados_backup.txt"
#define SITUACAO_ESCOLAR_TXT "situacao_Escolar_Estudantes.txt"
#define SITUACAO_ESCOLAR_BACKUP_TXT "situacao_Escolar_Estudantes_backup.txt"
#define ERROS_TXT "erros.txt" //Ficheiro onde serão armazenados todos os erros provenientes da leitura de dados(para evitar a eliminação dos mesmos)
#define CONFIG_TXT "config.txt" //flag para verificar estado do programa ao abrir
#define LOGS_BIN "logs.bin"
#define LOGS_BACKUP_BIN "logs_backup.bin"
#define TAMANHO_INICIAL_ARRAYS 1000
#define TAMANHO_INICIAL_BUFFER 100
#define SEPARADOR '\t' //Necessário alterar em carregar_dados, nas mensagens de erro, caso seja mudado
#define PARAMETROS_ESTUDANTE 4
#define PARAMETROS_DADOS_ESCOLARES 5 //parametros a serem lidos, não os na struct
#define IDADE_MINIMA 12
#define IDADE_MAXIMA 80
#define ANO_MIN 1940
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
#define CREDITOS_FINALISTA 154 //Créditos necessários para ser finalista.
#define ECTS_3MATRICULAS 60
#define ECTS_4MATRICULAS 120
#define MAX_INTERVALOS 6
#define MAX_NACIONALIDADES_PEDIDA 5
#define TAMANHO_SUGESTOES 5
#define DIAS_QUARESMA 46
#define ANOS_AVANCO_PROCURAS 5 //Anos em que o utilizador pode procurar mais à frente, como na quaresma
#define PAUSA_LISTAGEM 20
#define MAX_ELIMINACOES_POR_INTERVALO 400

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
#include <time.h> //Para calcular idades
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
    char finalista;
}Dados;

//Struct para os dados estatísticos gerais. Podem ser úteis em outros campos.
typedef struct estatisticas {
	float media_matriculas;
    float media;
	int finalistas;
    int risco_prescrever;
    char atualizado; //'0' - não; '1' - sim; Evita muitas iterações O(n) desnecessárias;
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

//Variável global para assinalar a Data atual.
//Nota: usa-se extern para permitir partilhar a variável entre todos os ficheiros do programa.
//Só declaramos aqui porque acima deu erro (não reconhecia o tipo Data)
//Apenas se declara, é necessário inicializar a variável noutro ficheiro.
extern Data DATA_ATUAL; //Não se usa const porque temos de a ir modificar logo no ínicio.
extern char autosaveON;

//Protótipos das funções

//Ficheiros e gestão de dados
char * ler_linha_txt(FILE * ficheiro, int * n_linhas);
int carregar_dados_txt(const char * nome_ficheiro_dados, const char * nome_ficheiro_escolar, Uni * bd);
int carregar_dados_bin(const char * nome_ficheiro, Uni * bd);
int ler_dados_binarios(void * ptr, size_t tamanho, size_t cont, FILE * ficheiro);
void guardar_dados_txt(const char * nome_ficheiro_dados, const char * nome_ficheiro_escolares, Uni * bd);
void guardar_dados_bin(const char * nome_ficheiro, Uni * bd, const char modo);
void autosave(Uni * bd);
int fase_instalacao(const char * flag, const char abrir);
int eliminar_ficheiro(const char * nome, const char modo);
FILE * pedir_listagem(char * formato_selecionado);
int verificar_extensao(const char * nome_ficheiro);
void mostrar_dados_ficheiro(const char * nome_ficheiro);
void repor_estado_inicial(Uni * bd);
//Gestão de memória
void inicializar_aluno(Uni * bd, int indice_aluno);
void inicializar_escolares(Uni * bd, int indice_escolares);
void inicializar_estatisticas(Estatisticas * stats);
void free_nome_nacionalidade(Uni * bd);
void free_tudo(Uni * bd);
int realocar_aluno(Uni * bd, const char modo);
int realocar_escolares(Uni * bd, const char modo);
int realocar_nome(Estudante * aluno, const char modo);
//Procura e validações
int procurar_codigo_aluno(int codigo, Uni * bd);
int procurar_codigo_escolares(int codigo, Uni * bd);
char ** procurar_nacionalidades(Uni * bd, const short n_nacionalidades, char * mensagem);
int validar_codigo_ao_inserir(int codigo, Uni * bd);
int validar_codigo_eliminar(int codigo, Uni * bd, const char modo);
int validar_nome_ficheiro(const char * nome_ficheiro);
FILE * validar_ficheiro_e_abrir(const char * nome);
void verificar_codigos_duplicados(Uni * bd, FILE * erros);
void verificar_codigos_escolares_sem_aluno(Uni * bd, FILE * erros, char * primeiro_erro);
int validar_data(short dia, short mes, short ano, const char modo);
int validar_data_entre_intervalo(Data inferior, Data superior, Data atual);
int comparar_data(Data d1, Data d2, const char ignorar_ano);
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
int ordenar_ao_eliminar(int codigo, Uni * bd, const char modo);
//Menus
char mostrar_menu(void (*escrever_menu)(), char min_opcao, char max_opcao);
void menu_principal();
void menu_gerir_estudantes();
void menu_consultar_dados();
void menu_estatisticas();
void menu_ficheiros();
void menu_opcoes();
void menu_aniversarios();
void menu_dias_da_semana();
void menu_formatos_disponiveis();
void menu_media_matriculas();
void guia_de_utilizacao();
void the_architect(Uni * bd);
void processar_gerir_estudantes(Uni * bd);
void processar_consultar_dados(Uni * bd);
void processar_estatisticas(Uni * bd);
void processar_ficheiros(Uni * bd);
void processar_aniversarios(Uni * bd);
void processar_opcoes(Uni * bd);
//Inserção/leitura de dados
void ler_data(Data * aluno, char * str, const char modo);
void inserir_estudante(Uni * bd);
void eliminar_estudante(Uni * bd);
//Estatísticas
void calcular_estatisticas(Uni * bd);
void calcular_media_matriculas(Uni * bd);
int alunos_por_media_e_ano(Uni * bd, float media_min, float media_max, short ano_atual);
void tabela_idade_por_escalao(Uni * bd);
void tabela_medias_ano(Uni * bd);
void media_idades_por_nacionalidade_e_ano(Uni * bd);
//Listagens
void listar(Uni * bd, int indice_aluno, FILE * ficheiro, char separador, short * contador);
void listar_info_estudante(Uni * bd);
void procurar_estudante_por_nome(Uni * bd);
void listar_apelidos_alfabeticamente(Uni * bd);
void listar_aniversarios_por_dia(Uni * bd);
void listar_aniversario_ao_domingo(Uni * bd);
void listar_aniversario_na_quaresma(Uni * bd);
void prescrito(Uni * bd);
void finalistas(Uni * bd);
void listar_estudantes_por_data_e_nacionalidades(Uni *bd);
//Funções auxiliares
void remover_espacos(char * str);
void separar_parametros(const char * linha, char ** parametros, int * num_parametros);
void limpar_buffer();
int verificar_e_limpar_buffer();
void limpar_terminal();
void pausa_listagem(short * contador);
void pressione_enter();
void colocar_terminal_utf8();
void verificar_primeiro_erro(FILE * erros, char * primeiro_erro, const char * nome_ficheiro);
void print_uso_backup();
void print_falha_carregar_dados();
void printf_fich_bin_alterado();
void listar_erro_ao_carregar(FILE * erros, char * primeiro_erro, const char * nome_ficheiro, char * erro, int n_linhas, const char * linha);
int string_para_int(const char * str, int * resultado);
int string_para_short(const char * str, short * resultado);
int string_para_float(const char * str, float * resultado);
short calcular_dia_da_semana(short dia, int mes, int ano);
Data calcular_domingo_pascoa(int ano);
Data calcular_quarta_feira_cinzas(Data pascoa);
void calcular_quaresma(int ano, Data * inicio, Data * fim);
unsigned long calcular_checksum(Uni * bd);
int sim_nao();
void pedir_codigo(int * codigo);
char obter_separador(FILE * ficheiro, char * formato);
short calcular_idade(Data nascimento);
char * normalizar_string(char * str);
void data_atual();

#endif