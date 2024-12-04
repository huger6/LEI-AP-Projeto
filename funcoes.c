#include "headers.h"

const short ANO_ATUAL = 2025; 
const short ANO_NASC_LIM_INF = 1908;

//Funções

void remover_espacos(char * str) {
    char * inicio = str;
    char * fim = NULL;

    //Se o início conter um espaço, vai avançar o ponteiro uma casa, até essa casa deixar de ser um espaço.
    while (*inicio == ' ') {
        inicio++;
    }

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


void separar_parametros(char * linha) {
    char * inicio = linha; //Ponteiro para o inicio da linha
    char * fim == NULL;

    while(*inicio != '\0') { //Se não for o fim da linha entramos no loop
        fim = inicio; //

        //Vamos veriricar se o ponteiro atual de fim é um separador ou o fim da linha, caso não seja avançamos
        while(*fim != SEPARADOR && *fim != '\0') fim++;
        //Se estamos aqui, é porque ou estamos num parametro ou a linha acabou
        char temp = *fim;
        *fim = '\0';

        remover_espacos(inicio);
        //TODO
        if (inicio != '\0')


    }
}

//Linha é alocada dinamicamente, pelo que deve ser libertada quando já não for necessária.
char * ler_linha(FILE * ficheiro, int * n_linhas) {
    //n_linhas não será inicializado aqui
    char buffer[TAMANHO_INICIAL_BUFFER]; //Buffer para armazenar parte da linha
    size_t tamanho_total = 0; //Comprimento da linha; size_t pois é sempre >0 e evita conversões que podem levar a erros com outras funções
    char * linha = NULL;

    while (fgets(buffer, sizeof(buffer), ficheiro)) { //fgets le ate buffer -1 caracteres ou \n ou EOF
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
        if (buffer[tamanho - 1] == '\n') break; //se tudo tiver sido copiado, o ultimo caracter do buffer(e da linha tbm) será o '\n'

        if (linha == NULL && feof(ficheiro)) {
            printf("Erro ao ler o ficheiro.\n")
            return NULL;
        }

        (*n_linhas)++;
        return linha; 
    }

}
FILE * abrir_ficheiro(const char * nome_ficheiro, const char * modo, char * modo_valido) {
    *modo_valido = '0'; //Definir o modo como não válido
    //Definir todos os modos de abertura possíveis
    const char * modos_possíveis = {"r", "w", "a", "r+", "w+", "a+", "rb", "wb", "ab", "r+b", "w+b", "a+b"};
    //Rever este comentário
    int tamanho_modos = sizeof(modos_possíveis) / sizeof(modos_possíveis[0]); //tamanho do array em bytes a dividir pelo tamanho de um dos elementos (são todos iguais pois são ponteiros)

    for(int i = 0; i < tamanho_modos; i++) {
        if (strcmp(modo, modos_possíveis[i]) == 0) {
            *modo_valido = '1';
            break; //já não é necessário avançar no loop
        }
    }
    
    //Esta mensagem de erro é destinada ao programador, afinal é ele que vai escolher o modo de abertura
    if (*modo_valido == '0') {
        printf("Modo de abertura do ficheiro '%s' é inválido.\n", nome_ficheiro);
        return NULL;
    }

    FILE * ficheiro = fopen(nome_ficheiro, modo);
    
    if (ficheiro == NULL) return NULL;

    return ficheiro;
    
}

void carregar_dados(const char * nome_ficheiro, Estudante * aluno, Dados * escolares, Estatisticas * stats) { 
    FILE * dados;
    FILE * situacao_escolar;
    char modo_abertura_valido = '0'; //Poderia ser evitada pela criação de uma struct apenas para erros mas dada a simplicidade do program não é necessário
    int n_linhas = 1;
    char * linha = NULL; //Ponteiro para armazenar uma linha

    //Abrimos os ficheiros para ver
    dados = abrir_ficheiro(DADOS_TXT, "r", modo_abertura_valido);
    situacao_escolar = abrir_ficheiro(SITUACAO_ESCOLAR_TXT, "r", modo_abertura_valido);

    if (dados == NULL || situacao_escolar == NULL) {
        //Se o ponteiro é NULL, o ficheiro não existe, logo não há dados para ler
        fclose(dados); 
        fclose(situacao_escolar);
        return; //Saímos da função
    }
    //Se der NULL em todos os modos de abertura, quer dizer que há um erro crítico. Dar manage
    //Se não der nulo nos outros modos de abertura quer dizer que o ficheiro não existia e foi criado
    for(int i = 0; i < n_linhas; i++ ) {
        linha = ler_linha(dados, n_linhas);
        
    }



    
    
    //Fechamos os ficheiros pois iremos efeturar todas as operações em memória
    fclose(dados);
    fclose(situacao_escolar);
}

void guardar_dados(const char * nome_ficheiro, Estudante * aluno, Dados * escolares, Estatisticas * stats) {
    
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

//Esta função é um pouco "inutil" neste momento. Foi desenhada para tratar erros com ints
void validacao_input_menu(const char limInf, const char limSup) {
    limpar_buffer();  // Limpar o buffer de entrada (o que foi escrito incorretamente)
	printf("Entrada inválida! Introduza um número do menu (%c a %c)\n", limInf, limSup);
	printf("Pressione Enter para continuar.\n");
	if (getchar() != '\n') limpar_buffer(); //Caso o user escreva algo e não enter limpar buffer
	limpar_terminal();
}

void validacao_numero_menu(const char limInf, const char limSup) {
    limpar_buffer();
	printf("Por favor, escolha um número do menu (%c a %c).\n", limInf, limSup);
	printf("Pressione Enter para continuar.\n");
	if (getchar() != '\n') limpar_buffer(); 
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

char validar_data(short dia, short mes, short ano) {
    if (ano < 1 || mes < 1 || mes > 12 || dia < 1) { //Limites dos anos são os que foram considerados mais realistas
        printf("Por favor, insira uma data válida.\n");
        return 0;
    }
    /*Apenas se podem inscrever alunos com mais de 16 anos (sensivelmente, porque devido ao mes de nascimento pode nao ser bem assim)
    Poderia ser resolvido com a adição de +2 constantes para mes e dia atuais mas achamos desnecessário devido a tratar-se da inscrição de alunos
    que por natureza, não têm de ser todos iguais ou ser inscritos com a mesma idade.
    */
    if (ano < ANO_NASC_LIM_INF || ano > ANO_ATUAL - 16) {
        printf("O ano inserido é inválido. Insira um ano entre %d e %d", ANO_NASC_LIM_INF, ANO_ATUAL);
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
        printf("O dia é inválido! O mês de %s tem apenas %hd dias.\n", nome_do_mes[mes - 1], dias_por_mes[mes -1]);
        return 0; //Falso
    }
    //Se não retornou 0(F), é V
    return 1;
}

void ler_data(Estudante * aluno) {
	char data[11]; //Vamos usar o formato DD/MM/AAAA (10 caracteres + \0)
    char erro = '0'; //1 há erro

	do {
        erro = '0';
		printf("Data de nascimento (DD/MM/AAAA): ");
		if(fgets(data, sizeof(data), stdin) ==NULL) { //stdin porque é daí que lemos os dados; //fgets retorna um ponteiro para "data", logo verificamos se é NULL ocorreu um erro
            limpar_buffer();
            printf("Ocorreu um erro ao tentar ler a data de nascimento!");
            erro = '1';
            continue; //O continue faz com que o loop avance para a proxima iteração
            //Nota: o uso do fgets faz com que o buffer nao rebente, pois ele so le ate ao limite de sizeof(data)-1(para o \0), apenas temos de limpar o buffer depois
        }
        //O sscanf lê as x entradas conforme o padrão apresentado
    	if (sscanf(data, "%hd/%hd/%hd", &aluno->nascimento.dia, &aluno->nascimento.mes, &aluno->nascimento.ano) != 3) { //sscanf tenta ler 3 shorts
            //Caso nao consiga, o formato é inválido
            printf("Formato inválido! Use o formato DD/MM/AAAA.\n");
            limpar_buffer();
            erro = '1';
            continue;
        }      
        if ((!validar_data(aluno->nascimento.dia, aluno->nascimento.mes, aluno->nascimento.ano))) { //Se já houver erro, não há necessidade de validar
            limpar_buffer();
            erro = '1'; //A mensagem de erro deve estar dentro de validar, de modo a especificar o que falhou
            continue;
        }
        erro = '0'; //Se nao for abrangido pelos continues, então não foi detetato nenhum erro, logo saimos do loop
        limpar_buffer(); //A entrada pode ter sido válida apesar de ter mais de 11 caracteres (ex: 15/12/2006EXTRA)
	} while (erro == '1'); //Continuar a pedir a data sempre que esta for inválida
}
