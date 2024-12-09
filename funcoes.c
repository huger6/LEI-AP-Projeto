#include "headers.h"

const short ANO_ATUAL = 2025; 
const short ANO_NASC_LIM_INF = 1908;

//Funções

void remover_espacos(char * str) {
    if(str == NULL) return;
    char * inicio = str;
    char * fim = NULL;

    //Se o início conter um espaço, vai avançar o ponteiro uma casa, até essa casa deixar de ser um espaço.
    while (*inicio == ' ') inicio++;

    // Copiar a string sem espaços para o array inicial
    if (inicio != str) {
        memmove(str, inicio, strlen(inicio) + 1); //É uma versão melhorada do memcpy (evita a sobreposição)
    }

    //Ponteiro para o último caractere
    fim = str + strlen(str) - 1;
    while (fim > str && *fim == ' ') { //verifica se o fim tem um espaço em branco, se sim, anda com o ponteiro uma casa para trás e repete
        fim--;
    }

    //Colocamos o nul char no final, de modo a sinalizar o fim da string. 
    *(fim + 1) = '\0';
}

//LIBERTAR A MEMÓRIA DE PARAMETROS
void separar_parametros(const char * linha, char ** parametros, int * num_parametros) { // char ** parametros serve para armazenar os ponteiros dos parametros, de modo a que não sejam perdidos
    if(linha == NULL || parametros == NULL || num_parametros == NULL) return;
    char * inicio = linha; //Ponteiro para o inicio da linha, que não deve ser alterado
    char * fim = NULL;
    //Não colocamos *num_parametros = 0 pois esta função poderá ser chamada várias vezes numa linha, e não queremos alterar a var nesse caso
    int indice = 0; //Indice do array

    while(*inicio != '\0') { //Se não for o fim da linha entramos no loop
        fim = inicio; 

        //Vamos veriricar se o ponteiro atual de fim é um separador ou o fim da linha, caso não seja avançamos
        while(*fim != SEPARADOR && *fim != '\0' && *fim != '\n') fim++;
        //Aqui fim está a apontar para o separador, o fim da linha ou um \n (se bem que neste caso \n é o fim da linha
        char temp = *fim; //Armazena o tab ou o nul charö
        *fim = '\0'; //vai terminar a string de inicio (ou seja, um parametro); também corta o \n aqui, se exitir
        remover_espacos(inicio);

        if (*inicio != '\0') { //Se o inicio não for o fim da linha, então temos um parametro
            //Alocamos memória para o parametro e copia o conteúdo
            parametros[indice] = malloc(strlen(inicio) + 1); //Lembrar que parametros recebe um ponteiro.
            if (parametros[indice] != NULL) {
                strcpy(parametros[indice], inicio);
                indice++;
            }
            else {
                printf("Erro!\n");
                //Libertar a memória alocada até ao indice atual
                for (int i = 0; i < indice; i++) 
                    free(parametros[i]); 
                break;
            }
        }
        *fim = temp; //Volta a colocar o tab ou '\0' onde estava 
        inicio = (*fim == '\0') ? fim : fim + 1; //Verifica se já estamos no fim da string, se sim, inicio = fim, se não inicio = fim +1 (avança uma casa)
    }
    *num_parametros = indice;
}

//Linha é alocada dinamicamente, pelo que deve ser libertada quando já não for necessária.
//Lê uma linha completa do ficheiro/teclado(ficheiro = stdin) sem que haja a possibilidade de ficar algo por ler\
//n_linhas NULL se não estivermos a ler de um ficheiro
char * ler_linha_txt(FILE * ficheiro, int * n_linhas) {
    if(ficheiro == NULL) return NULL;
    //n_linhas não será inicializado aqui
    char buffer[TAMANHO_INICIAL_BUFFER]; //Buffer para armazenar parte da linha
    size_t tamanho_total = 0; //Comprimento da linha; size_t pois é sempre >0 e evita conversões que podem levar a erros com outras funções
    char * linha = NULL;

    while (fgets(buffer, sizeof(buffer), ficheiro)) { //fgets le ate buffer -1 caracteres ou '\n' ou EOF
        size_t tamanho = strlen(buffer); //Calcula o tamanho do texto lido
        //NOTA IMPORTANTE: se o ponteiro passado para realloc for nulo, ele funciona como o malloc
        char * temp = realloc(linha, tamanho_total + tamanho + 1); //+1 para o nul char
        if (temp == NULL) {
            free(linha);
            printf("Ocorreu um erro ao alocar memória. A encerrar a aplicação.\n");
            return NULL;
        }
        linha = temp; //atualizar o ponteiro linha para apontar para a nova memória

        //Copiar o conteúdo lido para a linha total
        strcpy(linha + tamanho_total, buffer);
        tamanho_total += tamanho;

        //Verificamos se a linha está completa
        if (buffer[tamanho - 1] == '\n') {//se tudo tiver sido copiado, o ultimo caracter do buffer(e da linha tbm) será o '\n'
            if (n_linhas != NULL) (*n_linhas)++;
            return linha;
        }
    }

    if (linha && tamanho_total > 0) {
        //Linha final sem '\n' mas tem conteudo (por ex: ultima linha)
        if (n_linhas != NULL) (*n_linhas)++;
        return linha;
    }

    //Se chegarmos aqui é porque aconteceu algum erro ou o ficheiro está vazio
    free(linha);
    return NULL;
}
//Função recebe o ponteiro de um ponteiro para permitir a realocação do array caso necessário
void carregar_dados(const char * nome_ficheiro_dados,const char * nome_ficheiro_escolar, Estudante ** aluno, Dados ** escolares, int * tamanho_alunos) { 

    FILE * dados = fopen(nome_ficheiro_dados, "r");
    FILE * situacao_escolar = fopen(nome_ficheiro_escolar, "r");
    int n_linhas = 0;
    char * linha = NULL; //Ponteiro para armazenar uma linha
    int indice_aluno = 0;

    //Esta secção vai copiar, linha a linha, o conteúdo do ficheiro para a memória, nomeadamente na struct Estudante
    if(dados) { //apenas se a abertura tiver sido bem sucedida
        while ((linha = ler_linha_txt(dados, &n_linhas)) != NULL) {
            char * parametros[MAX_PARAMETROS] = {NULL}; //Array com PARAMETROS casas, onde cada pode armazenar um ponteiro para char (ou seja, uma string)
            int num_parametros = 0; //Armazena o numero real de parametros

            separar_parametros(linha, parametros, &num_parametros); //extrai os dados já formatados corretamente para parametros
            
            if(num_parametros == PARAMETROS_ESTUDANTE) {
                //Verificamos se o indice é igual ou superior ao tamanho do array, se sim, aumentamos o tamanho do array
                if (indice_aluno >= *tamanho_alunos) {
                    if (!realocar_structs(aluno, escolares, tamanho_alunos, sizeof(Estudante))) {
                        printf("Ocorreu um erro a gerir a memória.\n"); //Averiguar melhor 
                        fclose(dados);
                        return;
                    }
                    //A realocação dos campos de nacionalidade e nome são feitas em realocar_structs
                }
                 //Como estamos a carregar as structs do estudante, o número de parametros lidos tem que ser igual ao esperado
                (*aluno)[indice_aluno].codigo = atoi(parametros[0]); //atoi é uma função que converte strings para ints
                strcpy((*aluno)[indice_aluno].nome, parametros[1]); //nome é a segunda coluna nos dados.txt
                ler_data(&((*aluno)[indice_aluno]), parametros[2], '0'); //Data nascimento
                strcpy((*aluno)[indice_aluno].nacionalidade, parametros[3]);
                indice_aluno++; //Se os dados foram carregados, então passamos ao próximo aluno
            }
            
            //Libertamos a memória alocada para os parametros
            for(int i = 0; i < num_parametros; i++) 
                free(parametros[i]);

            free(linha); //Libertamos a memória alocada para a linha presente
        }
        fclose(dados);
    }
    else {
        printf("Ocorreu um erro a abrir o ficheiro '%s'.\n", nome_ficheiro_dados);
    }
    
    indice_aluno = 0;
    n_linhas = 0;

    if(situacao_escolar) {
        while ((linha = ler_linha_txt(situacao_escolar, &n_linhas)) != NULL) {
            char * parametros[MAX_PARAMETROS] = {NULL};
            int num_parametros = 0; 

            separar_parametros(linha, parametros, &num_parametros); 

            if(num_parametros == PARAMETROS_DADOS_ESCOLARES) {

                if (indice_aluno >= *tamanho_alunos) {
                    if (!realocar_structs(aluno, escolares, tamanho_alunos)) {
                        printf("Ocorreu um erro a gerir a memória.\n");
                        fclose(situacao_escolar);
                        return;
                    }
                }
                (*escolares)[indice_aluno].codigo = atoi(parametros[0]); //codigo
                (*escolares)[indice_aluno].matriculas = atoi(parametros[1]); //matriculas
                (*escolares)[indice_aluno].ects = atoi(parametros[2]); //ects
                (*escolares)[indice_aluno].media_atual = strtof(parametros[3], NULL); //media
                //IMPORTANTE: AO PASSAR NULL, ESTAMOS A DESCARTAR O RESTO DA STRING QUE NÃO FOI LIDA, E ISSO PODE SER UM TAB!!
                (*escolares)[indice_aluno].ano_atual = atoi(parametros[4]); //ano atual
                indice_aluno++;
            }
            
            //Libertamos a memória alocada para os parametros
            for(int i = 0; i < num_parametros; i++) 
                free(parametros[i]);

            free(linha); //Libertamos a memória alocada para a linha presente
        }
        fclose(situacao_escolar);
    }
    else {
        printf("Ocorreu um erro a abrir o ficheiro '%s'.\n", nome_ficheiro_escolar);
    }
}

void guardar_dados(const char * nome_ficheiro, Estudante * aluno, Estatisticas * stats) {
    return;
}
//Inicializar com valores nitidamente inválidos. Usar indice_atual = 0 para inicializar pela primeira vez, depois usar a última posição válida ocupada + 1
void inicializar_structs(Estudante * aluno, Dados * escolares, Estatisticas * stats, int indice_atual, const int tamanho_aluno) {
    if (!aluno || !escolares || !stats || tamanho_aluno <= 0 || indice_atual < 0) return;

    for (int i = indice_atual; i < tamanho_aluno; i++) {
        //Struct estudante
        aluno[i].codigo = -1;
        aluno[i].nascimento.dia = 0; //Para sinalizar que ainda não foi alterada
        aluno[i].nascimento.mes = 0;
        aluno[i].nascimento.ano = 0;
        //Aloca memória para guardar a nacionalidade e o nome
        aluno[i].nacionalidade = (char *) malloc (MAX_STRING_NACIONALIDADE * sizeof(char));
        aluno[i].nome = (char *) malloc (TAMANHO_INICIAL_NOME * sizeof(char));
        if(aluno[i].nacionalidade == NULL || aluno[i].nome == NULL) {
            for(int j = indice_atual; j < i; j++) {
                free(aluno[j].nacionalidade);
                free(aluno[j].nome);
            }
            printf("Ocorreu um erro ao alocar memória. A encerrar.\n");
            return;
        }
        strcpy(aluno[i].nacionalidade, "-1");
        strcpy(aluno[i].nome, "-1");
        //Struct dados
        escolares[i].codigo = -1;
        escolares[i].matriculas = -1;
        escolares[i].ects = -1;
        escolares[i].ano_atual = -1;
        escolares[i].media_atual = -1;
        escolares[i].prescrever = '0';
        //Struct estatísticas
        stats[i].finalistas = 0;
        stats[i].media = 0;
        stats[i].media_idade_ano = 0;
        stats[i].media_idade_nacionalidade = 0;
        stats[i].medias_matriculas = 0;
        stats[i].risco_prescrever = 0;
    }
}
//Duplica o espaço atual. Não inicializa o espaço alocado. Não adereça erros.
int realocar_espaco(void ** estrutura, int * tamanho_alunos, int tamanho_tipo) {
    //estrutura é void mas ainda aponto para o endereço de aluno/escolares,...
    void * temp = realloc(*estrutura, *tamanho_alunos * tamanho_tipo);
    //temp é void mas contém o novo bloco de memória do tipo passado em estrutura
    if (!temp) {
        return 0; //Erros devem ser tratados na função
    }
    *estrutura = temp; //aqui atualiza-se o ponteiro original
    return 1;
}

//Aloca memória para todas as structs se stats != NULL. Não endereça erros
int realocar_structs(Estudante ** aluno, Dados ** escolares, int * tamanho_alunos) {
    if(!aluno || !escolares || !tamanho_alunos) return 0;
    int indice = *tamanho_alunos; //Guardar o índice para realocar os campos das structs
    *tamanho_alunos *= 2; //Duplicar tamanho do array
    //A função está à espera de um argumento do tipo void **, logo temos de fazer o cast para evitar erros
    if(!realocar_espaco((void**)aluno, *tamanho_alunos, sizeof(Estudante))) return 0;
    if(!realocar_espaco((void**)escolares, *tamanho_alunos, sizeof(Dados))) return 0;
    //Note-se que a verificação para falhas na realocação é feita em realocar_espaco
    //Alocar strings para novos elementos Estudante
    //Inicializar novos elementos (é feita a alocação para os campos nacionalidade e nome)
    inicializar_structs(*aluno, *escolares, indice, *tamanho_alunos);
    return 1; 
}

void limpar_buffer() {
    int lixo;
    while ((lixo = getchar()) != '\n' && lixo != EOF);
}

void validacao_menus(short * valido, const char opcao, const char limInf, const char limSup) { //const pois não devem ser alterados
    if (*valido != 1) {
        validacao_input_menu(limInf, limSup);
    }
    else if (opcao < limInf || opcao > limSup) { 
        *valido = 0; //Scanf leu corretamente e retornou 1, mas como não queremos esses números, voltamos a definir a zero para dizer que é inválido
        validacao_numero_menu(limInf, limSup); 
    }
}
//Modo '1' para validar 
int validar_codigo(int * codigo, Estudante * aluno, Dados * escolares, int * tamanho, const char modo) {
    if(*codigo < 0) {
        limpar_buffer();
        if (modo == '1') {
            printf("Código inválido! Insira um número inteiro positivo.\n");
            pressione_enter();
        }
        return 0;
    }
    if(*codigo == procurar_codigo(*codigo, aluno, escolares, *tamanho)) {
        limpar_buffer();
        if (modo == '1') {
            printf("Código já existente! Insira um código diferente.\n");
            pressione_enter();
        }
        return 0;
    }
    return 1;
}

int validar_nome(char * nome, Estudante * aluno, int * tamanho_alunos, const char modo) {
    if (!nome) {
        if (modo == '1') printf("Nome inválido!\n");
        return 0;
    }
    if (strlen(nome) > TAMANHO_INICIAL_NOME - 1) {
        //tratar do realloc 
    }
    for (int i = 0; i < strlen(nome); i++) {
        if (nome[i] == SEPARADOR) return 0;
    }

    if (!isalpha(nome[i]) && nome[i] != ' ' && nome[i] != '-') {
        if (modo == '1') printf("Nome contém caracteres inválidos!\n");
        return 0;
    }
    return 1;
}
//Limpar o terminal consoante o sistema operativo
void limpar_terminal() {
    #ifdef _WIN32 //_WIN32 é uma variável local automaticamente pre-definida pelo windows em todos os sistemas
        system("cls"); //Sistemas windows
    #else
        system("clear"); //Sistemas linux, macOs, etc
    #endif
}

void colocar_terminal_pt() {
    #ifdef _WIN32
        //SetConsoleOutputCP retorna 0 se houver um erro
        if ((SetConsoleOutputCP(CP_UTF8) == 0)||SetConsoleCP(CP_UTF8) == 0) {
            printf("Ocorreu um erro ao configurar o terminal do Windows para UTF-8.\n");
            printf("A aplicação irá continuar. Desformatação será visível. Para resolver, reinicie a aplicação.\n");
        }
    #else
        //Feito com a ajuda do chatgpt
        // Configuração específica para sistemas Unix-like (Linux, macOS)
        //setenv pertence ao stdio.h e é usada para configurar var de ambiente, apenas existe no linux,etc
        if (setenv("LANG", "pt_PT.UTF-8", 1) != 0) {
            printf("Ocorreu um erro ao configurar o terminal para UTF-8.\n");
            printf("A aplicação irá continuar. Desformatação será visível. Para resolver, reinicie a aplicação.\n");
            return;
        }
        if (setenv("LC_ALL", "pt_PT.UTF-8", 1) != 0) {
            printf("Ocorreu um erro ao configurar o terminal do Windows para UTF-8.\n");
            printf("A aplicação irá continuar. Desformatação será visível. Para resolver, reinicie a aplicação.\n");
            return;
        }
    #endif
}

void pressione_enter() {
    printf("Pressione Enter para continuar.\n");
    while (getchar() != '\n'); //Caso o user escreva algo e não enter limpar buffer
}

//Esta função é um pouco "inutil" neste momento. Foi desenhada para tratar erros com ints
void validacao_input_menu(const char limInf, const char limSup) {
    limpar_buffer();  // Limpar o buffer de entrada (o que foi escrito incorretamente)
	printf("Entrada inválida! Introduza um número do menu (%c a %c)\n", limInf, limSup); 
    pressione_enter();
	limpar_terminal();
}

void validacao_numero_menu(const char limInf, const char limSup) {
    limpar_buffer();
	printf("Por favor, escolha um número do menu (%c a %c).\n", limInf, limSup);
	pressione_enter();
	limpar_terminal();
}


char menu_principal() {
	short valido = 0; //Usamos apenas short devido ao facto do scanf poder retornar -1 (ainda que improvavel), acontece no EOF ou Ctrl Z
	char opcao = '0';
    do {
        limpar_terminal(); //Limpar terminal apenas se voltarmos a escrever o menu (ou seja, já não estão a ser necessárias as informações anteriores)
        //https://desenvolvedorinteroperavel.wordpress.com/2011/09/11/tabela-ascii-completa/
        //Link da tabela ASCII completa de onde foram retirados as duplas barras do menu (a partir do 185 decimal)
        printf("╔══════════════════════════════════╗\n");
        printf("║          MENU PRINCIPAL          ║\n");
        printf("╠══════════════════════════════════╣\n");
        printf("║  1. Gerir estudantes             ║\n");
        printf("║  2. Consultar dados              ║\n");
        printf("║  3. Estatísticas                 ║\n");
        printf("║  4. Extras                       ║\n");
        printf("║  0. Sair do programa             ║\n");
        printf("╚══════════════════════════════════╝\n\n");
        printf("   Escolha uma opção: ");

        valido = scanf(" %c", &opcao); //scanf retorna 1 se conseguir ler de acordo com o esperado, " %c" para evitar ler \n's
        
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
		printf("╔════════════════════════════════════╗\n");
        printf("║          GERIR ESTUDANTES          ║\n");
        printf("╠════════════════════════════════════╣\n");
        printf("║  1. Inserir estudante              ║\n");
        printf("║  2. Eliminar estudante             ║\n");
        printf("║  3. Atualizar dados do estudante   ║\n");
        printf("║  0. Voltar ao menu anterior        ║\n");
        printf("╚════════════════════════════════════╝\n\n");
        printf("   Escolha uma opção: ");

		valido = scanf(" %c", &opcao);
		
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
		printf("╔═════════════════════════════════════════════════════════════╗\n");
        printf("║                       CONSULTAR DADOS                       ║\n");
        printf("╠═════════════════════════════════════════════════════════════╣\n");
        printf("║  1. Procurar estudante por nome                             ║\n");
        printf("║  2. Listar estudantes por intervalo de datas de nascimento  ║\n");
        printf("║  3. Listar estudantes por nacionalidade                     ║\n");
        printf("║  4. Listar estudantes por ordem alfabética de apelido       ║\n");
        printf("║  0. Voltar ao menu anterior                                 ║\n");
        printf("╚═════════════════════════════════════════════════════════════╝\n\n");
        printf("   Escolha uma opção: ");

		valido = scanf(" %c", &opcao);
		
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
		printf("╔════════════════════════════════════════════════════════╗\n");
        printf("║                      ESTATÍSTICAS                      ║\n");
        printf("╠════════════════════════════════════════════════════════╣\n");
        printf("║  1. Estudantes por escalão de média atual              ║\n");
        printf("║  2. Número médio de matrículas (geral/nacionalidade)   ║\n");
        printf("║  3. Número de finalistas                               ║\n");
        printf("║  4. Média de idades por nacionalidade e ano            ║\n");
        printf("║  5. Listar estudantes em risco de prescrição           ║\n");
        printf("║  0. Voltar ao menu anterior                            ║\n");
        printf("╚════════════════════════════════════════════════════════╝\n\n");
        printf("   Escolha uma opção: ");

		valido = scanf(" %c", &opcao);
		
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
		printf("╔════════════════════════════════════════════════════════════╗\n");
        printf("║                           EXTRAS                           ║\n");
        printf("╠════════════════════════════════════════════════════════════╣\n");
        printf("║  1. Estudantes nascidos em dias específicos da semana      ║\n");
        printf("║  2. Estudantes cujo aniversário em certo ano é ao domingo  ║\n");
        printf("║  3. Relacionar ano de inscrição com intervalos de notas    ║\n");
        printf("║  0. Voltar ao menu anterior                                ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n\n");
        printf("   Escolha uma opção: ");

		valido = scanf(" %c", &opcao);
		
		validacao_menus(&valido,opcao,'0','3');
        
		if (valido == 1) {
			return opcao;
		}
	} while (valido == 0);	 
}

void processar_gerir_estudantes(Escolha * escolha) {
    do {
        escolha->opcao_submenu = menu_gerir_estudantes();
        switch(escolha->opcao_submenu) {
            case '0':
                escolha->menu_atual = 'P';
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
    } while (escolha->menu_atual == 'G');
}

void processar_consultar_dados(Escolha * escolha) {
    do {
        escolha->opcao_submenu = menu_consultar_dados();
        switch(escolha->opcao_submenu) {
            case '0':
                escolha->menu_atual = 'P';
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
    } while (escolha->menu_atual == 'C');
}

void processar_estatisticas(Escolha * escolha) {
    do {
        escolha->opcao_submenu = menu_estatisticas();
        switch(escolha->opcao_submenu) {
            case '0':
                escolha->menu_atual = 'P';
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
    } while (escolha->menu_atual == 'E');
}

void processar_extras(Escolha * escolha) {
    do {
        escolha->opcao_submenu = menu_extras(); //Obter a opção do segundo menu
        switch(escolha->opcao_submenu) {
            case '0':
                escolha->menu_atual = 'P';
                break;
            case '1':
                //Listar estudantes nascidos em dias específicos da semana
                break;
            case '2':
                //Listar os estudantes cujo aniversário num determinado ano é ao domingo
                break;
            case '3':
                //Relacionar o ano de inscrição com intervalos das classificações
                break;
            default:
                printf("Erro!!"); //Aprofundar
                break;
    }
    } while (escolha->menu_atual == 'X');
}

Escolha escolha_menus() {
	Escolha escolha; //C é case sensitive (pode parecer má prática mas estava difícil de encontrar outro nome igualmente expressivo :))

    escolha.menu_atual = 'P';
    escolha.opcao_principal = '0';
    escolha.opcao_submenu = '0'; //Esta variavel nunca é alterada fora das funcoes, ver isto

    do {
        limpar_terminal();
        escolha.opcao_principal = menu_principal();

        switch(escolha.opcao_principal) {
            case '1':
                escolha.menu_atual = 'G';
                processar_gerir_estudantes(&escolha); //Passamos menu atual por referência no caso da opção escolhida ser voltar atrás
                break;
            case '2':
                escolha.menu_atual = 'C';
                processar_consultar_dados(&escolha);
                break;
            case '3':
                escolha.menu_atual = 'E';
                processar_estatisticas(&escolha);
                break;
            case '4':
                escolha.menu_atual = 'X';
                processar_extras(&escolha);
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

//modo '1' para imprimir mensagens de erro, '0' para não imprimir
int validar_data(short dia, short mes, short ano, const char modo) {
    if (ano < 1 || mes < 1 || mes > 12 || dia < 1) { //Limites dos anos são os que foram considerados mais realistas
        if (modo == '1') printf("Por favor, insira uma data válida.\n");
        return 0;
    }
    /*Apenas se podem inscrever alunos com mais de 16 anos (sensivelmente, porque devido ao mes de nascimento pode nao ser bem assim)
    Poderia ser resolvido com a adição de +2 constantes para mes e dia atuais mas achamos desnecessário devido a tratar-se da inscrição de alunos
    que por natureza, não têm de ser todos iguais ou ser inscritos com a mesma idade.
    */
    if (ano < ANO_NASC_LIM_INF || ano > ANO_ATUAL - 16) {
        if (modo == '1') printf("O ano inserido é inválido. Insira um ano entre %d e %d.\n", ANO_NASC_LIM_INF, ANO_ATUAL);
        return 0;
    }

    // Criamos um vetor com os dias de cada mes, fevereiro com 28 pois é o mais comum
    short dias_por_mes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    //Apenas criado para propóstios de informação ao user
    const char * nome_do_mes[] = {"janeiro", "fevereiro", "março", "abril", "maio", "junho", "julho", "agosto", "setembro", "outubro", "novembro", "dezembro"};

    // verificar anos bissextos
    if (mes == 2 && ((ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0))) {
        dias_por_mes[1] = 29;
    }

    if (dia > dias_por_mes[mes - 1]) {
        if (modo == '1') printf("O dia é inválido! O mês de %s tem apenas %hd dias.\n", nome_do_mes[mes - 1], dias_por_mes[mes -1]);
        return 0; //Falso
    }
    //Se não retornou 0(F), é V
    return 1;
}
//Se for para ler, usar str = '\0', modo('1'/'0') para verificar se queremos imprimir mensagens de erro
void ler_data(Estudante * aluno, char * str, const char modo) {

	char data[11]; //Vamos usar o formato DD-MM-AAAA (10 caracteres + \0)
    char erro = '0'; //1 há erro

    short dia_temp, mes_temp, ano_temp; //Variáveis temporárias para armazenar os valores lidos
	do {
        if (!str) { //Se str for NULL queremos ler a data
            erro = '0';
            printf("Data de nascimento (DD-MM-AAAA): ");
            if(fgets(data, sizeof(data), stdin) ==NULL) { //stdin porque é daí que lemos os dados; //fgets retorna um ponteiro para "data", logo verificamos se é NULL ocorreu um erro
                limpar_buffer();
                printf("Ocorreu um erro ao tentar ler a data de nascimento!\n");
                erro = '1';
                continue; //O continue faz com que o loop avance para a proxima iteração (aqui podemos usar porque não há perigo de ficar um loop infinito com !str)
                //Nota: o uso do fgets faz com que o buffer nao rebente, pois ele so le ate ao limite de sizeof(data)-1(para o \0), apenas temos de limpar o buffer depois
            }
        }
        else {
            strcpy(data, str);
        }
        
        
        //O sscanf lê as x entradas conforme o padrão apresentado
    	if (sscanf(data, "%hd-%hd-%hd", &dia_temp, &mes_temp, &ano_temp) != 3) { //sscanf tenta ler 3 shorts para as var temp
            //Aqui não se usa && erro == '0'porque só pode ser 0 se !str, e nesse caso há um continue
            //Caso nao consiga, o formato é inválido
            if (modo == '1') printf("Formato inválido! Use o formato DD-MM-AAAA.\n");
            limpar_buffer();
            erro = '1';
        }
        //Sse data for válida é que passamos os dados à stuct
        if (validar_data(dia_temp, mes_temp, ano_temp, modo) && erro == '0') {
            aluno->nascimento.dia = dia_temp;
            aluno->nascimento.mes = mes_temp;
            aluno->nascimento.ano = ano_temp;
        } 
        else erro = '1';

        if(str && erro == '1') { //Caso contrário ficavamos num loop infinito
            if (modo == '1') printf("Data inválida! A retornar.\n");
            return;
        }
	} while (erro == '1' && !str); //Continuar a pedir a data sempre que esta for inválida
    
    if (!str) {
        limpar_buffer();
    } //A entrada pode ter sido válida apesar de ter mais de 11 caracteres (ex: 15/12/2006EXTRA)
}


//Implementa uma procura binária
int procurar_codigo(int codigo, Estudante * aluno, Dados * escolares, int tamanho) {
    int limInf, limSup, meio;
    limInf = 0;
    limSup = tamanho - 1;

    while (limSup >= limInf) {
        meio = (int)(limSup + limInf) / 2;
        if (aluno[meio].codigo == codigo && escolares[meio].codigo == codigo) return meio; //Dá return do índice
        else {
            if (aluno[meio].codigo < codigo) limInf = meio + 1;
            else limSup = meio - 1;
        }
    }
    return -1; //Estão todos cheios ou o código não existe
}

void inserir_estudante(Estudante ** aluno, Dados ** escolares, int * tamanho_alunos) {
    int indice = procurar_codigo(-1, *aluno, *escolares, *tamanho_alunos); //Verifica se há espaço livre
    if (indice == -1) { //Trata os casos em que não há espaço livre
        if (!realocar_structs(aluno, escolares, tamanho_alunos)) {
            printf("Ocorreu um erro ao alocar memória para o aluno. A retornar.\n");
            return;
        }
    }
    char repetir = 'n'; //s/n
    limpar_buffer();
    do {
        limpar_terminal(); //Caso de repetição
        repetir = 'n';
        do {
            int codigo_temp = -1; 
            limpar_terminal();
            printf("Insira o código do estudante: ");
            if (scanf("%d", &codigo_temp) != 1) { //Verifica entradas inválidas como letras
                printf("Código inválido! Insira um número inteiro positivo.\n");
                limpar_buffer();
                pressione_enter();
                continue;
            }

            if (!validar_codigo(&codigo_temp, aluno, escolares, *tamanho_alunos, '1')) {
                limpar_buffer();
                pressione_enter();
                continue;
            }
            //Se for válido, passamos o valor para a struct
            (*aluno)[indice].codigo = codigo_temp;
            (*escolares)[indice].codigo = codigo_temp;
            break; //Se não for inválido saimos do loop
        } while (1);
        
        do {
            char * nome_temp = {NULL};
            printf("Insira o nome do estudante: ");
            nome_temp = ler_linha_txt(stdin, NULL);
            if(!validar_nome(nome_temp, '1')) {
                limpar_buffer();
                pressione_enter();
                continue;
            }
            (*aluno)[indice].nome = nome_temp;
        } while( aluno[indice].nome == NULL);

        do {
            printf("Insira a data de nascimento do estudante (DD-MM-AAAA): ");
            ler_data(&aluno[indice], NULL, '1');
            limpar_buffer();
        } while (aluno[indice].nascimento.dia == 0); //Apenas verificamos um pois em ler_data a data só é copiada para as structs se toda ela for válida
        
        do {
            printf("Quer inserir mais estudantes? (S/N): ");
            scanf(" %c", &repetir);
            if (repetir == 's' || repetir == 'S') {
                repetir = 's';
                break;
            }
            else if (repetir == 'n' || repetir == 'N') {
                repetir == 'n';
                break;
            }
            limpar_buffer();
            //se repetir for s ou S, então sai do loop pois cumpre a condição do while
        } while (repetir != 's' && repetir != 'S' && repetir != 'n' && repetir != 'N');
    }while(repetir == 's');
}

