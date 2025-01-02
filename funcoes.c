#include "headers.h"

//Variáveis globais

Data DATA_ATUAL = {0,0,0}; //Vai ser alterada logo no ínicio do programa

//Ficheiros e gestão de dados

//Linha é alocada dinamicamente, pelo que deve ser libertada quando já não for necessária
//Lê uma linha completa do ficheiro/teclado(ficheiro = stdin) sem que haja a possibilidade de ficar algo por ler
//n_linhas == NULL caso não seja necessário
//Não retorna a string com o \n no fim
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
            //Evitar ao máximo retornar NULL, pois isso terminaria o loop em carregar_dados
            if (linha) {
                linha[tamanho_total] = '\0';
                if (n_linhas != NULL) (*n_linhas)++;
                return linha;
            }
            return NULL; //Caso haja um erro fatal, temos que interromper
        }
        linha = temp; //atualizar o ponteiro linha para apontar para a nova memória

        //Copiar o conteúdo lido para a linha total
        memmove(linha + tamanho_total, buffer, tamanho); //linha + tamanho_total é um ponteiro para a posição da memória seguinte para onde a próxima parte de buffer será copiada
        tamanho_total += tamanho;
        //Se houver mais que uma leitura, o primeiro char da segunda leitura de fgets irá substituir o nul char, pelo que fica sempre no fim
        linha[tamanho_total] = '\0'; 

        //Verificamos se a linha está completa
        if (linha[tamanho_total - 1] == '\n') { //se tudo tiver sido copiado, o ultimo caracter do buffer(e da linha também) será o '\n'
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

//Carrega os dados para as structs
//Faz verificações de duplicados, etc
//Erros são colocadas em ERROS_TXT
//Ordena os arrays
void carregar_dados_txt(const char * nome_ficheiro_dados,const char * nome_ficheiro_escolar, Uni * bd) { 
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
                ler_data(&(bd->aluno[bd->tamanho_aluno].nascimento), parametros[2], '0');
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

void carregar_dados_bin(const char * nome_ficheiro, Uni * bd) {
    FILE * ficheiro = fopen(nome_ficheiro, "rb");
    if (!ficheiro) {
        printf("Ocorreu um erro ao abrir o ficheiro de dados.\n");
        printf("Por favor verifique se o ficheiro '%s' está no mesmo diretório do programa.\n", nome_ficheiro);
        return;
    }

    //Dados dos arrays
    fread(&bd->tamanho_aluno, sizeof(int), 1, ficheiro);
    fread(&bd->capacidade_aluno, sizeof(int), 1, ficheiro);

    fread(&bd->tamanho_escolares, sizeof(int), 1, ficheiro);
    fread(&bd->capacidade_escolares, sizeof(int), 1, ficheiro);

    //É necessário alocar a memória
    bd->aluno = (Estudante *) malloc(bd->capacidade_aluno * sizeof(Estudante));
    bd->escolares = (Dados *) malloc(bd->capacidade_escolares * sizeof(Dados));

    if (!bd->aluno || !bd->escolares) {
		printf("Ocorreu um erro ao alocar memória para os alunos.\n");
        if (bd->aluno) free(bd->aluno);
        if (bd->escolares) free(bd->escolares);
        fclose(ficheiro);
        return;
	}

    //Aluno
    for (int i = 0; i < bd->tamanho_aluno; i++) {
        fread(&bd->aluno[i].codigo, sizeof(int), 1, ficheiro);
        fread(&bd->aluno[i].nascimento, sizeof(Data), 1, ficheiro);

        //Ao carregar os respetivos tamanhos, as variáveis vão ficar com valor
        size_t tamanho_nome, tamanho_nacionalidade;
        
        //Nome
        fread(&tamanho_nome, sizeof(size_t), 1, ficheiro);
        bd->aluno[i].nome = (char *) malloc(tamanho_nome);
        fread(bd->aluno[i].nome, tamanho_nome, 1, ficheiro);
        //Nacionalidade
        fread(&tamanho_nacionalidade, sizeof(size_t), 1, ficheiro);
        bd->aluno[i].nacionalidade = (char *) malloc(tamanho_nacionalidade);
        fread(bd->aluno[i].nacionalidade, tamanho_nacionalidade, 1, ficheiro);
    }
    //Ler array de escolares
    fread(bd->escolares, sizeof(Dados), bd->tamanho_escolares, ficheiro);
    //Estatísticas
    fread(&bd->stats, sizeof(Estatisticas), 1, ficheiro);

    fclose(ficheiro);
}

//Guarda os dados no mesmo ficheiro do qual leu inicialmente, ordenados pelo código
void guardar_dados_txt(const char * nome_ficheiro_dados, const char * nome_ficheiro_escolares, Uni * bd) {
    FILE * aluno = fopen(nome_ficheiro_dados, "w");
    FILE * dados = fopen(nome_ficheiro_escolares, "w");

    if (!aluno) {
        printf("Ocorreu um erro ao abrir o ficheiro %s para guardar os dados.\n", nome_ficheiro_dados);
    }
    else {
        for(int i = 0; i < bd->tamanho_aluno; i++) {
            //Colocar \n apenas se não for nem a primeira nem a última entrada.
            if (i > 0) fprintf(aluno, "\n");
            fprintf(aluno, "%d%c%s%c%02hd-%02hd-%04hd%c%s",
                bd->aluno[i].codigo, SEPARADOR,
                bd->aluno[i].nome, SEPARADOR,
                bd->aluno[i].nascimento.dia, bd->aluno[i].nascimento.mes, bd->aluno[i].nascimento.ano, SEPARADOR, 
                bd->aluno[i].nacionalidade);
            }
        fclose(aluno);
        printf("Os dados foram guardados com sucesso no ficheiro '%s'.\n", nome_ficheiro_dados);
    }

    if (!dados) {
        printf("Ocorreu um erro ao abrir o ficheiro %s para guardar os dados.\n", nome_ficheiro_escolares);
        return; //Já não há mais nada a guardar
    }
    for(int i = 0; i < bd->tamanho_escolares; i++) {
        if (i > 0) fprintf(dados, "\n");
        fprintf(dados, "%d%c%hd%c%hd%c%hd%c%.1f",
            bd->escolares[i].codigo, SEPARADOR,
            bd->escolares[i].matriculas, SEPARADOR,
            bd->escolares[i].ects, SEPARADOR, 
            bd->escolares[i].ano_atual, SEPARADOR,
            bd->escolares[i].media_atual);
    }
    fclose(dados);
    printf("Os dados foram guardados com sucesso no ficheiro '%s'.\n", nome_ficheiro_escolares);

    pressione_enter();
}

void guardar_dados_bin(const char * nome_ficheiro, Uni * bd, const char modo) {
    FILE * ficheiro = fopen(nome_ficheiro, "wb");
    if (!ficheiro) {
        if (modo == '1') printf("Ocorreu um erro ao guardar os dados.\n");
        return;
    }
    //Dados dos arrays
    fwrite(&bd->tamanho_aluno, sizeof(int), 1, ficheiro);
    fwrite(&bd->capacidade_aluno, sizeof(int), 1, ficheiro);

    fwrite(&bd->tamanho_escolares, sizeof(int), 1, ficheiro);
    fwrite(&bd->capacidade_escolares, sizeof(int), 1, ficheiro);

    //Escrever aluno
    for (int i = 0; i < bd->tamanho_aluno; i++) {
        fwrite(&bd->aluno[i].codigo, sizeof(int), 1, ficheiro);
        fwrite(&bd->aluno[i].nascimento, sizeof(Data), 1, ficheiro);

        //Para as strings, guardamos o tamanho e a própria string por uma questão de ser mais fácil de ler mais tarde
        size_t tamanho_nome = strlen(bd->aluno[i].nome) + 1;
        size_t tamanho_nacionalidade = strlen(bd->aluno[i].nacionalidade) + 1;
        
        fwrite(&tamanho_nome, sizeof(size_t), 1, ficheiro);
        fwrite(bd->aluno[i].nome, tamanho_nome, 1, ficheiro);

        fwrite(&tamanho_nacionalidade, sizeof(size_t), 1, ficheiro);
        fwrite(bd->aluno[i].nacionalidade, tamanho_nacionalidade, 1, ficheiro);
    }
    //Escrever escolares
    fwrite(bd->escolares, sizeof(Dados), bd->tamanho_escolares, ficheiro);
    //Estatísticas
    fwrite(&bd->stats, sizeof(Estatisticas), 1, ficheiro);

    fclose(ficheiro);
    if (modo == '1') {
        printf("Os dados foram guardados com sucesso em '%s'\n", nome_ficheiro);
    }
}

//flag deve ser um ficheiro txt
//Verifica se o programa já foi instaladou ou não
//Retorna 1 caso seja necessário instalar
int fase_instalacao(const char * flag) {
    if (!flag) return 0;

    FILE * ficheiro = fopen(flag, "r");

    //Primeira iteração do programa
    if (!ficheiro) {
        ficheiro = fopen(flag, "w");
        fprintf(ficheiro, "NÃO ELIMINAR ESTE FICHEIRO SOB QUALQUER CIRCUNSTÂNCIA!");
        fclose(ficheiro);
        return 1;
    }

    return 0;
}

//nome já deve incluir a extensão
void eliminar_ficheiro(const char * nome) {
    if (!nome) return;

    if (remove(nome) == 0) {
        printf("O ficheiro '%s' foi eliminado com sucesso.\n", nome);
    }
    else {
        printf("Ocorreu um erro ao eliminar o ficheiro '%s'.\n", nome);
        printf("Por favor verifique que o nome do ficheiro inclui a extensão e se está no mesmo diretório do programa.\n");
    }
}

//Pede ao user se quer efetuar uma cópia em ficheiro da listagem atual
//Mostra os formatos disponíveis e pede o nome do ficheiro a escrever
//Verifica se o nome é válido
//Retorna um ponteiro para o ficheiro ABERTO, se for o caso
//Retorna NULL em caso de não querer listar ou erro
//São adereçados todos os erros com uma mensagem ao user
FILE * pedir_listagem(char * formato_selecionado) {
    const char * formatos[] = {".txt", ".csv"};
    short opcao;
    char * nome_ficheiro;
    FILE * ficheiro = NULL;

    printf("Deseja guardar a listagem num ficheiro? (S/N): ");
    if (sim_nao()) {
        opcao = mostrar_menu(menu_formatos_disponiveis, '0', '2') - '0';
        if (opcao == 0) {
            limpar_terminal();
            return NULL; //Sair
        }
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

//Abre o ficheiro dado e escreve toda a sua informação no terminal
//Verifica a extensão: apenas suporta .txt e .csv
void mostrar_dados_ficheiro(const char * nome_ficheiro) {
     //Verificar a extensão do ficheiro
     //Se houvesse uma extensão que não fosse .txt ou .csv a função não funcionava e levava a erro.
    const char * extensao = strrchr(nome_ficheiro, '.');
    if (!extensao) {
        printf("O ficheiro não tem extensão.\n");
        pressione_enter();
        return;
    }

    if (strcmp(extensao, ".txt") != 0 && strcmp(extensao, ".csv") != 0) {
        printf("A extensão %s não é suportada.\n", extensao);
        pressione_enter();
        return;
    }

    FILE * ficheiro = fopen(nome_ficheiro, "r");
    char * linha = NULL;
    int num_linhas = 0;
    //Verificar se o ficheiro existe
    if (!ficheiro) {
        printf("O ficheiro \"%s\" não existe.\n", nome_ficheiro);
        pressione_enter();
        return;
    }
    //Verificar se o ficheiro está vazio
    fseek(ficheiro, 0, SEEK_END);
    if (ftell(ficheiro) == 0) {
        printf("O ficheiro \"%s\" está vazio.\n", nome_ficheiro);
        fclose(ficheiro);
        pressione_enter();
        return;
    }
    rewind(ficheiro);
    //Ler o ficheiro e escrever no terminal
    while((linha = ler_linha_txt(ficheiro, &num_linhas)) != NULL) {
        if (num_linhas != 0 && num_linhas % PAUSA_LISTAGEM == 0) {
            printf("\n");
            pressione_enter();
        }
        printf("%s\n", linha);
        free(linha);
    }
    fclose(ficheiro);
    pressione_enter();
}

//Gestão de memória

//Inicializa tudo com -1 (até strings) exceto data(0)
//Inicializa de indice_aluno até bd->capacidade_aluno
//Inicializa memória para nome e nacionalidade dinamicamente
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

//Inicializa tudo com -1 exceto prescrever e finalista('-')
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

//Inicializa tudo a 0 , incluindo atualizado
void inicializar_estatisticas(Estatisticas * stats) {
    stats->finalistas = 0;
    stats->media = 0.0;
    stats->media_matriculas = 0.0;
    stats->risco_prescrever = 0;
    stats->atualizado = '0'; //Inicializamos a 0 pois quando carregamos os dados, estamos possivelmente a introduzir dados novos que podem contar para a estatística.
}

//Libertar a memória alocada em aluno ao sair do programa.
void free_aluno(Uni * bd) {
    if (!bd || !bd->aluno) return;

    for(int i = 0; i < bd->tamanho_aluno; i++) {
        if (bd->aluno[i].nome) free(bd->aluno[i].nome);
        if (bd->aluno[i].nacionalidade) free(bd->aluno[i].nacionalidade);
    }
}

//Duplica o espaço atual 
//Não inicializa o espaço alocado
//modo = '1' para adereçar erros
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

//Duplica o espaço atual 
//Não inicializa o espaço alocado
//modo = '1' para adereçar erros
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

//Duplica o espaço atual 
//Não inicializa o espaço alocado
//modo = '1' para adereçar erros
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

//Apenas retorna 0 em caso de erro
//Retorno < 0 se não existe. Representa a posição de inserção -(indice no array + 1)
//Retorno > 0 se existe. Representa o índice do código no array + 1
//O array deve estar ordenado
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

//Apenas retorna 0 em caso de erro
//Retorno < 0 se não existe. Representa a posição de inserção -(indice no array + 1)
//Retorno > 0 se existe. Representa o índice do código no array + 1
//O array deve estar ordenado
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

//Retorna NULL em caso de erro, do user escrever 'sair' ou de não haver nacionalidades suficientes para satisfazer n_nacionalidades
//Usar mensagem NULL para usar mensagem padrão. mensagem não deve incluir ': '
//Aloca memória dinamicamente para um array de ponteiros de tamanho n_nacionalidades
char ** procurar_nacionalidades(Uni * bd, const short n_nacionalidades, char * mensagem) {
    if (n_nacionalidades <= 0) return NULL;
    char ** nacionalidades = NULL;
    
    //Criamos um array de ponteiros para guardar as nacionalidades, no máximo as indicadas.
    nacionalidades = (char**) malloc(n_nacionalidades * sizeof(char*));
    if (!nacionalidades) {
        printf("Ocorreu um erro a alocar memória para as nacionalidades!\n");
        return NULL;
    }
    
    //Inicializar ponteiros a NULL
    for (int i = 0; i < n_nacionalidades; i++) {
        nacionalidades[i] = NULL;
    }

    limpar_terminal();

    //Procurar nacionalidades
    for(int i = 0; i < n_nacionalidades; i++) {
        char * nacionalidade = NULL;
        char encontrado = '0';

        //Array de string que irá conter todas as sugestões que foram feitas ao utilizador numa mesma nacionalidade.
        //Apenas serão retidas 5 sugestões, pelo que depois,  o sistema de sugestões será desabilitado num todo.
        char ** sugeridas = NULL;
        short n_sugestao = 0;
        sugeridas = (char**) malloc(TAMANHO_SUGESTOES * sizeof(char *));
        for (int j = 0; j < TAMANHO_SUGESTOES; j++) {
            sugeridas[j] = NULL;
        }
        //Pedimos a nacionalidade e depois verificamos se existe. Continua a pedir até que 
        do {
            if (!mensagem) {
                printf("Insira a nacionalidade %d (ou 'sair' para cancelar): ", i + 1);
            }
            else printf("%s (ou 'sair' para cancelar): ", mensagem);

            nacionalidade = ler_linha_txt(stdin, NULL);
            if (!nacionalidade) continue;

            //Verificar saída
            if (strcmp(nacionalidade, "sair") == 0 || strcmp(nacionalidade, "SAIR") == 0) {
                free(nacionalidade);
                for (int j = 0; j < i; j++) {
                    free(nacionalidades[j]);
                }
                free(nacionalidades);
                return NULL;
            }

            //Validar a nacionalidade
            if (!validar_nacionalidade(nacionalidade, '1')) {
                free(nacionalidade);
                continue;
            }

            char * original = strdup(nacionalidade); //original fica com a nacionalidade original
            if (!original) {
                printf("Erro ao alocar memória.\n");
                free(nacionalidade);
                for (int j = 0; j < i; j++) {
                    free(nacionalidades[j]);
                }
                free(nacionalidades);
                return NULL;
            }
            nacionalidade = normalizar_string(nacionalidade);
            if (!nacionalidade) {
                printf("Ocorreu um erro a normalizar a string. Por favor, tente novamente.\n");
                free(original);
                for (int j = 0; j < i; j++) {
                    free(nacionalidades[j]);
                }
                free(nacionalidades);
                return NULL;
                
            }
            
            //Verificar se a nacionalidade existe.
            for(int j = 0; j < bd->tamanho_aluno && encontrado == '0'; j++) {
                //Normalizar o elemento atual.
                char * nac = normalizar_string(bd->aluno[j].nacionalidade); //nac é a nacionalidade atual normalizada
                if (!nac) continue;

                if (strcmp(nacionalidade, nac) == 0) {
                    nacionalidades[i] = strdup(bd->aluno[j].nacionalidade);
                    if (!nacionalidades[i]) {
                        printf("Ocorreu um erro a alocar memória. Por favor tente novamente.\n");
                        free(nacionalidade);
                        free(nac);
                        continue;
                    }
                    encontrado = '1';
                    free(nacionalidade);
                    free(nac);
                    break; //Não é necessário continuar a verificar o array todo.
                } 
                else if (n_sugestao < TAMANHO_SUGESTOES && strncmp(nacionalidade, nac, 3) == 0) {
                    //Verificar se já sugerimos esta nacionalidade
                    if (n_sugestao != 0) {
                        char sair = '0';
                        for (int k = 0; k < n_sugestao; k++) {
                            if (strcmp(nac, sugeridas[k]) == 0) {
                                free(nac);
                                sair = '1'; //necessário porque o continue não funciona aqui.
                                break; 
                            }
                        }
                        if (sair == '1') continue;
                    }
                    //Sugerir
                    printf("Quer dizer \"%s\"? (S/N) ", bd->aluno[j].nacionalidade);
                    if(sim_nao()) {
                        //Alterar a nacionalidade para o que foi proposto
                        free(nacionalidade);
                        nacionalidade = strdup(nac);
                        free(nac);
                        j--;
                        continue;
                    }
                    //Em caso negativo, queremos guardar essa string para não a apresentar de novo
                    sugeridas[n_sugestao++] = strdup(nac);
                }
                free(nac);
            }

            if (encontrado == '0') {
                printf("Nenhum aluno possui nacionalidade \"%s\".\n", original);
            }
            free(original);
        } while(encontrado == '0');
        //Libertar o array das sugestões
        for (int j = 0; j < n_sugestao; j++) {
            free(sugeridas[j]);
        }
        free(sugeridas);
    }
    
    return nacionalidades;
}

//Faz a validação do código e retorna -(indice + 1)s
//Caso não seja válido retorna 0
int validar_codigo_ao_inserir(int codigo, Uni * bd) {
    if(codigo <= 0) {
        printf("Código inválido! Insira um número inteiro positivo.\n");
        pressione_enter();
        return 0;
    }
    int temp = procurar_codigo_aluno(codigo, bd); 
    //basta procurar no aluno porque não pode haver nenhum código em escolares que não esteja em aluno
    
    if(temp > 0) {
        printf("O código já existe! Insira um código diferente.\n");
        pressione_enter();
        return 0;
    }
    //Se não existir o código podemos usá-lo(<0).
    else if (temp < 0) {
        return temp; //Retornamos a posição onde será inserido
    }
    return 0; //Caso seja 0
}

//Faz a validação do código e retorna indice + 1
//Caso não seja válido retorna 0
//modo == '1' para mostrar mensagens de erro
int validar_codigo_eliminar(int codigo, Uni * bd, const char modo) {
    if(codigo <= 0) {
        if (modo == '1') {
            printf("Código inválido! Insira um número inteiro positivo.\n");
            pressione_enter();
        }
        return 0;
    }
    int temp = procurar_codigo_aluno(codigo, bd); //basta procurar no aluno porque não pode haver nenhum código em escolares que não esteja em aluno
    
    if(temp > 0) {
        return temp; //codigo existe logo pode ser eliminado
    }
    //Se não existir o código podesmos usá-lo(<0).
    else if (temp < 0) {
        if (modo == '1') {
            printf("O código não existe! Insira um código diferente.\n");
            pressione_enter();
        }
        return 0; //<0 significa que foi retornada a posição onde o código devia ser inserido, logo não existe.
    }
    return 0; 
}

//Retorna 1 se o nome for válido. 0 caso contrário
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

//Verifica se já existe algum ficheiro com o mesmo nome
//Retorna NULL se o ficheiro existir e o user não queira sobreescrevê-lo
//Retorna um ponteiro para o ficheiro aberto em modo "w" nos outros casos
FILE * validar_ficheiro_e_abrir(const char * nome) {
    /*
    Esta função segue os padrões do Windows, mas obviamente que também funciona nos outros sistemas operativos, porém de forma mais restritiva
    O nome do ficheiro é case insensitive (tal como no windows)
    */
    FILE * ficheiro = fopen(nome, "r"); 
    //Verificar se o ficheiro existe
    if (ficheiro) {
        fclose(ficheiro);
        printf("Já existe um ficheiro com o nome \"%s\". Quer substituir o ficheiro no destino? (S/N) ", nome);
        if (!sim_nao()) {
            return NULL;
        }
        limpar_terminal();
    }
    //Abrimos em modo escrita como normal
    ficheiro = fopen(nome, "w");
    if (!ficheiro) {
        printf("Ocorreu um erro a abrir o ficheiro. Por favor, tente novamente mais tarde.\n");
        return NULL;
    }
    
    return ficheiro;
}

//Usar apenas ao carregar dados
//Mantém o primeiro código a ser inserido
//Requer que o array esteja ordenado
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
//Elimina e ordena o array
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
//Valida ano, mês e dia
//Verifica anos bissextos
int validar_data(short dia, short mes, short ano, const char modo) {
    if (ano < 1 || mes < 1 || mes > 12 || dia < 1) { //Limites dos anos são os que foram considerados mais realistas
        if (modo == '1') printf("\nPor favor, insira uma data válida.\n");
        return 0;
    }
    
    if (ano < DATA_ATUAL.ano - IDADE_MAXIMA || ano > DATA_ATUAL.ano - IDADE_MINIMA) {
        if (modo == '1') printf("\nO ano inserido é inválido. Insira um ano entre %d e %d.\n", DATA_ATUAL.ano - IDADE_MAXIMA, DATA_ATUAL.ano - IDADE_MINIMA);
        return 0;
    }

    //Criamos um vetor com os dias de cada mes, fevereiro com 28 pois é o mais comum
    short dias_por_mes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    //Apenas criado para propóstios de informação ao user
    const char * nome_do_mes[] = {"janeiro", "fevereiro", "março", "abril", "maio", "junho", "julho", "agosto", "setembro", "outubro", "novembro", "dezembro"};

    //Verificar anos bissextos
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

//Verifica se a data de 'atual' está entre a de inferior e superior, respetivamente
int validar_data_entre_intervalo(Data inferior, Data superior, Data atual) {
    int comp = comparar_data(superior, inferior, '0');
    switch(comp) {
        case -1: 
            return 0; //superior < inferior.
        case 0: 
            //Se inf == sup, então atual == inf == sup.
            if (comparar_data(inferior, atual, '0') != 0) return 0;
            return 1; //atual == inf == sup logo é válido (está entre os dois, é valor único).
        case 1: 
            if ((comparar_data(atual, inferior, '0') != -1)&&(comparar_data(atual, superior, '0') != 1)) return 1;
            return 0;
        default:
            return 0;
    }
}

//Retorna -1, 0 e 1 para d1 menor, igual, maior a d2
//Usar ignorar_ano == '1' para comparar apenas mês e dia. '0' para comparar tudo
int comparar_data(Data d1, Data d2, const char ignorar_ano) {
    if (ignorar_ano == '0') {
        if (d1.ano < d2.ano) return -1;
        if (d1.ano > d2.ano) return 1;
    }
    
    if (d1.mes < d2.mes) return -1;
    if (d1.mes > d2.mes) return 1;
    
    if (d1.dia < d2.dia) return -1;
    if (d1.dia > d2.dia) return 1;
    
    return 0;  //Datas são iguais.
}

//Realoca o array de nome caso seja necessário
//Pode ser usada ao carregar_dados
//modo == '1' para mostrar mensagens de erro
int validar_nome(Estudante * aluno, char * nome, const char modo) {
    if (!nome) {
        if (modo == '1') printf("\nNome em branco!\n");
        return 0;
    }
    int comprimento = strlen(nome);
    
    //Retirar o \n no final, se tiver, mas apenas se a string for apenas \n não contamos como válido
    if (nome[comprimento - 1] == '\n' && comprimento != 1) { 
        nome[comprimento - 1] = '\0';
        comprimento--;
    }
    // Verificar o último caractere
    if (nome[0] == ' ' || nome[comprimento - 1] == ' ') {
        if (modo == '1') printf("\nO nome tem espaços indevidos!\n");
        return 0;
    }

    for (int i = 0; i < comprimento; i++) {
        if (nome[i] == SEPARADOR) {
            if (modo == '1') printf("\nO nome contém um caracter separador inválido (%c).\n", SEPARADOR);
            return 0;
        }
        // Verificar se há dois espaços seguidos
        if (i < comprimento -1 && nome[i] == ' ' && nome[i+1] == ' ') {
            if (modo == '1') printf("\nO nome não pode conter espaços consecutivos!\n");
            return 0;
        }
        //Verificar caracteres inválidos
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

//Não realoca nacionalidade, ao invés disso, mostra uma mensagem de erro
//Pode ser usada ao carregar_dados
//modo == '1' para mostrar mensagens de erro
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

//Faz a validação dos limites do menu
//Verifica entradas inválidas que não sejam números do menu
//Limpa sempre o buffer
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
    //Verificar entradas com mais de um char.
    char lixo = getchar();
    if (lixo != '\n') {
        *valido = 0;
        limpar_buffer(); //\n
        printf("Por favor, escolha um número do menu (%c a %c).\n", limInf, limSup);
        pressione_enter();
    }
    //Assim saímos do menu sempre com o buffer limpo. (menos uma preocupação)
}

//Ordenação

//As funções que se seguem implementam o algoritmo de ordenação Merge Sort.
//Dado o algoritmo já ser bastante conhecido, não há necessidade de reinventar, e como tal, foi retirado o código base do algoritmo do website https://www.geeksforgeeks.org/c-program-for-merge-sort/
//Claro que foram necessários ajustes ao contexto do programa. Os nome das variáveis também foram ajustados para ser mais fácil de perceber o que se pretende

//Combina duas metades do array em uma única sequência ordenada
//Cuidado: inicio e fim são os ÍNDICES dos arrays, não o tamanho
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

//Ordena o array de aluno
//Usar quando o array estiver muito desordenado ou não se souber o estado do mesmo
//Cuidado: inicio e fim são os ÍNDICES dos arrays, não o tamanho
void merge_sort_aluno(Uni * bd, int inicio, int fim) {
    if (inicio < fim) { //Se inicio >= fim, o array tem 1 
        int meio = inicio + (fim - inicio) / 2;
        merge_sort_aluno(bd, inicio, meio); //Ordena a primeira metade
        merge_sort_aluno(bd, meio + 1, fim); //Ordena a segunda metade
        //Isto vai ser usado recursivamente, pelo que cada metade vai ser novamente cortada a metade,... até o array ter um elemento

        merge_aluno(bd, inicio, meio, fim); //Combina as duas metades ordenadas
    }
}

//Ordena o array de escolares 
//Cuidado: inicio e fim são os ÍNDICES dos arrays, não o tamanho
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

//Combina duas metades do array em uma única sequência ordenada
//Usar quando o array estiver muito desordenado ou não se souber o estado do mesmo
//Cuidado: inicio e fim são os ÍNDICES dos arrays, não o tamanho
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
//Usar ao inserir alunos manualmente, nunca em carregar_dados
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
    //Incrementar tamanho total
    bd->tamanho_aluno++;
    
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
    
    bd->tamanho_escolares++;
}

//Altera o tamanho total do array.
//Retorna 0 caso o código não exista ou tenha ocorrido um erro.
//Retorna 1 em caso de sucesso.
//Usar ao eliminar alunos manualmente
int ordenar_ao_eliminar(int codigo, Uni * bd, const char modo) {
    int posicao_eliminacao_aluno;
    int posicao_eliminacao_escolares;

    posicao_eliminacao_aluno = validar_codigo_eliminar(codigo, bd, modo);
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

//Função genérica para mostrar um menu
//1º argumento é uma função do tipo void que apenas printa o menu
//Argumento 2 e 3 são os valores mínimos e máximos que o menu deve aceitar
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

    return '0'; //colocado aqui porque o compilador não gostava de não haver return 
}

//As seguintes funções que começam por "menu" servem apenas para printar o menu
//Os comentários das funções são o limInf e limSup do menu

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

//0-2
void menu_gerir_estudantes() { 
    printf("╔════════════════════════════════════╗\n");
    printf("║          GERIR ESTUDANTES          ║\n");
    printf("╠════════════════════════════════════╣\n");
    printf("║  1. Inserir estudante              ║\n");
    printf("║  2. Eliminar estudante             ║\n");
    printf("║  0. Voltar ao menu anterior        ║\n");
    printf("╚════════════════════════════════════╝\n\n");
}

//0-4
void menu_consultar_dados() {
    printf("╔═════════════════════════════════════════════════════════════╗\n");
    printf("║                       CONSULTAR DADOS                       ║\n");
    printf("╠═════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Procurar estudante por nome                             ║\n");
    printf("║  2. Listar estudantes por intervalo de datas de nascimento  ║\n");
    printf("║     e pertencentes a uma nacionalidade                      ║\n");
    printf("║  3. Listar estudantes por ordem alfabética de apelido       ║\n");
    printf("║  4. Apresentar dados de um certo estudante                  ║\n");
    printf("║  0. Voltar ao menu anterior                                 ║\n");
    printf("╚═════════════════════════════════════════════════════════════╝\n\n");
}

//0-7
void menu_estatisticas() {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                      ESTATÍSTICAS                      ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  1. Estudantes por escalão de média atual              ║\n");
    printf("║  2. Número médio de matrículas (geral/nacionalidade)   ║\n");
    printf("║  3. Média atual geral                                  ║\n");
    printf("║  4. Número de finalistas                               ║\n");
    printf("║  5. Média de idades por nacionalidade e ano            ║\n");
    printf("║  6. Número de estudantes por escalão de idade          ║\n");
    printf("║  7. Estudantes em risco de prescrição                  ║\n");
    printf("║  0. Voltar ao menu anterior                            ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
}

//Necessário mudar o nome dos ficheiros caso sofram alterações
//0-5
void menu_ficheiros() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                         FICHEIROS                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Guardar dados                                          ║\n");
    printf("║  2. Mostrar dados de erros.txt (erros ao carregar dados)   ║\n");
    printf("║  3. Mostrar dados de dados.txt                             ║\n");
    printf("║  4. Mostrar dados de situacao_Escolar_Estudantes.txt       ║\n");
    printf("║  5. Mostrar dados de um ficheiro (.txt ou .csv) por nome   ║\n");
    printf("║  0. Voltar ao menu anterior                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
}

//0-3
void menu_aniversarios() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                        ANIVERSÁRIOS                        ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Estudantes nascidos em dias específicos da semana      ║\n");
    printf("║  2. Estudantes cujo aniversário em certo ano é ao domingo  ║\n");
    printf("║  3. Estudantes cujo aniversário em certo ano é na Quaresma ║\n");
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

//0-2
void menu_media_matriculas() {
    printf("╔══════════════════════════╗\n");
    printf("║     MÉDIA MATRÍCULAS     ║\n");
    printf("╠══════════════════════════╣\n");
    printf("║  1. Geral                ║\n");
    printf("║  2. Por nacionalidade    ║\n");
    printf("║  0. Sair                 ║\n");
    printf("╚══════════════════════════╝\n\n");
}

//Coração do programa
//Apresenta o menu principal e depois chama os submenus
void the_architect(Uni * bd) {
    char opcao;

    do {
        opcao = mostrar_menu(menu_principal, '0', '5');
        switch(opcao) {
            case '0':
                limpar_terminal();
                printf("Tem a certeza que quer sair do programa? (S/N) ");
                if (!sim_nao()) {
                    opcao = '1'; //Forçar o loop a continuar
                    //Não há problemas porque no ínicio do loop pedimos sempre o menu.
                    continue;
                }
                break;
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
                processar_aniversarios(bd);
                break;
            default:
                opcao = '0'; //Sair caso haja erro
                break;
        }
    } while(opcao != '0');
}

//Apresenta o menu e faz todas as operações
void processar_gerir_estudantes(Uni * bd) {
    char opcao;
    do {
        opcao = mostrar_menu(menu_gerir_estudantes, '0', '2');
        switch(opcao) {
            case '0': break;
            case '1':
                inserir_estudante(bd);
                break;
            case '2':
                eliminar_estudante(bd);
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Apresenta o menu e faz todas as operações
void processar_consultar_dados(Uni * bd) {
    char opcao;
    do {
        opcao = mostrar_menu(menu_consultar_dados, '0', '4');
        switch(opcao) {
            case '0': break;
            case '1':
                procurar_estudante_por_nome(bd);
                break;
            case '2':
                listar_estudantes_por_data_e_nacionalidades(bd);
                break;
            case '3':
                listar_apelidos_alfabeticamente(bd);
                break;
            case '4':
                listar_info_estudante(bd);
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Apresenta o menu e faz todas as operações
//Calcula as estatísticas gerais se opcao != '0'
void processar_estatisticas(Uni * bd) {
    char opcao;
    do {
        opcao = mostrar_menu(menu_estatisticas, '0', '7');
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
                tabela_medias_ano(bd);
                break;
            case '2':
                calcular_media_matriculas(bd);
                break;
            case '3':
                limpar_terminal();
                printf("A média atual é de %.1f.\n", bd->stats.media);
                pressione_enter();
                break;
            case '4':
                finalistas(bd);
                break;
            case '5':
                media_idades_por_nacionalidade(bd);
                break;
            case '6':
                tabela_idade_por_escalao(bd);
                break;
            case '7':
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
        opcao = mostrar_menu(menu_ficheiros, '0', '5');
        switch(opcao) {
            case '0': break;
            case '1':
                guardar_dados_txt(DADOS_TXT, SITUACAO_ESCOLAR_TXT, bd);
                break;
            case '2':
                mostrar_dados_ficheiro(ERROS_TXT);
                break;
            case '3':
                mostrar_dados_ficheiro(DADOS_TXT);
                break;
            case '4':
                mostrar_dados_ficheiro(SITUACAO_ESCOLAR_TXT);
                break;
            case '5':
                char * nome_ficheiro = NULL;
                limpar_terminal();
                do {
                    printf("Introduza o nome do ficheiro (com a extensão) do qual quer ver os dados (ou 'sair' para cancelar): ");
                    nome_ficheiro = ler_linha_txt(stdin, NULL);
                    if (!nome_ficheiro) continue;
                    else if (strcmp(nome_ficheiro, "sair") == 0 || strcmp(nome_ficheiro, "SAIR") == 0) {
                        free(nome_ficheiro);
                        break;
                    }
                    limpar_terminal();
                    mostrar_dados_ficheiro(nome_ficheiro);
                    break;
                } while(1);
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Apresenta o menu e faz todas as operações.
void processar_aniversarios(Uni * bd) {
    char opcao;
    do {
        opcao = mostrar_menu(menu_aniversarios, '0', '3');
        switch(opcao) {
            case '0': break;
            case '1':
                listar_aniversarios_por_dia(bd);
                break;
            case '2':
                listar_aniversario_ao_domingo(bd);
                break;
            case '3':
                listar_aniversario_na_quaresma(bd);
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Inserção/leitura de dados

//modo '1' para imprimir mensagens de erro
//str == NULL para ler de stdin
//Insere e valida a data
void ler_data(Data * aluno, char * str, const char modo) {
	char data[11]; //Vamos usar o formato DD-MM-AAAA (10 caracteres + \0)
    char erro = '0'; //1 há erro
    short dia_temp, mes_temp, ano_temp; //Variáveis temporárias para armazenar os valores lidos
    char extra;

	do {
        erro = '0';
        if (!str) { //Se str for NULL queremos ler a data
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
            //Verificar se foi escrito mais do que o suposto
            else {
                if (!verificar_e_limpar_buffer()) {
                    if (modo == '1') printf("Entrada inválida. Por favor, escreva a data no formato DD-MM-AAAA.\n");
                    erro = '1';
                    continue;
                }
            }
        }
        //Verificar se foi escrito mais do que o suposto mas para quando a string é passada
        else {
            if (strlen(str) >= sizeof(data)) {
                if (modo == '1') printf("Entrada inválida. Por favor, escreva a data no formato DD-MM-AAAA.\n");
                erro = '1';
                continue;
            }
            //Copiar string para array
            strcpy(data, str);
        }
        
        //O sscanf lê as x entradas conforme o padrão apresentado
        //Nota importante: %c serve para detetar caracteres a mais não válidos
        //Se isso acontecer, sscanf retorna 4 e há erro. Se não, retorna 3, não há erro, e avança
    	if (sscanf(data, "%hd-%hd-%hd%c", &dia_temp, &mes_temp, &ano_temp, &extra) != 3) { //sscanf tenta ler 3 shorts para as var temp
            //Aqui não se usa && erro == '0'porque só pode ser 0 se !str, e nesse caso há um continue
            if (modo == '1') printf("Formato inválido! Use o formato DD-MM-AAAA.\n");
            erro = '1';
        }
        //Sse data for válida é que passamos os dados à stuct
        if (erro == '0' && validar_data(dia_temp, mes_temp, ano_temp, modo)) {
            aluno->dia = dia_temp;
            aluno->mes = mes_temp;
            aluno->ano = ano_temp;
            return;
        } 
        else erro = '1';

        if(str && erro == '1') { //Caso contrário ficavamos num loop infinito
            return;
        }
	} while (erro == '1' && !str); //Continuar a pedir a data sempre que esta for inválida
}

//Insere todos os dados de um estudante
//Faz as validações necessárias
//Ordena os dados após inserção
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
            limpar_terminal();
            int codigo_temp = -1;
            pedir_codigo(&codigo_temp);
            if (codigo_temp == 0) return;
            
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

        //Ler data já faz as validações necessárias e coloca a data.
        ler_data(&(bd->aluno[posicao_insercao_aluno].nascimento), NULL, '1');
        
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
            else if (!verificar_e_limpar_buffer()) {
                printf("Entrada inválida! Por favor, escreva apenas o número de matrículas do aluno.\n");
                pressione_enter();
                continue;
            }
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
            else if (!verificar_e_limpar_buffer()) {
                printf("Entrada inválida! Por favor, escreva apenas o número de créditos ECTS do aluno.\n");
                pressione_enter();
                continue;
            }
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
            printf("Insira o ano atual de curso do estudante: ");
            if (scanf("%hd", &ano_atual_temp) != 1) { //Verifica entradas inválidas como letras
                printf("Ano atual de curso inválido! Insira um número inteiro positivo.\n");
                limpar_buffer();
                pressione_enter();
                continue; //Continue faz com que salte o resto do loop e passe à próxima iteração
            }
            else if (!verificar_e_limpar_buffer()) {
                printf("Entrada inválida! Por favor, escreva apenas o ano atual de curso do aluno.\n");
                pressione_enter();
                continue;
            }
            if (ano_atual_temp < 1 || ano_atual_temp > MAX_ANO_ATUAL) {
                printf("O ano atual de curso é inválido. Deve estar entre 0 e %d.\n", MAX_ANO_ATUAL);
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
            else if (!verificar_e_limpar_buffer()) {
                printf("Entrada inválida! Por favor, escreva apenas a média do aluno.\n");
                pressione_enter();
                continue;
            }
            if (media_temp < 0 || media_temp > 20) {
                printf("A média atual é inválida. Deve estar entre 0 e 20.\n");
                pressione_enter();
                continue;
            }
            bd->escolares[posicao_insercao_escolares].media_atual = media_temp;
            break;
        } while(1);

        printf("\nO código %d foi introduzido com sucesso!\n", bd->aluno[posicao_insercao_aluno].codigo);

        printf("\nQuer inserir mais estudantes? (S/N): ");
        if (!sim_nao()) return;
    }while(1); //Não precisamos de verificar repetir pois só chega aqui se repetir == 's'
    //Introduzimos um estudante, logo os dados estatísticos têm de ser recalculados.
    bd->stats.atualizado = '0';
}

//Elimina um estudante por código
//Oferece a possibilidade de eliminar todos os alunos num dado intervalo (menos eficiente porque não foi otimizado)
//Ordena os dados após eliminar
void eliminar_estudante(Uni * bd) {
    do {
        limpar_terminal();
        printf("Quer eliminar alunos por intervalos? (S/N): ");
        //Intervalos
        if (sim_nao()) {
            int codigo_inf, codigo_sup;
            short contador;
            limpar_terminal();

            //Limite inferior
            do {
                limpar_terminal();
                codigo_inf = -1;
                printf("Intervalo inferior: \n");
                pedir_codigo(&codigo_inf);
                if (codigo_inf == 0) return;
                //Intervalos podem ser de quaisquer valores, desde que sejam > 0 e de extensão máxima permitida
                if (codigo_inf < 0) {
                    printf("Código inválido. Introduza um número inteiro positivo.\n");
                    pressione_enter();
                    continue;
                } 
                //Casos em que não haveria nada a ser eliminado
                else if (codigo_inf > bd->aluno[bd->tamanho_aluno - 1].codigo) {
                    printf("O código não existe e é superior a todos os existentes atualmente. Por favor introduza um novo código!\n");
                    pressione_enter();
                    continue;
                }
                break;
            } while(1);

            pressione_enter();
            //Limite superior
            do {
                limpar_terminal();
                codigo_sup = -1;
                printf("Intervalo superior: \n");
                pedir_codigo(&codigo_sup);
                if (codigo_sup == 0) return;
                //Intervalos podem ser de quaisquer valores, desde que sejam > 0 e de extensão máxima permitida
                if (codigo_sup < 0) {
                    printf("Código inválido. Introduza um número inteiro positivo.\n");\
                    pressione_enter();
                    continue;
                } 
                //Casos em que não haveria nada a ser eliminado
                else if (codigo_sup <= codigo_inf) {
                    printf("O código superior não pode ser igual ou inferior ao código inferior.\n");
                    printf("Por favor introduza um código superior a %d.\n", codigo_inf);
                    pressione_enter();
                    continue;
                }
                //Limite de eliminações por intervalo
                else if ((codigo_sup - codigo_inf + 1) > MAX_ELIMINACOES_POR_INTERVALO) {
                    printf("O intervalo de códigos a eliminar não pode exceder %d.\n", MAX_ELIMINACOES_POR_INTERVALO);
                    printf("Por favor introduza um novo código superior.\n");
                    pressione_enter();
                    continue;
                }
                break;
            } while(1);

            contador = 0;
            //Eliminar todos os códigos do intervalo, se existirem
            for (int i = codigo_inf; i <= codigo_sup; i++) {
                if (ordenar_ao_eliminar(i, bd, '0') == 1) {
                    printf("O aluno %d foi eliminado!\n", i);
                    pausa_listagem(&contador);
                }
            }
            //ta a pausar quando encontra um codigo que nao existe

            if (contador == 0) {
                printf("Nenhum aluno foi encontrado dentro do intervalo.\n");
            }
        }
        //Um a um
        else {
            limpar_terminal(); //Caso de repetição
            do {
                int codigo_temp = -1;
                pedir_codigo(&codigo_temp);
                if (codigo_temp == 0) return;
                
                //Elimina e ordena o código dado, caso seja válido.
                if(ordenar_ao_eliminar(codigo_temp, bd, '1')) {
                    printf("O aluno com o código %d foi eliminado com sucesso!\n", codigo_temp);
                    break;
                }
            } while (1);
        }

        printf("\nQuer eliminar mais estudantes? (S/N): ");
        if(!sim_nao()) return;
    } while(1);
    bd->stats.atualizado = '0';
}

//Estatísticas

//Calcula todas as estatísticas gerais(que abrangem todos os estudantes)
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

//Calcula o número médio de matrículas a nível geral ou por uma dada nacionalidade
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
                int indice = 0; //Indice em escolares.
                short contador = 0;
                //Pedir uma nacionalidade correta.
                
                char ** nacionalidades = procurar_nacionalidades(bd, 1, "Insira a nacionalidade da qual quer saber a média de matrículas");
                if (!nacionalidades) {
                    pressione_enter();
                    break;
                }
                
                for(int i = 0; i < bd->tamanho_aluno; i++) {
                    if (strcmp(nacionalidades[0], bd->aluno[i].nacionalidade) == 0) {
                        //O índice de aluno pode ser diferente de escolares.
                        indice = procurar_codigo_escolares(bd->aluno[i].codigo, bd);
                        if (indice > 0) {
                            indice -= 1;
                            media_matriculas_nacionalidade += bd->escolares[indice].matriculas;
                            contador++;
                        }
                    }
                }
                //Evitar divisão por 0.
                if (contador == 0) {
                    printf("Não foi encontrado nenhum aluno de nacionalidade %s ou o mesmo não possui número de matrículas!\n", nacionalidades[0]);
                    free(nacionalidades[0]);
                    free(nacionalidades);
                    pressione_enter();
                    break;
                }

                media_matriculas_nacionalidade /= contador;
                printf("O número médio de matrículas dos estudantes de nacionalidade %s é de %.1f.\n", nacionalidades[0], media_matriculas_nacionalidade);
                
                free(nacionalidades[0]);
                free(nacionalidades);
                pressione_enter();
                break;
            default: return;
        }
    } while(1); //Saímos em cima caso seja 0.
}

//Conta e retorna o número de alunos entre duas médias de um dado ano atual.
int alunos_por_media_e_ano(Uni * bd, float media_min, float media_max, short ano_atual) {
    int n_alunos = 0;
    for(int i = 0; i < bd->tamanho_escolares; i++) {
        if (bd->escolares[i].ano_atual == ano_atual && bd->escolares[i].media_atual >= media_min && bd->escolares[i].media_atual <= media_max) {
            n_alunos++;
        }
    }
    return n_alunos;
}

//Pede intervalos de idades e mostra quantos alunos existem por intervalo
void tabela_idade_por_escalao(Uni * bd) {
    short n_intervalos = 0;
    short idade_inf[MAX_INTERVALOS];
    short idade_sup[MAX_INTERVALOS];
    char valido = '1';

    //Número de intervalos
    do {
        printf("Quantos intervalos de idade pretende analisar (1-%d)? ", MAX_INTERVALOS);
        if (scanf("%hd", &n_intervalos) != 1 || n_intervalos < 1 || n_intervalos > MAX_INTERVALOS) {
            printf("Número inválido. Por favor insira um número entre 1 e %d.\n", MAX_INTERVALOS);
            limpar_buffer();
            pressione_enter();
            continue;
        }
        break;
    } while(1);
    limpar_buffer();
    
    for(int i = 0; i < n_intervalos && valido == '1'; i++) {
        do {
            printf("\nIntervalo %d:\n", i + 1);
            printf("Idade inferior: ");
            if (scanf("%hd", &idade_inf[i]) != 1 || idade_inf[i] < IDADE_MINIMA || idade_inf[i] > IDADE_MAXIMA) {
                printf("Idade inválida. Por favor insira um número entre %d e %d\n", IDADE_MINIMA, IDADE_MAXIMA);
                limpar_buffer();
                continue;
            }
            limpar_buffer();

            printf("Idade superior: ");
            if (scanf("%hd", &idade_sup[i]) != 1 || idade_sup[i] <= idade_inf[i] || idade_sup[i] > IDADE_MAXIMA) {
                printf("Idade inválida. Deve ser maior que %hd e menor ou igual a %hd.\n", idade_inf[i], IDADE_MAXIMA);
                limpar_buffer();
                continue;
            }
            limpar_buffer();

            //Verificar se há sobreposição com intervalos anteriores
            for(int j = 0; j < i; j++) {
                if((idade_inf[i] >= idade_inf[j] && idade_inf[i] <= idade_sup[j]) || //A idade inferior introduzida está entre algum intervalo
                   (idade_sup[i] >= idade_inf[j] && idade_sup[i] <= idade_sup[j]) || //A idade superior introduzida está entre algum intervalo
                   (idade_inf[i] <= idade_inf[j] && idade_sup[i] >= idade_sup[j])) { //O intervalo atual contém algum intervalo
                    printf("Este intervalo sobrepõe-se com o intervalo %hd-%hd.\n", idade_inf[j], idade_sup[j]);
                    printf("Quer inserir este intervalo de novo (S) ou sair para o menu (N)? ");
                    if(!sim_nao()) {
                        valido = '0'; //Sair
                        break;
                    }
                    //Redefinir intervalo
                    i--; 
                    continue;
                }
            }
            break;
        } while(valido == '1');
    }

    if(valido == '1') {
        limpar_terminal();
        
        //Cabeçalho
        printf("    Intervalo de idades    |    Número de estudantes   \n");
        
        //Separar o cabeçalho
        printf("---------------------------|---------------------------\n"); //27 caracteres de cada lado
        
        //Linhas para cada intervalo
        for(int i = 0; i < n_intervalos; i++) {
            //Idade terá sempre 2 algarismos
            printf("    %2hd a %2hd anos           |", idade_inf[i], idade_sup[i]);
            int contador = 0;
            //Contar número de alunos dentro do intervalo
            for(int j = 0; j < bd->tamanho_aluno; j++) {
                short idade = calcular_idade(bd->aluno[j].nascimento);
                if(idade >= idade_inf[i] && idade <= idade_sup[i]) {
                    contador++;
                }
            }
            //Printar linha com o número de idades
            printf("            %d", contador);
            printf("\n");
        }
    }
    printf("\n");
    pressione_enter();


}

//Pede intervalos de médias ao utilizador e verifica quantos alunos existem por intervalo em cada ano de curso.
void tabela_medias_ano(Uni * bd) {
    short n_intervalos = 0;
    float media_inferior[MAX_INTERVALOS];
    float media_superior[MAX_INTERVALOS];
    char valido = '1';

    //Encontrar número máximo de anos em escolares
    short max_ano = 0;
    for(int i = 0; i < bd->tamanho_escolares; i++) {
        if(bd->escolares[i].ano_atual > max_ano) {
            max_ano = bd->escolares[i].ano_atual;
        }
    }

    if(max_ano == 0) {
        printf("Não existem dados escolares.\n");
        pressione_enter();
        return;
    }

    limpar_terminal();

    //Número de intervalos
    do {
        printf("Quantos intervalos de média pretende analisar (1-%d)? ", MAX_INTERVALOS);
        if (scanf("%hd", &n_intervalos) != 1 || n_intervalos < 1 || n_intervalos > MAX_INTERVALOS) {
            printf("Número inválido. Por favor insira um número entre 1 e %d.\n", MAX_INTERVALOS);
            limpar_buffer();
            pressione_enter();
            continue;
        }
        break;
    } while(1);
    limpar_buffer();

    //Pedir intervalos
    for(int i = 0; i < n_intervalos && valido == '1'; i++) {
        do {
            printf("\nIntervalo %d:\n", i + 1);
            printf("Média inferior: ");
            if (scanf("%f", &media_inferior[i]) != 1 || media_inferior[i] < 0 || media_inferior[i] >= 20) {
                printf("Média inválida. Por favor insira um número entre 0 e 20.\n");
                limpar_buffer();
                continue;
            }
            limpar_buffer();

            printf("Média superior: ");
            if (scanf("%f", &media_superior[i]) != 1 || media_superior[i] <= media_inferior[i] || media_superior[i] > 20) {
                printf("Média inválida. Deve ser maior que %.1f e menor ou igual a 20.\n", media_inferior[i]);
                limpar_buffer();
                continue;
            }
            limpar_buffer();

            //Verificar se há sobreposição com intervalos anteriores
            for(int j = 0; j < i; j++) {
                if((media_inferior[i] >= media_inferior[j] && media_inferior[i] <= media_superior[j]) || //A média inferior está entre algum intervalo
                   (media_superior[i] >= media_inferior[j] && media_superior[i] <= media_superior[j]) || //A média superior está entre algum intervalo
                   (media_inferior[i] <= media_inferior[j] && media_superior[i] >= media_superior[j])) { //O intervalo atual contém algum intervalo
                    printf("Este intervalo sobrepõe-se com o intervalo %.1f-%.1f.\n", media_inferior[j], media_superior[j]);
                    printf("Quer inserir este intervalo de novo (S) ou sair para o menu (N)? ");
                    if(!sim_nao()) {
                        valido = '0'; //Sair
                        break;
                    }
                    i--;
                    continue;
                }
            }
            break;
        } while(valido == '1');
    }

    if(valido == '1') {
        limpar_terminal();
        
        //Cabeçalho
        printf("Intervalo de média      ");
        for(int i = 1; i <= max_ano; i++) {
            printf("%dº Ano  ", i);
        }
        printf("\n");
        
        //Separar o cabeçalho
        printf("-----------------------");
        for(int i = 0; i < max_ano; i++) {
            printf("--------");
        }
        printf("\n");

        //Estudantes que ou não têm média ou não estão em algum intervalo
        printf("Sem classificação     ");
        for(int ano = 1; ano <= max_ano; ano++) {
            int total = 0;
            for(int i = 0; i < bd->tamanho_escolares; i++) {
                if(bd->escolares[i].ano_atual == ano) {
                    char dentro = '0';
                    for(int j = 0; j < n_intervalos; j++) {
                        if(bd->escolares[i].media_atual >= media_inferior[j] && bd->escolares[i].media_atual <= media_superior[j]) {
                            dentro = '1';
                            break;
                        }
                    }
                    if(dentro == '0') total++;
                }
            }
            printf("%6d  ", total); //Linha dos sem classificação
        }
        printf("\n");

        //Linhas para cada intervalo
        for(int i = 0; i < n_intervalos; i++) {
            printf("%.1f a %.1f V         ", media_inferior[i], media_superior[i]);
            for(int ano = 1; ano <= max_ano; ano++) {
                printf("%6d  ", alunos_por_media_e_ano(bd, media_inferior[i], media_superior[i], ano));
            }
            printf("\n");
        }
    }
    printf("\n");
    pressione_enter();
}

void media_idades_por_nacionalidade(Uni * bd) {
    char ** nacionalidades = NULL;
    short contador = 0;
    short ano = 0;
    float media_idades = 0.0;

    limpar_terminal();

    //Encontrar número máximo de anos em escolares
    short max_ano = 0;
    for(int i = 0; i < bd->tamanho_escolares; i++) {
        if(bd->escolares[i].ano_atual > max_ano) {
            max_ano = bd->escolares[i].ano_atual;
        }
    }
    if (max_ano == 0) {
        printf("Não existem dados escolares.\n");
        pressione_enter();
        return;
    }
    
    nacionalidades = procurar_nacionalidades(bd, 1, "Insira a nacionalidade da qual quer saber a média de idades");
    //Como nacionalidades só tem um parametro, só precisamos de dar free em nacionalidades[0] e nacionalidades.
    if (!nacionalidades) {
        return;
    }
    
    do {
        printf("Quer saber a média de idades de que estudantes de que ano? (1-%hd) ", max_ano);
        if (scanf("%hd", &ano) != 1 || ano < 1 || ano > max_ano) {
            printf("Por favor insira um número entre 1 e %hd.\n", max_ano);
            limpar_buffer();
            continue;
        }
        break;
    } while(1);

    limpar_buffer(); //enter do scanf

    //Loopar pelos estudantes que se encaixem nos critérios
    for (int i = 0; i < bd->tamanho_escolares; i++) {
        if (bd->escolares[i].ano_atual == ano) {
            int indice = procurar_codigo_aluno(bd->escolares[i].codigo, bd);
            if (indice > 0 && strcmp(bd->aluno[indice - 1].nacionalidade, nacionalidades[0]) == 0) {
                media_idades += calcular_idade(bd->aluno[indice - 1].nascimento);
                contador++;
            }
        }
    }
    if (contador == 0) {
        printf("Não foram encontrados estudantes para os critérios especificados.\n");
        free(nacionalidades[0]);
        free(nacionalidades);
        return;
    }

    printf("\nA média de idades para estudantes do %hdº ano e de nacionalidade %s é de %.1f.\n",
        ano, nacionalidades[0], media_idades / contador);

    pressione_enter();

    //Libertar a memória
    free(nacionalidades[0]);
    free(nacionalidades);
}

//Listagens (inclui procuras)

//Lista os parametros no terminal e no ficheiro, se existir.
//Pode-se usar contador a NULL caso seja apenas um estudante.
void listar(Uni * bd, int indice_aluno, FILE * ficheiro, char separador, short * contador) {
    int indice_escolares = 0;
    indice_escolares = procurar_codigo_escolares(bd->aluno[indice_aluno].codigo, bd);
    pausa_listagem(contador);

    printf("Código: %d\n", bd->aluno[indice_aluno].codigo);
    printf("Nome: %s\n", bd->aluno[indice_aluno].nome);
    printf("Data de Nascimento: %02d-%02d-%04d\n", bd->aluno[indice_aluno].nascimento.dia, bd->aluno[indice_aluno].nascimento.mes, bd->aluno[indice_aluno].nascimento.ano);
    printf("Nacionalidade: %s\n", bd->aluno[indice_aluno].nacionalidade);
    if (indice_escolares > 0) {
        indice_escolares--;
        printf("Número de matrículas: %hd\n", bd->escolares[indice_escolares].matriculas);
        printf("ECTS: %hd\n", bd->escolares[indice_escolares].ects);
        printf("Ano atual de curso: %hd\n", bd->escolares[indice_escolares].ano_atual);
        printf("Média atual: %.1f\n", bd->escolares[indice_escolares].media_atual);

    }
    printf("\n");
    indice_escolares++; //Precisamos de uma 'flag' para verificar se escrevemos o escolares nos ficheiros ou não.
    if (ficheiro) {
        //Colocar \n apenas se não for nem a primeira nem a última entrada.
        if (contador && *contador > 0) fprintf(ficheiro, "\n");

        fprintf(ficheiro, "%d%c", bd->aluno[indice_aluno].codigo, separador);
        fprintf(ficheiro, "%s%c", bd->aluno[indice_aluno].nome, separador);
        fprintf(ficheiro, "%02d-%02d-%04d%c", bd->aluno[indice_aluno].nascimento.dia, bd->aluno[indice_aluno].nascimento.mes, bd->aluno[indice_aluno].nascimento.ano, separador);
        fprintf(ficheiro, "%s%c", bd->aluno[indice_aluno].nacionalidade, separador);
        if (indice_escolares > 0) {
            indice_escolares--;
            fprintf(ficheiro, "%hd%c", bd->escolares[indice_escolares].matriculas, separador);
            fprintf(ficheiro, "%hd%c", bd->escolares[indice_escolares].ects, separador);
            fprintf(ficheiro, "%hd%c", bd->escolares[indice_escolares].ano_atual, separador);
            fprintf(ficheiro, "%.1f", bd->escolares[indice_escolares].media_atual);
        }
    }
}

//Lista toda a informação conhecida sobre um aluno se este existir.
//Usa o código do aluno.
//Não lista em ficheiros.
void listar_info_estudante(Uni * bd) {
    int codigo;
    int indice_aluno;
    do{
        limpar_terminal();
        codigo = 0;
        indice_aluno = 0;
        pedir_codigo(&codigo);
        if (codigo == 0) return;

        indice_aluno = procurar_codigo_aluno(codigo, bd);
        if (indice_aluno > 0) {
            indice_aluno--;
            printf("\n");
            listar(bd, indice_aluno, NULL, '\0', NULL); 
        }
        else printf("Não foi encontrado nenhum estudante com o código %d.\n", codigo);

        printf("Quer repetir a procura? (S/N) ");
        if(!sim_nao()) return;
    }while(1);
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
            if (!parte_nome || strlen(parte_nome) < 2) {
                printf("A entrada é inválida. Por favor, insira parte do nome do aluno, com um mínimo de 2 caracteres.\n");
                pressione_enter();
                if (parte_nome) free(parte_nome);
                continue;
            }
            break;
        } while(1);
        //Faz-se isto para evitar mudar o nome da variável.
        //Colocar a string introduzida em minúsculas para evitar erros de procura.
        char * temp = strdup(parte_nome);
        parte_nome = normalizar_string(parte_nome);

        printf("Resultados da pesquisa para \"%s\": \n\n", temp);
        for(int i = 0; i < bd->tamanho_aluno; i++) {
            //Colocar todos os dados a letras minúsculas.
            char * nome = normalizar_string(bd->aluno[i].nome);
            if (nome) {
                //strstr(s1, s2) é uma função que retorna a primeira ocorrência de s2 em s1
                //https://www.geeksforgeeks.org/strstr-in-ccpp/
                if (strstr(nome, parte_nome) != NULL) {
                    listar(bd, i, listagem, separador, &contador);
                }
                free(nome);
            }
        }
        if (contador == 0) 
            printf("Não foi encontrado nenhum estudante com parte do nome \"%s\".\n", temp);
        free(temp); //Libertamos o ponteiro original para não haver memory leaks.

        pressione_enter();
        printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");

        free(parte_nome);
        if (listagem) fclose(listagem);

        printf("\nQuer repetir a procura? (S/N): ");
        if (!sim_nao()) return;
    } while(1);
}

//Lista os estudantes por ordem alfabética do apelido.
void listar_apelidos_alfabeticamente(Uni * bd) {
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

    //Ordenar alfabeticamente por bubble sort O(n2) mas n é pequeno e também nao usamos a lisatgem muitas vezes.
    for(int i = 0; i < bd->tamanho_aluno - 1; i++) {
        for(int j = 0; j < bd->tamanho_aluno - i - 1; j++) {
            //Obter último nome de cada estudante com strrchr.
            //strchr verifica a última ocorrência de char em str e retorna um ponteiro para essa ocorrência.
            char * ultimo_nome1 = strrchr(copia[j].nome, ' ');
            char * ultimo_nome2 = strrchr(copia[j+1].nome, ' ');
            
            //Caso apenas haja um nome.
            if (!ultimo_nome1) ultimo_nome1 = copia[j].nome;
            else ultimo_nome1++; //ultimo_nome1 aponta agora para o espaço e portanto temos de o avançar.
            
            if (!ultimo_nome2) ultimo_nome2 = copia[j+1].nome;
            else ultimo_nome2++;

            char * nome1_normalizado = normalizar_string(ultimo_nome1);
            char * nome2_normalizado = normalizar_string(ultimo_nome2);
            if (!nome1_normalizado || !nome2_normalizado) {
                //Evitar dar free em NULL
                if (ultimo_nome1) free(ultimo_nome1);
                if (ultimo_nome2) free(ultimo_nome2);
                free(ultimo_nome1);
                free(ultimo_nome2);
                continue;
            }

            //Comparar por ordem alfabética.
            if(strcmp(normalizar_string(ultimo_nome1), normalizar_string(ultimo_nome2)) > 0) {
                Estudante temp = copia[j];
                copia[j] = copia[j+1];
                copia[j+1] = temp;
            }

            free(nome1_normalizado);
            free(nome2_normalizado);
            //free(ultimo_nome1);
            //free(ultimo_nome2);
        }
    }

    //Preparar listagem
    listagem = pedir_listagem(formato);
    separador = obter_separador(listagem, formato);

    printf("Estudantes por ordem alfabética do apelido:\n\n");
    for(int i = 0; i < bd->tamanho_aluno; i++) {
        //Temos de ajustar porque listar apenas funciona com bd e indice.
        int temp_indice = procurar_codigo_aluno(copia[i].codigo, bd);
        if(temp_indice > 0) {
            temp_indice--;
            listar(bd, temp_indice, listagem, separador, &contador);
        }
    }

    //Libertar memória de copia
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
    const char * dias_da_semana[] = {"sábado", "domingo", "segunda-feira", "terça-feira", "quarta-feira", "quinta-feira", "sexta-feira"};
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
                listar(bd, i, listagem, separador, &contador);
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
            if (scanf("%hd", &ano) != 1 || ano <= 0 || ano > DATA_ATUAL.ano + ANOS_AVANCO_PROCURAS) {
                printf("Por favor insira um ano entre 1 e %hd.\n", DATA_ATUAL.ano + ANOS_AVANCO_PROCURAS);
                limpar_buffer();
                continue;
            }
        break;
        } while (1);

        printf("Estudantes cujo aniversário é ao domingo em %hd:\n\n", ano);
        for(int i = 0; i < bd->tamanho_aluno; i++) {
            dia = bd->aluno[i].nascimento.dia;
            mes = bd->aluno[i].nascimento.mes;
            //O aluno não pode fazer anos se ainda não era nascido!
            if (ano < bd->aluno[i].nascimento.ano) continue;

            if (calcular_dia_da_semana(dia, mes, ano) == 1) { 
                listar(bd, i, listagem, separador, &contador);
            }
        }
        if (contador == 0) {
            printf("Não foi encontrado nenhum estudante cujo aniversário fosse ao domingo em %hd.\n", ano);
        }
        printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
        pressione_enter();
        if (listagem) fclose(listagem);

        printf("\nQuer inserir um ano diferente? (S/N): ");
        if(!sim_nao()) return;
    } while(1);
}

void listar_aniversario_na_quaresma(Uni * bd) {
    Data inicio, fim;
    short contador = 0;
    short ano = 0;
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
            if (scanf("%hd", &ano) != 1 || ano <= 0 || ano > DATA_ATUAL.ano + ANOS_AVANCO_PROCURAS) {
                printf("Por favor insira um ano entre 1 e %hd.\n", DATA_ATUAL.ano + ANOS_AVANCO_PROCURAS);
                limpar_buffer();
                continue;
            }
        break;
        } while (1);
        limpar_buffer();
        limpar_terminal();

        //Calcular a data da quaresma no ano pedido.
        calcular_quaresma(ano, &inicio, &fim);

        printf("Estudantes cujo aniversário é na Quaresma em %hd:\n\n", ano);
        for(int i = 0; i < bd->tamanho_aluno; i++) {
            //Comparar_data não funcionava originalmente porque comparava tudo, logo foi acrescentada uma opção de exluir o ano e comparar apenas mes e dia.
            if (comparar_data(inicio, bd->aluno[i].nascimento, '1') == -1 && comparar_data(fim, bd->aluno[i].nascimento, '1') == 1) { 
                listar(bd, i, listagem, separador, &contador);
            }
        }
        if (contador == 0) {
            printf("Não foi encontrado nenhum estudante cujo aniversário fosse na Quaresma em %hd.\n", ano);
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
                listar(bd, indice, listagem, separador, &contador);
            }
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
                listar(bd, indice, listagem, separador, &contador);
            }
        }
    }
    if (contador == 0) {
        printf("Não foram encontrados alunos finalistas.\n");
    }
    printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
    pressione_enter();

}

void listar_estudantes_por_data_e_nacionalidades(Uni *bd) {
    short n_nacionalidades = 0;
    char ** nacionalidades = NULL;
    short contador = 0;
    char formato[MAX_FORMATO];
    FILE * listagem = NULL;
    char separador;
    //Inicializar datas.
    Data inf = {0, 0, 0};  
    Data sup = {0, 0, 0};
    

    limpar_terminal();

    listagem = pedir_listagem(formato);
    separador = obter_separador(listagem, formato);

    //Pedir nº nacionalidades
    do {
        printf("Quantas nacionalidades deseja incluir na procura (1-%d)? ", MAX_NACIONALIDADES_PEDIDA);
        if (scanf("%hd", &n_nacionalidades) != 1 || n_nacionalidades <= 0 || n_nacionalidades > 5) {
            printf("Por favor insira um número entre 1 e %d.\n", MAX_NACIONALIDADES_PEDIDA);
            limpar_buffer();
            continue;
        }
        break;
    } while(1);
    
    limpar_buffer();
    limpar_terminal();

    nacionalidades = procurar_nacionalidades(bd, n_nacionalidades, NULL);
    if (!nacionalidades) return;
    do {
        //Data inferior.
        do {
            char * data = NULL;
            printf("Insira a data inferior (DD-MM-AAAA): ");
            data = ler_linha_txt(stdin, NULL);
            if (!data) continue;

            ler_data(&inf, data, '1');
            free(data); //Libertamos porque temos a certeza que != NULL.
        } while(inf.dia == 0);
        //Data superior.
        do {
            char * data = NULL;
            printf("Insira a data superior (DD-MM-AAAA): ");
            data = ler_linha_txt(stdin, NULL);
            if (!data) continue;

            ler_data(&sup, data, '1');
            free(data);
        } while(sup.dia == 0);
    } while(comparar_data(sup, inf, '0') == -1); //Apenas saímos se sup >= inf.

    printf("\nAlunos nascidos entre %02hd-%02hd-%04hd e %02hd-%02hd-%04hd e pertencentes às nacionalidades escolhidas: \n\n", 
        inf.dia, inf.mes, inf.ano, sup.dia, sup.mes, sup.ano);

    for(int i = 0; i < bd->tamanho_escolares; i++) {
        for(int j = 0; j < n_nacionalidades; j++) {
            if ((strcmp(bd->aluno[i].nacionalidade, nacionalidades[j]) == 0)&&(validar_data_entre_intervalo(inf, sup, bd->aluno[i].nascimento))) {
                listar(bd, i, listagem, separador, &contador);
            }
        }
    }

    if (contador == 0) {
        printf("Não foram encontrados estudantes para os critérios especificados.\n");
    }

    printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
    pressione_enter();
    if (listagem) fclose(listagem);

    //Libertar as nacionalidades
    for (int i = 0; i < n_nacionalidades; i++)
        if (nacionalidades[i] != NULL) 
            free(nacionalidades[i]);
    //Libertar o array.
    free(nacionalidades);
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

//Limpa o que ficou no buffer do teclado, como os enters
//Usar apenas se houver certezas de haver algo no buffer
void limpar_buffer() {
    int lixo;
    while ((lixo = getchar()) != '\n' && lixo != EOF);
}

//Verifica se ficou algo no buffer para alem do enter
//Retorna 1 caso esteja apenas o enter
//Retorna 0 caso estejam outros caracteres ainda no buffer
//Limpa sempre o buffer
//Nunca se deve chamar se não houver certezas de que há algo no buffer
int verificar_e_limpar_buffer() {
    //Verificar o caracter que está no buffer
    char lixo = getchar();
    //Se não for o \n, ou seja, a entrada foi inválida
    if (lixo != '\n') {
        limpar_buffer(); //limpamos o buffer (pois ainda há o enter, pelo menos)
        return 0;
    }
    //Se estamos aqui, então o caracter é o \n, e já foi lido, logo não precisamos de fazer nada
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

//Pede enter a cada PAUSA_LISTAGEM iterações
//Incrementa o contador por 1
void pausa_listagem(short * contador) {
    if (!contador) return;

    if (*contador > 0 && *contador % PAUSA_LISTAGEM == 0) {
        pressione_enter();
    }
    (*contador)++;
}

//Dá prompt ao user para dar enter
//Não avança até o user introduzir um enter
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
        fprintf(erros, "==>ERROS AO LER O FICHEIRO %s\n\n", nome_ficheiro);
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

//Calcula a data do domingo de páscoa de um dado ano.
Data calcular_domingo_pascoa(int ano) {
    //int porque os números podem ficar muito grandes e estourar com short
    //https://www.matematica.pt/en/faq/calculate-easter-day.php
    Data pascoa;

    int a = ano % 19;
    int b = ano / 100;
    int c = ano % 100;
    int d = b / 4;
    int e = b % 4;
    int f = (b + 8) / 25;
    int g = (b - f + 1) / 3;
    int h = (19 * a + b - d - g + 15) % 30;
    int i = c / 4;
    int k = c % 4;
    int l = (32 + 2 * e + 2 * i - h - k) % 7;
    int m = (a + 11 * h + 22 * l) / 451;
    int mes = (h + l - 7 * m + 114) / 31;
    int dia = ((h + l - 7 * m + 114) % 31) + 1;

    pascoa.dia = (short) dia;
    pascoa.mes = (short) mes;
    pascoa.ano = (short) ano;
    return pascoa;
}

//Quarta feira de cinzas é o dia da Páscoa - 46 dias (Domingos não contam)
Data calcular_quarta_feira_cinzas(Data pascoa) {
    Data cinzas = pascoa;
    cinzas.dia -= DIAS_QUARESMA;
    
    //Como o dia ficará negativo, temos de fazer as respetivas alterações
    while (cinzas.dia <= 0) {
        cinzas.mes--;
        if (cinzas.mes <= 0) {
            cinzas.mes = 12;
            cinzas.ano--;
        }
        
        //Determinar os dias do mês anterior
        switch (cinzas.mes) {
            //Fevereiro 
            case 2:
                //Caso o ano seja bissexto
                if ((cinzas.ano % 4 == 0 && cinzas.ano % 100 != 0) || (cinzas.ano % 400 == 0))
                    cinzas.dia += 29;
                else
                    cinzas.dia += 28;
                break;
            //Meses com 30 dias
            case 4: //Abril
            case 6: //Junho
            case 9: //Setembro
            case 11: //Novembro
                cinzas.dia += 30;
                break;
            //Meses com 31 dias
            default:
                cinzas.dia += 31;
                break;
        }
    }
    
    return cinzas;
}

//Calcula a data de início e fim da quaresma
void calcular_quaresma(int ano, Data * inicio, Data * fim) {
    if (!inicio || !fim) return;

    *fim = calcular_domingo_pascoa(ano);
    *inicio = calcular_quarta_feira_cinzas(*fim);

    //Quaresma acaba na quinta feira santa, logo temos de subtrair 3(domingo, sábado, sexta -> quinta)
    fim->dia -=3;

    if (fim->dia <= 0) {
        fim->mes--;
        //Fevereiro
        if (fim->mes == 2) {
            fim->dia += ((fim->ano % 4 == 0 && fim->ano % 100 != 0) || (fim->ano % 400 == 0)) ? 29 : 28;
        } 
        //Meses com 30 dias
        else if (fim->mes == 4 || fim->mes == 6 || fim->mes == 9 || fim->mes == 11) {
            fim->dia += 30;
        } 
        //31 dias
        else {
            fim->dia += 31;
        }
    }
}

//Sim - 1; Não - 0;
int sim_nao() {
    char opcao;
    do {
        scanf(" %c", &opcao);
        if (!verificar_e_limpar_buffer()) {
            printf("Entrada inválida! Por favor tente novamente. Utilize 'S' ou 'N': ");
            continue;
        }
        if (opcao == 's' || opcao == 'S') {
            return 1;
        }
        else if (opcao == 'n' || opcao == 'N') {
            return 0;
        }
        printf("\nOpção inválida. Use S ou N: ");
    } while (1);
}

//Limpa o terminal
//Limpa o buffer
//Não faz qualquer validação sobre o código
//Coloca o código a 0 se o user quiser sair
void pedir_codigo(int * codigo) {
    *codigo = 0;
    do {
        printf("Insira o código do estudante (ou '0' para sair): ");
        if (scanf("%d", codigo) != 1) { //Verifica entradas inválidas como letras
            printf("Código inválido! Insira um número inteiro positivo.\n");
            limpar_buffer();
            pressione_enter();
            continue; 
        }
        else if (!verificar_e_limpar_buffer()) { //caso haja um int + chars no input
            printf("Entrada inválida! Por favor, escreva apenas o código do aluno.\n");
            pressione_enter();
            continue;
        }
        break;
    } while(1);
}

//Retorna o separador entre os parametros a escrever no ficheiro.
//Caso o ficheiro == NULL, retorna '0';
//Requer ajuste caso sejam adicionados mais tipo de ficheiros.
//Escreve o cabeçalho do ficheiro.
char obter_separador(FILE * ficheiro, char * formato) {
    char separador = '\0'; 
    if(ficheiro) {
        if (strcmp(formato, ".txt") == 0) separador = '\t';
        else if (strcmp(formato, ".csv") == 0) separador = ',';
        //Cabeçalho do ficheiro de listagem.
        fprintf(ficheiro, "Código%cNome%cData de Nascimento%cNacionalidade%cMatrículas%cECTS%cAno de curso%cMédia\n", separador, separador, separador, separador,
            separador, separador, separador); 
    }
    return separador;
}

//Calcula a idade atual dada uma determinada data de nascimento.
short calcular_idade(Data nascimento) {
    short idade = DATA_ATUAL.ano - nascimento.ano;
    //Ainda não passou o mês logo a idade está +1.
    if (DATA_ATUAL.mes < nascimento.mes) return (idade - 1);
    //Já passou o mês logo já fizeram anos. 
    else if (DATA_ATUAL.mes > nascimento.mes) return idade;
    //Meses iguais:
    if (DATA_ATUAL.dia >= nascimento.dia) return idade; //Ou é o aniverário ou já passou.
    return (idade - 1);
}

//UTILIZA UMA CÓPIA!!
//Coloca str a minúsculas.
//Retira os acentos das strings ou ç.
//Retorna NULL em caso de erro.
char * normalizar_string(char * str) {
    //Retirar os acentos das frases para evitar erros.
    //Apenas minúsculas porque usamos o strlwr.
    char acentuados[] = {
        0xE0, 0xE1, 0xE2, 0xE3,  // à á â ã
        0xE7,                    // ç
        0xE8, 0xE9, 0xEA,        // è é ê
        0xEC, 0xED, 0xEE,        // ì í î
        0xF2, 0xF3, 0xF4, 0xF5,  // ò ó ô õ
        0xF9, 0xFA, 0xFB,        // ù ú û
        0                        //Nul char
    };
    char sem_acentos[] = "aaaaceeeiiioooouuu";
    
    //Cópia da string para não alterar o original.
    char * resultado = strdup(str);
    if (!resultado) return NULL;
    //Colocar tudo a minúsculas.
    strlwr(resultado);

    for (int i = 0; resultado[i]; i++) {
        char * acento = strchr(acentuados, resultado[i]);
        if (acento) {
            int pos = acento - acentuados; //- acentuados porque estamos a usar ponteiros.
            resultado[i] = sem_acentos[pos];
        }
    }
    
    return resultado;
}

//Atualiza a data atual.
void data_atual() {
    //https://www.geeksforgeeks.org/time-h-header-file-in-c-with-examples/
    time_t t = time(NULL);
    struct tm * tm_atual = localtime(&t);
    
    DATA_ATUAL.dia = tm_atual->tm_mday;
    DATA_ATUAL.mes = tm_atual->tm_mon + 1; //tm_mon vai de 0-11
    DATA_ATUAL.ano = tm_atual->tm_year + 1900;
}



