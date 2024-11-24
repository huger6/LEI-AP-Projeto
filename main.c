#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <locale.h>

//TODO LIST
/*
-Perguntar à professora da funcionalidade 1: está-nos a ser pedido para carregar todos os dados de uma só vez para o programa? Se sim, isso não seria
...pouco eficiente e não seria melhor carregar os dados apenas quando nos é solicitado algum tipo de dado do ficheiro, onde poderíamos implementar algum
...tipo de merge sort
-Perguntar se podemos pedir a data de nascimento separada ou como uma string
-Perguntar o que querem dizer com mecanismo de avanço página a página
*/



//Structs

typedef struct data_nascimento {
    unsigned int dia, mes, ano; //TODO
}Data;

//Vamos dividir os dados enter uma struct Estudante, para dados pessoais, e outra para dados escolares ()
typedef struct estudante {
    unsigned int codigo; //int para prevenir, caso o código tenha, imagine-se, 
    Data nascimento; 
    char * nacionalidade; //Declara-se um ponteiro para posterioremente alocar memória dinamicamente
    unsigned short matriculas; //unsigned porque matriculas sempre > 0 e short porque usa apenas 2 bytes em x dos 4 de um int
    unsigned short ects; //Mesma lógica das matriculas
}Estudante;

typedef struct dados_escolares {
    unsigned int codigo;
    unsigned char prescrever;

}Escola;
//Protótipos das funções




//Main
int main() {
    //Colocar a consola em PT-PT (caracteres UTF8)
    SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Portuguese");
}







//Funções


