#include "headers.h"

const short ANO_ATUAL = 2025; 
const short ANO_NASC_LIM_INF = 1908;

//Ficheiros e gestão de dados

//Linha é alocada dinamicamente, pelo que deve ser libertada quando já não for necessária.
//Lê uma linha completa do ficheiro/teclado(ficheiro = stdin) sem que haja a possibilidade de ficar algo por ler
//n_linhas NULL se não estivermos a ler de um ficheiro
//LÊ O \n NO FINAL DA LINHA!!!
char * ler_linha_txt(FILE * ficheiro, int * n_linhas) {
    if(!ficheiro) return NULL;
    //n_linhas não será inicializado aqui
    char buffer[TAMANHO_INICIAL_BUFFER]; //Buffer para armazenar parte da linha
    size_t tamanho_total = 0; //Comprimento da linha; size_t pois é sempre >0 e evita conversões que podem levar a erros com outras funções
    char * linha = NULL;

    while (fgets(buffer, sizeof(buffer), ficheiro)) { //fgets le ate buffer -1 caracteres ou '\n' ou EOF
        size_t tamanho = strlen(buffer); //Calcula o tamanho do texto lido
        //NOTA IMPORTANTE: se o ponteiro passado para realloc for nulo, ele funciona como o malloc
        char * temp = realloc(linha, tamanho_total + tamanho + 1); //+1 para o nul char
        if (!temp) {
            free(linha);
            return NULL; //Verificar isto mais tarde!!Pode terminar o loop em carregar_dados mais cedo que o suposto
        }
        linha = temp; //atualizar o ponteiro linha para apontar para a nova memória

        //Copiar o conteúdo lido para a linha total
        memmove(linha + tamanho_total, buffer, tamanho); //linha + tamanho_total é um ponteiro para a posição da memória seguinte para onde a próxima parte de buffer será copiada
        tamanho_total += tamanho;
        linha[tamanho_total] = '\0'; //Se houver mais que uma leitura, o primeiro char da segunda leitura de fgets irá substituir o nul char, pelo que fica sempre no fim

        //Verificamos se a linha está completa
        if (buffer[tamanho_total - 1] == '\n') {//se tudo tiver sido copiado, o ultimo caracter do buffer(e da linha tbm) será o '\n'
            if (n_linhas != NULL) (*n_linhas)++;
            linha[tamanho_total - 1] = '\0'; //Substitui o \n por \0
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

//Carrega os dados para a struct.
//Faz verificações de duplicados, etc.
//Ordena os arrays.
void carregar_dados(const char * nome_ficheiro_dados,const char * nome_ficheiro_escolar, Uni * bd) { 
    FILE * dados = fopen(nome_ficheiro_dados, "r");
    FILE * situacao_escolar = fopen(nome_ficheiro_escolar, "r");
    FILE * erros = fopen(ERROS_TXT, "w"); //Vai anexar ao ficheiro de erros os erros encontrados
    if (!erros) return; //Se prosseguissemos iria resultar em segmentation fault
    fprintf(erros, "------------------------------------------NOVA ITERAÇÃO DO PROGRAMA------------------------------------------\n\n\n");
    int n_linhas = 0;
    char * linha = NULL; //Ponteiro para armazenar uma linha lida do ficheiro
    char primeiro_erro = '1'; //'1' significa que ainda não houve erro. 
    char erro_geral = '0'; //Flag para dizer ao user que houve erro

    //Esta secção vai copiar, linha a linha, o conteúdo do ficheiro para a memória, nomeadamente na struct Estudante
    if(dados) { 
        int codigo_temp; //Necessário para passar para string_para_int
        while ((linha = ler_linha_txt(dados, &n_linhas)) != NULL) {
            char erro = '0'; //diferente de primeiro_erro pois este vai marcar se existe um erro em cada iteração
            char * parametros[PARAMETROS_ESTUDANTE] = {NULL}; //Array com PARAMETROS casas, onde cada pode armazenar um ponteiro para char (ou seja, uma string)
            int num_parametros = 0; //Armazena o numero REAL de parametros
            separar_parametros(linha, parametros, &num_parametros); //extrai os dados já formatados corretamente para parametros
            
            if(num_parametros == PARAMETROS_ESTUDANTE) {
                //+1 é FUNDAMENTAL, caso contrário a inicialização não é feita corretamente nos indices da borda
                if (bd->tamanho_aluno + 1 >= bd->capacidade_aluno) {
                    if (!realocar_aluno(bd, '0')) {
                        printf("Ocorreu um erro a gerir a memória.\n"); //Averiguar melhor 
                        for(int i = 0; i < num_parametros; i++) //Libertar a memória para os parâmetros alocados dinamicamente até ao momento
                            free(parametros[i]);
                        free(linha);
                        fclose(dados);
                        fclose(erros);
                        return;
                    }
                    //A realocação do campos de nome é feita em validar_nome
                }
                 //Código
                if (!string_para_int(parametros[0], &codigo_temp)) { //APENAS SE VERIFICA SE O CÓDIGO DE FACTO É VÁLIDO DEPOIS DE CARREGAR TUDO E ORDENAR TUDO
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_dados);
                    erro = '1';
                    fprintf(erros,"Linha %d: %s\n",n_linhas, linha);
                    fprintf(erros, "Razão: O código é inválido!\n\n");
                }
                else if(codigo_temp <= 0) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_dados);
                    erro = '1';
                    fprintf(erros,"Linha %d: %s\n",n_linhas, linha);
                    fprintf(erros, "Razão: O código é inválido!Deve ser maior que 0.\n\n");
                }
                //Nome
                if (!validar_nome(&(bd->aluno[bd->tamanho_aluno]), parametros[1], '0')) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_dados);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: O nome é inválido!\n\n");
                }
                //Data de nascimento
                ler_data(&(bd->aluno[bd->tamanho_aluno]), parametros[2], '0');
                if (bd->aluno[bd->tamanho_aluno].nascimento.dia == 0) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_dados);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: A data de nascimento é inválida!\n\n");
                }
                //Nacionalidade
                if (!validar_nacionalidade(parametros[3], '0')) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_dados);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: A nacionalidade é inválida!\n\n");
                }
                //Copiar os parâmetros caso não haja erros
                if (erro == '0') {
                    bd->aluno[bd->tamanho_aluno].codigo = codigo_temp; 
                    strcpy(bd->aluno[bd->tamanho_aluno].nome, parametros[1]);
                    //Data de nascimento já é copiada se for válida.
                    strcpy(bd->aluno[bd->tamanho_aluno].nacionalidade, parametros[3]);
                    bd->tamanho_aluno += 1; //Aumentar o tamanho do array
                }
                //A data pode ter sido copiada e depois ter havido erro
                else {
                    bd->aluno[bd->tamanho_aluno].nascimento.dia = 0;
                    bd->aluno[bd->tamanho_aluno].nascimento.mes = 0;
                    bd->aluno[bd->tamanho_aluno].nascimento.ano = 0;
                }
            }
            else if (num_parametros < PARAMETROS_ESTUDANTE) {
                verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_dados);
                fprintf(erros, "Linha %d inválida: %s\n", n_linhas, linha);
                fprintf(erros, "Razão: A linha tem parâmetros insuficientes. Verifique se há parâmetros não separados por \\t (obrigatório)\n\n"); //Separar cada erro com uma linha
            }
            else {
                verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_dados);
                fprintf(erros, "Linha %d inválida: %s\n", n_linhas, linha);
                fprintf(erros, "Razão: A linha tem parâmetros a mais. Verifique se algum parâmetro contém um \\t (não pode).\n\n");
            }
            
            //Libertamos a memória alocada para os parametros
            for(int i = 0; i < num_parametros; i++) 
                free(parametros[i]);

            free(linha); //Libertamos a memória alocada para a linha presente
        }
        fclose(dados);
        //Ordenar o array de aluno
        merge_sort_aluno(bd, 0, bd->tamanho_aluno - 1);
    }
    else {
        printf("Ocorreu um erro a abrir o ficheiro '%s'.\n", nome_ficheiro_dados);
        return; //Não pode haver dados escolares sem ficha pessoal, logo mesmo que o outro ficheiro abrisse, era inutil
    }
    
    if (erro_geral == '0' && primeiro_erro == '0') erro_geral = '1'; //flag para especificar que houve erro em geral
    //Necessário porque primeiro_erro muda consoante a operação a fazer, logo não é viável
    n_linhas = 0;
    primeiro_erro = '1';
    if(situacao_escolar) { //apenas se a abertura tiver sido bem sucedida
        int codigo_temp;
        short matriculas_temp;
        short ects_temp;
        short ano_atual_temp;
        float media_temp;
        while ((linha = ler_linha_txt(situacao_escolar, &n_linhas)) != NULL) {
            char erro = '0'; 
            char * parametros[PARAMETROS_DADOS_ESCOLARES] = {NULL}; 
            int num_parametros = 0; 
            separar_parametros(linha, parametros, &num_parametros); //retorna num_parametros a zero se algo falhar.
            
            if(num_parametros == PARAMETROS_DADOS_ESCOLARES) {
                if (bd->tamanho_escolares + 1 >= bd->capacidade_escolares) {
                    if (!realocar_escolares(bd, '0')) {
                        printf("Ocorreu um erro a gerir a memória.\n"); //Averiguar melhor 
                        for(int i = 0; i < num_parametros; i++) 
                            free(parametros[i]);
                        free(linha);
                        fclose(situacao_escolar);
                        fclose(erros);
                        return;
                    }
                }
                 //Código
                if (!string_para_int(parametros[0], &codigo_temp)) { 
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n",n_linhas, linha);
                    fprintf(erros, "Razão: O código é inválido!\n\n");
                }
                else if(codigo_temp <= 0) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n",n_linhas, linha);
                    fprintf(erros, "Razão: O código é inválido!Deve ser maior que 0.\n\n");
                }
                //Matriculas
                if (!string_para_short(parametros[1], &matriculas_temp)) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: O número de matrículas é inválido!\n\n");
                }
                else if (matriculas_temp < 0 || matriculas_temp > MAX_MATRICULAS) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: O número de matrículas é inválido. Deve estar entre 0 e %d.\n\n", MAX_MATRICULAS);
                }
                //ECTS
                if (!string_para_short(parametros[2], &ects_temp)) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: O número de créditos é inválido!\n\n");
                }
                else if (ects_temp < 0 || ects_temp > MAX_ECTS) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: O número de créditos é inválido! Deve estar entre 0 e %d.\n\n", MAX_ECTS);
                }
                //Ano atual
                if (!string_para_short(parametros[3], &ano_atual_temp)) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: O ano atual é inválido!\n\n");
                }
                else if (ano_atual_temp < 1 || ano_atual_temp > MAX_ANO_ATUAL) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: O ano atual é inválido! Deve estar entre 1 e %d.\n\n", MAX_ANO_ATUAL);
                }
                //Media atual do aluno
                if (!string_para_float(parametros[4], &media_temp)) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: A média é inválida!\n\n");
                }
                else if(media_temp < 0 || media_temp > 20) {
                    verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                    erro = '1';
                    fprintf(erros, "Linha %d: %s\n", n_linhas, linha);
                    fprintf(erros, "Razão: A média é inválida! Deve estar entre 0 e 20.\n\n");
                }
                if (erro == '0') { //ainda não houve erros
                    bd->escolares[bd->tamanho_escolares].codigo = codigo_temp; //Copiar código
                    bd->escolares[bd->tamanho_escolares].matriculas = matriculas_temp;
                    bd->escolares[bd->tamanho_escolares].ects = ects_temp;
                    bd->escolares[bd->tamanho_escolares].ano_atual = ano_atual_temp;
                    bd->escolares[bd->tamanho_escolares].media_atual = media_temp;
                    bd->tamanho_escolares += 1; //Aumentar o tamanho do array
                }
            }
            else if (num_parametros < PARAMETROS_DADOS_ESCOLARES) {
                verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                erro = '1';
                fprintf(erros, "Linha %d inválida: %s\n", n_linhas, linha);
                fprintf(erros, "Razão: A linha tem parâmetros insuficientes.\n\n"); //Separar cada erro com uma linha
            }
            else {
                verificar_primeiro_erro(erros, &primeiro_erro, nome_ficheiro_escolar);
                erro = '1';
                fprintf(erros, "Linha %d inválida: %s\n", n_linhas, linha);
                fprintf(erros, "Razão: A linha tem parâmetros a mais.\n\n");
            }
            
            //Libertamos a memória alocada para os parametros
            for(int i = 0; i < num_parametros; i++) 
                free(parametros[i]);

            free(linha); //Libertamos a memória alocada para a linha presente
        }
        fclose(situacao_escolar);
        //Ordenar o array de escolares
        merge_sort_escolares(bd, 0, bd->tamanho_escolares - 1);
    }
    else {
        printf("Ocorreu um erro a abrir o ficheiro '%s'.\n", nome_ficheiro_escolar);
    }
    if (erro_geral == '0' && primeiro_erro == '0') erro_geral = '1';
    primeiro_erro = '1';
    //Agora que está tudo ordenado vamos procurar por possíveis erros como:
    //Códigos duplicados (o merge sort mantém a ordem pela qual foram lidos pelo que mantemos o primeiro que foi lido)
    //Códigos que estejam em escolares mas não estejam em aluno
    verificar_codigos_duplicados(bd, erros); //verifica duplicados em ambos os arrays
    verificar_codigos_escolares_sem_aluno(bd, erros, &primeiro_erro);
    fprintf(erros, "------------------------------------------FIM DE ITERAÇÃO------------------------------------------\n\n\n");

    if (erro_geral == '0' && primeiro_erro == '0') erro_geral = '1';
    if (erro_geral == '1') { 
        printf("\nOcorreram erros a carregar os dados. Pode consultar o que foi descartado e porquê no ficheiro %s\n", ERROS_TXT);
        pressione_enter();
    }
    fclose(erros);
}

//Guarda os dados no mesmo ficheiro do qual leu inicialmente, ordenados pelo código.
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
        fprintf(dados, "%d%c%hd%c%hd%c%hd%c%.1f\n",
            bd->escolares[i].codigo, SEPARADOR,
            bd->escolares[i].matriculas, SEPARADOR,
            bd->escolares[i].ects, SEPARADOR, 
            bd->escolares[i].ano_atual, SEPARADOR,
            bd->escolares[i].media_atual);
    }
    fclose(dados);
    return;
}

//Pede ao user se quer efetuar uma cópia em ficheiro da listagem atual.
//Mostra os formatos disponíveis e pede o nome do ficheiro a escrever.
//Verifica se o nome é válido.
//Retorna um ponteiro para o ficheiro ABERTO, se for o caso.
//Retorna NULL em caso de não querer listar ou erro.
//São adereçados todos os erros com uma mensagem ao user.
FILE * pedir_listagem(char * formato_selecionado) {
    const char * formatos[] = {".txt", ".csv"};
    short opcao;
    char * nome_ficheiro;
    FILE * ficheiro = NULL;

    printf("Deseja guardar a listagem num ficheiro? (S/N):");
    if (sim_nao()) {
        opcao = mostrar_menu(menu_formatos_disponiveis, '0', '2') - '0';
        if (opcao == 0) return NULL; //Sair

        opcao--; //Fazemos a opcao ser igual ao indice de formatos
        strcpy(formato_selecionado, formatos[opcao]); 

        //Nome do ficheiro fica à escolha do utilizador.
        do {
            printf("Nome do ficheiro a guardar: ");
            nome_ficheiro = ler_linha_txt(stdin, NULL);
            if (validar_nome_ficheiro(nome_ficheiro)) break;
            else free(nome_ficheiro);
        } while(1);

        //Garantir que a string a que vamos concatenar o nome da extensão do ficheiro tem tamanho suficiente para isso.
        char * temp = realloc(nome_ficheiro, strlen(nome_ficheiro) + strlen(formatos[opcao]) + 1);
        if (!temp) { //Realocação falhou
            free(nome_ficheiro);
            printf("Erro ao alocar memória. Por favor tente novamente mais tarde.\n");
            return NULL;
        }
        nome_ficheiro = temp;
        //strcat dá append na 2º string ao fim da 1ª.
        strcat(nome_ficheiro, formatos[opcao]);
        ficheiro = validar_ficheiro_e_abrir(nome_ficheiro);
        if (!ficheiro) {
            free(nome_ficheiro);
            return NULL; //Caso seja inválido
        }
        limpar_terminal();
        printf("O ficheiro \"%s\" foi aberto com sucesso!\n", nome_ficheiro);
        return ficheiro; //Retornamos um ponteiro para o ficheiro.
    }
    return NULL;
}

//Gestão de memória

//Inicializa tudo com -1 (até strings) exceto data(0).
//Inicializa de indice_aluno até bd->capacidade_aluno.
//Inicializa memória para nome e nacionalidade dinamicamente.
void inicializar_aluno(Uni * bd, int indice_aluno) {
    if (!bd || !bd->aluno || !bd->escolares) return;

    for (int i = indice_aluno; i < bd->capacidade_aluno; i++) {
        //Struct estudante
        //Aloca memória para guardar a nacionalidade e o nome (primeiro pois pode falhar, e seria escusado inicializar outras coisas antes)
        bd->aluno[i].nacionalidade = (char *) malloc (MAX_STRING_NACIONALIDADE * sizeof(char));
        bd->aluno[i].nome = (char *) malloc (TAMANHO_INICIAL_NOME * sizeof(char));
        if(bd->aluno[i].nacionalidade == NULL || bd->aluno[i].nome == NULL) {
            for(int j = indice_aluno; j < i; j++) {
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
}

//Inicializa tudo com -1 exceto prescrever e finalista('-').
void inicializar_escolares(Uni * bd, int indice_escolares) {
    for(int i = indice_escolares; i < bd->capacidade_escolares; i++) {
        bd->escolares[i].codigo = -1;
        bd->escolares[i].matriculas = -1;
        bd->escolares[i].ects = -1;
        bd->escolares[i].ano_atual = -1;
        bd->escolares[i].media_atual = -1;
        bd->escolares[i].prescrever = '-';
        bd->escolares[i].finalista = '-';
    }
}

void inicializar_estatisticas(Estatisticas * stats) {
    stats->finalistas = 0;
    stats->media = 0.0;
    stats->media_matriculas = 0.0;
    stats->risco_prescrever = 0;
    stats->atualizado = '0'; //Inicializamos a 0 pois quando carregamos os dados, estamos possivelmente a introduzir dados novos que podem contar para a estatística.
}

//Duplica o espaço atual. Não inicializa o espaço alocado. modo = '1' para endereçar erros.
int realocar_aluno(Uni * bd, const char modo) {
    int tamanho_novo = bd->capacidade_aluno * 2;
    Estudante * temp = (Estudante *) realloc(bd->aluno, tamanho_novo * sizeof(Estudante));

    if (!temp) {
        if (modo == '1') printf("Ocorreu um erro ao realocar memória para o aluno. A encerrar.\n");
        return 0;
    }
    bd->aluno = temp;
    bd->capacidade_aluno = tamanho_novo;
    //Não se faz free de temp pois temp aponta para a mesma morada de bd->aluno, logo ao dar free em temp, estavamos a dar free em bd->alunos, e perdeíamos os dados
    inicializar_aluno(bd, bd->tamanho_aluno); //Inicializar logo a memória alocada
    return 1;
}

int realocar_escolares(Uni * bd, const char modo) {
    int tamanho_novo = bd->capacidade_escolares * 2;
    Dados * temp = (Dados *) realloc(bd->escolares, tamanho_novo * sizeof(Dados));

    if (!temp) {
        if (modo == '1') printf("Ocorreu um erro ao realocar memória para os dados escolares. A encerrar.\n");
        return 0;
    }
    bd->escolares = temp;
    bd->capacidade_escolares = tamanho_novo;
    inicializar_escolares(bd, bd->tamanho_escolares);
    return 1;
}

int realocar_nome(Estudante * aluno, const char modo) {
    int tamanho_novo = strlen(aluno->nome) * 2;
    char * temp = realloc(aluno->nome, tamanho_novo * sizeof(char));

    if (!temp) {
        if (modo == '1') printf("Erro ao realocar memória para o nome!\n");
        return 0;
    }

    aluno->nome = temp;
    return 1;
}

//Procura e validações

//Apenas retorna 0 em caso de erro.
//Retorno < 0 se não existe. Representa a posição de inserção (indice no array) - 1.
//Retorno > 0 se existe. Representa o índice do código no array + 1.
int procurar_codigo_aluno(int codigo, Uni * bd) {
    if (!bd || !bd->aluno || bd->tamanho_aluno <= 0) {
        return 0;
    }
    if (bd->aluno[0].codigo > codigo) return -1; //Se o array está sempre ordenado e o codigo é menor que que do array 0, então não existe e está abaixo do indice zero
    int limInf, meio, limSup;
    limInf = 0;
    limSup = bd->tamanho_aluno - 1;

    while (limSup >= limInf) {
        meio = (limSup + limInf) / 2;
        if (bd->aluno[meio].codigo == codigo) return (meio + 1) ; //Dá return do índice
        else {
            if (bd->aluno[meio].codigo < codigo) limInf = meio + 1;
            else limSup = meio - 1;
        }
    }
    return -(limInf + 1); //Retorna a posição de inserção + 1(fazer < 0 para verificar código)
    //o +1 está a precaver no caso de limInf ser 0, para distinguir do return de um erro
}

//Apenas retorna 0 em caso de erro.
//Retorno < 0 se não existe. Representa a posição de inserção (indice no array) - 1.
//Retorno > 0 se existe. Representa o índice do código no array + 1.
int procurar_codigo_escolares(int codigo, Uni * bd) {
    if (!bd || !bd->aluno || bd->tamanho_escolares <= 0) {
        return 0;
    }
    if (bd->escolares[0].codigo > codigo) return -1; //Retornamos a posição de inserção -1 pois o codigo é menor que todos os outros no array. -1 pois -(0+1)
    int limInf, limSup, meio;
    limInf = 0;
    limSup = bd->tamanho_escolares - 1;

    while (limSup >= limInf) {
        meio = (limSup + limInf) / 2;
        if (bd->escolares[meio].codigo == codigo) return (meio + 1); //Dá return do índice + 1 para distinguir do 0 de erro.
        else {
            if (bd->escolares[meio].codigo < codigo) limInf = meio + 1;
            else limSup = meio - 1;
        }
    }
    return -(limInf + 1); //Não há esse código, RETORNA A POSIÇÃO DE INSERÇÃO + 1
}

//Faz a validação do código e retorna -(indice + 1).
//Caso não seja válido retorna 0.
int validar_codigo_ao_inserir(int codigo, Uni * bd) {
    if(codigo <= 0) {
        printf("Código inválido! Insira um número inteiro positivo.\n");
        pressione_enter();
        return 0;
    }
    int temp = procurar_codigo_aluno(codigo, bd); //basta procurar no aluno porque não pode haver nenhum código em escolares que não esteja em aluno
    
    if(temp > 0) {
        printf("O código já existe! Insira um código diferente.\n");
        pressione_enter();
        return 0;
    }
    //Se não existir o código podesmos usá-lo(<0).
    else if (temp < 0) {
        return temp; //Retornamos a posição onde será inserido
    }
    return 0; //Caso seja 0
}

//Faz a validação do código e retorna indice + 1
//Caso não seja válido retorna 0.
int validar_codigo_eliminar(int codigo, Uni * bd) {
    if(codigo <= 0) {
        printf("Código inválido! Insira um número inteiro positivo.\n");
        pressione_enter();
        return 0;
    }
    int temp = procurar_codigo_aluno(codigo, bd); //basta procurar no aluno porque não pode haver nenhum código em escolares que não esteja em aluno
    
    if(temp > 0) {
        return temp; //codigo existe logo pode ser eliminado
    }
    //Se não existir o código podesmos usá-lo(<0).
    else if (temp < 0) {
        printf("O código não existe! Insira um código diferente.\n");
        pressione_enter();
        return 0; //<0 significa que foi retornada a posição onde o código devia ser inserido, logo não existe.
    }
    return 0; 
}

//Retorna 1 se o nome for válido. 0 caso contrário.
int validar_nome_ficheiro(const char * nome_ficheiro) {
    //Caracteres inválidos ao escrever ficheiros.
    const char * chars_invalidos = "\\/:*?\"<>|";

    for (int i = 0; nome_ficheiro[i] != '\0'; i++) {
        //strchr é semelhante à strstr já usada, mas procura um char dentro da string(1º param).
        if (strchr(chars_invalidos, nome_ficheiro[i]) != NULL) { 
            printf("O nome do ficheiro é inválido. Por favor, escreva um nome sem os seguintes caracteres: \n");
            printf("%s\n", chars_invalidos);
            return 0;
        }
    }
    return 1;
}

//Verifica se já existe algum ficheiro com o mesmo nome.
//Retorna NULL se o ficheiro existir e o user não queira sobreescrevê-lo.
//Retorna um ponteiro para o ficheiro aberto em modo "w" nos outros casos.
FILE * validar_ficheiro_e_abrir(const char * nome) {
    FILE * ficheiro = fopen(nome, "r"); //Verificar se o fichereiro existe.
    if (ficheiro) {
        fclose(ficheiro);
        printf("Já existe um ficheiro com o nome \"%s\". Quer substituir o ficheiro no destino? (S/N) ", nome);
        if (!sim_nao()) {
            return NULL;
        }
        limpar_terminal();
    }
    //Abrimos em modo escrita como normal.
    ficheiro = fopen(nome, "w");
    if (!ficheiro) {
        printf("Ocorreu um erro a abrir o ficheiro. Por favor, tente novamente mais tarde.\n");
        return NULL;
    }
    
    return ficheiro;
}

//Usar ao carregar dados
void verificar_codigos_duplicados(Uni * bd, FILE * erros) {
    //O(n)
    char erro = '0';
    for(int i = 0; i < bd->tamanho_aluno - 1; i++) { //-1 porque o último não pode ser igual a nenhum sem ser o penúltimo
        if (bd->aluno[i].codigo == bd->aluno[i + 1].codigo) {
            if (erro == '0') {
                erro = '1';
                fprintf(erros, "\n\n"); 
                fprintf(erros, "\t\tCÓDIGOS DUPLICADOS EM %s\n\n", DADOS_TXT);
            }
            fprintf(erros, "O código %d do ficheiro de %s estava duplicado.\n", bd->aluno[i].codigo, DADOS_TXT);
            fprintf(erros, "Foi mantido o primeiro código a ser inserido, e foi eliminada a linha %d%c%s%c%02hd-%02hd-%02hd%c%s\n\n", bd->aluno[i + 1].codigo, SEPARADOR,
                bd->aluno[i + 1].nome, SEPARADOR, bd->aluno[i + 1].nascimento.dia, bd->aluno[i + 1].nascimento.mes, bd->aluno[i + 1].nascimento.ano, SEPARADOR,
                     bd->aluno[i + 1].nacionalidade);

            for(int j = i+1; j < bd->tamanho_aluno - 1; j++)
                bd->aluno[j] = bd->aluno[j + 1]; //Colocamos o próximo elemento do array na posição do elemento duplicado   
            bd->tamanho_aluno -= 1;
            i--; //Precisamos de verificar o novo elemento que ficou na posição do elemento eliminado
        }   
    }

    erro = '0';
    for(int i = 0; i < bd->tamanho_escolares - 1; i++) { 
        if (bd->escolares[i].codigo == bd->escolares[i + 1].codigo) {
            if (erro == '0') {
                erro = '1';
                fprintf(erros, "\n\n"); 
                fprintf(erros, "\t\tCÓDIGOS DUPLICADOS EM %s\n\n", SITUACAO_ESCOLAR_TXT);
            }
            fprintf(erros, "O código %d do ficheiro de %s estava duplicado.\n", bd->escolares[i].codigo, SITUACAO_ESCOLAR_TXT);
            fprintf(erros, "Foi mantido o primeiro código a ser inserido, e foi eliminada a linha %d%c%hd%c%hd%c%hd%c%.1f\n\n", bd->escolares[i + 1].codigo, SEPARADOR, 
                bd->escolares[i + 1].matriculas, SEPARADOR, bd->escolares[i + 1].ects, SEPARADOR, bd->escolares[i + 1].ano_atual, SEPARADOR, bd->escolares[i + 1].media_atual);
            
            for(int j = i+1; j < bd->tamanho_escolares - 1; j++)
                bd->escolares[j] = bd->escolares[j + 1]; 
            bd->tamanho_escolares -= 1;
            i--;
        }
    }
}

//Verifica se há algum código em escolares que não está em aluno
//Elimina e ordena o array.
void verificar_codigos_escolares_sem_aluno(Uni * bd, FILE * erros, char * primeiro_erro) {
    int i = 0, j = 0; //i - indice de escolares; j - indice de aluno
    while(i < bd->tamanho_escolares && j < bd->tamanho_aluno) {
        if(bd->escolares[i].codigo == bd->aluno[j].codigo) { //Se forem iguais avançamos nos dois
            i++;
            j++;
        } 
        else if(bd->escolares[i].codigo > bd->aluno[j].codigo)
            j++; //Se o de escolares for maior, significa que houve um "salto" no ficheiro de escolares, logo apenas avançamos este até ser igual ao de aluno
        
        else { //Isto implica que o código de escolares é menor que o de aluno. Se é menor então quer dizer que houve um salto em "aluno" e os códigos desse salto estão presentes em escolares
            verificar_primeiro_erro(erros, primeiro_erro, SITUACAO_ESCOLAR_TXT);
            fprintf(erros, "Código %d no ficheiro %s não foi encontrado no ficheiro %s.\n", bd->escolares[i].codigo, SITUACAO_ESCOLAR_TXT, DADOS_TXT);
            fprintf(erros, "Razão: Não podem existir dados escolares sem a informação pessoal do aluno!\n\n");
            
            for(int k = i; k < bd->tamanho_escolares - 1; k++) {
                bd->escolares[k] = bd->escolares[k + 1]; //Colocamos o próximo elemento do array na posição do elemento inválido.
            }
            bd->tamanho_escolares -= 1; //O array fica com menos 1 elemento
        }
    }
    
    //Caso o array de escolares seja maior do que o de aluno mas tenham começado "validos" é necessário descartar o final
    while(i < bd->tamanho_escolares) {
        verificar_primeiro_erro(erros, primeiro_erro, SITUACAO_ESCOLAR_TXT);
        fprintf(erros, "Código %d no ficheiro %s não foi encontrado no ficheiro %s.\n", bd->escolares[i].codigo, SITUACAO_ESCOLAR_TXT, DADOS_TXT);
        for(int k = i; k < bd->tamanho_escolares - 1; k++) { //Provavelmente há uma solução mais eficiente (eliminar todos de uma vez) (ver se houver tempo)
            bd->escolares[k] = bd->escolares[k + 1];
        }
        bd->tamanho_escolares -= 1; // Reduz o tamanho do array
    }
}

//modo '1' para imprimir mensagens de erro, '0' para não imprimir
int validar_data(short dia, short mes, short ano, const char modo) {
    if (ano < 1 || mes < 1 || mes > 12 || dia < 1) { //Limites dos anos são os que foram considerados mais realistas
        if (modo == '1') printf("\nPor favor, insira uma data válida.\n");
        return 0;
    }
    /*Apenas se podem inscrever alunos com mais de 16 anos (sensivelmente, porque devido ao mes de nascimento pode nao ser bem assim)
    Poderia ser resolvido com a adição de +2 constantes para mes e dia atuais mas achamos desnecessário devido a tratar-se da inscrição de alunos
    que por natureza, não têm de ser todos iguais ou ser inscritos com a mesma idade.
    */
    if (ano < ANO_NASC_LIM_INF || ano > ANO_ATUAL - 16) {
        if (modo == '1') printf("\nO ano inserido é inválido. Insira um ano entre %d e %d.\n", ANO_NASC_LIM_INF, ANO_ATUAL - 16);
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

//Realoca o array de nome. Pode ser usada ao carregar_dados
int validar_nome(Estudante * aluno, char * nome, const char modo) {
    if (!nome) {
        if (modo == '1') printf("\nNome em branco!\n");
        return 0;
    }
    int comprimento = strlen(nome);
    
    for (int i = 0; i < comprimento; i++) {
        if (nome[i] == SEPARADOR) {
            if (modo == '1') printf("\nO nome contém um caracter separador inválido (%c).\n", SEPARADOR);
            return 0;
        }
        //Retirar o \n no final, se tiver, mas apenas se a string for apenas \n não contamos como válido
        if (nome[comprimento - 1] == '\n' && comprimento != 1) { 
            nome[comprimento - 1] = '\0';
            comprimento--;
        }
        if (!isalpha(nome[i]) && nome[i] != ' ' && nome[i] != '-') { //isalpha apenas retorna válido a-z e A-Z, logo as outras condições validam espaços e hifens
            if (modo == '1') printf("\nNome contém caracteres inválidos!\n");
            return 0;
        }
    }
    //Se estamos aqui é válido, e precisamos de verificar se o nome cabe no array
    if (comprimento > TAMANHO_INICIAL_NOME - 1) {
        //Realocar o nome 
        realocar_nome(aluno, modo);
    }

    return 1;
}

int validar_nacionalidade(char * nacionalidade, const char modo) {
    if (!nacionalidade) {
        if (modo == '1') printf("\nNacionalidade em branco!\n");
        return 0;
    }
    int comprimento = strlen(nacionalidade);
    if (comprimento > MAX_STRING_NACIONALIDADE - 1) {
        if (modo == '1') printf("\nA nacionalidade é inválida. Insira uma nacionalidade com menos de %d caracteres.\n", MAX_STRING_NACIONALIDADE - 1);
        return 0;
    }
    for (int i = 0; i < comprimento; i++) {
        if (nacionalidade[i] == SEPARADOR) {
            if (modo == '1') printf("\nA nacionalidade contém um caracter separador inválido (%c).\n", SEPARADOR);
            return 0;
        }

        if (nacionalidade[comprimento - 1] == '\n' && comprimento != 1) { 
            nacionalidade[comprimento - 1] = '\0';
            comprimento--;
        }

        if (!isalpha(nacionalidade[i]) && nacionalidade[i] != ' ' && nacionalidade[i] != '-') { //isalpha apenas retorna válido a-z e A-Z, logo as outras condições validam espaços e hifens
            if (modo == '1') printf("\nA nacionalidade contém caracteres inválidos!\n");
            return 0;
        }
    }

    return 1;
}

void validacao_menus(short * valido, const char opcao, const char limInf, const char limSup) { 
    if (*valido != 1) {
        printf("Entrada inválida! Introduza um número do menu (%c a %c)\n", limInf, limSup); 
        pressione_enter();
    }
    else if (opcao < limInf || opcao > limSup) { 
        *valido = 0; //Scanf leu corretamente e retornou 1, mas como não queremos esses números, voltamos a definir a zero.
        printf("Por favor, escolha um número do menu (%c a %c).\n", limInf, limSup);
        pressione_enter();
    }
    //Limpamos o buffer porque sabemos que há, pelo menos, o enter no buffer de stdin.
    limpar_buffer();
    //Assim saímos do menu sempre com o buffer limpo. (menos uma preocupação)
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
        direita[i] = bd->aluno[meio + 1 + i];
    
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
        direita[i] = bd->escolares[meio + 1 + i];
    
    int indice_esquerda = 0;
    int indice_direita = 0;
    int indice = inicio;

    while(indice_esquerda < tamanho_esquerda && indice_direita < tamanho_direita) {
        if (esquerda[indice_esquerda].codigo <= direita[indice_direita].codigo) { 
            bd->escolares[indice] = esquerda[indice_esquerda]; //Estamos a copiar todo o conteúdo da struct, logo já fica tudo ordenado, e não apenas o código
            indice_esquerda++;
        }
        else {
            bd->escolares[indice] = direita[indice_direita];
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
    //Copia-se por partes para evitar ponteiros duplicados.
    for(int i = bd->tamanho_aluno; i > indice_aluno; i--) {
        //Libertamos a memória do elemento atual
        free(bd->aluno[i].nome);
        free(bd->aluno[i].nacionalidade);
        //Alocamos nova memória para o elemento atual, para não termos ponteiros duplicados a apontar para o mesmo sítio.
        bd->aluno[i].nome = malloc(strlen(bd->aluno[i - 1].nome) + 1);
        bd->aluno[i].nacionalidade = malloc(strlen(bd->aluno[i - 1].nacionalidade) + 1);

        if (bd->aluno[i].nome && bd->aluno[i].nacionalidade) {
            strcpy(bd->aluno[i].nome, bd->aluno[i - 1].nome);
            strcpy(bd->aluno[i].nacionalidade, bd->aluno[i - 1].nacionalidade);
        }

        //Copiar o resto da struct
        bd->aluno[i].codigo = bd->aluno[i - 1].codigo; 
        bd->aluno[i].nascimento = bd->aluno[i - 1].nascimento; 
    }
    //Precisamos de colocar os dados do novo aluno, no caso o código, os outros inicializamos para não ficar com os dados do anterior.
    bd->aluno[indice_aluno].codigo = codigo;
    bd->aluno[indice_aluno].nascimento.dia = 0;
    bd->aluno[indice_aluno].nascimento.mes = 0;
    bd->aluno[indice_aluno].nascimento.ano = 0;
    bd->aluno[indice_aluno].nacionalidade = (char *) malloc (MAX_STRING_NACIONALIDADE * sizeof(char));
    bd->aluno[indice_aluno].nome = (char *) malloc (TAMANHO_INICIAL_NOME * sizeof(char));
    if (bd->aluno[indice_aluno].nacionalidade && bd->aluno[indice_aluno].nome) {
        strcpy(bd->aluno[indice_aluno].nome ,"-1");
        strcpy(bd->aluno[indice_aluno].nacionalidade, "-1");
    }
    
    //Ordenar escolares
    for(int i = bd->tamanho_escolares; i > indice_escolares; i--) {
        bd->escolares[i] = bd->escolares[i - 1];
    }
    bd->escolares[indice_escolares].codigo = codigo;
    bd->escolares[indice_escolares].matriculas = -1;
    bd->escolares[indice_escolares].ects = -1;
    bd->escolares[indice_escolares].ano_atual = -1;
    bd->escolares[indice_escolares].prescrever = '0';
    bd->escolares[indice_escolares].media_atual = -1;
}

//Altera o tamanho total do array.
//Retorna 0 caso o código não exista ou tenha ocorrido um erro.
//Retorna 1 em caso de sucesso.
int ordenar_ao_eliminar(int codigo, Uni * bd) {
    int posicao_eliminacao_aluno;
    int posicao_eliminacao_escolares;

    posicao_eliminacao_aluno = validar_codigo_eliminar(codigo, bd);
    if(posicao_eliminacao_aluno <= 0) return 0; //O código não existe ou houve erro.
    posicao_eliminacao_aluno -=1;

    //Se vamos eliminar o estudante então é necessário dar free na memória alocada dinamicamente nessas structs.
    free(bd->aluno[posicao_eliminacao_aluno].nome);
    free(bd->aluno[posicao_eliminacao_aluno].nacionalidade);
    //i começa no índice que queremos eliminar(vai ser substituído)
    //Não se usa -1 no tamanho pois queremos inicializar a última antiga casa do array
    for(int i = posicao_eliminacao_aluno; i < bd->tamanho_aluno; i++) {
        bd->aluno[i] = bd->aluno[i + 1];
    }
    bd->tamanho_aluno -= 1;
    
    posicao_eliminacao_escolares = procurar_codigo_escolares(codigo, bd);
    if (posicao_eliminacao_escolares <= 0) return 1; //Aqui não retornamos 0 porque pode haver aluno sem dados escolares.
    posicao_eliminacao_escolares -= 1;

    for(int i = posicao_eliminacao_escolares; i < bd->tamanho_escolares; i++) {
        bd->escolares[i] = bd->escolares[i + 1];
    }
    bd->tamanho_escolares -= 1;

    return 1;
}

//Menus

//Função genérica para mostrar um menu.
//1º argumento é uma função do tipo void que apenas printa o menu. 
//Argumento 2 e 3 são os valores mínimos e máximos que o menu deve aceitar.
char mostrar_menu(void (*escrever_menu)(), char min_opcao, char max_opcao) { 
    short valido = 0;
    char opcao = '0';
    do {
        limpar_terminal();
        escrever_menu();
        printf("=>Escolha uma opção: ");

        valido = scanf(" %c", &opcao); //scanf retorna 1 se conseguir ler corretamente
        validacao_menus(&valido, opcao, min_opcao, max_opcao);

        if (valido == 1) {
            return opcao;
        }
    } while (valido == 0);
}

//As seguintes funções que começam por "menu" servem apenas para printar o menu.

//0-5
void menu_principal() {
    //https://desenvolvedorinteroperavel.wordpress.com/2011/09/11/tabela-ascii-completa/
    //Link da tabela ASCII completa de onde foram retirados as duplas barras do menu (a partir do 185 decimal)
    printf("╔══════════════════════════════════╗\n");
    printf("║          MENU PRINCIPAL          ║\n");
    printf("╠══════════════════════════════════╣\n");
    printf("║  1. Gerir estudantes             ║\n");
    printf("║  2. Consultar dados              ║\n");
    printf("║  3. Estatísticas                 ║\n");
    printf("║  4. Ficheiros                    ║\n");
    printf("║  5. Extras                       ║\n");
    printf("║  0. Sair do programa             ║\n");
    printf("╚══════════════════════════════════╝\n\n");
}

//0-3
void menu_gerir_estudantes() { 
    printf("╔════════════════════════════════════╗\n");
    printf("║          GERIR ESTUDANTES          ║\n");
    printf("╠════════════════════════════════════╣\n");
    printf("║  1. Inserir estudante              ║\n");
    printf("║  2. Eliminar estudante             ║\n");
    printf("║  3. Atualizar dados do estudante   ║\n");
    printf("║  0. Voltar ao menu anterior        ║\n");
    printf("╚════════════════════════════════════╝\n\n");
}

//0-3
void menu_consultar_dados() {
    printf("╔═════════════════════════════════════════════════════════════╗\n");
    printf("║                       CONSULTAR DADOS                       ║\n");
    printf("╠═════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Procurar estudante por nome                             ║\n");
    printf("║  2. Listar estudantes por intervalo de datas de nascimento  ║\n");
    printf("║     e pertencentes a uma nacionalidade                      ║\n");
    printf("║  3. Listar estudantes por ordem alfabética de apelido       ║\n");
    printf("║  0. Voltar ao menu anterior                                 ║\n");
    printf("╚═════════════════════════════════════════════════════════════╝\n\n");
}

//0-5
void menu_estatisticas() {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                      ESTATÍSTICAS                      ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  1. Estudantes por escalão de média atual              ║\n");
    printf("║  2. Número médio de matrículas (geral/nacionalidade)   ║\n");
    printf("║  3. Número de finalistas                               ║\n");
    printf("║  4. Média de idades por nacionalidade e ano            ║\n");
    printf("║  5. Estudantes em risco de prescrição                  ║\n");
    printf("║  0. Voltar ao menu anterior                            ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
}

//0-4
//Necessário mudar o nome dos ficheiros caso sofram alterações.
void menu_ficheiros() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                         FICHEIROS                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Guardar dados                                          ║\n");
    printf("║  2. Mostrar erros encontrados ao ler os dados              ║\n");
    printf("║  3. Mostrar dados de dados.txt                             ║\n");
    printf("║  4. Mostrar dados de situacao_Escolar_Estudantes.txt       ║\n");
    printf("║  0. Voltar ao menu anterior                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
}

//0-2
void menu_extras() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                           EXTRAS                           ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Estudantes nascidos em dias específicos da semana      ║\n");
    printf("║  2. Estudantes cujo aniversário em certo ano é ao domingo  ║\n");
    printf("║  0. Voltar ao menu anterior                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
}

//0-7; 1-Domingo, 7-Sábado
void menu_dias_da_semana() {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   LISTAR ANIVERSARIANTES POR DIA DA SEMANA    ║\n");
    printf("╠═══════════════════════════════════════════════╣\n");
    printf("║  1. Domingo                                   ║\n");
    printf("║  2. Segunda-feira                             ║\n");
    printf("║  3. Terça-feira                               ║\n");
    printf("║  4. Quarta-feira                              ║\n");
    printf("║  5. Quinta-feira                              ║\n");
    printf("║  6. Sexta-feira                               ║\n");
    printf("║  7. Sábado                                    ║\n");
    printf("║  0. Sair                                      ║\n");
    printf("╚═══════════════════════════════════════════════╝\n\n");
}

//0-2
void menu_formatos_disponiveis() {
    printf("╔══════════════════════════╗\n");
    printf("║   FORMATOS DISPONÍVEIS   ║\n");
    printf("╠══════════════════════════╣\n");
    printf("║  1. .txt                 ║\n");
    printf("║  2. .csv                 ║\n");
    printf("║  0. Sair                 ║\n");
    printf("╚══════════════════════════╝\n\n");
}

void menu_media_matriculas() {
    printf("╔══════════════════════════╗\n");
    printf("║     MÉDIA MATRÍCULAS     ║\n");
    printf("╠══════════════════════════╣\n");
    printf("║  1. Geral                ║\n");
    printf("║  2. Por nacionalidade    ║\n");
    printf("║  0. Sair                 ║\n");
    printf("╚══════════════════════════╝\n\n");
}

//Apresenta o menu e faz todas as operações.
void processar_gerir_estudantes(Uni * bd) {
    char opcao;
    do {
        opcao = mostrar_menu(menu_gerir_estudantes, '0', '3');
        switch(opcao) {
            case '0': break;
            case '1':
                inserir_estudante(bd);
                break;
            case '2':
                eliminar_estudante(bd);
                break;
            case '3':
                //atualizar estudante (opcional)
                //Como fazer: pedir código, ir buscar o índice, e fazer uma tabela com os campos que quer atualizar.
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Apresenta o menu e faz todas as operações.
void processar_consultar_dados(Uni * bd) {
    char opcao;
    do {
        opcao = mostrar_menu(menu_consultar_dados, '0', '3');
        switch(opcao) {
            case '0': break;
            case '1':
                procurar_estudante_por_nome(bd);
                break;
            case '2':
                //Listar estudantes por intervalo de datas de nascimento  e pertencentes a uma nacionalidade
                break;
            case '3':
            listar_estudantes_alfabeticamente(bd);
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Apresenta o menu e faz todas as operações.
//Calcula as estatísticas gerais se opcao != '0'.
void processar_estatisticas(Uni * bd) {
    char opcao;
    do {
        opcao = mostrar_menu(menu_estatisticas, '0', '5');
        if (opcao == '0') break;
        /*A lógica diferente por detrás deste menu envolve o facto de que:
        -Se o menu é de estatísticas, muito provavelmente o utilizador vai consultar mais do que uma estatística sobre o aluno;
        -De qualquer das formas, só se calcula se opcao != '0';
        -Isto evita muitas operações O(n) já que teríamos que fazer uma diferente em cada opção do menu, e assim apeans fazemos uma toda junta.
        -No pior dos casos, o utilizador apenas utiliza uma funcionalidade, faz alterações e volta, repetidamente, mas dadas as circunstâncias do programa, improvável.
        -A flag atualizado garante que só se calculam as estatísticas se os dados sofrerem alterações.
        -Assim, as estatísticas só se calculam se for necessário e uma vez, sendo mais eficiente;
        -As operações nas funções de listar já são mais reduzidas(se não for o pior caso, não chegam a O(n)).
        -Para além disso, facilita a manutenção do código e a expansão possível futura do mesmo, como por exemplo, pela implementação de 
        checksums para verificar se os dados continuam iguais ou não, e dessa forma não precisaríamos de recalcular as estatísticas cada vez que 
        carregarmos ficheiros .txt novos.
        */
    	if(bd->stats.atualizado == '0') {
            calcular_estatisticas(bd);
        }
        switch(opcao) {
            case '1':
                //Esta opção é a tabela dos extras.
                //fazer uma função que @param sejam media min, media max e ano atual
                break;
            case '2':
                calcular_media_matriculas(bd);
                break;
            case '3':
                finalistas(bd);
                break;
            case '4':
                //Média de idades por nacionalidade e ano 
                break;
            case '5':
                prescrito(bd);
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Apresenta o menu e faz todas as operações.
void processar_ficheiros(Uni * bd) {
    char opcao;
    do {
        opcao = mostrar_menu(menu_ficheiros, '0', '4');
        switch(opcao) {
            case '0': break;
            case '1':
                guardar_dados(DADOS_TXT, SITUACAO_ESCOLAR_TXT, bd);
                break;
            case '2':
                //opcional
                break;
            case '3':
                //opcional
                break;
            case '4':
                //opcional
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Apresenta o menu e faz todas as operações.
void processar_extras(Uni * bd) {
    char opcao;
    do {
        opcao = mostrar_menu(menu_extras, '0', '2');
        switch(opcao) {
            case '0': break;
            case '1':
                listar_aniversarios_por_dia(bd);
                break;
            case '2':
                listar_aniversario_ao_domingo(bd);
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Apresenta os menus e depois chama as funções de processar para 
void the_architect(Uni * bd) {
    char opcao;

    do {
        opcao = mostrar_menu(menu_principal, '0', '5');
        switch(opcao) {
            case '0': break;
            case '1':
                processar_gerir_estudantes(bd);
                break;
            case '2':
                processar_consultar_dados(bd);
                break;
            case '3':
                processar_estatisticas(bd);
                break;
            case '4':
                processar_ficheiros(bd);
                break;
            case '5': 
                processar_extras(bd);
                break;
            default:
                opcao = '0'; //Sair caso haja erro.
                break;
        }
    } while(opcao != '0');
}

//Inserção/leitura de dados

//modo '1' para imprimir mensagens de erro.
//str == NULL para ler de stdin
//Insere e valida a data.
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
            //Remover o \n
            size_t comprimento = strlen(data);
            if (comprimento > 0 && data[comprimento - 1] == '\n') {
                data[comprimento - 1] = '\0';
            } 
            else limpar_buffer(); //Se o \n não está na string, está em stdin
        }
        else {
            strcpy(data, str);
        }
        
        //O sscanf lê as x entradas conforme o padrão apresentado
    	if (sscanf(data, "%hd-%hd-%hd", &dia_temp, &mes_temp, &ano_temp) != 3) { //sscanf tenta ler 3 shorts para as var temp
            //Aqui não se usa && erro == '0'porque só pode ser 0 se !str, e nesse caso há um continue
            if (modo == '1') printf("Formato inválido! Use o formato DD-MM-AAAA.\n");
            erro = '1';
        }
        //Sse data for válida é que passamos os dados à stuct
        if (erro == '0' && validar_data(dia_temp, mes_temp, ano_temp, modo)) {
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
}

void inserir_estudante(Uni * bd) {
    int posicao_insercao_aluno;
    int posicao_insercao_escolares;
    //A metodologia vai ser colocar tudo no final do array e depois ordená-lo
    do {
        //Realocação dentro do do{}while para evitar que sejam inseridos alunos até não haver espaço
        if (bd->capacidade_aluno <= bd->tamanho_aluno + 1) { //Trata os casos em que não há espaço livre
            if (!realocar_aluno(bd, '1')) return;
            inicializar_aluno(bd, bd->tamanho_aluno);
        }
        if (bd->capacidade_escolares <= bd->tamanho_escolares + 1) {
            if (!realocar_escolares(bd, '1')) return;
            inicializar_escolares(bd, bd->tamanho_escolares);
        }
        limpar_terminal(); //Caso de repetição
        do {
            int codigo_temp = -1;
            limpar_terminal();
            printf("Insira o código do estudante: ");
            if (scanf("%d", &codigo_temp) != 1) { //Verifica entradas inválidas como letras
                printf("Código inválido! Insira um número inteiro positivo.\n");
                limpar_buffer();
                pressione_enter();
                continue; //Continue faz com que salte o resto do loop e passe à próxima iteração
            }
            limpar_buffer();
            
            posicao_insercao_aluno = validar_codigo_ao_inserir(codigo_temp, bd);
            if(posicao_insercao_aluno < 0) { //Não se verifica escolares pois nunca há códigos em escolares que não estejam em aluno
                posicao_insercao_aluno = -(posicao_insercao_aluno + 1);
                posicao_insercao_escolares = procurar_codigo_escolares(codigo_temp, bd);
                if (posicao_insercao_escolares == 0) { //Necessário porque procurar não printa erros.
                    printf("Ocorreu um erro a procurar o índice dos dados escolares.\n");
                    printf("Por favor tente novamente.\n");
                    pressione_enter();
                    continue;
                }
                //Não se verifica se é < 0 porque sabemos que se não existe em aluno não existe em escolares
                posicao_insercao_escolares = -(posicao_insercao_escolares + 1);
                ordenar_ao_inserir(codigo_temp, bd, posicao_insercao_aluno, posicao_insercao_escolares);
                //Trocam-se as posições do array. O código é inserido. O resto inicializado.
                break; 
            }
        } while (1);
        
        do {
            char * nome_temp = NULL;
            printf("Insira o nome do estudante: ");
            nome_temp = ler_linha_txt(stdin, NULL);
            if(!validar_nome(bd->aluno, nome_temp, '1')) {
                pressione_enter();
                free(nome_temp);
                continue;
            }
            strcpy(bd->aluno[posicao_insercao_aluno].nome, nome_temp);
            free(nome_temp);
            break;
        } while(1);

        do {
            ler_data(&(bd->aluno[posicao_insercao_aluno]), NULL, '1');
        } while (bd->aluno[posicao_insercao_aluno].nascimento.dia == 0); //Apenas verificamos um pois em ler_data a data só é copiada para as structs se toda ela for válida
        
        do {
            char * nacionalidade_temp = NULL;
            printf("Insira a nacionalidade do estudante: ");
            nacionalidade_temp = ler_linha_txt(stdin, NULL);

            if(!validar_nacionalidade(nacionalidade_temp, '1')) {
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
            limpar_buffer();
            if (matriculas_temp < 0 || matriculas_temp > MAX_MATRICULAS) {
                printf("Número de matrículas é inválido. Deve estar entre 0 e %d.\n", MAX_MATRICULAS);
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
            limpar_buffer();
            if (ects_temp < 0 || ects_temp > MAX_ECTS) {
                printf("O número de créditos ECTS é inválido. Deve estar entre 0 e %d.\n", MAX_ECTS);
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
            limpar_buffer();
            if (ano_atual_temp < 1 || ano_atual_temp > MAX_ANO_ATUAL) {
                printf("O ano atual é inválido. Deve estar entre 0 e %d.\n", MAX_ANO_ATUAL);
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
            limpar_buffer();
            if (media_temp < 0 || media_temp > 20) {
                printf("A média atual é inválida. Deve estar entre 0 e 20.\n");
                pressione_enter();
                continue;
            }
            bd->escolares[posicao_insercao_escolares].media_atual = media_temp;
            break;
        } while(1);

        //Incrementar o tamanho dos arrays
        bd->tamanho_aluno += 1;
        bd->tamanho_escolares += 1;
        printf("\nO código %d foi introduzido com sucesso!\n", bd->aluno[posicao_insercao_aluno].codigo);

        printf("\nQuer inserir mais estudantes? (S/N): ");
        if (!sim_nao()) return;
    }while(1); //Não precisamos de verificar repetir pois só chega aqui se repetir == 's'
    //Introduzimos um estudante, logo os dados estatísticos têm de ser recalculados.
    bd->stats.atualizado = '0';
}

void eliminar_estudante(Uni * bd) {
    //**Implementar lógica de eliminar em intervalos de códigos**
    do {
        limpar_terminal(); //Caso de repetição
        do {
            int codigo_temp = -1;
            limpar_terminal();
            printf("Insira o código do estudante: ");
            if (scanf("%d", &codigo_temp) != 1) { //Verifica entradas inválidas como letras
                printf("Código inválido! Insira um número inteiro positivo.\n");
                limpar_buffer();
                pressione_enter();
                continue; //Continue faz com que salte o resto do loop e passe à próxima iteração
            }
            limpar_buffer();
            
            //Elimina e ordena o código dado, caso seja válido.
            if(ordenar_ao_eliminar(codigo_temp, bd)) {
                printf("O aluno com o código %d foi eliminado com sucesso!\n", codigo_temp);
                break;
            }
        } while (1);

        printf("\nQuer eliminar mais estudantes? (S/N): ");
        if(!sim_nao()) return;
    } while(1);
    bd->stats.atualizado = '0';
}

//Estatísticas

//Aqui serão calculadas todas as estatísticas gerais(que abrangem todos os estudantes).
void calcular_estatisticas(Uni * bd) {
    //Inicializar caso já tenhamos chamado a função antes.
    bd->stats.media_matriculas = 0.0;
    bd->stats.finalistas = 0;
    bd->stats.media = 0.0;
    bd->stats.risco_prescrever = 0;


    for(int i = 0; i < bd->tamanho_escolares; i++) {
        //Matrículas
        bd->stats.media_matriculas += bd->escolares[i].matriculas;
        //Média
        bd->stats.media += bd->escolares[i].media_atual;
        //Finalistas
        if (bd->escolares[i].ects >= CREDITOS_FINALISTA) {
            bd->escolares[i].finalista = '1'; //Marcar o aluno como finalista.
            bd->stats.finalistas++;
        }
        else bd->escolares[i].finalista = '0';
        //Risco de prescrição
        if ((bd->escolares[i].matriculas == 3 && bd->escolares[i].ects < ECTS_3MATRICULAS) ||
            (bd->escolares[i].matriculas == 4 && bd->escolares[i].ects < ECTS_4MATRICULAS) ||
            (bd->escolares[i].matriculas >= 5 && bd->escolares[i].finalista == '0')) {
                bd->escolares[i].prescrever = '1';
                bd->stats.risco_prescrever++;
        }
        else bd->escolares[i].prescrever = '0';
    }
    //FALTA CALCULAR OS ALUNOS EM RISCO DE PRESCREVER.
    //Tratar da divisão por zero.
    if (bd->tamanho_escolares != 0) {
        bd->stats.media_matriculas /= bd->tamanho_escolares;
        bd->stats.media /= bd->tamanho_escolares;
    }
    else {
        bd->stats.media_matriculas = 0.0;
        bd->stats.media = 0.0;
    }
    //Estatísticas estão atualizadas.
    bd->stats.atualizado = '1';
}

//Calcula o número médio de matrículas a nível geral ou por uma dada nacionalidade.
void calcular_media_matriculas(Uni * bd) {
    char opcao;
    do {
        limpar_terminal();
        opcao = mostrar_menu(menu_media_matriculas, '0', '2');
        switch(opcao) {
            case '0': return;
            case '1':
                //Tratar do caso da divisão por zero.
                if (bd->tamanho_escolares == 0) {
                    printf("Não há alunos!\n"); 
                    break;
                }
                //Media já foi calculada.
                printf("O número médio de matrículas geral é de %.1f.\n", bd->stats.media_matriculas);
                pressione_enter();
                break;
            case '2':
                float media_matriculas_nacionalidade = 0.0; 
                char * nacionalidade = NULL; //Array para guardar a nacionalidade que o utilizador quiser.
                int indice = 0; //Indice em escolares.
                short contador = 0;
                //Pedir uma nacionalidade correta.
                do {
                    printf("Insira a nacinalidade da qual quer saber a média de matrículas: ");
                    nacionalidade = ler_linha_txt(stdin, NULL);
                    //Se a nacionalidade não for válida, então também não será encontrada no array, e poupamos uma operação O(n).
                    if (!validar_nacionalidade(nacionalidade, '1')) {
                        free(nacionalidade);
                        continue;
                    }
                    break;
                } while(1);
                for(int i = 0; i < bd->tamanho_aluno; i++) {
                    if (strcmp(nacionalidade, bd->aluno[i].nacionalidade) == 0) {
                        //O índice de aluno pode ser diferente de escolares.
                        indice = procurar_codigo_escolares(bd->aluno[i].codigo, bd);
                        if (indice > 0) {
                            indice -= 1;
                            media_matriculas_nacionalidade += bd->escolares[indice].matriculas;
                            contador++;
                        }
                    }
                    else {
                        //Se parte for semelhante
                        if (strncmp(nacionalidade, bd->aluno[i].nacionalidade, 3) == 0) {
                            printf("Quer dizer \"%s\"? (S/N) ", bd->aluno[i].nacionalidade);
                            if(sim_nao()) {
                                //Atualizar/corrigir a nacionalidade.
                                free(nacionalidade);
                                nacionalidade = strdup(bd->aluno[i].nacionalidade);
                                //Mantém o i igual para já ir buscar a média deste elemento.
                                i--;
                            }
                        }
                    }
                }
                //Evitar divisão por 0.
                if (contador == 0) {
                    printf("Não foi encontrado nenhum aluno de nacionalidade %s ou o mesmo não possui número de matrículas!\n", nacionalidade);
                    free(nacionalidade);
                    pressione_enter();
                    break;
                }
                media_matriculas_nacionalidade /= contador;
                printf("O número médio de matrículas dos estudantes de nacionalidade %s é de %.1f.\n", nacionalidade, media_matriculas_nacionalidade);
                pressione_enter();
                free(nacionalidade);
                break;
            default: return;
        }
    } while(1); //Saímos em cima caso seja 0.
}

//Listagens (inclui procuras)

//Lista os parametros no terminal e no ficheiro se existir.
void listar(Estudante aluno, FILE * ficheiro, char separador, short * contador) {
    if (*contador % 20 == 0 && *contador != 0) pressione_enter();

    printf("Código: %d\n", aluno.codigo);
    printf("Nome: %s\n", aluno.nome);
    printf("Data de Nascimento: %02d-%02d-%04d\n", aluno.nascimento.dia, aluno.nascimento.mes, aluno.nascimento.ano);
    printf("Nacionalidade: %s\n", aluno.nacionalidade);
    printf("\n");
    if (ficheiro) {
        //Colocar \n apenas se não for nem a primeira nem a última entrada.
        if (*contador > 0) fprintf(ficheiro, "\n");

        fprintf(ficheiro, "%d%c", aluno.codigo, separador);
        fprintf(ficheiro, "%s%c", aluno.nome, separador);
        fprintf(ficheiro, "%02d-%02d-%04d%c", aluno.nascimento.dia, aluno.nascimento.mes, aluno.nascimento.ano, separador);
        fprintf(ficheiro, "%s", aluno.nacionalidade);
    }
    (*contador)++;
}

//Procura um estudante dado uma parte do nome com 2+ caracteres.
//Não suporta formatos com mais de 9 caracteres.
//Lista no terminal e em ficheiro(se requerido).
void procurar_estudante_por_nome(Uni * bd) {
    short contador = 0;
    char * parte_nome;
    char formato[MAX_FORMATO]; //Espaço suficiente para .txt ou .csv (10)
    char separador;
    FILE * listagem;

    do {
        contador = 0;
        listagem = NULL;
        limpar_terminal();
        //Verificar se queremos fazer uma listagem
        listagem = pedir_listagem(formato);
        separador = obter_separador(listagem, formato);
        
        do {
            parte_nome = NULL;
            printf("Introduza parte do nome do estudante a procurar: ");
            parte_nome = ler_linha_txt(stdin, NULL);
            if(strlen(parte_nome) < 2 || !parte_nome) {
                printf("A entrada é inválida. Por favor, insira parte do nome do aluno, com um mínimo de 2 caracteres.");
                pressione_enter();
                free(parte_nome);
                continue;
            }
            break;
        } while(1);
        printf("Resultados da pesquisa para \"%s\": \n\n", parte_nome);
        for(int i = 0; i < bd->tamanho_aluno; i++) {
            //strstr(s1, s2) é uma função que retorna a primeira ocorrência de s2 em s1
            //https://www.geeksforgeeks.org/strstr-in-ccpp/
            if (strstr(bd->aluno[i].nome, parte_nome) != NULL) {
                listar(bd->aluno[i], listagem, separador, &contador);
            }
        }
        if (contador == 0) {
            printf("Não foi encontrado nenhum estudante com parte do nome \"%s\".\n", parte_nome);
        }
        pressione_enter();
        printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");

        free(parte_nome);
        if (listagem) fclose(listagem);

        printf("\nQuer repetir a procura? (S/N): ");
        if (!sim_nao()) return;
    } while(1);
}

void listar_estudantes_alfabeticamente(Uni * bd) {
    short contador = 0;
    char formato[MAX_FORMATO];
    char separador;
    FILE * listagem = NULL;

    //Criar uma cópia temporária de aluno.
    Estudante * copia = (Estudante *) malloc(bd->tamanho_aluno * sizeof(Estudante));
    if (!copia) {
        printf("Ocorreu um erro a listar os estudantes por ordem alfabética. Por favor tente novamente mais tarde.\n");
        return;
    }

    //Copiamos à mão e não com memcpy para evitar ponteiros duplicados.
    for(int i = 0; i < bd->tamanho_aluno; i++) {
        copia[i].codigo = bd->aluno[i].codigo;
        copia[i].nome = strdup(bd->aluno[i].nome);
        copia[i].nacionalidade = strdup(bd->aluno[i].nacionalidade); 
        copia[i].nascimento = bd->aluno[i].nascimento;
    }

    //Ordenar alfabeticamente por bubble sort O(n2) mas n é pequeno e além disso, não se usa muitas vezes esta listagem.
    for(int i = 0; i < bd->tamanho_aluno - 1; i++) {
        for(int j = 0; j < bd->tamanho_aluno - i - 1; j++) {
            //strcmp retorna > 0 se o primeiro caracter diferente da str1 for de maior valor na tabela ascii que de str2.
            if(strcmp(copia[j].nome, copia[j+1].nome) > 0) {
                Estudante temp = copia[j];
                copia[j] = copia[j+1];
                copia[j+1] = temp;
            }
        }
    }

    //Preparar listagem.
    listagem = pedir_listagem(formato);
    separador = obter_separador(listagem, formato);

    printf("Estudantes por ordem alfabética:\n\n");
    for(int i = 0; i < bd->tamanho_aluno; i++) {
        listar(copia[i], listagem, separador, &contador);
    }

    //Libertar memória de copia.
    for(int i = 0; i < bd->tamanho_aluno; i++) {
        free(copia[i].nome);
        free(copia[i].nacionalidade);
    }
    free(copia);

    printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
    pressione_enter();
    if (listagem) fclose(listagem);
}

void listar_aniversarios_por_dia(Uni * bd) {
    const char * dias_da_semana[] = {"Sábado", "Domingo", "Segunda-feira", "Terça-feira", "Quarta-feira", "Quinta-feira", "Sexta-feira"};
    short dia, mes, ano, opcao, dia_da_semana, contador;
    char formato[MAX_FORMATO]; 
    char separador;
    FILE * listagem;
    do {
        contador = 0;
        listagem = NULL;
        limpar_terminal();

        opcao = (short) mostrar_menu(menu_dias_da_semana, '0', '7') - '0'; // - '0' converte para int
        if (opcao == 0) break;
        //Preparar listagem
        listagem = pedir_listagem(formato);
        separador = obter_separador(listagem, formato);

        //Lembrar que se for 7, na verdade é o 0 na função calcular_dia_da_semana.
        if (opcao == 7) opcao = 0; //Como já saímos do menu, ajustamos para a mesma ordem que a função calcular_dia_da_semana retornará.
        
        if (opcao == 0 || opcao == 1) printf("Estudantes nascidos no %s:\n\n", dias_da_semana[opcao]); //Ajuste gramatical
        else printf("Estudantes nascidos na %s:\n\n", dias_da_semana[opcao]);

        for(int i = 0; i < bd->tamanho_aluno; i++) {
            dia = bd->aluno[i].nascimento.dia;
            mes = bd->aluno[i].nascimento.mes;
            ano = bd->aluno[i].nascimento.ano;
            dia_da_semana = calcular_dia_da_semana(dia, mes, ano);
            if (dia_da_semana == opcao) {
                listar(bd->aluno[i], listagem, separador, &contador);
            }
        }
        if (contador == 0) {
            if (opcao == 0 || opcao == 1) printf("Não foi encontrado nenhum estudante nascido no %s.\n", dias_da_semana[opcao]); 
            else printf("Não foi encontrado nenhum estudante nascido na %s.\n", dias_da_semana[opcao]);
        }
        printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
        pressione_enter();
        if (listagem) fclose(listagem);
    } while (1); //while(1) porque devido às circusntâncias já saímos em cima.
}

void listar_aniversario_ao_domingo(Uni * bd) {
    short dia, mes, ano;
    short contador = 0;
    char formato[MAX_FORMATO]; 
    char separador;
    FILE * listagem;
    do {
        contador = 0;
        listagem = NULL;
        limpar_terminal();

        listagem = pedir_listagem(formato);
        separador = obter_separador(listagem, formato);

        do {
            printf("Insira o ano: ");
            scanf("%hd", &ano);
        } while (ano > ANO_ATUAL || ano < ANO_NASC_LIM_INF);

        printf("Estudantes nascidos no Domingo de %hd:\n\n", ano);
        for(int i = 0; i < bd->tamanho_aluno; i++) {
            dia = bd->aluno[i].nascimento.dia;
            mes = bd->aluno[i].nascimento.mes;
            if (calcular_dia_da_semana(dia, mes, ano) == 1) { 
                listar(bd->aluno[i], listagem, separador, &contador);
            }
        }
        if (contador == 0) {
            printf("Não foi encontrado nenhum estudante nascido ao Domingo em %hd.\n", ano);
        }
        printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
        pressione_enter();
        if (listagem) fclose(listagem);

        printf("\nQuer inserir um ano diferente? (S/N): ");
        if(!sim_nao()) return;
    } while(1);
}

void prescrito(Uni * bd) {
    int indice = 0;
    short contador = 0;
    char formato[MAX_FORMATO]; 
    FILE * listagem = NULL;
    char separador;

    limpar_terminal();
    listagem = pedir_listagem(formato);
    separador = obter_separador(listagem, formato);

    printf("Alunos em risco de prescrição: \n\n");
    //Ao fazer este loop evitamos ir até ao final do array caso no fim não haja prescrevidos.
    for (int i = 0; contador < bd->stats.risco_prescrever; i++) {
        if (bd->escolares[i].prescrever == '1') {
            indice = procurar_codigo_aluno(bd->escolares[i].codigo, bd);
            if (indice > 0) {
                indice--;
                listar(bd->aluno[indice], listagem, separador, &contador);
            }
            contador++; //Apenas incrementamos se for válido
        }
    }
    if (contador == 0) {
            printf("Não foi encontrado nenhum estudante em risco de prescrição.\n");
    }
    printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
    pressione_enter();
    if (listagem) fclose(listagem);
}

void finalistas(Uni * bd) {
    int indice = 0;
    short contador = 0;
    char formato[MAX_FORMATO]; 
    FILE * listagem = NULL;
    char separador;

    limpar_terminal();
    listagem = pedir_listagem(formato);
    separador = obter_separador(listagem, formato);

    //Ao fazer este loop evitamos ir até ao final do array caso no fim não haja prescrevidos.
    printf("Alunos finalistas: \n\n");
    for (int i = 0; contador < bd->stats.finalistas; i++) {
        if (bd->escolares[i].finalista == '1') {
            indice = procurar_codigo_aluno(bd->escolares[i].codigo, bd);
            if (indice > 0) {
                indice--;
                listar(bd->aluno[indice], listagem, separador, &contador);
            }
            contador++; //Apenas incrementamos se for válido
        }
    }
    if (contador == 0) {
        printf("Não foram encontrados alunos finalistas.\n");
    }
    printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
    pressione_enter();
    if (listagem) fclose(listagem);
}

//Provisório
void listar_estudantes_por_intervalo_e_nacionalidades(Uni *bd) {
    Data data_inf, data_sup;
    char nacionalidades[MAX_STRING_NACIONALIDADE][5];
    int num_nacionalidades = 0;
    char formato[MAX_FORMATO];
    char separador;
    FILE *listagem = NULL;

    // Entrada de Dados
    printf("Insira a data inferior (dd mm aaaa): ");
    scanf("%d %d %d", &data_inf.dia, &data_inf.mes, &data_inf.ano);
    printf("Insira a data superior (dd mm aaaa): ");
    scanf("%d %d %d", &data_sup.dia, &data_sup.mes, &data_sup.ano);

    printf("Insira até 5 nacionalidades (digite 'fim' para terminar):\n");
    for (int i = 0; i < MAX_NACIONALIDADES; i++) {
        printf("Nacionalidade %d: ", i + 1);
        scanf("%s", nacionalidades[i]);
        if (strcmp(nacionalidades[i], "fim") == 0) {
            break;
        }
        num_nacionalidades++;
    }

    // Preparar listagem
    listagem = pedir_listagem(formato);
    separador = obter_separador(listagem, formato);

    // Cabeçalho do ficheiro de listagem
    if (listagem) {
        fprintf(listagem, "Código%cNome%cData de Nascimento%cNacionalidade\n", separador, separador, separador);
    }

    // Filtragem de Estudantes
    for (int i = 0; i < bd->tamanho_aluno; i++) {
        Estudante est = bd->aluno[i];
        Data nasc = est.nascimento;

        // Verificar se a data de nascimento está dentro do intervalo
        if ((nasc.ano > data_inf.ano || (nasc.ano == data_inf.ano && nasc.mes > data_inf.mes) || (nasc.ano == data_inf.ano && nasc.mes == data_inf.mes && nasc.dia >= data_inf.dia)) &&
            (nasc.ano < data_sup.ano || (nasc.ano == data_sup.ano && nasc.mes < data_sup.mes) || (nasc.ano == data_sup.ano && nasc.mes == data_sup.mes && nasc.dia <= data_sup.dia))) {
            
            // Verificar se a nacionalidade está entre as fornecidas
            for (int j = 0; j < num_nacionalidades; j++) {
                if (strcmp(est.nacionalidade, nacionalidades[j]) == 0) {
                    // Listar no terminal
                    printf("Código: %d\n", est.codigo);
                    printf("Nome: %s\n", est.nome);
                    printf("Data de Nascimento: %02d-%02d-%04d\n", nasc.dia, nasc.mes, nasc.ano);
                    printf("Nacionalidade: %s\n\n", est.nacionalidade);

                    // Listar no ficheiro
                    if (listagem) {
                        fprintf(listagem, "%d%c%s%c%02d-%02d-%04d%c%s\n",
                                est.codigo, separador,
                                est.nome, separador,
                                nasc.dia, nasc.mes, nasc.ano, separador,
                                est.nacionalidade);
                    }
                    break;
                }
            }
        }
    }

    // Fechar o ficheiro de listagem
    if (listagem) {
        fclose(listagem);
    }
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
    char * inicio = strdup(linha); //Ponteiro para o inicio da linha, que não deve ser alterado
    //!!NOTA IMPORTANTE - É feita uma cópia da linha pois caso contrário, caso a função tenha um erro em qualquer parte, a linha original será perdida!!
    char * fim = NULL;
    //Não colocamos *num_parametros = 0 pois esta função poderá ser chamada várias vezes numa linha, e não queremos alterar a var nesse caso
    int indice = 0; //Indice do array

    while(*inicio != '\0') { //Se não for o fim da linha entramos no loop
        fim = inicio; 

        //Vamos veriricar se o ponteiro atual de fim é um separador ou o fim da linha, caso não seja avançamos
        while(*fim != SEPARADOR && *fim != '\0' && *fim != '\n') fim++;
        //Aqui fim está a apontar para o separador, o fim da linha ou um \n (se bem que neste caso \n é o fim da linha
        char temp = *fim; //Armazena o tab ou o nul char
        *fim = '\0'; //vai terminar a string de inicio (ou seja, um parametro); também corta o \n aqui, se exitir
        remover_espacos(inicio);

        if (*inicio != '\0') { //Se o inicio não for o fim da linha, então temos um parametro
            //Alocamos memória para o parametro e copia o conteúdo
            parametros[indice] = malloc(strlen(inicio) + 1); //Lembrar que parametros recebe um ponteiro.
            if (parametros[indice] != NULL) {
                strcpy(parametros[indice], inicio);
                indice++;
                (*num_parametros)++;
            }
            else {
                //ERRO!!
                *num_parametros = 0; //Usamos num_parametros como uma flag para evitar que a linha seja lida.
                //Talvez fosse bom encontrar uma forma diferente de fazer isso sem levar o user a erro.
                //Libertar a memória alocada até ao indice atual
                for (int i = 0; i < indice; i++) 
                    free(parametros[i]); 
                break;
            }
        }
        *fim = temp; //Volta a colocar o tab ou '\0' onde estava 
        inicio = (*fim == '\0') ? fim : fim + 1; //Verifica se já estamos no fim da string, se sim, inicio = fim, se não inicio = fim +1 (avança uma casa)
    }
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
	    setlocale(LC_NUMERIC, "Portuguese"); //Apenas afeta os números , ou seja, muda a notação de floats de . para ,
    #else
        setlocale(LC_ALL, "pt_PT.UTF-8");
    #endif
}

void verificar_primeiro_erro(FILE * erros, char * primeiro_erro, const char * nome_ficheiro) {
    //Não é necessário verificar se algum ponteiro é NULL porque já os utilizamos antes, logo sabemos que não são NULL
    if (*primeiro_erro == '1') {
        *primeiro_erro = '0';
        fprintf(erros, "\n\n"); //Caso seja o primeiro erro queremos separar o ficheiro de erros com 3\n
        fprintf(erros, "\t\tERROS AO LER O FICHEIRO %s\n\n", nome_ficheiro);
    }
}

//Converte para int com validações de conversão. 
int string_para_int(const char * str, int * resultado) {
    char * ptr_fim;
    long valor = strtol(str, &ptr_fim, 10);
    
    //Verificar erros de conversão
    if (ptr_fim == str || *ptr_fim != '\0') {
        return 0;  //Se o ponteiro de fim da str for igual ao ptr do inicio, não há um int; Se for diferente de nul char então também houve erro porque há caracteres indesejados
    }
    
    //Como strtol converte para longs e nós só queremos ints, pode ler números mais altos que o int conseguiria guardar
    if (valor > MAX_INT || valor < MIN_INT) {
        return 0; 
    }
    
    *resultado = (int) valor;
    return 1;
}

int string_para_short(const char * str, short * resultado) {
    char * ptr_fim;
    long valor = strtol(str, &ptr_fim, 10);
    
    //Verificar erros de conversão
    if (ptr_fim == str || *ptr_fim != '\0') {
        return 0;  //Se o ponteiro de fim da str for igual ao ptr do inicio, não há um int; Se for diferente de nul char então também houve erro porque há caracteres indesejados
    }
    
    //Como strtol converte para longs e nós só queremos ints, pode ler números mais altos que o int conseguiria guardar
    if (valor > MAX_SHORT || valor < MIN_SHORT) {
        return 0; 
    }
    
    *resultado = (short) valor;
    return 1;
}

int string_para_float(const char * str, float * resultado) {
    char * ptr_fim;
    float valor = strtof(str, &ptr_fim);
    
    //Verificar erros de conversão
    if (ptr_fim == str || *ptr_fim != '\0') {
        return 0;  //Se o ponteiro de fim da str for igual ao ptr do inicio, não há um int; Se for diferente de nul char então também houve erro porque há caracteres indesejados
    }
    //Já não há necessidade de verificar limites pois converte diretamente para float
    *resultado = (float) valor;
    return 1;
}

//Calcula o dia da semana de acordo com a fórmula da Congruência de Zeller.
//https://www.geeksforgeeks.org/zellers-congruence-find-day-date/
//Retorna 0 - Sábado, 1 - Domingo, ..., 6 - Sexta-feira.
short calcular_dia_da_semana(short dia, int mes, int ano) {
    if (mes < 3) {
        mes += 12;
        ano -= 1;
    }
    short k = ano % 100;
    short j = ano / 100;
    short dia_da_semana = (dia + 13 * (mes + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    return dia_da_semana;
}

//Sim - 1; Não - 0;
int sim_nao() {
    char opcao;
    do {
        scanf(" %c", &opcao);
        limpar_buffer();
        if (opcao == 's' || opcao == 'S') {
            return 1;
        }
        else if (opcao == 'n' || opcao == 'N') {
            return 0;
        }
        printf("\nOpção inválida. Use S ou N.\n");
        pressione_enter();
    } while (1);
}

//Retorna o separador entre os parametros a escrever no ficheiro.
//Caso o ficheiro == NULL, retorna '0';
//Requer ajuste caso sejam adicionados mais tipo de ficheiros.
//Escreve o cabeçalho do ficheiro.
char obter_separador(FILE * ficheiro, char * formato) {
    char separador; //Não é necesário inicializar porque se !ficheiro, então nunca iremos escrever no mesmo.
    if(ficheiro) {
        if (strcmp(formato, ".txt") == 0) separador = '\t';
        else if (strcmp(formato, ".csv") == 0) separador = ',';
        fprintf(ficheiro, "Código%cNome%cData de Nascimento%cNacionalidade\n", separador, separador, separador); //Cabeçalho do ficheiro de listagem.
    }
    return separador;
}

//Provisório
short calcular_idade(Data nascimento) {
    short idade = 0;
    //Precisamos de saber a data atual.
    return idade;
}
