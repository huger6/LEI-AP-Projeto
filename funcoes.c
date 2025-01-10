#include "headers.h"

//Variáveis globais

Data DATA_ATUAL = {0,0,0}; //Vai ser alterada logo no ínicio do programa
char autosaveON = '0'; //desativado por padrão

//Ficheiros e gestão de dados

/* Lê uma linha completa de um ficheiro, qualquer que seja o tamanho
 *
 * @param ficheiro    Ponteiro para o ficheiro a ler (pode ser stdin)
 * @param n_linhas    Ponteiro para contador de linhas (pode ser NULL); 
 *                    incrementado para cada linha lida
 *
 * @return Uma string alocada dinamicamente ou NULL se:
 *         - O ponteiro do ficheiro é NULL
 *         - A alocação de memória falhar
 *         - O ficheiro estiver vazio
 *         
 * @note A string retornada tem de ser libertada 
 * @note O caractere de nova linha é removido da string retornada
 * @note Se a alocação de memória falhar mas alguns dados foram lidos, retorna a linha parcial
 * @note n_linhas não é inicializado
 * @note ficheiro deve estar aberto; não é fechado
 */
char * ler_linha_txt(FILE * ficheiro, int * n_linhas) {
    if(!ficheiro) return NULL;
    char buffer[TAMANHO_INICIAL_BUFFER]; //Buffer para armazenar parte da linha
    size_t tamanho_total = 0; //Comprimento da linha; size_t pois é sempre >0 e evita conversões que podem levar a erros com outras funções
    char * linha = NULL;

    while (fgets(buffer, sizeof(buffer), ficheiro)) { //fgets le ate buffer -1 caracteres ou '\n' ou EOF
        size_t tamanho = strlen(buffer); //Calcula o tamanho do texto lido
        char * temp = realloc(linha, tamanho_total + tamanho + 1); //+1 para o nul char

        //Verificar erros na realocação/alocação
        if (!temp) {
            //Evitar ao máximo retornar NULL, pois isso terminaria o loop em carregar_dados (ou a ler qualquer ficheiro)
            if (linha) {
                linha[tamanho_total] = '\0'; //Linha incompleta mas pelo menos retorna
                if (n_linhas != NULL) (*n_linhas)++;
                return linha;
            }
            return NULL; //Caso haja um erro fatal, temos que interromper
        }
        linha = temp; //atualizar o ponteiro linha para apontar para a nova memória

        //Copiar o conteúdo lido para a linha total
        //linha + tamanho_total é um ponteiro para a posição da memória seguinte para onde a próxima parte de buffer será copiada
        memmove(linha + tamanho_total, buffer, tamanho); 
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

    //Linha sem '\n' mas tem conteúdo (por ex: última linha)
    if (linha && tamanho_total > 0) {
        if (n_linhas != NULL) (*n_linhas)++;
        return linha;
    }

    //Se chegarmos aqui é porque aconteceu algum erro ou o ficheiro está vazio
    free(linha);
    return NULL;
}

/* Carrega os dados dos alunos e escolares para a estrutura da universidade
 *
 * @param nome_ficheiro_dados     Nome do ficheiro com os dados dos alunos
 * @param nome_ficheiro_escolar   Nome do ficheiro com os dados escolares
 * @param bd                      Ponteiro para a estrutura da universidade
 *
 * @return 1 se os dados foram carregados com sucesso, 0 se ocorrer:
 *         - Erro ao abrir os ficheiros
 *         - Erro ao alocar memória
 *         - Erro na validação dos dados
 *         
 * @note Todos os erros são registados no ficheiro ERROS_TXT
 * @note Os dados inválidos são, assim, ignorados
 * @note A estrutura é redimensionada automaticamente se necessário
 * @note É possível que bd->capacidade_escolares fique o dobro de bd->capacidade_aluno, 
 *      isso deve-se à existência de erros no ficheiro de dados mas não do de escolares.
 *      Quando assim é, a memória não é reduzida.
 */
int carregar_dados_txt(const char * nome_ficheiro_dados,const char * nome_ficheiro_escolar, Uni * bd) { 
    FILE * dados = fopen(nome_ficheiro_dados, "r");
    FILE * situacao_escolar = fopen(nome_ficheiro_escolar, "r");
    //Abrir ficheiro de erros e escrever o cabeçalho;
    FILE * erros = fopen(ERROS_TXT, "w"); //Vai anexar ao ficheiro de erros os erros encontrados
    if (!erros) {
        return 0; //Se prosseguissemos iria resultar em segmentation fault (pode ser contornado verificando se erros != NULL)
    } 

    fprintf(erros, "------------------------------------------NOVA ITERAÇÃO DO PROGRAMA------------------------------------------\n\n\n");
    
    int n_linhas = 0;
    char * linha = NULL; //Ponteiro para armazenar uma linha lida do ficheiro
    char primeiro_erro = '1'; //'1' significa que ainda não houve erro. Usar para ERROS_TXT
    char erro_geral = '0'; //Flag para dizer ao user que houve erro (apenas usada no final)

    //Ler dados para a struct estudante
    if(dados) { 
        int codigo_temp; //Necessário para passar para string_para_int
        while ((linha = ler_linha_txt(dados, &n_linhas)) != NULL) {
            char erro = '0'; //diferente de primeiro_erro pois este vai marcar se existe um erro em cada iteração
            char * parametros[PARAMETROS_ESTUDANTE] = {NULL}; //Array com PARAMETROS casas, onde cada pode armazenar um ponteiro para char (ou seja, uma string)
            int num_parametros = 0; //Armazena o número real de parâmetros
            separar_parametros(linha, parametros, &num_parametros); //Extrai os dados já formatados corretamente para parametros
            
            //Verificar se temos os parâmetros necessários
            if(num_parametros == PARAMETROS_ESTUDANTE) {
                //Realocar memória caso necessário
                //Nota: +1 é FUNDAMENTAL, caso contrário a inicialização não é feita corretamente nos índices da borda
                if (bd->tamanho_aluno + 1 >= bd->capacidade_aluno) {
                    if (!realocar_aluno(bd, '0')) {
                        printf("Ocorreu um erro a gerir a memória.\n");
                        for(int i = 0; i < num_parametros; i++) //Libertar a memória para os parâmetros alocados dinamicamente até ao momento
                            free(parametros[i]);
                        free(linha);
                        fclose(dados);
                        fclose(erros);
                        return 0;
                    }
                    //A realocação do campos de nome é feita em validar_nome
                }
                //Código
                if (!string_para_int(parametros[0], &codigo_temp)) { //Código só é validado no final de carregar e ordenar tudo
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_dados, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O código é inválido!\n\n");
                }
                else if(codigo_temp <= 0) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_dados, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O código é inválido! Deve ser maior que 0.\n\n");
                }
                //A seguir não são usados else if's para o caso de haver mais de um erro na mesma linha(assim identifica-os todos)
                //Nome
                if (!validar_nome(&(bd->aluno[bd->tamanho_aluno]), parametros[1], '0')) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_dados, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O nome é inválido!\n\n");
                }
                //Data de nascimento
                ler_data(&(bd->aluno[bd->tamanho_aluno].nascimento), parametros[2], '0');
                if (bd->aluno[bd->tamanho_aluno].nascimento.dia == 0) { //apenas se verifica dia pois em ler_data os dados, se forem colocados na struct, são colocados todos juntos
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_dados, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: A data de nascimento é inválida!\n\n");
                }
                //Nacionalidade
                if (!validar_nacionalidade(parametros[3], '0')) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_dados, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: A nacionalidade é inválida!\n\n");
                }
                //Copiar os parâmetros caso não haja erros
                if (erro == '0') {
                    bd->aluno[bd->tamanho_aluno].codigo = codigo_temp; 
                    strcpy(bd->aluno[bd->tamanho_aluno].nome, parametros[1]);
                    //Data de nascimento já é copiada se for válida.
                    strcpy(bd->aluno[bd->tamanho_aluno].nacionalidade, parametros[3]);
                    bd->tamanho_aluno += 1; //Incrementar o tamanho do array
                }
                //A data pode ter sido copiada e depois ter havido erro noutro parâmetro, nesse caso, inicializamos de novo
                else {
                    bd->aluno[bd->tamanho_aluno].nascimento.dia = 0;
                    bd->aluno[bd->tamanho_aluno].nascimento.mes = 0;
                    bd->aluno[bd->tamanho_aluno].nascimento.ano = 0;
                }
            }
            //param a menos
            else if (num_parametros < PARAMETROS_ESTUDANTE) {
                listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_dados, &erro, n_linhas, linha);
                fprintf(erros, "Razão: A linha tem parâmetros insuficientes. Verifique se há parâmetros não separados por \\t (obrigatório)\n\n");
            }
            //param a mais
            else {
                listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_dados, &erro, n_linhas, linha);
                fprintf(erros, "Razão: A linha tem parâmetros a mais. Verifique se algum parâmetro contém um \\t (não pode).\n\n");
            }
            
            //Libertar a memória alocada para parametros
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
        return 0; //Não pode haver dados escolares sem ficha pessoal, logo mesmo que o outro ficheiro abrisse, era inutil
    }
    
    if (erro_geral == '0' && primeiro_erro == '0') erro_geral = '1'; //flag para especificar que houve erro em geral
    //Necessário porque primeiro_erro muda consoante a operação a fazer, logo não é viável
    n_linhas = 0;
    primeiro_erro = '1';

    //Ler dados para a struct de escolares 
    //Segue-se a mesma lógica do carregamento para estudante mas com os param diferentes
    if(situacao_escolar) {
        int codigo_temp;
        short matriculas_temp;
        short ects_temp;
        short ano_atual_temp;
        float media_temp;
        while ((linha = ler_linha_txt(situacao_escolar, &n_linhas)) != NULL) {
            char erro = '0'; 
            char * parametros[PARAMETROS_DADOS_ESCOLARES] = {NULL}; 
            int num_parametros = 0; 
            separar_parametros(linha, parametros, &num_parametros);
            
            if(num_parametros == PARAMETROS_DADOS_ESCOLARES) {
                if (bd->tamanho_escolares + 1 >= bd->capacidade_escolares) {
                    if (!realocar_escolares(bd, '0')) {
                        printf("Ocorreu um erro a gerir a memória.\n");
                        for(int i = 0; i < num_parametros; i++) 
                            free(parametros[i]);
                        free(linha);
                        fclose(situacao_escolar);
                        fclose(erros);
                        return 0;
                    }
                }
                //Código
                if (!string_para_int(parametros[0], &codigo_temp)) { 
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O código é inválido!\n\n");
                }
                else if(codigo_temp <= 0) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O código é inválido!Deve ser maior que 0.\n\n");
                }
                //Matriculas
                if (!string_para_short(parametros[1], &matriculas_temp)) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O número de matrículas é inválido!\n\n");
                }
                else if (matriculas_temp < 0 || matriculas_temp > MAX_MATRICULAS) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O número de matrículas é inválido. Deve estar entre 0 e %d.\n\n", MAX_MATRICULAS);
                }
                //ECTS
                if (!string_para_short(parametros[2], &ects_temp)) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O número de créditos é inválido!\n\n");
                }
                else if (ects_temp < 0 || ects_temp > MAX_ECTS) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O número de créditos é inválido! Deve estar entre 0 e %d.\n\n", MAX_ECTS);
                }
                //Ano atual
                if (!string_para_short(parametros[3], &ano_atual_temp)) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O ano atual é inválido!\n\n");
                }
                else if (ano_atual_temp < 1 || ano_atual_temp > MAX_ANO_ATUAL) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: O ano atual é inválido! Deve estar entre 1 e %d.\n\n", MAX_ANO_ATUAL);
                }
                //Media atual do aluno
                if (!string_para_float(parametros[4], &media_temp)) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: A média é inválida!\n\n");
                }
                else if(media_temp < 0 || media_temp > 20) {
                    listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                    fprintf(erros, "Razão: A média é inválida! Deve estar entre 0 e 20.\n\n");
                }

                if (erro == '0') { 
                    bd->escolares[bd->tamanho_escolares].codigo = codigo_temp;
                    bd->escolares[bd->tamanho_escolares].matriculas = matriculas_temp;
                    bd->escolares[bd->tamanho_escolares].ects = ects_temp;
                    bd->escolares[bd->tamanho_escolares].ano_atual = ano_atual_temp;
                    bd->escolares[bd->tamanho_escolares].media_atual = media_temp;
                    bd->tamanho_escolares += 1; //Aumentar o tamanho do array
                }
            }
            else if (num_parametros < PARAMETROS_DADOS_ESCOLARES) {
                listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                fprintf(erros, "Razão: A linha tem parâmetros insuficientes.\n\n"); 
            }
            else {
                listar_erro_ao_carregar(erros, &primeiro_erro, nome_ficheiro_escolar, &erro, n_linhas, linha);
                fprintf(erros, "Razão: A linha tem parâmetros a mais.\n\n");
            }
            
            //Libertamos a memória alocada para os parametros
            for(int i = 0; i < num_parametros; i++) 
                free(parametros[i]);

            free(linha); 
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
    // Agora que está tudo ordenado vamos procurar por possíveis erros como:
    //Códigos duplicados (o merge sort mantém a ordem pela qual foram lidos pelo que mantemos o primeiro que foi lido)
    //Códigos que estejam em escolares mas não estejam em aluno
    verificar_codigos_duplicados(bd, erros); //verifica duplicados em ambos os arrays
    verificar_codigos_escolares_sem_aluno(bd, erros, &primeiro_erro);
    fprintf(erros, "------------------------------------------FIM DE ITERAÇÃO------------------------------------------\n\n\n");
    fclose(erros);

    //Verificar se houve erro
    if (erro_geral == '0' && primeiro_erro == '0') erro_geral = '1';

    //Informar o user
    if (erro_geral == '1') { 
        printf("Informação sobre os dados carregados:\n");
        printf("Foram detetados alguns erros e alguns dados foram descartados.\n");
        printf("Pode consultar o que foi descartado e porquê no ficheiro '%s'.\n\n", ERROS_TXT);
        pressione_enter();
    }
    else {
        //Eliminar o ficheiro de erros já que não houve erros a carregar os dados
        eliminar_ficheiro(ERROS_TXT, '0');
    }
    return 1;
}

/* Carrega dados binários de um ficheiro para a estrutura da universidade
 *
 * @param nome_ficheiro    Nome do ficheiro binário a carregar
 * @param bd               Ponteiro para a estrutura da universidade
 *
 * @return 1 se sucesso, 0 se se verificar:
 *         - Erro ao abrir o ficheiro
 *         - Erro ao ler dados
 *         - Checksum inválido
 *         
 * @note Valida a integridade dos dados através de um checksum
 * @note Aloca memória dinamicamente para as estruturas
 * @note Inicializa a memória alocada para todas as structs
 */
int carregar_dados_bin(const char * nome_ficheiro, Uni * bd) {
    //Abrir ficheiro em modo leitura binária
    FILE * ficheiro = fopen(nome_ficheiro, "rb");
    if (!ficheiro) {
        printf("Ocorreu um erro ao abrir o ficheiro de dados '%s'.\n", nome_ficheiro);
        printf("Por favor verifique se o ficheiro '%s' está no mesmo diretório do programa.\n", nome_ficheiro);
        pressione_enter();
        return 0;
    }
    //Ler checksum do ficheiro 
    unsigned long checksum_guardado;
    if (!ler_dados_binarios(&checksum_guardado, sizeof(unsigned long), 1, ficheiro)) {
        fclose(ficheiro);
        printf_fich_bin_alterado();
        return 0;
    }

    //Configs
    if (!ler_dados_binarios(&autosaveON, sizeof(char), 1, ficheiro)) {
        fclose(ficheiro);
        printf_fich_bin_alterado();
        return 0;
    }
    
    //Dados dos arrays
    if (!ler_dados_binarios(&bd->tamanho_aluno, sizeof(int), 1, ficheiro) ||
        !ler_dados_binarios(&bd->capacidade_aluno, sizeof(int), 1, ficheiro) ||
        !ler_dados_binarios(&bd->tamanho_escolares, sizeof(int), 1, ficheiro) ||
        !ler_dados_binarios(&bd->capacidade_escolares, sizeof(int), 1, ficheiro)) {
            fclose(ficheiro);
            printf_fich_bin_alterado();
            return 0;
    }
    //Capacidade sempre superior a tamanho e tamanho aluno sempre >= ao de escolares
    if (bd->tamanho_aluno > bd->capacidade_aluno || bd->tamanho_escolares > bd->capacidade_escolares || 
        bd->tamanho_aluno < bd->tamanho_escolares || bd->tamanho_aluno < 0 || bd->tamanho_escolares < 0 ||
        bd->capacidade_aluno < 0 || bd->capacidade_escolares < 0) {
            fclose(ficheiro);
            printf_fich_bin_alterado();
            return 0;
    }

    //É necessário alocar a memória para os arrays
    bd->aluno = (Estudante *) malloc(bd->capacidade_aluno * sizeof(Estudante));
    bd->escolares = (Dados *) malloc(bd->capacidade_escolares * sizeof(Dados));

    //Erros de alocação
    if (!bd->aluno || !bd->escolares) {
        printf("Ocorreu um erro ao alocar memória para os alunos.\n");
        pressione_enter();
        if (bd->aluno) free(bd->aluno);
        if (bd->escolares) free(bd->escolares);
        fclose(ficheiro);
        return 0;
    }

    //Inicializar toda a memória
    inicializar_aluno(bd, bd->tamanho_aluno);
    inicializar_escolares(bd, bd->tamanho_escolares);
    inicializar_estatisticas(&(bd->stats));
    
    //Aluno
    for (int i = 0; i < bd->tamanho_aluno; i++) {
        if (!ler_dados_binarios(&bd->aluno[i].codigo, sizeof(int), 1, ficheiro)) {
            fclose(ficheiro);
            printf_fich_bin_alterado();
            return 0;
        }
        if (!ler_dados_binarios(&bd->aluno[i].nascimento, sizeof(Data), 1, ficheiro)) {
            fclose(ficheiro);
            printf_fich_bin_alterado();
            return 0;
        }
        //Ao carregar os respetivos tamanhos, as variáveis vão ficar com valor
        size_t tamanho_nome, tamanho_nacionalidade;

        //Nome
        if (!ler_dados_binarios(&tamanho_nome, sizeof(size_t), 1, ficheiro)) {
            fclose(ficheiro);
            printf_fich_bin_alterado();
            return 0;
        }
        bd->aluno[i].nome = (char *) malloc(tamanho_nome);
        if (!ler_dados_binarios(bd->aluno[i].nome, tamanho_nome, 1, ficheiro)) {
            fclose(ficheiro);
            printf_fich_bin_alterado();
            return 0;
        }
        
        //Nacionalidade  
        if (!ler_dados_binarios(&tamanho_nacionalidade, sizeof(size_t), 1, ficheiro)) {
            fclose(ficheiro);
            printf_fich_bin_alterado();
            return 0;
        }
        bd->aluno[i].nacionalidade = (char *) malloc(tamanho_nacionalidade);
        if (!ler_dados_binarios(bd->aluno[i].nacionalidade, tamanho_nacionalidade, 1, ficheiro)) {
            fclose(ficheiro);
            printf_fich_bin_alterado();
            return 0;
        }
    }
    
    //Ler array de escolares
    if (!ler_dados_binarios(bd->escolares, sizeof(Dados), bd->tamanho_escolares, ficheiro)) {
        fclose(ficheiro);
        printf_fich_bin_alterado();
        return 0;
    }
    
    //Estatísticas
    if (!ler_dados_binarios(&bd->stats, sizeof(Estatisticas), 1, ficheiro)) {
        fclose(ficheiro);
        printf_fich_bin_alterado();
        return 0;
    }

    fclose(ficheiro);

    //Falta verificar se os checksums batem certo
    unsigned long checksum_atual = calcular_checksum(bd);
    if (checksum_atual != checksum_guardado) {
        printf_fich_bin_alterado();
        return 0;
    }

    return 1;
}

/* Lê dados binários de um ficheiro
 *
 * @param ptr          Ponteiro para onde os dados serão copiados
 * @param tamanho      Tamanho em bytes de cada elemento a ler
 * @param cont         Quantidade de elementos a ler
 * @param ficheiro     Ponteiro para o ficheiro a ler
 * 
 * @return 1 se sucesso, 0 se houver:
 *         - Ponteiros inválidos
 *         - Erro na leitura
 *         
 * @note Verifica a quantidade de elementos lidos
 * @note Indica erro se a leitura for parcial
 */
int ler_dados_binarios(void * ptr, size_t tamanho, size_t cont, FILE * ficheiro) {
    if (!ptr || !ficheiro) return 0;
    
    if (fread(ptr, tamanho, cont, ficheiro) != cont) {
        printf("Erro: Leitura inválida do ficheiro.\n");
        return 0;
    }
    return 1;
}

/* Guarda os dados dos alunos e dados escolares em ficheiros de texto
 *
 * @param nome_ficheiro_dados     Nome do ficheiro para guardar dados dos alunos
 * @param nome_ficheiro_escolar   Nome do ficheiro para guardar dados escolares
 * @param bd                      Ponteiro para a estrutura da universidade
 *
 * @return void
 *         
 * @note Os dados são guardados ordenados por código
 * @note Usa o separador definido em SEPARADOR
 * @note Os ficheiros são criados se não existirem ou sobrescritos se existirem
 * @note Escreve mensagens de erro, caso existam
 */
void guardar_dados_txt(const char * nome_ficheiro_dados, const char * nome_ficheiro_escolares, Uni * bd) {
    //Abrir ficheiros txt em modo de escrita
    FILE * aluno = fopen(nome_ficheiro_dados, "w");
    FILE * dados = fopen(nome_ficheiro_escolares, "w");

    if (!aluno) {
        printf("Ocorreu um erro ao abrir o ficheiro %s para guardar os dados.\n", nome_ficheiro_dados);
    }
    //Escrever aluno
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
    //Escrever escolares
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

/* Guarda dados binários importantes ao funcionamento do programa num ficheiro
 *
 * @param nome_ficheiro   Nome do ficheiro binário para guardar dados
 * @param bd              Ponteiro para a estrutura da universidade
 * @param modo            Modo de operação ('1' para mostrar erros, '0' para silencioso)
 *
 * @return void
 *         
 * @note Guarda um checksum para validar integridade dos dados
 * @note Guarda configurações globais (autosaveON)
 * @note Guarda arrays de alunos e dados escolares
 * @note Guarda estatísticas 
 */
void guardar_dados_bin(const char * nome_ficheiro, Uni * bd, const char modo) {
    //Abrir ficheiro binário em modo de escrita
    FILE * ficheiro = fopen(nome_ficheiro, "wb");
    if (!ficheiro) {
        if (modo == '1') printf("Ocorreu um erro ao guardar os dados.\n");
        pressione_enter();
        return;
    }
    //Checksum
    unsigned long checksum = calcular_checksum(bd);
    fwrite(&checksum, sizeof(unsigned long), 1, ficheiro);

    //Configurações
    fwrite(&autosaveON, sizeof(char), 1, ficheiro);

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
        printf("Os dados foram guardados com sucesso em '%s'.\n", nome_ficheiro);
        pressione_enter();
    }
}

/* Guarda dados binários automaticamente se autosave estiver ativo
 *
 * @param bd     Ponteiro para a estrutura da universidade
 *
 * @return void
 *         
 * @note Guarda apenas se autosaveON == '1'
 * @note Opera em modo silencioso (sem mensagens de erro)
 */
void autosave(Uni * bd) {
    if (autosaveON == '1') {
        guardar_dados_bin(LOGS_BIN, bd, '0');
    }
}

/* Verifica o estado de instalação do programa através de um ficheiro flag
 *
 * @param flag    Nome do ficheiro de configuração
 * @param abrir   Modo de operação ('1' para criar ficheiro flag, '0' para apenas verificar a sua presença)
 *
 * @return 1 se:
 *         - Primeira execução do programa
 *         - Erro ao aceder ao ficheiro
 *         0 se programa já estiver instalado
 *         
 * @note O ficheiro é essencial para o correto funcionamento do programa
 */
int fase_instalacao(const char * flag, const char abrir) {
    if (!flag) return 1;

    FILE * ficheiro = fopen(flag, "r");
    //Verificar primeira abertura
    if (!ficheiro && abrir == '0') return 1;
    //Abrir o ficheiro quando pedido (na primeira abertura)
    else if (!ficheiro && abrir == '1') {
        ficheiro = fopen(flag, "w");
        fprintf(ficheiro, "NÃO ELIMINAR ESTE FICHEIRO SOB QUALQUER CIRCUNSTÂNCIA!");
        fclose(ficheiro);
        return 0; //A partir deste momento já passamos a ter o ficheiro aberto
    }
    //Só chega aqui se o ficheiro já existir previamente
    fclose(ficheiro);
    return 0;
}

/* Elimina um ficheiro especificado
 *
 * @param nome     Nome do ficheiro a eliminar
 * @param modo     Modo de operação ('1' para mostrar erros, '0' para silencioso)
 *
 * @return 1 se sucesso, 0 se:
 *         - Nome inválido
 *         - Erro ao eliminar ficheiro
 *         
 * @note Verifica se o ficheiro existe antes de tentar eliminar
 * @note Em modo '1' mostra mensagens de erro detalhadas
 * @note Não verifica se o ficheiro é o próprio programa
 */
int eliminar_ficheiro(const char * nome, const char modo) {
    if (!nome) return 0;
    //Por segurança devia ser verificado se o nome do ficheiro a eliminar não é o nosso próprio programa, mas seria demasiado complexo, e já não temos tempo ;-;
    if (remove(nome) == 0) {
        if (modo == '1') printf("O ficheiro '%s' foi eliminado com sucesso.\n", nome);
        return 1;
    }
    else {
        if (modo == '1') { 
            printf("Ocorreu um erro ao eliminar o ficheiro '%s'.\n", nome);
            printf("Por favor verifique que o nome do ficheiro inclui a extensão e se está no mesmo diretório do programa.\n");
            printf("Certifique-se ainda que o ficheiro '%s' não está aberto noutro lugar.\n", nome);
        }
        return 0;
    }
}

/* Cria um ficheiro para guardar uma listagem, caso requerido
 *
 * @param formato_selecionado   Ponteiro para armazenar extensão escolhida (.txt/.csv)
 *                              (deve ter espaço alocado)
 *
 * @return Ponteiro para o ficheiro aberto ou NULL se:
 *         - Utilizador cancelar operação
 *         - Erro ao criar ficheiro
 *         - Nome de ficheiro inválido
 *         
 * @note Verifica extensão apropriada (.txt ou .csv)
 * @note Pede nome do ficheiro ao utilizador
 * @note Verifica caracteres inválidos no nome do ficheiro
 * @note Verifica se já existe algum ficheiro com o mesmo nome
 * @note Confirma antes de sobrescrever ficheiro existente
 * @note Todos os erros são escritos no terminal
 * @note Retorna o ficheiro aberto em modo de escrita
 */
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

        //Nome do ficheiro fica à escolha do utilizador
        do {
            printf("Nome do ficheiro a guardar: ");
            nome_ficheiro = ler_linha_txt(stdin, NULL);
            if (validar_nome_ficheiro(nome_ficheiro)) break;
            else free(nome_ficheiro);
        } while(1);

        //Garantir que a string a que vamos concatenar o nome da extensão do ficheiro tem tamanho suficiente para isso
        char * temp = realloc(nome_ficheiro, strlen(nome_ficheiro) + strlen(formatos[opcao]) + 1);
        if (!temp) { 
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
        return ficheiro;
    }
    return NULL;
}

/* Verifica se um ficheiro tem extensão suportada
 *
 * @param nome_ficheiro    Nome do ficheiro a verificar
 *
 * @return 1 se a extensão é válida (.txt ou .csv), 0 se:
 *         - Ficheiro sem extensão
 *         - Extensão não suportada
 *         
 * @note Considera apenas .txt e .csv como válidas
 * @note Mostra mensagens de erro no terminal
 */
int verificar_extensao(const char * nome_ficheiro) {
    //Verificar a extensão do ficheiro
    const char * extensao = strrchr(nome_ficheiro, '.');

    if (!extensao) {
        printf("O ficheiro não tem extensão.\n");
        pressione_enter();
        return 0;
    }

    //Verificar se é suportada
    if (strcmp(extensao, ".txt") != 0 && strcmp(extensao, ".csv") != 0) {
        printf("A extensão %s não é suportada.\n", extensao);
        pressione_enter();
        return 0;
    }
    return 1;
}

/* Mostra o conteúdo de um ficheiro de texto no terminal
 *
 * @param nome_ficheiro    Nome do ficheiro a mostrar
 *
 * @return void
 *         
 * @note Suporta apenas ficheiros .txt e .csv
 * @note Verifica existência e extensão do ficheiro
 * @note Mostra mensagens de erro se necessário
 * @note Limpa o terminal
 */
void mostrar_dados_ficheiro(const char * nome_ficheiro) {
    if (!verificar_extensao(nome_ficheiro)) {
        return;
    }

    FILE * ficheiro = fopen(nome_ficheiro, "r");
    char * linha = NULL;
    int num_linhas = 0;
    limpar_terminal();
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

/* Repõe o estado inicial do programa
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Elimina CONFIG_TXT e LOGS_BIN
 * @note Não elimina backups existentes
 * @note Oferece opção de backup para os ficheiros de dados carregados .txt
 * @note Requer recarregamento dos dados após execução
 * @note Fecha o programa, tomando todas as precauções
 */
void repor_estado_inicial(Uni * bd) {
    limpar_terminal();

    printf("Todos os dados serão excluídos e será necessário carregar os ficheiros .txt.\n");
    printf("Quer guardar uma cópia dos dados em ficheiro .txt? (S/N) ");
    if(sim_nao()) { //Guardar cópias
        FILE * dados;
        FILE * escolar;
        char * dados_nome;
        char * escolar_nome;
        do {
            dados = NULL;
            dados_nome = NULL;
            printf("Escreva o nome do ficheiro cópia de '%s': ", DADOS_TXT);
            dados_nome = ler_linha_txt(stdin, NULL);
            if (!dados_nome) continue;

            if (!verificar_extensao(dados_nome)) {
                free(dados_nome);
                continue;
            }
            if ((dados = validar_ficheiro_e_abrir(dados_nome)) == NULL) {
                free(dados_nome);
                continue;
            }
            fclose(dados); //Neste caso, não queremos o ficheiro aberto porque guardar_dados já o abre
            break;
        } while(1);
        do {
            escolar = NULL;
            escolar_nome = NULL;
            printf("Escreva o nome do ficheiro cópia de '%s': ", SITUACAO_ESCOLAR_TXT);
            escolar_nome = ler_linha_txt(stdin, NULL);
            if (!escolar_nome) continue;

            if (!verificar_extensao(escolar_nome)) {
                free(escolar_nome);
                continue;
            }
            if ((escolar = validar_ficheiro_e_abrir(escolar_nome)) == NULL) {
                free(escolar_nome);
                continue;
            }
            fclose(escolar);
            break;
        } while(1);
        guardar_dados_txt(dados_nome, escolar_nome, bd);
    }
    printf("Tem a certeza que quer continuar? (S/N) ");
    if (!sim_nao()) return;

    //Eliminamos o ficheiro de instalação para usar o programa do zero
    if (!eliminar_ficheiro(CONFIG_TXT, '1')){
        printf("Por favor, elimine o ficheiro manualmente.\n");
        printf("Caso não o faça, o programa pode malfuncionar.\n");
    }
    printf("\n");

    //Verificar se há logs
    FILE * logs = fopen(LOGS_BIN, "r");
    if (logs) {
        fclose(logs);
        //Eliminar logs
        if (!eliminar_ficheiro(LOGS_BIN, '1')) {
            printf("Por favor, elimine o ficheiro manualmente.\n");
            printf("Caso não o faça, o programa pode malfuncionar.\n");
        }
    }
    
    printf("\nPara concluir a reposição do programa, a aplicação será fechada.\n");
    pressione_enter();

    free_tudo(bd);
    exit(EXIT_SUCCESS);
}

//Gestão de memória

/* Inicializa estruturas de alunos na memória
 *
 * @param bd             Ponteiro para estrutura da universidade 
 * @param indice_aluno   Índice a partir do qual inicializar (inclusive)
 *
 * @return void
 *         
 * @note Aloca memória dinamicamente para nome e nacionalidade
 * @note Inicializa campos numéricos e strings com -1 
 * @note Inicializa data com zeros
 * @note Em caso de erro na alocação, liberta memória já alocada
 */
void inicializar_aluno(Uni * bd, int indice_aluno) {
    if (!bd || !bd->aluno) return;

    for (int i = indice_aluno; i < bd->capacidade_aluno; i++) {
        //Aloca memória para guardar a nacionalidade e o nome (primeiro, pois pode falhar, e seria escusado inicializar outras coisas antes)
        bd->aluno[i].nacionalidade = (char *) malloc (MAX_STRING_NACIONALIDADE * sizeof(char));
        bd->aluno[i].nome = (char *) malloc (TAMANHO_INICIAL_NOME * sizeof(char));
        if(bd->aluno[i].nacionalidade == NULL || bd->aluno[i].nome == NULL) {
            for(int j = indice_aluno; j < i; j++) {
                free(bd->aluno[j].nacionalidade);
                free(bd->aluno[j].nome);
            }
            printf("Ocorreu um erro ao alocar memória. A encerrar.\n");
            //Guardar antes de sair
            guardar_dados_bin(LOGS_BIN, bd, '0');
            //Sair do programa para evitar erros.
            free_tudo(bd);
            exit(EXIT_FAILURE);
        }
        bd->aluno[i].codigo = -1;
        bd->aluno[i].nascimento.dia = 0; 
        bd->aluno[i].nascimento.mes = 0;
        bd->aluno[i].nascimento.ano = 0;
        //Não era necessário mas sempre pode dar jeito
        strcpy(bd->aluno[i].nacionalidade, "-1");
        strcpy(bd->aluno[i].nome, "-1");
    }
}

/* Inicializa estruturas de dados escolares na memória
 *
 * @param bd                  Ponteiro para estrutura da universidade
 * @param indice_escolares    Índice a partir do qual inicializar (inclusive)
 * 
 * @return void
 *
 * @note Inicializa todos os campos numéricos com -1
 * @note Inicializa todos os chars com '-'
 */
void inicializar_escolares(Uni * bd, int indice_escolares) {
    if (!bd || !bd->aluno || !bd->escolares) return;

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

/* Inicializa struct de estatísticas com valores padrão
 *
 * @param stats    Ponteiro para estrutura de estatísticas
 *
 * @return void
 *         
 * @note Inicializa contadores com 0
 * @note Inicializa médias com 0.0
 * @note Define estado como não atualizado ('0')
 */
void inicializar_estatisticas(Estatisticas * stats) {
    stats->finalistas = 0;
    stats->media = 0.0;
    stats->media_matriculas = 0.0;
    stats->risco_prescrever = 0;
    stats->atualizado = '0'; 
}

/* Liberta memória alocada para strings de alunos
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Liberta apenas nome e nacionalidade
 * @note Verifica ponteiros antes de libertar
 */
void free_nome_nacionalidade(Uni * bd) {
    if (!bd && !bd->aluno) return;

    for(int i = 0; i < bd->tamanho_aluno; i++) {
        if (bd->aluno[i].nome) free(bd->aluno[i].nome);
        if (bd->aluno[i].nacionalidade) free(bd->aluno[i].nacionalidade);
    }
}

/* Liberta toda a memória alocada dinamicamente
 *
 * @param bd    Ponteiro para estrutura da universidade 
 *
 * @return void
 *         
 * @note Liberta memória de strings via free_nome_nacionalidade()
 * @note Liberta arrays principais de alunos e dados escolares
 */
void free_tudo(Uni * bd) {
    free_nome_nacionalidade(bd);
    free(bd->aluno);
	free(bd->escolares);
}

/* Duplica o tamanho do array de alunos
 *
 * @param bd     Ponteiro para estrutura da universidade
 * @param modo   Modo de operação ('1' para mostrar erros, '0' para silencioso)
 *
 * @return 1 se sucesso, 0 se houver erro na realocação
 *         
 * @note Duplica capacidade atual
 * @note Inicializa nova memória via inicializar_aluno()
 */
int realocar_aluno(Uni * bd, const char modo) {
    //Dobrar o tamanho
    int tamanho_novo = bd->capacidade_aluno * 2;
    Estudante * temp = (Estudante *) realloc(bd->aluno, tamanho_novo * sizeof(Estudante));

    if (!temp) {
        if (modo == '1') printf("Ocorreu um erro ao realocar memória para o aluno. A encerrar.\n");
        return 0;
    }
    bd->aluno = temp;
    bd->capacidade_aluno = tamanho_novo;
    //Inicializar logo a memória alocada
    inicializar_aluno(bd, bd->tamanho_aluno); 
    return 1;
}

/* Duplica o tamanho do array de dados escolares
 *
 * @param bd     Ponteiro para estrutura da universidade
 * @param modo   Modo de operação ('1' para mostrar erros, '0' para silencioso)
 *
 * @return 1 se sucesso, 0 se houver erro na realocação
 *         
 * @note Duplica capacidade atual
 * @note Inicializa nova memória via inicializar_escolares()
 */
int realocar_escolares(Uni * bd, const char modo) {
    //Dobrar o tamanho
    int tamanho_novo = bd->capacidade_escolares * 2;
    Dados * temp = (Dados *) realloc(bd->escolares, tamanho_novo * sizeof(Dados));

    if (!temp) {
        if (modo == '1') printf("Ocorreu um erro ao realocar memória para os dados escolares. A encerrar.\n");
        return 0;
    }
    bd->escolares = temp;
    bd->capacidade_escolares = tamanho_novo;
    //Inicializar logo a memória alocada
    inicializar_escolares(bd, bd->tamanho_escolares);
    return 1;
}

/* Aumenta tamanho alocado para nome de aluno
 *
 * @param aluno   Ponteiro para estrutura de aluno
 * @param modo   Modo de operação ('1' para mostrar erros, '0' para silencioso)
 * 
 *
 * @return 1 se sucesso, 0 se erro na realocação
 */
int realocar_nome(Estudante * aluno, const char modo) {
    //Dobrar o tamanho
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

/* Procura código de aluno usando pesquisa binária O(log n)
 *
 * @param codigo   Código a procurar
 * @param bd       Ponteiro para estrutura da universidade
 *
 * @return Em caso de sucesso:
 *         > 0: índice onde código foi encontrado + 1
 *         < 0: -(posição de inserção + 1) se não foi encontrado
 *         = 0: erro (ponteiros inválidos/array vazio)
 *         
 * @note Array deve estar ordenado por código
 * @note Retorna posição como +/- (índice + 1) para distinguir erro de índice 0
 */
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

/* Procura código em dados escolares usando pesquisa binária O(log n)
 * 
 * @param codigo   Código a procurar
 * @param bd       Ponteiro para estrutura da universidade
 *
 * @return Em caso de sucesso:
 *         > 0: índice onde código foi encontrado + 1  
 *         < 0: -(posição de inserção + 1) se não encontrado
 *         = 0: erro (ponteiros inválidos/array vazio)
 *         
 * @note Array deve estar ordenado por código
 * @note Retorna posição como +/- (índice + 1) para distinguir erro de índice 0
 */
int procurar_codigo_escolares(int codigo, Uni * bd) {
    if (!bd || !bd->aluno|| !bd->escolares || bd->tamanho_escolares <= 0) {
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
    return -(limInf + 1); //Não há esse código, logo retorna a posição de inserção
}

/* Procura nacionalidades existentes com sistema de sugestões
 *
 * @param bd                 Ponteiro para estrutura da universidade
 * @param n_nacionalidades   Número de nacionalidades a procurar
 * @param mensagem           Mensagem customizada para prompt (NULL usa default)
 *
 * @return Array de strings com nacionalidades encontradas ou NULL se:
 *         - n_nacionalidades <= 0
 *         - Erro de alocação de memória
 *         - Utilizador cancelar operação ('sair')
 *         - Erro ao normalizar strings
 *         
 * @note Memória deve ser libertada
 * @note Implementa sistema de sugestões (max 5 por nacionalidade)
 * @note Normaliza strings para comparação case-insensitive com normalizar_string()
 * @note Valida existência das nacionalidades na base de dados
 */
char ** procurar_nacionalidades(Uni * bd, const short n_nacionalidades, char * mensagem) {
    if (n_nacionalidades <= 0) return NULL;
    char ** nacionalidades = NULL;
    
    //Criamos um array de ponteiros para guardar as nacionalidades, no máximo as indicadas
    nacionalidades = (char **) malloc(n_nacionalidades * sizeof(char*));
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

        //Array de string que irá conter todas as sugestões que foram feitas ao utilizador numa mesma nacionalidade
        //Apenas serão retidas 5 sugestões, pelo que depois,  o sistema de sugestões será desabilitado num todo
        char ** sugeridas = NULL;
        short n_sugestao = 0;
        sugeridas = (char**) malloc(TAMANHO_SUGESTOES * sizeof(char *));
        for (int j = 0; j < TAMANHO_SUGESTOES; j++) {
            sugeridas[j] = NULL;
        }
        //Pedimos a nacionalidade e depois verificamos se existe
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

            char * original = strdup(nacionalidade); //original fica com a nacionalidade introduzida
            if (!original) {
                printf("Erro ao alocar memória.\n");
                free(nacionalidade);
                for (int j = 0; j < i; j++) {
                    free(nacionalidades[j]);
                }
                free(nacionalidades);
                return NULL;
            }
            free(nacionalidade); //Necessário pois normalizar_string não o faz, e neste caso, como já temos uma cópia, eliminamos para não haver memory leaks
            nacionalidade = NULL;
            nacionalidade = normalizar_string(original);
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

/* Valida código antes de inserir novo aluno
 *
 * @param codigo   Código a validar
 * @param bd       Ponteiro para estrutura da universidade
 *
 * @return Se código válido:
 *         < 0: -(posição de inserção + 1)
 *         = 0: se código inválido ou erro:
 *              - Código <= 0 
 *              - Código já existe
 *              - Erro ao procurar código
 *         
 * @note Verifica existência no array de alunos
 * @note Mostra mensagens de erro no terminal
 */
int validar_codigo_ao_inserir(int codigo, Uni * bd) {
    if(codigo <= 0) {
        printf("Código inválido! Insira um número inteiro positivo.\n");
        pressione_enter();
        return 0;
    }
    int temp = procurar_codigo_aluno(codigo, bd); 
    //Basta procurar no aluno porque não pode haver nenhum código em escolares que não esteja em aluno
    
    if(temp > 0) {
        printf("O código já existe! Insira um código diferente.\n");
        pressione_enter();
        return 0;
    }
    //Se não existir o código podemos usá-lo(<0)
    else if (temp < 0) {
        return temp; //Retornamos a posição onde será inserido
    }
    return 0; //Caso seja 0
}

/* Valida código antes de eliminar aluno
 *
 * @param codigo   Código a validar
 * @param bd       Ponteiro para estrutura da universidade
 * @param modo     Modo de operação ('1' para mostrar erros, '0' para silencioso)
 *
 * @return índice+1 se código existe, 0 se:
 *         - Código <= 0
 *         - Código não existe
 *         - Erro ao procurar código
 *
 * @note Verifica existência no array de alunos
 */
int validar_codigo_eliminar(int codigo, Uni * bd, const char modo) {
    if(codigo <= 0) {
        if (modo == '1') {
            printf("Código inválido! Insira um número inteiro positivo.\n");
            pressione_enter();
        }
        return 0;
    }
    int temp = procurar_codigo_aluno(codigo, bd);
    
    if(temp > 0) {
        return temp; //Código existe logo pode ser eliminado
    }
    //Se não existir o código podemos usá-lo(<0).
    else if (temp < 0) {
        if (modo == '1') {
            printf("O código não existe! Insira um código diferente.\n");
            pressione_enter();
        }
        return 0; //<0 significa que foi retornada a posição onde o código devia ser inserido, logo não existe
    }
    return 0; 
}

/* Valida nome de ficheiro para escrita
 *
 * @param nome_ficheiro   Nome a validar
 *
 * @return 1 se válido, 0 se contiver caracteres inválidos
 *         
 * @note Mostra mensagens de erro no terminal
 * @note Lista caracteres inválidos se falhar
 */
int validar_nome_ficheiro(const char * nome_ficheiro) {
    //Caracteres inválidos ao escrever ficheiros
    const char * chars_invalidos = "\\/:*?\"<>|";

    for (int i = 0; nome_ficheiro[i] != '\0'; i++) {
        //strchr é semelhante à strstr já usada, mas procura um char dentro da string(1º param)
        if (strchr(chars_invalidos, nome_ficheiro[i]) != NULL) { 
            printf("O nome do ficheiro é inválido. Por favor, escreva um nome sem os seguintes caracteres: \n");
            printf("%s\n", chars_invalidos);
            return 0;
        }
    }
    return 1;
}

/* Valida e abre um ficheiro para escrita
 *
 * @param nome_ficheiro    Nome do ficheiro a abrir/criar
 *
 * @return Ponteiro para ficheiro aberto ou NULL se:
 *         - Nome inválido
 *         - Erro ao criar/abrir ficheiro
 *         - User cancelar reescrita do ficheiro
 *         
 * @note Verifica existência do ficheiro
 * @note Pede confirmação antes de sobrescrever
 * @note Abre em modo escrita ('w')
 */
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

/* Verifica e processa códigos duplicados ao carregar dados
 *
 * @param bd      Ponteiro para estrutura da universidade
 * @param erros   Ponteiro para ficheiro de log de erros
 *
 * @return void
 *         
 * @note Mantém primeira ocorrência de cada código
 * @note Requer array ordenado por código
 * @note Complexidade O(n)
 * @note Escreve erros no ficheiro de log
 */
void verificar_codigos_duplicados(Uni * bd, FILE * erros) {
    //O(n)
    char erro = '0';
    //Estudante
    for(int i = 0; i < bd->tamanho_aluno - 1; i++) { 
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

    erro = '0'; //Dar reset na flag

    //Escolares
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

/* Verifica códigos em dados escolares sem aluno correspondente
 *
 * @param bd              Ponteiro para estrutura da universidade
 * @param erros           Ponteiro para ficheiro de log de erros
 * @param primeiro_erro   Ponteiro para assinalar o primeiro erro no carregamento dos dados
 *
 * @return void
 *         
 * @note Procura códigos em dados escolares sem aluno correspondente
 * @note Remove dados escolares sem aluno
 * @note Registra erros no ficheiro de log
 */
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

/* Valida data de nascimento
 *
 * @param dia     Dia a validar
 * @param mes     Mês a validar  
 * @param ano     Ano a validar
 * @param modo    Modo de operação ('1' para mostrar erros, '0' para silencioso)
 *
 * @return 1 se data válida, 0 se:
 *         - Dia <= 0 ou > 31
 *         - Mês <= 0 ou > 12 
 *         - Ano fora do intervalo permitido
 *         - Dia inválido para o mês
 *         
 * @note Verifica anos bissextos    
 */
int validar_data(short dia, short mes, short ano, const char modo) {
    if (ano < 1 || mes < 1 || mes > 12 || dia < 1) { 
        if (modo == '1') printf("\nPor favor, insira uma data válida.\n");
        return 0;
    }
    
    //Limites dos anos são os que foram considerados mais realistas
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
        return 0; 
    }
    return 1;
}

/* Verifica se data está dentro de um intervalo
 *
 * @param inferior   Data limite inferior
 * @param superior   Data limite superior
 * @param atual      Data a verificar
 *
 * @return 1 se dentro do intervalo, 0 se:
 *         - Data atual fora do intervalo
 *         - Data superior menor que inferior
 *         - Erro na comparação
 */
int validar_data_entre_intervalo(Data inferior, Data superior, Data atual) {
    int comp = comparar_data(superior, inferior, '0');
    switch(comp) {
        case -1: 
            return 0; //superior < inferior
        case 0: 
            //Se inf == sup, então atual == inf == sup
            if (comparar_data(inferior, atual, '0') != 0) return 0;
            return 1; //atual == inf == sup logo é válido (está entre os dois, é valor único)
        case 1: 
            if ((comparar_data(atual, inferior, '0') != -1)&&(comparar_data(atual, superior, '0') != 1)) return 1;
            return 0;
        default:
            return 0;
    }
}

/* Compara duas datas
 *
 * @param d1             Primeira data a comparar
 * @param d2             Segunda data a comparar
 * @param ignorar_ano    Modo de comparação ('1' para ignorar ano, '0' para comparar tudo)
 *
 * @return -1 se d1 < d2
 *          0 se d1 = d2
 *          1 se d1 > d2
 *         
 * @note Compara na ordem: ano, mês, dia
 */
int comparar_data(Data d1, Data d2, const char ignorar_ano) {
    //Ano
    if (ignorar_ano == '0') {
        if (d1.ano < d2.ano) return -1;
        if (d1.ano > d2.ano) return 1;
    }
    //Mês
    if (d1.mes < d2.mes) return -1;
    if (d1.mes > d2.mes) return 1;
    //Dia
    if (d1.dia < d2.dia) return -1;
    if (d1.dia > d2.dia) return 1;
    
    return 0;  //Datas são iguais
}

/* Valida e armazena nome de aluno
 *
 * @param aluno    Ponteiro para estrutura de aluno
 * @param nome     Nome a validar e armazenar  
 * @param modo     Modo de operação ('1' para mostrar erros, '0' para silencioso)
 *
 * @return 1 se sucesso, 0 se:
 *         - Nome inválido (vazio/caracteres especiais)
 *         - Erro ao alocar memória
 *         - Erro ao copiar nome
 * 
 * @note Realoca memória se exceder tamanho inicial
 * @note Remove '\n' final se presente
 * @note Verifica espaços no início/fim
 * @note Caracteres permitidos (letras, espaço, hífen)
 */
int validar_nome(Estudante * aluno, char * nome, const char modo) {
    //Verificar nome vazio
    if (!nome || nome[0] == '\0') { //NULL != '\0'
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
        //Verificar se tem separador
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
        if (!iswalpha(nome[i]) && nome[i] != ' ' && nome[i] != '-') { //iswalpha apenas retorna válido a-z e A-Z, logo as outras condições validam espaços e hifens
            if (modo == '1') printf("\nNome contém caracteres inválidos!\n");
            return 0;
        }
    
    }
    //Se estamos aqui é válido, e precisamos de verificar se o nome cabe no array
    if (comprimento > TAMANHO_INICIAL_NOME - 1) {
        //Realocar o nome 
        if (!realocar_nome(aluno, modo)) {
            return 0;
        }
    }
    return 1;
}

/* Valida a nacionalidade do aluno
 *
 * @param nacionalidade    String com nacionalidade a validar
 * @param modo             Modo de operação ('1' para mostrar erros, '0' para silencioso)
 *
 * @return 1 se válida, 0 se:
 *         - String vazia/NULL
 *         - Comprimento > MAX_STRING_NACIONALIDADE
 *         - Caracteres inválidos
 *         - Formato incorreto
 *         
 * @note Não realoca memória se exceder tamanho máximo
 * @note Remove '\n' final se presente
 * @note Verifica espaços no início/fim
 * @note Caracteres permitidos (letras, espaço, hífen)
 */
int validar_nacionalidade(char * nacionalidade, const char modo) {
    if (!nacionalidade || nacionalidade[0] == '\0') {
        if (modo == '1') printf("\nNacionalidade em branco!\n");
        return 0; 
    }
    
    int comprimento = strlen(nacionalidade);
    
    //Remover \n
    if (nacionalidade[comprimento - 1] == '\n' && comprimento != 1) { 
        nacionalidade[comprimento - 1] = '\0';
        comprimento--;
    }
    // Verificar o último caractere
    if (nacionalidade[0] == ' ' || nacionalidade[comprimento - 1] == ' ') {
        if (modo == '1') printf("\nA nacionalidade tem espaços indevidos!\n");
        return 0;
    }
    //Verificar se excede o tamanho máximo permitido
    if (comprimento > MAX_STRING_NACIONALIDADE - 1) {
        if (modo == '1') printf("\nA nacionalidade é inválida. Insira uma nacionalidade com menos de %d caracteres.\n", MAX_STRING_NACIONALIDADE - 1);
        return 0;
    }

    for (int i = 0; i < comprimento; i++) {
        if (nacionalidade[i] == SEPARADOR) {
            if (modo == '1') printf("\nA nacionalidade contém um caracter separador inválido (%c).\n", SEPARADOR);
            return 0;
        }

        // Verificar se há dois espaços seguidos
        if (i < comprimento - 1 && nacionalidade[i] == ' ' && nacionalidade[i+1] == ' ') {
            if (modo == '1') printf("\nA nacionalidade não pode conter espaços consecutivos!\n");
            return 0;
        }

        if (!iswalpha(nacionalidade[i]) && nacionalidade[i] != ' ' && nacionalidade[i] != '-') {
            if (modo == '1') printf("\nA nacionalidade contém caracteres inválidos!\n");
            return 0;
        }
    }

    return 1;
}

/* Valida entrada de menus numéricos
 *
 * @param valido    Ponteiro para flag de validação
 * @param opcao     Opção escolhida pelo utilizador
 * @param limInf    Limite inferior do menu (inclusive)
 * @param limSup    Limite superior do menu (inclusive)
 *
 * @return void
 *         
 * @note Atualiza flag de validação via ponteiro
 * @note Mostra mensagens de erro apropriadas
 * @note Limpa buffer de entrada após validação
 * @note Trata entradas não-numéricas e fora do intervalo
 */
void validacao_menus(short * valido, const char opcao, const char limInf, const char limSup) { 
    if (*valido != 1) {
        printf("Entrada inválida! Introduza um número do menu (%c a %c)\n", limInf, limSup); 
        pressione_enter();
    }
    else if (opcao < limInf || opcao > limSup) { 
        *valido = 0; //Scanf leu corretamente e retornou 1, mas como não queremos esses números, voltamos a definir a zero
        printf("Por favor, escolha um número do menu (%c a %c).\n", limInf, limSup);
        pressione_enter();
    }
    //Verificar entradas com mais de um char
    char lixo = getchar();
    if (lixo != '\n') {
        *valido = 0;
        limpar_buffer(); //\n
        printf("Por favor, escolha um número do menu (%c a %c).\n", limInf, limSup);
        pressione_enter();
    }
    //Assim saímos do menu sempre com o buffer limpo (menos uma preocupação)
}

//Ordenação

/*
As funções que se seguem implementam o algoritmo de ordenação Merge Sort
Dado o algoritmo já ser bastante conhecido, não há necessidade de reinventar
Como tal, foi retirado o código base do algoritmo do website https://www.geeksforgeeks.org/c-program-for-merge-sort/
Claro que foram necessários ajustes ao contexto do programa
Os nome das variáveis também foram ajustados para ser mais fácil de perceber o que se pretende
*/

/* Combina arrays ordenados de alunos
 *
 * @param bd       Ponteiro para estrutura da universidade
 * @param inicio   Índice inicial do subarray
 * @param meio     Índice do meio do subarray
 * @param fim      Índice final do subarray
 *
 * @return void
 *         
 * @note Aloca memória temporária para subarrays
 * @note Ordena por código do aluno
 * @note Complexidade O(n)
 */
void merge_aluno(Uni * bd, int inicio, int meio, int fim) {
    int tamanho_esquerda = meio - inicio + 1; //Limitar a metade esquerda
    int tamanho_direita = fim - meio;

    //Criar arrays temporários para esquerda e direita
    Estudante * esquerda = malloc(tamanho_esquerda * sizeof(Estudante));
    Estudante * direita = malloc(tamanho_direita * sizeof(Estudante));

    //Copiar o conteúdo para os arrays temporários. É COPIADO TODO O CONTEÚDO DO ARRAY
    for(int i = 0; i < tamanho_esquerda; i++) 
        esquerda[i] = bd->aluno[inicio + i];
    
    for(int i = 0; i < tamanho_direita; i++) 
        direita[i] = bd->aluno[meio + 1 + i];
    
    int indice_esquerda = 0;
    int indice_direita = 0;
    int indice = inicio;

    while(indice_esquerda < tamanho_esquerda && indice_direita < tamanho_direita) {
        if (esquerda[indice_esquerda].codigo <= direita[indice_direita].codigo) { 
            bd->aluno[indice] = esquerda[indice_esquerda];
            indice_esquerda++;
        }
        else {
            bd->aluno[indice] = direita[indice_direita];
            indice_direita++;
        }
        indice++; 
    }
    //Copiar elementos do array da direita que sobraram (ex: num da esquerda > num direita)
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

/* Merge Sort para array de alunos
 *
 * @param bd       Ponteiro para estrutura da universidade
 * @param inicio   Índice inicial do array
 * @param fim      Índice final do array
 *
 * @return void
 *         
 * @note Divide array recursivamente
 * @note Ordena por código do aluno
 * @note Usar ao carregar os dados de ficheiros .txt
 * @note Complexidade O(n log n)
 */
void merge_sort_aluno(Uni * bd, int inicio, int fim) {
    if (inicio < fim) { //Se inicio >= fim, o array tem 1 
        int meio = inicio + (fim - inicio) / 2;
        merge_sort_aluno(bd, inicio, meio); //Ordena a primeira metade
        merge_sort_aluno(bd, meio + 1, fim); //Ordena a segunda metade
        //Isto vai ser usado recursivamente, pelo que cada metade vai ser novamente cortada a metade,... até o array ter um elemento

        merge_aluno(bd, inicio, meio, fim); //Combina as duas metades ordenadas
    }
}

/* Combina arrays ordenados de dados escolares
 *
 * @param bd       Ponteiro para estrutura da universidade
 * @param inicio   Índice inicial do subarray  
 * @param meio     Índice do meio do subarray
 * @param fim      Índice final do subarray
 *
 * @return void
 *         
 * @note Aloca memória temporária para subarrays
 * @note Ordena por código do aluno
 * @note Complexidade O(n)
 */
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
            bd->escolares[indice] = esquerda[indice_esquerda]; 
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

/* Merge Sort para array de dados escolares
 * 
 * @param bd       Ponteiro para estrutura da universidade
 * @param inicio   Índice inicial do array
 * @param fim      Índice final do array
 *
 * @return void
 *         
 * @note Divide array recursivamente
 * @note Ordena por código do aluno
 * @note Usar ao carregar os dados de ficheiros .txt
 * @note Complexidade O(n log n)
 */
void merge_sort_escolares(Uni * bd, int inicio, int fim) {
    if (inicio < fim) { //Se inicio >= fim, o array tem 1 
        int meio = inicio + (fim - inicio) / 2;
        merge_sort_escolares(bd, inicio, meio); //Ordena a primeira metade
        merge_sort_escolares(bd, meio + 1, fim); //Ordena a segunda metade
        //Isto vai ser usado recursivamente, pelo que cada metade vai ser novamente cortada a metade,... até o array ter um elemento

        merge_escolares(bd, inicio, meio, fim); //Combina as duas metades ordenadas
    }
} //Termina aqui o bloco da implementação de merge sort

/* Ordena arrays ao inserir novo aluno
 *
 * @param codigo             Código do novo aluno
 * @param bd                 Ponteiro para estrutura da universidade
 * @param indice_aluno       Índice onde inserir dados do aluno
 * @param indice_escolares   Índice onde inserir dados escolares
 *
 * @return void
 *         
 * @note Insere o código de aluno
 * @note Incrementa tamanho dos arrays
 * @note Mantém arrays ordenados por código
 * @note Aloca memória nova para strings
 * @note Evita ponteiros duplicados
 * @note Não realoca tamanho total dos arrays
 */
void ordenar_ao_inserir(int codigo, Uni * bd, int indice_aluno, int indice_escolares) {
    //Ordenar aluno
    //Copia-se por partes para evitar ponteiros duplicados
    for(int i = bd->tamanho_aluno; i > indice_aluno; i--) {
        //Libertamos a memória do elemento atual
        free(bd->aluno[i].nome);
        free(bd->aluno[i].nacionalidade);
        //Alocamos nova memória para o elemento atual, para não termos ponteiros duplicados a apontar para o mesmo sítio
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
    //Precisamos de colocar os dados do novo aluno, no caso o código, os outros inicializamos para não ficar com os dados do anterior
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
    bd->escolares[indice_escolares].prescrever = '-';
    bd->escolares[indice_escolares].finalista = '-';
    bd->escolares[indice_escolares].media_atual = -1;
    
    bd->tamanho_escolares++;
}

/* Ajusta arrays ao eliminar aluno
 *
 * @param bd                 Ponteiro para estrutura da universidade  
 * @param indice_aluno       Índice do aluno a eliminar
 * @param indice_escolares   Índice dos dados escolares a eliminar
 *
 * @return void
 *         
 * @note Decrementa tamanho dos arrays
 * @note Liberta memória alocada
 * @note Mantém arrays ordenados
 * @note Não realoca tamanho total dos arrays
 */
int ordenar_ao_eliminar(int codigo, Uni * bd, const char modo) {
    int posicao_eliminacao_aluno;
    int posicao_eliminacao_escolares;

    posicao_eliminacao_aluno = validar_codigo_eliminar(codigo, bd, modo);
    if(posicao_eliminacao_aluno <= 0) return 0; //O código não existe ou houve erro
    posicao_eliminacao_aluno -=1;

    //Se vamos eliminar o estudante então é necessário dar free na memória alocada dinamicamente nessas structs
    free(bd->aluno[posicao_eliminacao_aluno].nome);
    free(bd->aluno[posicao_eliminacao_aluno].nacionalidade);
    //i começa no índice que queremos eliminar(vai ser substituído)
    //Não se usa -1 no tamanho pois queremos inicializar a última antiga casa do array
    for(int i = posicao_eliminacao_aluno; i < bd->tamanho_aluno; i++) {
        bd->aluno[i] = bd->aluno[i + 1];
    }
    bd->tamanho_aluno -= 1;
    
    posicao_eliminacao_escolares = procurar_codigo_escolares(codigo, bd);
    if (posicao_eliminacao_escolares <= 0) return 1; //Aqui não retornamos 0 porque pode haver aluno sem dados escolares
    posicao_eliminacao_escolares -= 1;

    for(int i = posicao_eliminacao_escolares; i < bd->tamanho_escolares; i++) {
        bd->escolares[i] = bd->escolares[i + 1];
    }
    bd->tamanho_escolares -= 1;

    return 1;
}

//Menus

/* Mostra menu e processa entrada do utilizador
 *
 * @param escrever_menu   Ponteiro para função que mostra um menu específico
 * @param min_opcao       Valor mínimo aceite do menu
 * @param max_opcao       Valor máximo aceite do menu
 *
 * @return Opção escolhida ou '0' se erro
 *         
 * @note Limpa terminal antes de mostrar menu com limpar_terminal()
 * @note Valida entrada via validacao_menus()
 * @note Só sai quando adquire uma entrada válida
 */
char mostrar_menu(void (*escrever_menu)(), char min_opcao, char max_opcao) { 
    short valido = 0;
    char opcao = '0';
    do {
        limpar_terminal();
        escrever_menu(); //Escreve a função do menu
        printf("=>Escolha uma opção: ");

        valido = scanf(" %c", &opcao); //scanf retorna 1 se conseguir ler corretamente
        validacao_menus(&valido, opcao, min_opcao, max_opcao);

        if (valido == 1) {
            return opcao;
        }
    } while (valido == 0);

    return '0'; //colocado aqui porque o compilador não gostava de não haver return 
}

/*
As seguintes funções que começam por "menu" servem apenas para printar o menu
Nas notas das funções estão os limites de cada menu
*/

/* Menu principal da aplicação
 *
 * @return void
 *         
 * @note Opções: 0-6
 */
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
    printf("║  5. Aniversários                 ║\n");
    printf("║  6. Opções                       ║\n");
    printf("║  0. Sair do programa             ║\n");
    printf("╚══════════════════════════════════╝\n\n");
}

/* Menu de gestão de estudantes
 * 
 * @return void
 *
 * @note Opções: 0-2 
 */
void menu_gerir_estudantes() { 
    printf("╔════════════════════════════════════╗\n");
    printf("║          GERIR ESTUDANTES          ║\n");
    printf("╠════════════════════════════════════╣\n");
    printf("║  1. Inserir estudante              ║\n");
    printf("║  2. Eliminar estudante             ║\n");
    printf("║  0. Voltar ao menu anterior        ║\n");
    printf("╚════════════════════════════════════╝\n\n");
}

/* Menu de consultar dados
 * 
 * @return void
 *
 * @note Opções: 0-4 
 */
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

/* Menu de estatísticas
 * 
 * @return void
 *
 * @note Opções: 0-7 
 */
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

/* Menu de ficheiros
 * 
 * @return void
 *
 * @note Opções: 0-7 
 * @note Os nomes dos ficheiros estão escritos à mão
 */
void menu_ficheiros() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                         FICHEIROS                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Guardar dados em ficheiros .txt                        ║\n");
    printf("║  2. Mostrar dados de erros.txt (erros ao carregar dados)   ║\n");
    printf("║  3. Mostrar dados de dados.txt                             ║\n");
    printf("║  4. Mostrar dados de situacao_Escolar_Estudantes.txt       ║\n");
    printf("║  5. Mostrar dados de um ficheiro (.txt ou .csv) por nome   ║\n");
    printf("║  6. Guardar dados em ficheiro binário                      ║\n");
    printf("║  7. Fazer backup dos ficheiros .txt atuais                 ║\n");
    printf("║  0. Voltar ao menu anterior                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
}

/* Menu de opções
 * 
 * @return void
 *
 * @note Opções: 0-3 
 */
void menu_opcoes() {
    printf("╔══════════════════════════════════╗\n");
    printf("║              OPÇÕES              ║\n");
    printf("╠══════════════════════════════════╣\n");
    printf("║  1. Ativar/Desativar autosave    ║\n");
    printf("║  2. Repor definições             ║\n");
    printf("║  3. Guia de utilização           ║\n");
    printf("║  0. Voltar ao menu anterior      ║\n");
    printf("╚══════════════════════════════════╝\n\n");
}

/* Menu de aniversários
 * 
 * @return void
 *
 * @note Opções: 0-3 
 */
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

/* Menu de listar aniversariantes por dia da semana
 * 
 * @return void
 *
 * @note Opções: 0-7 
 * @note Opção 1 - Domingo
 * @note Opção 7 - Sábado
 */
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

/* Menu de formatos disponíveis
 * 
 * @return void
 *
 * @note Opções: 0-2 
 * @note Opção 1 - txt
 * @note Opção 2 - csv
 */
void menu_formatos_disponiveis() {
    printf("╔══════════════════════════╗\n");
    printf("║   FORMATOS DISPONÍVEIS   ║\n");
    printf("╠══════════════════════════╣\n");
    printf("║  1. .txt                 ║\n");
    printf("║  2. .csv                 ║\n");
    printf("║  0. Sair                 ║\n");
    printf("╚══════════════════════════╝\n\n");
}

/* Menu da média de matrículas
 * 
 * @return void
 *
 * @note Opções: 0-2 
 * @note Opção 1 - Geral
 * @note Opção 2 - Nacionalidade
 */
void menu_media_matriculas() {
    printf("╔══════════════════════════╗\n");
    printf("║     MÉDIA MATRÍCULAS     ║\n");
    printf("╠══════════════════════════╣\n");
    printf("║  1. Geral                ║\n");
    printf("║  2. Por nacionalidade    ║\n");
    printf("║  0. Sair                 ║\n");
    printf("╚══════════════════════════╝\n\n");
}

/* Guia de utilização do programa
 * 
 * @return void
 *
 * @note Pede enter para continuar via pressione_enter()
 */
void guia_de_utilizacao() {
    limpar_terminal();
    printf("╔══════════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                  GUIA DE UTILIZAÇÃO                                  ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ 1. MENU PRINCIPAL                                                                    ║\n");
    printf("║    - Gerir estudantes: Inserir ou eliminar estudantes                                ║\n");
    printf("║    - Consultar dados: Procurar e listar estudantes por vários critérios              ║\n"); 
    printf("║    - Estatísticas: Visualizar dados estatísticos sobre os estudantes                 ║\n");
    printf("║    - Ficheiros: Gestão de ficheiros e visualização de dados                          ║\n");
    printf("║    - Aniversários: Listar estudantes por várias datas especiais                      ║\n");
    printf("║    - Opções: Configurações do programa                                               ║\n");
    printf("║                                                                                      ║\n");
    printf("║ 2. NOTAS IMPORTANTES                                                                 ║\n");
    printf("║    - O programa carrega dados de ficheiros .txt apenas no primeiro uso               ║\n");
    printf("║    - Para voltar a carregar dados de ficheiro .txt, selecione a opção 6.2 do menu    ║\n");
    printf("║    - Erros a carregar dados são registados no ficheiro erros.txt                     ║\n");
    printf("║    - A opção de autosave guarda automaticamente em ficheiro binário                  ║\n");
    printf("║    - O ficheiro instalacao.txt jamais deve ser alterado pelo utilizador, podendo     ║\n");
    printf("║    resultar na inadvertida manipulação de dados                                      ║\n");
    printf("║    - Na primeira utilização do programa é sempre feito um backup dos ficheiros .txt  ║\n");
    printf("║                                                                                      ║\n");
    printf("║ 3. DICAS                                                                             ║\n");
    printf("║    - Use '0' para voltar ao menu anterior ou sair                                    ║\n");
    printf("║    - Pressione Enter para avançar rapidamente nas listagens                          ║\n");
    printf("║    - Pode guardar listagens em ficheiros .txt ou .csv                                ║\n");
    printf("║    - Consulte erros.txt para ver problemas com dados carregados na opção 4.2 do menu ║\n");
    printf("║    - Erros a carregar dados/ler/guardar ficheiros? Certifique-se que todos os        ║\n");
    printf("║    ficheiros que quer usar estão na mesma pasta que o programa!                      ║\n");
    printf("║    - Fazer um backup dos seus ficheiros .txt pode ser uma boa opção!                 ║\n");
    printf("║                                                                                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════════════════╝\n");

    pressione_enter();
}

/* Função principal de controlo do programa aka Coração do programa
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Guarda automaticamente caso autosaveON == '1'
 * @note Processa navegação entre menus:
 *       - Gestão de estudantes
 *       - Consulta de dados
 *       - Estatísticas
 *       - Ficheiros
 *       - Aniversários
 *       - Opções
 */
void the_architect(Uni * bd) {
    char opcao;
    do {
        autosave(bd); //Guarda automaticamente caso autosaveON ativo
        opcao = mostrar_menu(menu_principal, '0', '6');
        switch(opcao) {
            case '0':
                limpar_terminal();
                printf("Tem a certeza que quer sair do programa? (S/N) ");
                if (!sim_nao()) {
                    opcao = '1'; //Forçar o loop a continuar
                    //Não há problemas porque no ínicio do loop pedimos sempre o menu
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
            case '6': 
                processar_opcoes(bd);
                break;
            default:
                opcao = '0'; //Sair caso haja erro
                break;
        }
    } while(opcao != '0');
}

/* Processa menu de gestão de estudantes
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Guarda automaticamente caso autosaveON == '1'
 * @note Chama funcionalidade consoante opção
 */
void processar_gerir_estudantes(Uni * bd) {
    char opcao;
    do {
        autosave(bd);
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

/* Processa menu de consulta de dados
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *        
 * @note Guarda automaticamente caso autosaveON == '1'
 * @note Chama funcionalidade consoante opção
 */
void processar_consultar_dados(Uni * bd) {
    char opcao;
    do {
        autosave(bd);
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

/* Processa menu de estatísticas
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 * 
 * @note Calcula as estatísticas se opcao != '0'       
 * @note Guarda automaticamente caso autosaveON == '1'
 * @note Chama funcionalidade consoante opção
 */
void processar_estatisticas(Uni * bd) {
    char opcao;
    do {
        autosave(bd);
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
                media_idades_por_nacionalidade_e_ano(bd);
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

/* Processa menu de ficheiros
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 * 
 * @note Guarda automaticamente caso autosaveON == '1'
 * @note Chama funcionalidade consoante opção
 */
void processar_ficheiros(Uni * bd) {
    char opcao;
    do {
        autosave(bd);
        opcao = mostrar_menu(menu_ficheiros, '0', '7');
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
            case '6':
                guardar_dados_bin(LOGS_BIN, bd, '1');
                break;
            case '7': 
                limpar_terminal();
                guardar_dados_txt(DADOS_BACKUP_TXT, SITUACAO_ESCOLAR_BACKUP_TXT, bd);
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

/* Processa menu de aniversários
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 * 
 * @note Guarda automaticamente caso autosaveON == '1'
 * @note Chama funcionalidade consoante opção
 */
void processar_aniversarios(Uni * bd) {
    char opcao;
    do {
        autosave(bd);
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

/* Processa menu de opções
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 * 
 * @note Guarda automaticamente caso autosaveON == '1'
 * @note Chama funcionalidade consoante opção
 */
void processar_opcoes(Uni * bd) {
    char opcao;
    do {
        autosave(bd);
        opcao = mostrar_menu(menu_opcoes, '0', '3');
        switch(opcao) {
            case '0': break;
            case '1':
                limpar_terminal();
                if (autosaveON == '0') {
                    autosaveON = '1';
                    printf("O autosave foi ativado.\n");
                }
                else {
                    autosaveON = '0';
                    printf("O autosave foi desativado.\n");
                }
                pressione_enter();
                break;
            case '2':
                repor_estado_inicial(bd);
                break;
            case '3':
                guia_de_utilizacao();
                break;
            default: 
                opcao = '0';
                break;
        }
    } while (opcao != '0');
}

//Inserção/leitura de dados

/* Lê, valida e insere data de nascimento em aluno
 *
 * @param aluno   Ponteiro para estrutura do tipo Data do aluno
 * @param str     String com data (NULL para ler do stdin)
 * @param modo    Modo de operação ('1' para mostrar erros, '0' para silencioso)
 *
 * @return void
 *         
 * @note Formato esperado: DD-MM-AAAA
 * @note Valida data via validar_data()
 * @note Loop até entrada válida se str=NULL
 */
void ler_data(Data * aluno, char * str, const char modo) {
	char data[11]; //Vamos usar o formato DD-MM-AAAA (10 caracteres + \0)
    char erro = '0'; 
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
                continue;  
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
        //Verificar se foi escrito mais do que o supostoa
        else {
            if (strlen(str) >= sizeof(data)) {
                if (modo == '1') printf("Entrada inválida. Por favor, escreva a data no formato DD-MM-AAAA.\n");
                erro = '1';
                continue;
            }
            //Copiar a string para array
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

/* Insere novo estudante na base de dados
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Valida todos os campos
 * @note Continua a pedir entradas até serem válidas
 * @note Mantém ordenação por código
 * @note Gere alocação de strings
 * @note Realoca arrays se necessário
 * @note Incrementa tamanho dos arrays
 */
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
        do {
            limpar_terminal();
            int codigo_temp = -1;
            pedir_codigo(&codigo_temp);
            if (codigo_temp == 0) return;
            
            posicao_insercao_aluno = validar_codigo_ao_inserir(codigo_temp, bd);
            if(posicao_insercao_aluno < 0) { //Não se verifica escolares pois nunca há códigos em escolares que não estejam em aluno
                posicao_insercao_aluno = -(posicao_insercao_aluno + 1);
                posicao_insercao_escolares = procurar_codigo_escolares(codigo_temp, bd);
                if (posicao_insercao_escolares == 0) { //Necessário porque procurar não printa erros
                    printf("Ocorreu um erro a procurar o índice dos dados escolares.\n");
                    printf("Por favor tente novamente.\n");
                    pressione_enter();
                    continue;
                }
                //Não se verifica se é < 0 porque sabemos que se não existe em aluno não existe em escolares
                posicao_insercao_escolares = -(posicao_insercao_escolares + 1);
                ordenar_ao_inserir(codigo_temp, bd, posicao_insercao_aluno, posicao_insercao_escolares);
                //Trocam-se as posições do array. O código é inserido. O resto inicializado
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

        //Ler data já faz as validações necessárias e coloca a data
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
            if (ano_atual_temp < 1 || ano_atual_temp > MAX_ANO_ATUAL ) {
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
        //Estatísticas estão desatualizadas
        bd->stats.atualizado = '0';

        printf("\nO código %d foi introduzido com sucesso!\n", bd->aluno[posicao_insercao_aluno].codigo);

        printf("\nQuer inserir mais estudantes? (S/N): ");
        if (!sim_nao()) return;
    }while(1); //Não precisamos de verificar repetir pois só chega aqui se repetir == 's'
    //Introduzimos um estudante, logo os dados estatísticos têm de ser recalculados.
}

/* Elimina estudante da base de dados
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Oferece a possibilidade de eliminar alunos por intervalos de códigos
 * @note Remove todos os dados associados ao(s) código(s) dado(s)
 * @note Mantém ordenação por código
 * @note Liberta memória alocada
 * @note Decrementa tamanho dos arrays
 */
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

/* Calcula estatísticas gerais dos alunos
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Calcula médias:
 *       - Matrículas
 *       - Notas
 * @note Identifica alunos:
 *       - Finalistas 
 *       - Em risco de prescrição
 */
void calcular_estatisticas(Uni * bd) {
    //Inicializar caso já tenhamos chamado a função antes
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
            bd->escolares[i].finalista = '1'; //Marcar o aluno como finalista
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
    //Tratar da divisão por zero
    if (bd->tamanho_escolares != 0) {
        bd->stats.media_matriculas /= bd->tamanho_escolares;
        bd->stats.media /= bd->tamanho_escolares;
    }
    else {
        bd->stats.media_matriculas = 0.0;
        bd->stats.media = 0.0;
    }
    //Estatísticas estão atualizadas
    bd->stats.atualizado = '1';
}

/* Calcula média de matrículas 
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Opção geral ou por nacionalidade
 * @note Valida divisão por zero
 * @note Nacionalidade é procurada via procurar_nacionalidades()
 */
void calcular_media_matriculas(Uni * bd) {
    char opcao;
    do {
        limpar_terminal();
        opcao = mostrar_menu(menu_media_matriculas, '0', '2');
        switch(opcao) {
            case '0': return;
            case '1':
                //Tratar do caso da divisão por zero
                if (bd->tamanho_escolares == 0) {
                    printf("Não há alunos!\n"); 
                    break;
                }
                //Media já foi calculada
                printf("O número médio de matrículas geral é de %.1f.\n", bd->stats.media_matriculas);
                pressione_enter();
                break;
            case '2':
                float media_matriculas_nacionalidade = 0.0; 
                int indice = 0; //Indice em escolares
                short contador = 0;
                
                //Pedir uma nacionalidade correta
                char ** nacionalidades = procurar_nacionalidades(bd, 1, "Insira a nacionalidade da qual quer saber a média de matrículas");
                if (!nacionalidades) {
                    pressione_enter();
                    break;
                }
                
                for(int i = 0; i < bd->tamanho_aluno; i++) {
                    if (strcmp(nacionalidades[0], bd->aluno[i].nacionalidade) == 0) {
                        //O índice de aluno pode ser diferente de escolares
                        indice = procurar_codigo_escolares(bd->aluno[i].codigo, bd);
                        if (indice > 0) {
                            indice -= 1;
                            media_matriculas_nacionalidade += bd->escolares[indice].matriculas;
                            contador++;
                        }
                    }
                }
                //Evitar divisão por 0
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
    } while(1); //Saímos em cima caso seja 0
}

/* Conta alunos num intervalo de médias para um ano específico
 *
 * @param bd          Ponteiro para estrutura da universidade
 * @param media_min   Média mínima do intervalo (inclusive)
 * @param media_max   Média máxima do intervalo (inclusive)
 * @param ano_atual   Ano curricular a considerar
 *
 * @return Número de alunos que satisfazem os critérios
 *         
 * @note Complexidade O(n)
 */
int alunos_por_media_e_ano(Uni * bd, float media_min, float media_max, short ano_atual) {
    int n_alunos = 0;
    for(int i = 0; i < bd->tamanho_escolares; i++) {
        if (bd->escolares[i].ano_atual == ano_atual && bd->escolares[i].media_atual >= media_min 
        && bd->escolares[i].media_atual <= media_max) {
            n_alunos++;
        }
    }
    return n_alunos;
}

/* Mostra distribuição de alunos por escalões etários numa tabela
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Máximo de intervalos definido por MAX_INTERVALOS 
 * @note Valida sobreposição de intervalos
 * @note Calcula idade via calcular_idade()
 */
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
        if (!verificar_e_limpar_buffer()) {
            printf("Número de intervalos inválido. Por favor tente novamente.\n");
            continue;
        }
        break;
    } while(1);
    
    //Pedir pelos intervalos
    for(int i = 0; i < n_intervalos && valido == '1'; i++) {
        do {
            printf("\nIntervalo %d:\n", i + 1);
            printf("Idade inferior: ");
            if (scanf("%hd", &idade_inf[i]) != 1 || idade_inf[i] < IDADE_MINIMA || idade_inf[i] > IDADE_MAXIMA) {
                printf("Idade inválida. Por favor insira um número entre %d e %d\n", IDADE_MINIMA, IDADE_MAXIMA);
                limpar_buffer();
                continue;
            }
            if (!verificar_e_limpar_buffer()) {
                printf("Idade inválida. Por favor tente novamente.\n");
                continue;
            }

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

/* Mostra distribuição de alunos por médias e anos numa tabela
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Formato tabela com anos nas linhas e médias nas colunas
 */
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
        if (!verificar_e_limpar_buffer()) {
            printf("Entrada inválida. Por favor insira um número entre 1 e %d.\n", MAX_INTERVALOS);
            pressione_enter();
            continue;
        }
        break;
    } while(1);

    //Pedir intervalos
    for(int i = 0; i < n_intervalos && valido == '1'; i++) {
        do {
            printf("\nIntervalo %d:\n", i + 1);
            printf("Média inferior: ");
            if (scanf("%f", &media_inferior[i]) != 1 || media_inferior[i] < 0 || media_inferior[i] >= 20) {
                printf("Média inválida. Por favor introduza um número entre 0 e 20.\n");
                limpar_buffer();
                continue;
            }
            if (!verificar_e_limpar_buffer()) {
                printf("Entrada inválida. Por favor introduza um número entre 1 e 20\n");
                pressione_enter();
                continue;
            }

            printf("Média superior: ");
            if (scanf("%f", &media_superior[i]) != 1 || media_superior[i] <= media_inferior[i] || media_superior[i] > 20) {
                printf("Média inválida. Deve ser maior que %.1f e menor ou igual a 20.\n", media_inferior[i]);
                limpar_buffer();
                continue;
            }
            if (!verificar_e_limpar_buffer()) {
                printf("Entrada inválida. Por favor introduza um número maior que %.1f e menor ou igual a 20\n", media_inferior[i]);
                pressione_enter();
                continue;
            }

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

/* Calcula média de idades por nacionalidade
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Calcula idade via calcular_idade()
 * @note Usa procurar_nacionalidades() para pedir nacionalidades
 */
void media_idades_por_nacionalidade_e_ano(Uni * bd) {
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
    //Como nacionalidades só tem um parâmetro, só precisamos de dar free em nacionalidades[0] e nacionalidades.
    if (!nacionalidades) {
        pressione_enter();
        return;
    }
    
    //Pedir a média de idades
    do {
        printf("Quer saber a média de idades de que estudantes de que ano? (1-%hd) ", max_ano);
        if (scanf("%hd", &ano) != 1 || ano < 1 || ano > max_ano) {
            printf("Por favor insira um número entre 1 e %hd.\n", max_ano);
            limpar_buffer();
            continue;
        }
        if (!verificar_e_limpar_buffer()) {
            printf("O ano é inválido. Por favor tente novamente.\n");
            continue;
        }
        break;
    } while(1);

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

/* Lista múltiplos estudantes com paginação
 *
 * @param bd               Ponteiro para estrutura da universidade
 * @param indice_aluno     Índice do aluno no array
 * @param ficheiro         Ponteiro para ficheiro onde guardar (NULL para apenas mostrar no terminal)
 * @param separador        Caractere separador para ficheiro ('\t' ou ',')
 * @param contador         Ponteiro para contador de paginação (NULL se não usar)
 *
 * @return void
 *         
 * @note Mostra dados pessoais e escolares
 * @note Mostra se é finalista e se está em risco de prescrever, mas apenas no terminal
 * @note Formata saída em colunas alinhadas 
 * @note Usa paginação via pausa_listagem() se contador != NULL (altamente recomendável)
 */
void listar(Uni * bd, int indice_aluno, FILE * ficheiro, char separador, short * contador) {
    int indice_escolares = 0;
    indice_escolares = procurar_codigo_escolares(bd->aluno[indice_aluno].codigo, bd);
    pausa_listagem(contador);

    //Listar no terminal
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
        //Verificar se é finalista
        if (bd->escolares[indice_escolares].finalista == '1') 
            printf("Finalista: Sim\n");
        else if (bd->escolares[indice_escolares].finalista == '0')
            printf("Finalista: Não\n");

        //Verificar se está em risco de prescrever
        if (bd->escolares[indice_escolares].prescrever == '1') 
            printf("Em risco de prescrição: Sim\n");
        else if (bd->escolares[indice_escolares].prescrever == '0')
            printf("Em risco de prescrição: Não\n");
    }
    printf("\n");
    indice_escolares++; //Precisamos de uma 'flag' para verificar se escrevemos o escolares nos ficheiros ou não
    //Listar no ficheiro
    if (ficheiro) {
        //Colocar \n apenas se não for nem a primeira nem a última entrada
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
            //Converter média para ponto (para o caso de ser ficheiro .csv)
            short media_decimal = 0;
            media_decimal = bd->escolares[indice_escolares].media_atual * 10;
            media_decimal %= 10;
            fprintf(ficheiro, "%d.%hd", (int) bd->escolares[indice_escolares].media_atual, media_decimal);
        }
    }
}

/* Lista informações completas de um estudante
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Mostra dados pessoais e escolares
 */
void listar_info_estudante(Uni * bd) {
    int codigo;
    int indice_aluno;
    do{
        limpar_terminal();
        codigo = 0;
        indice_aluno = 0;
        pedir_codigo(&codigo);
        if (codigo == 0) return;

        //Procurar índice 
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

/* Procura e lista estudantes por nome parcial
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Procura por substring com 2+ caracteres 
 * @note Case-insensitive para melhor usabilidade
 * @note Permite exportar resultados para ficheiro
 * @note Suporta formatos:
 *       - txt (separador tab)
 *       - csv (separador vírgula)
 * @note Implementa paginação na listagem
 */
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
        //Listagem
        listagem = pedir_listagem(formato);
        separador = obter_separador(listagem, formato);
        
        //Pedir nome
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
        //Faz-se isto para evitar mudar o nome da variável
        char * orig = strdup(parte_nome);
        parte_nome = normalizar_string(parte_nome);

        printf("Resultados da pesquisa para \"%s\": \n\n", orig);
        for(int i = 0; i < bd->tamanho_aluno; i++) {
            //Colocar todos os dados a letras minúsculas
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
            printf("Não foi encontrado nenhum estudante com parte do nome \"%s\".\n", orig);
        free(orig); //Libertamos o ponteiro original para não haver memory leaks

        pressione_enter();
        printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");

        free(parte_nome);
        if (listagem) fclose(listagem);

        printf("\nQuer repetir a procura? (S/N): ");
        if (!sim_nao()) return;
    } while(1);
}

/* Lista estudantes por apelido alfabeticamente
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Extrai último nome como apelido
 * @note Ordenação case-insensitive
 * @note Permite exportar para ficheiro
 * @note Implementa paginação na listagem
 */
void listar_apelidos_alfabeticamente(Uni * bd) {
    short contador = 0;
    char formato[MAX_FORMATO];
    char separador; 
    FILE * listagem = NULL;

    limpar_terminal();
    //Listagem
    listagem = pedir_listagem(formato);
    separador = obter_separador(listagem, formato);

    //Criar uma cópia temporária de aluno 
    Estudante * copia = (Estudante *) malloc(bd->tamanho_aluno * sizeof(Estudante));

    if (!copia) {
        printf("Ocorreu um erro a listar os estudantes por ordem alfabética. Por favor tente novamente mais tarde.\n");
        pressione_enter();
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
            //strrchr verifica a última ocorrência de char em str e retorna um ponteiro para essa ocorrência.
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
                if (nome1_normalizado) free(nome1_normalizado);
                if (nome2_normalizado) free(nome2_normalizado);
                continue;
            }

            //Comparar por ordem alfabética.
            if(strcmp(nome1_normalizado, nome2_normalizado) > 0) {
                Estudante temp = copia[j];
                copia[j] = copia[j+1];
                copia[j+1] = temp;
            }

            free(nome1_normalizado);
            free(nome2_normalizado);
        }
    }


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

/* Lista estudantes por dia de aniversário (da semana)
 *
 * @param bd     Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Calcula o dia da semana via dia_da_semana()
 * @note Permite exportar para ficheiro
 * @note Implementa paginação na listagem
 */
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

        //Lembrar que se for 7, na verdade é o 0 na função calcular_dia_da_semana
        if (opcao == 7) opcao = 0; //Como já saímos do menu, ajustamos para a mesma ordem que a função calcular_dia_da_semana retornará
        
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

/* Lista estudantes com aniversário ao domingo num dado ano
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Usa dia_da_semana() para calcular dia
 * @note Valida ano entre data de nascimento do aluno e DATA_ATUAL.ano + ANOS_AVANCO_PROCURAS
 * @note Permite exportar para ficheiro
 * @note Implementa paginação na listagem
 */
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

        //Listagem
        listagem = pedir_listagem(formato);
        separador = obter_separador(listagem, formato);

        //Pedir ano
        do {
            printf("Insira o ano: ");
            if (scanf("%hd", &ano) != 1 || ano < DATA_ATUAL.ano - ANOS_AVANCO_PROCURAS || ano > DATA_ATUAL.ano + ANOS_AVANCO_PROCURAS) {
                printf("Por favor insira um ano entre %hd e %hd.\n", DATA_ATUAL.ano - ANOS_AVANCO_PROCURAS, DATA_ATUAL.ano + ANOS_AVANCO_PROCURAS);
                limpar_buffer();
                continue;
            }
            if (!verificar_e_limpar_buffer()) {
                printf("Ano inválido. Tente novamente.\n");
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

/* Lista estudantes com aniversário na Quaresma
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Calcula datas da Quaresma via calcular_quaresma()
 * @note Permite exportar para ficheiro
 * @note Implementa paginação na listagem
 */
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

        //Listagem
        listagem = pedir_listagem(formato);
        separador = obter_separador(listagem, formato);

        //Pedir o ano
        do {
            printf("Insira o ano: ");
            if (scanf("%hd", &ano) != 1 || ano < DATA_ATUAL.ano - ANOS_AVANCO_PROCURAS || ano > DATA_ATUAL.ano + ANOS_AVANCO_PROCURAS) {
                printf("Por favor insira um ano entre %hd e %hd.\n", DATA_ATUAL.ano - ANOS_AVANCO_PROCURAS, DATA_ATUAL.ano + ANOS_AVANCO_PROCURAS);
                limpar_buffer();
                continue;
            }
            if (!verificar_e_limpar_buffer()) {
                printf("Ano inválido. Tente novamente.\n");
                continue;
            }
            break;
        } while (1);
        limpar_terminal();

        //Calcular a data da quaresma no ano pedido
        calcular_quaresma(ano, &inicio, &fim);
        
        printf("Estudantes cujo aniversário é na Quaresma em %hd:\n\n", ano);
        for(int i = 0; i < bd->tamanho_aluno; i++) {
            //Comparar_data não funcionava originalmente porque comparava tudo, logo foi acrescentada uma opção de exluir o ano e comparar apenas mês e dia
            if (comparar_data(inicio, bd->aluno[i].nascimento, '1') == -1 && comparar_data(fim, bd->aluno[i].nascimento, '1') == 1) { 
                listar(bd, i, listagem, separador, &contador);
            }
        }
        if (listagem) fclose(listagem);

        if (contador == 0) {
            printf("Não foi encontrado nenhum estudante cujo aniversário fosse na Quaresma em %hd.\n", ano);
        }
        printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
        pressione_enter();

        printf("\nQuer inserir um ano diferente? (S/N): ");
        if(!sim_nao()) return;
    } while(1);
}

/* Lista alunos em risco de prescrição
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Permite exportar para ficheiro
 * @note Implementa paginação na listagem
 */
void prescrito(Uni * bd) {
    int indice = 0;
    short contador = 0;
    char formato[MAX_FORMATO]; 
    FILE * listagem = NULL;
    char separador;
    limpar_terminal();

    //Listagem
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
    if (listagem) fclose(listagem);

    if (contador == 0) {
            printf("Não foi encontrado nenhum estudante em risco de prescrição.\n");
    }
    printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
    pressione_enter();
}

/* Lista alunos finalistas
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Permite exportar para ficheiro
 * @note Implementa paginação na listagem
 * @note Total de finalistas fica em bd->stats.finalistas
 */
void finalistas(Uni * bd) {
    int indice = 0;
    short contador = 0;
    char formato[MAX_FORMATO]; 
    FILE * listagem = NULL;
    char separador;
    limpar_terminal();
    
    //Listagem
    listagem = pedir_listagem(formato);
    separador = obter_separador(listagem, formato);

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
    if (listagem) fclose(listagem);

    if (contador == 0) {
        printf("Não foram encontrados alunos finalistas.\n");
    }
    printf("\n---------------------FIM DE LISTAGEM---------------------\n\n");
    pressione_enter();

}

/* Lista estudantes por intervalo de datas e nacionalidade
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return void
 *         
 * @note Usa procurar_nacionalidades()
 * @note Valida data via validar_data_entre_intervalo()
 * @note Permite exportar para ficheiro
 * @note Implementa paginação na listagem
 */
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

    //Listagem
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

    //Pedir as nacionalidades
    nacionalidades = procurar_nacionalidades(bd, n_nacionalidades, NULL);
    if (!nacionalidades) {
        pressione_enter();
        return;
    }

    do {
        //Data inferior
        do {
            char * data = NULL;
            printf("Insira a data inferior (DD-MM-AAAA): ");
            data = ler_linha_txt(stdin, NULL);
            if (!data) continue;

            ler_data(&inf, data, '1');
            free(data); //Libertamos porque temos a certeza que != NULL
        } while(inf.dia == 0);
        //Data superior
        do {
            char * data = NULL;
            printf("Insira a data superior (DD-MM-AAAA): ");
            data = ler_linha_txt(stdin, NULL);
            if (!data) continue;

            ler_data(&sup, data, '1');
            free(data);
        } while(sup.dia == 0);
    } while(comparar_data(sup, inf, '0') == -1); //Apenas saímos se sup >= inf

    //Listar
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

/* Remove espaços extra de uma string
 *
 * @param str    String a modificar (se necessário)
 *
 * @return void
 *         
 * @note Remove espaços:
 *       - No início
 *       - No fim  
 *       - Múltiplos entre palavras
 * @note Preserva espaço único entre palavras
 * @note Verifica string NULL/vazia
 */
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

    //Colocamos o nul char no final
    *(fim + 1) = '\0';
}

/* Separa linha em parâmetros usando SEPARADOR
 *
 * @param linha            String com linha completa a separar
 * @param parametros       Array de ponteiros para armazenar os parâmetros extraídos
 * @param num_parametros   Ponteiro para contar os parâmetros
 *
 * @return void
 *         
 * @note Remove espaços extra via remover_espacos()
 * @note Aloca memória para cada parâmetro
 * @note Em caso de erro:
 *       - Define num_parametros = 0
 *       - Liberta memória já alocada
 */
void separar_parametros(const char * linha, char ** parametros, int * num_parametros) { // char ** parametros serve para armazenar os ponteiros dos parametros, de modo a que não sejam perdidos
    if(linha == NULL || parametros == NULL || num_parametros == NULL) return;
    char * inicio = strdup(linha); //Ponteiro para o inicio da linha
    //!!NOTA IMPORTANTE - É feita uma cópia da linha pois caso contrário, caso a função tenha um erro em qualquer parte, a linha original será perdida!!
    char * fim = NULL;
    //Não colocamos *num_parametros = 0 pois esta função poderá ser chamada várias vezes numa linha, e não queremos alterar a var nesse caso
    int indice = 0; //Indice do array

    while(*inicio != '\0') { //Se não for o fim da linha entramos no loop
        fim = inicio; 

        //Vamos veriricar se o ponteiro atual de fim é um separador ou o fim da linha, caso não seja avançamos
        while(*fim != SEPARADOR && *fim != '\0' && *fim != '\n') fim++;
        //Aqui fim está a apontar para o separador, o fim da linha ou um \n (se bem que neste caso \n é o fim da linha)
        char temp = *fim; //Armazena o tab ou o nul char
        *fim = '\0'; //vai terminar a string de inicio (ou seja, um parâmetro); também corta o \n aqui, se exitir
        remover_espacos(inicio);

        if (*inicio != '\0') { //Se o inicio não for o fim da linha, então temos um parametro
            //Alocamos memória para o parametro e copia o conteúdo
            parametros[indice] = malloc(strlen(inicio) + 1); //Lembrar que parâmetros recebe um ponteiro
            if (parametros[indice] != NULL) {
                strcpy(parametros[indice], inicio); //inicio é copiado, logo não necessita de free() (apenas em carregar_dados)
                indice++;
                (*num_parametros)++;
            }
            else {
                //ERRO!!
                *num_parametros = 0; //Usamos num_parametros como uma flag para evitar que a linha seja lida
                //Talvez fosse bom encontrar uma forma diferente de fazer isso sem levar o user a erro
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

/* Limpa buffer do teclado
 *
 * @return void
 *         
 * @note Lê caracteres até encontrar \n ou EOF
 * @note Usar apenas se houver certeza de que há dados no buffer
 */
void limpar_buffer() {
    int lixo;
    while ((lixo = getchar()) != '\n' && lixo != EOF);
}

/* Verifica e limpa buffer do teclado
 *
 * @return int  1 se buffer só tem \n, 0 se tiver outros caracteres
 *         
 * @note Verifica primeiro caractere no buffer
 * @note Limpa sempre todo o buffer
 * @note Usar apenas se houver certeza de que há dados no buffer
 */
int verificar_e_limpar_buffer() {
    //Verificar o caracter que está no buffer
    char lixo = getchar();
    //Se não for o \n, ou seja, a entrada foi inválida
    if (lixo != '\n') {
        limpar_buffer(); //limpamos o buffer (pois ainda há o enter, pelo menos)
        return 0;
    }
    //Se estamos aqui, então o caracter é o \n, e já foi lido, logo não precisamos fazer nada
    return 1;
}

/* Limpa o terminal
 *
 * @return void
 *         
 * @note Multi-plataforma (Windows/Unix)
 */
void limpar_terminal() {
    #ifdef _WIN32 //_WIN32 é uma variável local automaticamente pre-definida pelo windows em todos os sistemas
        system("cls"); //Sistemas windows
    #else
        system("clear"); //Sistemas linux, macos, etc
    #endif
}

/* Implementa paginação em listagens
 *
 * @param contador    Ponteiro para contador de itens listados
 *
 * @return void
 *         
 * @note Pausa a cada PAUSA_LISTAGEM itens
 * @note Incrementa contador
 * @note Ignora se contador == NULL
 */
void pausa_listagem(short * contador) {
    if (!contador) return;

    if (*contador > 0 && *contador % PAUSA_LISTAGEM == 0) {
        pressione_enter();
    }
    (*contador)++;
}

/* Pede ao utilizador para pressionar Enter
 *
 * @return void
 *         
 * @note Limpa buffer se necessário
 * @note Bloqueia até receber \n
 */
void pressione_enter() {
    printf("Pressione Enter para continuar.\n");
    while (getchar() != '\n'); //Caso o user escreva algo e não enter
}

/* Coloca terminal para UTF-8
 *
 * @return void
 *         
 * @note Windows: CP_UTF8 + locale pt para números
 * @note Unix: locale para tudo
 * @note Avisa se configuração falhar
 */
void colocar_terminal_utf8() {
    #ifdef _WIN32
        //SetConsoleOutputCP retorna 0 se houver um erro
        if ((SetConsoleOutputCP(CP_UTF8) == 0)||SetConsoleCP(CP_UTF8) == 0) {
            printf("Ocorreu um erro ao configurar o terminal do Windows para UTF-8.\n");
            printf("A aplicação irá continuar. Desformatação será visível. Para resolver, reinicie a aplicação.\n");
        }
	    setlocale(LC_NUMERIC, "Portuguese"); //Apenas afeta os números , ou seja, muda a notação de floats de '.' para ','
    #else
        setlocale(LC_ALL, "pt_PT.UTF-8");
    #endif
}

/* Verifica e regista primeiro erro de um ficheiro
 *
 * @param erros          Ficheiro de registo de erros
 * @param primeiro_erro  Flag de primeiro erro ('1' = primeiro, ie, ainda não houve)
 * @param nome_ficheiro  Nome do ficheiro com erro
 *
 * @return void
 *         
 * @note Formata cabeçalho de erro
 */
void verificar_primeiro_erro(FILE * erros, char * primeiro_erro, const char * nome_ficheiro) {
    if (*primeiro_erro == '1' && erros) {
        *primeiro_erro = '0';
        fprintf(erros, "\n\n"); 
        fprintf(erros, "==>ERROS AO LER O FICHEIRO %s\n\n", nome_ficheiro);
    }
}

/* Mostra mensagem de uso de backup
 *
 * @return void
 *         
 * @note Pede Enter para continuar
 */
void print_uso_backup() {
    printf("Os dados originais não poderam ser carregados.\n");
	printf("Foi utilizado o último backup efetuado.\n");
    pressione_enter();
}

/* Mostra mensagem de falha ao carregar dados
 *
 * @return void
 *         
 * @note Indica passos de resolução
 * @note Pede Enter para continuar
 */
void print_falha_carregar_dados() {
    printf("Ocorreu um erro fatal ao carregar os dados.\n");
    printf("Por favor, verifique se os ficheiros .txt ou .bin estão na mesma pasta que o programa.\n");
    printf("Em último recurso, verifique se possui alguma cópia de segurança dos dados, e utilize-a.\n");
    printf("O programa será encerrado.\n");
    pressione_enter();
}

/* Mostra mensagem de ficheiro binário alterado
 *
 * @return void
 *         
 * @note Indica uso de backup
 * @note Pede Enter para continuar
 */
void printf_fich_bin_alterado() {
    printf("O ficheiro binário foi alterado desde a última utilização do programa ou está corrompido.\n");
    printf("Como medida preventiva, será utilizado um backup.\n");
    printf("Caso não haja um, será pedido o carregamento dos dados através de ficheiros .txt\n");
    pressione_enter();
}

/* Regista erro ao carregar linha de ficheiro
 *
 * @param erros          Ficheiro de registo de erros
 * @param primeiro_erro  Flag de primeiro erro
 * @param nome_ficheiro  Nome do ficheiro com erro
 * @param erro           Flag de erro encontrado
 * @param n_linhas       Número da linha com erro
 * @param linha          Linha com erro
 *
 * @return void
 *         
 * @note Verifica primeiro erro via verificar_primeiro_erro()
 * @note Define flag erro = '1'
 * @note Não indica a razão do erro
 */
void listar_erro_ao_carregar(FILE * erros, char * primeiro_erro, const char * nome_ficheiro, char * erro, int n_linhas, const char * linha) {
    verificar_primeiro_erro(erros, primeiro_erro, nome_ficheiro);
    fprintf(erros, "Linha %d inválida: %s\n", n_linhas, linha);
    *erro = '1';
}

/* Converte string para inteiro
 *
 * @param str         String a converter
 * @param resultado   Ponteiro para guardar resultado
 *
 * @return int      1 se sucesso, 0 se erro
 *         
 * @note Usa strtol() para conversão
 * @note Valida limites MIN_INT/MAX_INT
 * @note Valida conteúdo externo ao short
 * @note Requer string terminada em \0
 */
int string_para_int(const char * str, int * resultado) {
    char * ptr_fim;
    long valor = strtol(str, &ptr_fim, 10);
    //strtol trunca em caso de overflow, daí o <= abaixo

    //Verificar erros de conversão
    if (ptr_fim == str || *ptr_fim != '\0') {
        return 0;  //Se o ponteiro de fim da str for igual ao ptr do inicio, não há um int; Se for diferente de nul char então também houve erro porque há caracteres indesejados
    }
    
    //Como strtol converte para longs e nós só queremos ints, pode ler números mais altos que o int conseguiria guardar
    if (valor >= INT_MAX || valor <= INT_MIN) {
        return 0; 
    }
    
    *resultado = (int) valor;
    return 1;
}

/* Converte string para short
 *
 * @param str         String a converter
 * @param resultado   Ponteiro para guardar resultado
 *
 * @return int      1 se sucesso, 0 se erro
 *         
 * @note Usa strtol() para conversão 
 * @note Valida limites MIN_SHORT/MAX_SHORT
 * @note Valida conteúdo externo ao int
 * @note Requer string terminada em \0
 */
int string_para_short(const char * str, short * resultado) {
    char * ptr_fim;
    long valor = strtol(str, &ptr_fim, 10);
    
    //Verificar erros de conversão
    if (ptr_fim == str || *ptr_fim != '\0') {
        return 0; 
    }
    //Verificar limites
    if (valor >= SHRT_MAX || valor <= SHRT_MIN) {
        return 0; 
    }
    
    *resultado = (short) valor;
    return 1;
}

/* Converte string para float
 *
 * @param str         String a converter
 * @param resultado   Ponteiro para guardar resultado
 *
 * @return int      1 se sucesso, 0 se erro
 *         
 * @note Usa strtof() para conversão
 * @note Não precisa validar limites (strtof já o faz)
 * @note Valida conteúdo externo ao float
 * @note Requer string terminada em \0
 */
int string_para_float(const char * str, float * resultado) {
    char * ptr_fim;
    float valor = strtof(str, &ptr_fim);
    
    //Verificar erros de conversão
    if (ptr_fim == str || *ptr_fim != '\0') {
        return 0; 
    }
    //Já não há necessidade de verificar limites pois converte diretamente para float
    *resultado = (float) valor;
    return 1;
}

/* Calcula dia da semana
 *
 * @param dia   Dia do mês (1-31)
 * @param mes   Mês do ano (1-12) 
 * @param ano   Ano completo (ex: 2024)
 *
 * @return short  0=sábado até 6=sexta
 *         
 * @note Usa fórmula de Zeller (https://www.geeksforgeeks.org/zellers-congruence-find-day-date/)
 */
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

/* Calcula data do Domingo de Páscoa
 *
 * @param ano   Ano que queremos saber a data da Páscoa
 *
 * @return Data  Struct com dia/mês/ano da Páscoa
 *         
 * @note Usa algoritmo de Meeus/Jones/Butcher
 * @note Referência: https://www.matematica.pt/en/faq/calculate-easter-day.php
 */
Data calcular_domingo_pascoa(int ano) {
    //int porque os números podem ficar muito grandes e estourar com short
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

/* Calcula data da Quarta-feira de Cinzas
 *
 * @param pascoa    Data do Domingo de Páscoa
 *
 * @return Data     Data da Quarta-feira de Cinzas
 */
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

/* Calcula período da Quaresma
 *
 * @param ano      Ano para cálculo
 * @param inicio   Ponteiro para data início (Quarta Cinzas)
 * @param fim      Ponteiro para data fim (Quinta Santa)
 *
 * @return void
 *         
 * @note Usa calcular_domingo_pascoa()
 * @note Usa calcular_quarta_feira_cinzas()
 * @note Ajusta fim para Quinta-feira Santa (-3 dias)
 */
void calcular_quaresma(int ano, Data * inicio, Data * fim) {
    if (!inicio || !fim) return;

    //Calcular data inicial e final + 3
    *fim = calcular_domingo_pascoa(ano);
    *inicio = calcular_quarta_feira_cinzas(*fim);

    //Quaresma acaba na quinta feira santa, logo temos de subtrair 3(domingo, sábado, sexta -> quinta)
    fim->dia -=3;

    //Ajustar data
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

/* Calcula checksum dos dados
 *
 * @param bd    Ponteiro para estrutura da universidade
 *
 * @return unsigned long    Valor do checksum
 *         
 * @note Processa dados pessoais, escolares, stats e autosaveON
 * @note Inclui ainda o tamanho de cada string
 */
unsigned long calcular_checksum(Uni * bd) {
    unsigned long soma = 0;
    
    //Configs
    soma += (unsigned long) autosaveON; //'1' passa a 49 decimal (ASCII)

    //Dados dos arrays
    soma += bd->tamanho_aluno;
    soma += bd->capacidade_aluno;
    soma += bd->tamanho_escolares; 
    soma += bd->capacidade_escolares;

    //Aluno
    for(int i = 0; i < bd->tamanho_aluno; i++) {
        soma += bd->aluno[i].codigo;
        soma += bd->aluno[i].nascimento.dia;
        soma += bd->aluno[i].nascimento.mes;
        soma += bd->aluno[i].nascimento.ano;
        
        //Nome
        size_t tamanho_nome = strlen(bd->aluno[i].nome); //Não inclui '\0'
        soma += tamanho_nome + 1; //'\0'

        //Nota: '\0' está incluído pois j começa em 0
        for(size_t j = 0; j < tamanho_nome; j++) { //size_t porque strlen retorna size_t
            soma += (unsigned long) bd->aluno[i].nome[j];
        }
        
        //Nacionalidade
        size_t tamanho_nacionalidade = strlen(bd->aluno[i].nacionalidade);
        soma += tamanho_nacionalidade + 1;

        for(size_t j = 0; j < strlen(bd->aluno[i].nacionalidade); j++) { 
            soma += (unsigned long) bd->aluno[i].nacionalidade[j];
        }
    }

    //Escolares
    for(int i = 0; i < bd->tamanho_escolares; i++) {
        soma += bd->escolares[i].codigo;
        soma += bd->escolares[i].matriculas;
        soma += bd->escolares[i].ects;
        soma += bd->escolares[i].ano_atual;
        soma += (unsigned long) (bd->escolares[i].media_atual * 10); //como é float com 1 casa decimal multiplicamos por 10 para fizer inteiro
        soma += (unsigned long) bd->escolares[i].prescrever;
        soma += (unsigned long) bd->escolares[i].finalista;
    }
    //Estatísticas
    soma += (unsigned long) bd->stats.atualizado;
    soma += bd->stats.finalistas;
    soma += bd->stats.media;
    soma += bd->stats.media_matriculas;
    soma += bd->stats.risco_prescrever;

    return soma;
}

/* Pede confirmação S/N ao utilizador
 *
 * @return int    1 se S/s, 0 se N/n
 *         
 * @note Case insensitive
 * @note Limpa buffer
 * @note Só retorna se obter resposta
 */
int sim_nao() {
    char opcao;
    do {
        scanf(" %c", &opcao);
        //Verificar entradas inválidas
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
        printf("\nOpção inválida. Use 'S' ou 'N': ");
    } while (1);
}

/* Pede o código do aluno
 *
 * @param codigo    Ponteiro para guardar código
 *
 * @return void
 *         
 * @note Limpa terminal
 * @note Valida entre 1-MAX_CODIGO
 * @note 0 para cancelar
 */
void pedir_codigo(int * codigo) {
    *codigo = 0;
    char * input = NULL;
    do {
        //Usamos ler_linha_txt para evitar overflows
        printf("Insira o código do estudante (ou '0' para sair): ");
        input = ler_linha_txt(stdin, NULL);
        if (!input) continue;

        if (!string_para_int(input, codigo)) {
            printf("Código inválido! Insira um número inteiro positivo.\n");
            pressione_enter();
            free(input);
            continue;
        }
        free(input);
        
        if (*codigo < 0) {
            printf("Código inválido! O código não pode ser negativo.\n");
            continue;
        }
        break;
    } while(1);
}

/* Determina separador para ficheiro de listagem
 *
 * @param ficheiro    Ponteiro FILE ou NULL
 * @param formato     Extensão do ficheiro
 *
 * @return char       '\t' para TXT, ',' para CSV, '\0' se !ficheiro
 *         
 * @note Escreve o cabeçalho no ficheiro de listagem
 */
char obter_separador(FILE * ficheiro, char * formato) {
    char separador = '\0'; 
    if(ficheiro) {
        if (strcmp(formato, ".txt") == 0) separador = '\t';
        else if (strcmp(formato, ".csv") == 0) separador = ',';
        //Printar cabeçalho do ficheiro de listagem.
        fprintf(ficheiro, "Código%cNome%cData de Nascimento%cNacionalidade%cMatrículas%cECTS%cAno de curso%cMédia\n", separador, separador, separador, separador,
            separador, separador, separador); 
    }
    return separador;
}

/* Calcula idade baseada na data atual
 *
 * @param nascimento    Data de nascimento
 *
 * @return short        Idade calculada
 *         
 * @note Usa DATA_ATUAL global
 */
short calcular_idade(Data nascimento) {
    short idade = DATA_ATUAL.ano - nascimento.ano;
    //Ainda não passou o mês logo a idade está +1
    if (DATA_ATUAL.mes < nascimento.mes) return (idade - 1);
    //Já passou o mês logo já fizeram anos
    else if (DATA_ATUAL.mes > nascimento.mes) return idade;
    //Meses iguais:
    if (DATA_ATUAL.dia >= nascimento.dia) return idade; //Ou é o aniverário ou já passou
    return (idade - 1);
}

/* Normaliza string (minúsculas sem acentos)
 *
 * @param str    String original
 *
 * @return char* Nova string normalizada (alocar free)
 *         
 * @note Faz cópia da string
 * @note Remove acentos e ç (NÃO FUNCIONA)
 * @note Converte para minúsculas
 * @note Retorna NULL se erro
 */
char * normalizar_string(char * str) {
    /*
    !Não funciona:
    -O objetivo desta parte seria colocar tudo a minúsculas (funciona) e alterar todos os acentos ou 'ç' para caracteres normais
    -Nem usando os valores hexadecimais foi possível
    -Seria possível fazer isto funcionar, possivelmente usando a biblioteca <iconv.h>, mas por falta de tempo não foi implementado
    -Apesar disso, a função funciona sem erros, e as procuras também, no entanto, estão mais condicionadas
    */
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
    //Não damos free em str pois pode ser um dado original/dos arrays
    if (!resultado) return NULL;
    //Colocar tudo a minúsculas.
    strlwr(resultado);

    for (int i = 0; resultado[i]; i++) {
        char * acento = strchr(acentuados, resultado[i]);
        if (acento) {
            int pos = acento - acentuados;
            resultado[i] = sem_acentos[pos];
        }
    }
    
    return resultado;
}

/* Atualiza a data atual global
 *
 * @return void
 *         
 * @note Usa time.h para data do sistema
 * @note Ajusta mês de 0-11 para 1-12
 * @note Ajusta ano desde 1900
 * @note Referência: //https://www.geeksforgeeks.org/time-h-header-file-in-c-with-examples/
 */
void data_atual() {
    //Obter a data atual usando time.h
    time_t t = time(NULL);
    struct tm * tm_atual = localtime(&t);
    
    //Passar para a variável global
    DATA_ATUAL.dia = tm_atual->tm_mday;
    DATA_ATUAL.mes = tm_atual->tm_mon + 1; //tm_mon vai de 0-11
    DATA_ATUAL.ano = tm_atual->tm_year + 1900;
}
