#include "headers.h"

const short ANO_ATUAL = 2025; 
const short ANO_NASC_LIM_INF = 1908;

//Ficheiros e gestão de dados

//Linha é alocada dinamicamente, pelo que deve ser libertada quando já não for necessária.
//Lê uma linha completa do ficheiro/teclado(ficheiro = stdin) sem que haja a possibilidade de ficar algo por ler
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
            return NULL;
        }
        linha = temp; //atualizar o ponteiro linha para apontar para a nova memória

        //Copiar o conteúdo lido para a linha total
        memmove(linha + tamanho_total, buffer, tamanho); //linha + tamanho_total é um ponteiro para a posição da memória seguinte para onde a próxima parte de buffer será copiada
        tamanho_total += tamanho;
        linha[tamanho_total] = '\0'; //Se houver mais que uma leitura, o primeiro char da segunda leitura de fgets irá substituir o nul char, pelo que fica sempre no fim

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
void carregar_dados(const char * nome_ficheiro_dados,const char * nome_ficheiro_escolar, Uni * bd) { 

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

void guardar_dados(const char * nome_ficheiro_dados, const char * nome_ficheiro_escolares, Uni * bd) {
    FILE * aluno = fopen(nome_ficheiro_dados, "w");
    FILE * dados = fopen(nome_ficheiro_escolares, "w");

    if (!aluno) {
            printf("Ocorreu um erro ao abrir o ficheiro %s para guardar os dados.\n", nome_ficheiro_dados);
    }
    else {
        for(int i = 0; i < bd->tamanho_aluno; i++) {
            fprintf(aluno, "%d%c%s%c%02hd-%02hd-%04hd%c%s\n",
                bd->aluno[i].codigo, SEPARADOR,
                bd->aluno[i].nome, SEPARADOR,
                bd->aluno[i].nascimento.dia, bd->aluno[i].nascimento.mes, bd->aluno[i].nascimento.ano, SEPARADOR, 
                bd->aluno[i].nacionalidade);
            }
        fclose(aluno);
    }
        
    if (!dados) {
        printf("Ocorreu um erro ao abrir o ficheiro %s para guardar os dados.\n", nome_ficheiro_escolares);
        return; //Já não há mais nada a guardar
    }
    for(int i = 0; i < bd->tamanho_escolares; i++) {
        fprintf(dados, "%d%c%hd%c%hd%c%hd%.1f\n",
            bd->escolares[i].codigo, SEPARADOR,
            bd->escolares[i].matriculas, SEPARADOR,
            bd->escolares[i].ects, SEPARADOR, 
            bd->escolares[i].ano_atual, SEPARADOR,
            bd->escolares[i].media_atual);
    }
    fclose(dados);
    return;
}

//Gestão de memória

//Inicializar com valores nitidamente inválidos. Usar indice_atual = 0 para inicializar pela primeira vez, depois usar a última posição válida ocupada + 1
void inicializar_structs(Uni * bd) {
    if (!bd || !bd->aluno || !bd->escolares) return;

    for (int i = bd->tamanho_aluno; i < bd->capacidade_aluno; i++) {
        //Struct estudante
        //Aloca memória para guardar a nacionalidade e o nome (primeiro pois pode falhar, e seria escusado inicializar outras coisas antes)
        bd->aluno[i].nacionalidade = (char *) malloc (MAX_STRING_NACIONALIDADE * sizeof(char));
        bd->aluno[i].nome = (char *) malloc (TAMANHO_INICIAL_NOME * sizeof(char));
        if(bd->aluno[i].nacionalidade == NULL || bd->aluno[i].nome == NULL) {
            for(int j = bd->tamanho_aluno; j < i; j++) {
                free(bd->aluno[j].nacionalidade);
                free(bd->aluno[j].nome);
            }
            printf("Ocorreu um erro ao alocar memória. A encerrar.\n");
            return;
        }
        bd->aluno[i].codigo = -1;
        bd->aluno[i].nascimento.dia = 0; //Para sinalizar que ainda não foi alterada
        bd->aluno[i].nascimento.mes = 0;
        bd->aluno[i].nascimento.ano = 0;
        strcpy(bd->aluno[i].nacionalidade, "-1");
        strcpy(bd->aluno[i].nome, "-1");
    }
    //Struct dados
    for(int i = bd->tamanho_escolares; i < bd->capacidade_escolares; i++) {
        bd->escolares[i].codigo = -1;
        bd->escolares[i].matriculas = -1;
        bd->escolares[i].ects = -1;
        bd->escolares[i].ano_atual = -1;
        bd->escolares[i].media_atual = -1;
        bd->escolares[i].prescrever = '0';
    }
    //Structs tem uma função especial pois não é um array.
}

void inicializar_estatisticas(Estatisticas * stats) {
    stats->finalistas = 0;
    stats->media = 0;
    stats->media_idade_ano = 0;
    stats->media_idade_nacionalidade = 0;
    stats->medias_matriculas = 0;
    stats->risco_prescrever = 0;
}
//Duplica o espaço atual. Não inicializa o espaço alocado. modo = '1' para endereçar erros.
int realocar_aluno(Uni * bd, const char modo) {
    int tamanho_novo = bd->capacidade_aluno * 2;
    Estudante * temp = realloc(bd->aluno, tamanho_novo * sizeof(Estudante));

    if (!temp) {
        if (modo == '1') printf("Ocorreu um erro ao realocar memória para o aluno. A encerrar.\n");
        return 0;
    }
    bd->aluno = temp;
    bd->capacidade_aluno = tamanho_novo;
    //Não se faz free de temp pois temp aponta para a mesma morada de bd->aluno, logo ao dar free em temp, estavamos a dar free em bd->alunos, e perdeíamos os dados
    return 1;
}

int realocar_escolares(Uni * bd, const char modo) {
    int tamanho_novo = bd->capacidade_escolares * 2;
    Dados * temp = realloc(bd->escolares, tamanho_novo * sizeof(Dados));

    if (!temp) {
        if (modo == '1') printf("Ocorreu um erro ao realocar memória para os dados escolares. A encerrar.\n");
        return 0;
    }
    bd->escolares = temp;
    bd->capacidade_escolares = tamanho_novo;
    return 1;
}

int realocar_nome(Estudante * aluno, const char modo) {
    int tamanho_novo = strlen(aluno->nome) * 2;
    char * temp = realloc(aluno->nome, tamanho_novo * sizeof(char));

    if (!temp) {
        if (modo == '1') printf("Erro ao realocar memória para o nome!\n");
        return 0;
    }

    aluno.nome = temp;
    return 1;
}

//Procura e validações

//Retorna o índice a positivo se encontrar. A negativo se não encontrar, mas é a posição onde deveria ser inserido
//limSup = bd->tamanho_aluno (padrao). Se estivermos a inserir, usar limSup = bd->tamanho_aluno - n_insercoes(para ignorar os elementos acabados de inserir)
int procurar_codigo_aluno(int codigo, Uni * bd) {
    if (!bd || !bd->aluno || bd->tamanho_aluno <= 0) {
        return -1;
    }
    if (bd->aluno[0].codigo > codigo) return -2; //Se o array está sempre ordenado e o codigo é menor que que do array 0, então não existe e está abaixo do indice zero
    int limInf, meio, limSup;
    limInf = 0;
    limSup = bd->tamanho_aluno - 1;

    while (limSup >= limInf) {
        meio = (limSup + limInf) / 2;
        if (bd->aluno[meio].codigo == codigo) return meio; //Dá return do índice
        else {
            if (bd->aluno[meio].codigo < codigo) limInf = meio + 1;
            else limSup = meio - 1;
        }
    }
    return -(limInf + 1); //Retorna a posição de inserção + 1(fazer < 0 para verificar código)
    //o +1 está a precaver no caso de limInf ser 0, para distinguir do return de um indice
}

int procurar_codigo_escolares(int codigo, Uni * bd,) {
    if (bd->escolares[0].codigo > codigo) return -2;
    int limInf, limSup, meio;
    limInf = 0;
    limSup = bd->tamanho_escolares - 1;

    while (limSup >= limInf) {
        meio = (limSup + limInf) / 2;
        if (bd->escolares[meio].codigo == codigo) return meio; //Dá return do índice
        else {
            if (bd->escolares[meio].codigo < codigo) limInf = meio + 1;
            else limSup = meio - 1;
        }
    }
    return -(limInf + 1); //Não há esse código
}

//Modo '1' para printar mensagens de erro
//Valida codigo e retorna a posição de inserção de ALUNO APENAS
int validar_codigo_ao_inserir(int codigo, Uni * bd) {
    if(codigo < 0) {
        limpar_buffer();
        printf("Código inválido! Insira um número inteiro positivo.\n");
        pressione_enter();
        return 0;
    }
    int temp = procurar_codigo_aluno(codigo, bd); //basta procurar no aluno porque não pode haver nenhum código em escolares que não esteja em aluno
    
    if(temp == codigo) {
        limpar_buffer();
        printf("O código já existe! Insira um código diferente.\n");
        pressione_enter();
        return 0;
    }
    //Se não existir o código podesmos usá-lo(<0).
    else if (temp < 0) {
        return temp; //Retornamos a posição onde será inserido
    }
    return 0; //qualquer outro caso
}

//Faz a validação se todos os dados em escolares estão em aluno
//USAR AO CARREGAR DADOS APENAS!!
int validar_codigo_apos_carregar(int codigo, Uni * bd) {
    int indice_aluno = procurar_codigo_aluno(codigo, bd); //verifica se existe em aluno, já ordenado
    if (indice_aluno < 0) return 0; //Se o código não exitir em aluno, não o podemos introduzir porque não podemos ter dados escolares sem a ficha pessoal

    if (procurar_codigo_escolares(codigo, bd) < 0) return 1; //Se não existir, returnar válido
    return 0; //Qualquer outro caso
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

int validar_nome(char * nome, Uni * bd, const char modo) {
    if (!nome) {
        if (modo == '1') printf("Nome em branco!\n");
        return 0;
    }
    int comprimento = strlen(nome);
    
    for (int i = 0; i < comprimento; i++) {
        if (nome[i] == SEPARADOR) {
            if (modo == '1') printf("O nome contém um caracter separador inválido (%c).\n", SEPARADOR);
            return 0;
        }

        if (!isalpha(nome[i]) && nome[i] != ' ' && nome[i] != '-') { //isalpha apenas retorna válido a-z e A-Z, logo as outras condições validam espaços e hifens
            if (modo == '1') printf("Nome contém caracteres inválidos!\n");
            return 0;
        }
    }
    //Se estamos aqui é válido, e precisamos de verificar se o nome cabe no array
    if (comprimento > TAMANHO_INICIAL_NOME - 1) {
        //Realocar o nome 
        realocar_nome(bd, modo);
    }

    return 1;
}

int validar_nacionalidade(char * nacionalidade, Uni * bd, const char modo) {
    if (!nacionalidade) {
        if (modo == '1') printf("Nacionalidade em branco!\n");
        return 0;
    }
    int comprimento = strlen(nacionalidade);
    if (comprimento > MAX_STRING_NACIONALIDADE - 1) {
        if (modo == '1') printf("A nacionalidade é inválida. Insira uma nacionalidade com menos caracteres.\n");
        return 0;
    }
    for (int i = 0; i < comprimento; i++) {
        if (nome[i] == SEPARADOR) {
            if (modo == '1') printf("O nome contém um caracter separador inválido (%c).\n", SEPARADOR);
            return 0;
        }

        if (!isalpha(nome[i]) && nome[i] != ' ' && nome[i] != '-') { //isalpha apenas retorna válido a-z e A-Z, logo as outras condições validam espaços e hifens
            if (modo == '1') printf("Nome contém caracteres inválidos!\n");
            return 0;
        }
    }

    return 1;
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

//Ordenação

//As funções que se seguem implementam o algoritmo de ordenação Merge Sort.
//Dado o algoritmo já ser bastante conhecido, não há necessidade de reinventar, e como tal, foi retirado o código base do algoritmo do website https://www.geeksforgeeks.org/c-program-for-merge-sort/
//Claro que foram necessários ajustes ao contexto do programa. Os nome das variáveis também foram ajustados para ser mais fácil de perceber o que se pretende

void merge_aluno(Uni * bd, int inicio, int meio, int fim) {
    int tamanho_esquerda = meio - inicio + 1; //Limitar a metade esquerda
    int tamanho_direita = fim - meio;

    //Criar arrays temporários para esquerda e direita
    Estudante * esquerda = malloc(tamanho_esquerda * sizeof(Estudante));
    Estudante * direita = malloc(tamanho_direita * sizeof(Estudante));

    //Copiar o conteúdo para os arrays temporários. É COPIADO TODO O CONTEÚDO DO ARRAY.
    for(int i = 0; i < tamanho_esquerda; i++) 
        esquerda[i] = bd->aluno[inicio + i];
    
    for(int i = 0; i < tamanho_direita; i++) 
        direita[i] = bd->aluno[meio + 1 + j];
    
    int indice_esquerda = 0;
    int indice_direita = 0;
    int indice = inicio;

    while(indice_esquerda < tamanho_esquerda && indice_direita < tamanho_direita) {
        if (esquerda[indice_esquerda].codigo <= direita[indice_direita].codigo) { 
            bd->aluno[indice] = esquerda[indice_esquerda]; //Estamos a copiar todo o conteúdo da struct, logo já fica tudo ordenado, e não apenas o código
            indice_esquerda++;
        }
        else {
            bd->aluno[indice] = direita[indice_direita];
            indice_direita++;
        }
        indice++; 
    }
    //Copiar elementos do array da direita que sobraram (ex: numeros da esquerda > num direita)
    while(indice_esquerda < tamanho_esquerda) {
        bd->aluno[indice] = esquerda[indice_esquerda];
        indice_esquerda++;
        indice++;
    }

    //Copiar elementos do array da esquerda(ex: o primeiro while terminou pois o tamanho do array esquerda < direita)
    while(indice_direita < tamanho_direita) {
        bd->aluno[indice] = direita[indice_direita];
        indice_direita++;
        indice++;
    }

    free(esquerda);
    free(direita);
}

void merge_sort_aluno(Uni * bd, int inicio, int fim) {
    if (inicio < fim) { //Se inicio >= fim, o array tem 1 
        int meio = inicio + (fim - inicio) / 2;
        merge_sort_aluno(bd, inicio, meio); //Ordena a primeira metade
        merge_sort_aluno(bd, meio + 1, fim); //Ordena a segunda metade
        //Isto vai ser usado recursivamente, pelo que cada metade vai ser novamente cortada a metade,... até o array ter um elemento

        merge_aluno(bd, inicio, meio, fim); //Combina as duas metades ordenadas
    }
}

//Igual mas para o array de escolares
void merge_escolares(Uni * bd, int inicio, int meio, int fim) {
    int tamanho_esquerda = meio - inicio + 1; //Limitar a metade esquerda
    int tamanho_direita = fim - meio;

    //Criar arrays temporários para esquerda e direita
    Dados * esquerda = malloc(tamanho_esquerda * sizeof(Dados));
    Dados * direita = malloc(tamanho_direita * sizeof(Dados));

    //Copiar o conteúdo para os arrays temporários. É COPIADO TODO O CONTEÚDO DO ARRAY.
    for(int i = 0; i < tamanho_esquerda; i++) 
        esquerda[i] = bd->escolares[inicio + i];
    
    for(int i = 0; i < tamanho_direita; i++) 
        direita[i] = bd->escolares[meio + 1 + j];
    
    int indice_esquerda = 0;
    int indice_direita = 0;
    int indice = inicio;

    while(indice_esquerda < tamanho_esquerda && indice_direita < tamanho_direita) {
        if (esquerda[indice_esquerda].codigo <= direita[indice_direita].codigo) { 
            bd->escolares[indice] = esquerda[i]; //Estamos a copiar todo o conteúdo da struct, logo já fica tudo ordenado, e não apenas o código
            indice_esquerda++;
        }
        else {
            bd->escolares[indice] = direita[j];
            indice_direita++;
        }
        indice++; 
    }
    //Copiar elementos do array da direita que sobraram (ex: numeros da esquerda > num direita)
    while(indice_esquerda < tamanho_esquerda) {
        bd->escolares[indice] = esquerda[indice_esquerda];
        indice_esquerda++;
        indice++;
    }

    //Copiar elementos do array da esquerda(ex: o primeiro while terminou pois o tamanho do array esquerda < direita)
    while(indice_direita < tamanho_direita) {
        bd->escolares[indice] = direita[indice_direita];
        indice_direita++;
        indice++;
    }

    free(esquerda);
    free(direita);
}

void merge_sort_escolares(Uni * bd, int inicio, int fim) {
    if (inicio < fim) { //Se inicio >= fim, o array tem 1 
        int meio = inicio + (fim - inicio) / 2;
        merge_sort_escolares(bd, inicio, meio); //Ordena a primeira metade
        merge_sort_escolares(bd, meio + 1, fim); //Ordena a segunda metade
        //Isto vai ser usado recursivamente, pelo que cada metade vai ser novamente cortada a metade,... até o array ter um elemento

        merge_escolares(bd, inicio, meio, fim); //Combina as duas metades ordenadas
    }
} //Termina aqui o bloco da implementação de merge sort

//NÃO FAZ REALOCAÇÕES DE TAMANHO TOTAL
//Insere o código do aluno
void ordenar_ao_inserir(int codigo, Uni * bd, int indice_aluno, int indice_escolares) {
    //Ordenar aluno
    for(int i = bd->tamanho_aluno; i > indice_aluno; i--) {
        bd->aluno[i] = bd->aluno[i - 1]; //Copiamos a struct inteira
    }
    bd->aluno[indice_aluno].codigo = codigo;
    //Ordenar escolares
    for(int i = bd->tamanho_escolares; i > indice_escolares; i--) {
        bd->escolares[i] = bd->escolares[i - 1].;
    }
    bd->escolares[indice_escolares].codigo = codigo;
}

//Altera o tamanho total do array. NÃO FAZ REALOCAÇÕES DE TAMANHO TOTAL
//Insere o código do aluno
void ordenar_ao_eliminar(int codigo, Uni * bd) {
    int indice_aluno = procurar_codigo_aluno(codigo, bd);
    if (indice_aluno < 0) return; //o código não existe

    //i começa no índice que queremos eliminar(vai ser substituído)
    for(int i = indice_aluno; i < bd->tamanho_aluno - 1; i++) {
        bd->aluno[i].codigo = bd->aluno[i + 1].codigo;
    }
    bd->tamanho_aluno -= 1;

    int indice_escolares = procurar_codigo_escolares(codigo - 1, bd);
    if (indice_escolares < 0) return;

    for(int i = indice_escolares; i < bd->tamanho_escolares - 1; i++) {
        bd->escolares[i].codigo = bd->escolares[i + 1].codigo;
    }
    bd->tamanho_escolares -= 1;
}

//Menus

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

//Inserção/leitura de dados

//Se for para ler, usar str = NULL, modo('1'/'0') para verificar se queremos imprimir mensagens de erro
void ler_data(Estudante * aluno, char * str, const char modo) {

	char data[11]; //Vamos usar o formato DD-MM-AAAA (10 caracteres + \0)
    char erro = '0'; //1 há erro

    short dia_temp, mes_temp, ano_temp; //Variáveis temporárias para armazenar os valores lidos
	do {
        if (!str) { //Se str for NULL queremos ler a data
            erro = '0';
            printf("Data de nascimento (DD-MM-AAAA): ");
            if(fgets(data, sizeof(data), stdin) == NULL) { //stdin porque é daí que lemos os dados; //fgets retorna um ponteiro para "data", logo verificamos se é NULL ocorreu um erro
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

void inserir_estudante(Uni * bd) {
    //A metodologia vai ser colocar tudo no final do array e depois ordená-lo
    
    char repetir = 'n'; //s/n
    limpar_buffer();
    do {
        //Realocação dentro do do{}while para evitar que sejam inseridos alunos até não haver espaço
        if (bd->capacidade_aluno <= bd->tamanho_aluno) { //Trata os casos em que não há espaço livre
            if (!realocar_aluno(bd, '1') || !realocar_escolares(bd, '1')) { 
                printf("Ocorreu um erro ao alocar memória para o aluno. A retornar.\n");
                return;
            }
            inicializar_structs(bd, bd->tamanho_aluno);
        }
        int posicao_insercao_aluno;
        int posicao_insercao_escolares;
        limpar_terminal(); //Caso de repetição
        repetir = 'n';
        do {
            int codigo_temp = -1;
            int valido = 0;
            limpar_terminal();
            printf("Insira o código do estudante: ");
            if (scanf("%d", &codigo_temp) != 1) { //Verifica entradas inválidas como letras
                printf("Código inválido! Insira um número inteiro positivo.\n");
                limpar_buffer();
                pressione_enter();
                continue; //Continue faz com que salte o resto do loop e passe à próxima iteração
            }
            
            posicao_insercao_aluno = validar_codigo_ao_inserir(codigo_temp, bd);
            posicao_insercao_escolares = procurar_codigo_escolares(codigo_temp, bd);
            if(posicao_insercao_aluno < 0) { //Não se verifica escolares pois nunca há códigos em escolares que não estejam em aluno
                posicao_insercao_aluno = -(posicao_insercao_aluno + 1);
            }
            else {
                limpar_buffer();
                continue;
            }

            //Esta função já adiciona o código à struct na posição passada.
            ordenar_ao_inserir(codigo_temp, bd, posicao_insercao_aluno, posicao_insercao_escolares);
            break; 
        } while (1);
        
        do {
            char * nome_temp = NULL;
            printf("Insira o nome do estudante: ");
            nome_temp = ler_linha_txt(stdin, NULL);
            if(!validar_nome(nome_temp,bd, '1')) {
                limpar_buffer();
                pressione_enter();
                free(nome_temp);
                continue;
            }
            strcpy(bd->aluno[posicao_insercao_aluno].nome, nome_temp);
            free(nome_temp);
            break;
        } while(1);

        do {
            printf("Insira a data de nascimento do estudante (DD-MM-AAAA): ");
            ler_data(bd->aluno[posicao_insercao_aluno], NULL, '1');
            limpar_buffer();
        } while (bd->aluno[posicao_insercao_aluno].nascimento.dia == 0); //Apenas verificamos um pois em ler_data a data só é copiada para as structs se toda ela for válida
        
        do {
            char * nacionalidade_temp = NULL;
            printf("Insira a nacionalidade do estudante: ");
            nacionalidade_temp = ler_linha_txt(stdin, NULL);

            if(!validar_nacionalidade(nacionalidade_temp, bd, '1')) {
                limpar_buffer();
                pressione_enter();
                free(nacionalidade_temp);
                continue;
            }
            strcpy(bd->aluno[posicao_insercao_aluno].nacionalidade, nacionalidade_temp);
            free(nacionalidade_temp);
            break;
        } while(1);

    	do {
            short matriculas_temp = 0;
            printf("Insira o número de matrículas do estudante: ");
            if (scanf("%hd", &matriculas_temp) != 1) { //Verifica entradas inválidas como letras
                printf("Número de matrículas inválido! Insira um número inteiro positivo.\n");
                limpar_buffer();
                pressione_enter();
                continue; //Continue faz com que salte o resto do loop e passe à próxima iteração
            }
            if (matriculas_temp < 0 || matriculas_temp > MAX_MATRICULAS) {
                printf("Número de matrículas é inválido. Deve estar entre 0 e %d.\n", MAX_MATRICULAS);
                limpar_buffer();
                pressione_enter();
                continue;
            }
            bd->escolares[posicao_insercao_escolares].matriculas = matriculas_temp;
            break;
        } while(1);

        do {
            short ects_temp = 0;
            printf("Insira o número de créditos ECTS do estudante: ");
            if (scanf("%hd", &ects_temp) != 1) { //Verifica entradas inválidas como letras
                printf("Créditos ECTS inválido! Insira um número inteiro positivo.\n");
                limpar_buffer();
                pressione_enter();
                continue; //Continue faz com que salte o resto do loop e passe à próxima iteração
            }
            if (ects_temp < 0 || ects_temp > MAX_ECTS) {
                printf("O número de créditos ECTS é inválido. Deve estar entre 0 e %d.\n", MAX_ECTS);
                limpar_buffer();
                pressione_enter();
                continue;
            }
            bd->escolares[posicao_insercao_escolares].ects = ects_temp;
            break;
        } while(1);

        do {
            short ano_atual_temp = 0;
            printf("Insira o ano atual do estudante: ");
            if (scanf("%hd", &ano_atual_temp) != 1) { //Verifica entradas inválidas como letras
                printf("Ano atual inválido! Insira um número inteiro positivo.\n");
                limpar_buffer();
                pressione_enter();
                continue; //Continue faz com que salte o resto do loop e passe à próxima iteração
            }
            if (ano_atual_temp < 0 || ano_atual_temp > MAX_ANO_ATUAL) {
                printf("O ano atual é inválido. Deve estar entre 0 e %d.\n", MAX_ANO_ATUAL);
                limpar_buffer();
                pressione_enter();
                continue;
            }
            bd->escolares[posicao_insercao_escolares].ano_atual = ano_atual_temp;
            break;
        } while (1);

        do {
            float media_temp = 0.0;
            printf("Insira a média atual do estudante: ");
            if (scanf("%f", &media_temp) != 1) { //Verifica entradas inválidas como letras
                printf("Média atual inválido! Insira um número entre 0 e 20.\n");
                limpar_buffer();
                pressione_enter();
                continue; //Continue faz com que salte o resto do loop e passe à próxima iteração
            }
            if (media_temp < 0 || media_temp > 20) {
                printf("A média atual é inválida. Deve estar entre 0 e 20.\n");
                limpar_buffer();
                pressione_enter();
                continue;
            }
            bd->escolares[posicao_insercao_escolares].media_atual = media_temp;
            break;
        } while(1);

        //Incrementar o tamanho dos arrays
        bd->tamanho_aluno += 1;
        bd->tamanho_escolares += 1;

        do {
            printf("Quer inserir mais estudantes? (S/N): ");
            scanf(" %c", &repetir);
            limpar_buffer();
            if (repetir == 's' || repetir == 'S') {
                repetir = 's';
                break;
            }
            else if (repetir == 'n' || repetir == 'N') {
                //A ordenação já foi feita
                return;
            }
            printf("Opção inválida. Use S ou N.\n");
            pressione_enter();
        } while (1);
    }while(1); //Não precisamos de verificar repetir pois só chega aqui se repetir == 's'
}

//Funções auxiliares

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

void limpar_buffer() {
    int lixo;
    while ((lixo = getchar()) != '\n' && lixo != EOF);
}

//Limpar o terminal consoante o sistema operativo
void limpar_terminal() {
    #ifdef _WIN32 //_WIN32 é uma variável local automaticamente pre-definida pelo windows em todos os sistemas
        system("cls"); //Sistemas windows
    #else
        system("clear"); //Sistemas linux, macOs, etc
    #endif
}

void pressione_enter() {
    printf("Pressione Enter para continuar.\n");
    while (getchar() != '\n'); //Caso o user escreva algo e não enter limpar buffer
}

void colocar_terminal_utf8() {
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



