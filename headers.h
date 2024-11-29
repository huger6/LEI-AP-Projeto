#ifndef HEADERS_H //Se HEADERS_H não foi definida, o código entre o #ifndef e o #endif será processado, caso contrário, será ignorado.
//Ou seja, evita o processamento dos headers várias vezes
#define HEADERS_H //define caso seja a 1x

#define DADOS_TXT "dados.txt"
#define SITUACAO_ESCOLAR_TXT "situacao_Escolar_Estudantes.txt"


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <locale.h>
#include <string.h>


//Structs

typedef struct data_nascimento {
    short dia, mes, ano;
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

#endif