//Código protótipo gerado pelo chatgpt para tentar evitar repetições na escrita do menu
//Observações: parece-me ter ligeiramente menos linhas, o que não sei até que ponto ser menos benéfico devido à complexidade acrescida
//Também não tem a VALIDAÇÃO DOS INPUTS!!!


/**
 * Exibe e controla um menu genérico com possibilidade de retorno.
 * @param titulo: Título do menu.
 * @param opcoes: Array de strings com as opções do menu.
 * @param num_opcoes: Número total de opções no menu.
 * @param limInf: Limite inferior para as opções válidas.
 * @param limSup: Limite superior para as opções válidas.
 * @return: Carácter da opção escolhida.
 */
char menu_generico(const char *titulo, const char *opcoes[], size_t num_opcoes, char limInf, char limSup) {
    char opcao = '0';  // Opção escolhida
    short valido = 0;  // Validação de input

    do {
        limpar_terminal();  // Limpa o terminal antes de mostrar o menu
        printf("\t\t%s\n\n", titulo);

        // Exibir todas as opções
        for (size_t i = 0; i < num_opcoes; i++) {
            printf("%zu - %s\n", i, opcoes[i]);
        }

        printf("\n\n\tOpção: ");
        valido = scanf(" %c", &opcao);

        // Reutilizar a validação existente
        validacao_menus(&valido, opcao, limInf, limSup);

        // Se o input for válido, retornamos a opção
        if (valido == 1) {
            return opcao;
        }
    } while (1);  // Permanece no menu até o utilizador inserir algo válido

    return '0';  // Valor de retorno padrão (não deve ser alcançado)
}

void menu_principal_controller() {
    char escolha;

    do {
        // Opções do menu principal
        char *opcoes_principal[] = {
            "Gerir estudantes",
            "Consultar dados",
            "Estatísticas",
            "Extras",
            "Sair do programa"
        };

        // Chama o menu principal
        escolha = menu_generico("MENU PRINCIPAL", opcoes_principal, 5, '0', '4');

        switch (escolha) {
            case '0': // Sair do programa
                printf("A sair...\n");
                break;
            case '1': // Gerir estudantes
                menu_gerir_estudantes_controller();
                break;
            case '2': // Consultar dados
                menu_consultar_dados_controller();
                break;
            case '3': // Estatísticas
                menu_estatisticas_controller();
                break;
            case '4': // Extras
                menu_extras_controller();
                break;
            default:
                printf("Erro inesperado no menu principal.\n");
                break;
        }
    } while (escolha != '0');  // Loop até o utilizador sair
}

void menu_gerir_estudantes_controller() {
    char escolha;

    do {
        // Opções do menu de gerir estudantes
        char *opcoes_gerir[] = {
            "Inserir estudante",
            "Eliminar estudante",
            "Atualizar dados do estudante",
            "Voltar ao menu anterior"
        };

        // Chama o menu genérico
        escolha = menu_generico("GERIR ESTUDANTES", opcoes_gerir, 4, '0', '3');

        switch (escolha) {
            case '0': // Voltar ao menu principal
                return;
            case '1': // Inserir estudante
                inserir_estudante();  // Implementar esta função
                break;
            case '2': // Eliminar estudante
                eliminar_estudante();  // Implementar esta função
                break;
            case '3': // Atualizar dados do estudante
                atualizar_estudante();  // Implementar esta função
                break;
            default:
                printf("Erro inesperado no menu de gerir estudantes.\n");
                break;
        }
    } while (1);  // Loop até o utilizador escolher voltar
}


void menu_consultar_dados_controller() {
    char escolha;

    do {
        // Opções do menu de consultar dados
        char *opcoes_consultar[] = {
            "Procurar estudante por nome",
            "Listar estudantes por intervalo de datas de nascimento",
            "Listar estudantes por nacionalidade",
            "Listar estudantes por ordem alfabética do apelido",
            "Voltar ao menu anterior"
        };

        // Chama o menu genérico
        escolha = menu_generico("CONSULTAR DADOS", opcoes_consultar, 5, '0', '4');

        switch (escolha) {
            case '0': // Voltar ao menu principal
                return;
            case '1': // Procurar estudante por nome
                procurar_estudante_nome();  // Implementar esta função
                break;
            case '2': // Listar estudantes por intervalo de datas
                listar_estudantes_datas();  // Implementar esta função
                break;
            case '3': // Listar estudantes por nacionalidade
                listar_estudantes_nacionalidade();  // Implementar esta função
                break;
            case '4': // Listar estudantes por apelido
                listar_estudantes_apelido();  // Implementar esta função
                break;
            default:
                printf("Erro inesperado no menu de consultar dados.\n");
                break;
        }
    } while (1);  // Loop até o utilizador escolher voltar
}

void menu_estatisticas_controller() {
    char escolha;

    do {
        // Opções do menu de estatísticas
        char *opcoes_estatisticas[] = {
            "Contar estudantes por escalão de média atual",
            "Calcular número médio de matrículas",
            "Determinar número de finalistas",
            "Calcular média de idades por nacionalidade e ano",
            "Listar estudantes em risco de prescrição",
            "Voltar ao menu anterior"
        };

        // Chama o menu genérico
        escolha = menu_generico("ESTATÍSTICAS", opcoes_estatisticas, 6, '0', '5');

        switch (escolha) {
            case '0': // Voltar ao menu principal
                return;
            case '1': // Contar estudantes por escalão de média
                contar_estudantes_media();  // Implementar esta função
                break;
            case '2': // Calcular número médio de matrículas
                calcular_media_matriculas();  // Implementar esta função
                break;
            case '3': // Determinar número de finalistas
                determinar_finalistas();  // Implementar esta função
                break;
            case '4': // Calcular média de idades
                calcular_media_idades();  // Implementar esta função
                break;
            case '5': // Listar estudantes em risco de prescrição
                listar_risco_prescricao();  // Implementar esta função
                break;
            default:
                printf("Erro inesperado no menu de estatísticas.\n");
                break;
        }
    } while (1);  // Loop até o utilizador escolher voltar
}

void menu_extras_controller() {
    char escolha;

    do {
        // Opções do menu de extras
        char *opcoes_extras[] = {
            "Listar estudantes nascidos em dias específicos da semana",
            "Listar estudantes com aniversários ao domingo num ano específico",
            "Relacionar o ano de inscrição com intervalos das classificações",
            "Voltar ao menu anterior"
        };

        // Chama o menu genérico
        escolha = menu_generico("EXTRAS", opcoes_extras, 4, '0', '3');

        switch (escolha) {
            case '0': // Voltar ao menu principal
                return;
            case '1': // Listar estudantes nascidos em dias específicos
                listar_estudantes_dias_semana();  // Implementar esta função
                break;
            case '2': // Listar estudantes com aniversários ao domingo
                listar_aniversarios_domingo();  // Implementar esta função
                break;
            case '3': // Relacionar ano de inscrição
                relacionar_ano_classificacao();  // Implementar esta função
                break;
            default:
            //