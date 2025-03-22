GESTÃO UNIVERSITÁRIA - Sistema de Informação (Versão A)
===========================================

Descrição
---------
Sistema integrado para gestão de dados académicos desenvolvido em C. 
Permite gerir informações de estudantes universitários, incluindo dados pessoais e situação escolar.

Funcionalidades Principais
-------------------------
1. Gestão de Estudantes
   - Inserir novos estudantes
   - Eliminar estudantes
   - Pesquisar por nome/código
   - Listar por diversos critérios

2. Dados Académicos
   - Registo de matrículas
   - Controlo de ECTS
   - Médias e anos curriculares
   - Identificação de finalistas
   - Monitorização de risco de prescrição

3. Estatísticas
   - Médias por nacionalidade
   - Distribuição por escalões etários
   - Análise de performance académica
   - Número de finalistas
   - Estudantes em risco

4. Gestão de Ficheiros
   - Suporte para TXT e CSV
   - Backup automático
   - Autosave configurável
   - Log de erros detalhado

Requisitos Técnicos
------------------
- Sistema operativo: Windows/Unix
- Programa em Linux não carrega os dados corretamente (ver fix em https://github.com/huger6/TrabalhoAP)
- Compilador C compatível (De preferência C23 com o GCC13)
- Compilado como "gcc -Wall -Wextra -g -O0 -std=c23 -o (**FILENAME**) main.c funcoes.c" em Windows 11 Home 23H2 (64 bits)
- Compilado como "gcc -std=c2x -Wall -Wextra -o (**FILENAME**) main.c funcoes.c -D_XOPEN_SOURCE=700" em Linux (testado para Ubuntu 20.04.6 LTS)
- Suporte a UTF-8
- 1MB espaço em disco (mínimo)

Ficheiros do Sistema
-------------------
- dados.txt: Informações pessoais
- situacao_Escolar_Estudantes.txt: Dados académicos
- erros.txt: Log de erros
- config.txt: Configurações
- logs.bin: Dados binários
- *_backup.*: Ficheiros de backup

Notas de Utilização
------------------
1. Primeira execução carrega dados de .txt
2. Execuções seguintes usam dados binários
3. Backups são criados automaticamente
4. Erros de carregamento são registados em erros.txt
5. Interface oferece menus intuitivos

Autores
-------
Hugo Afonso 30032
Mateus Silva 29989

Desenvolvido para Algoritmos e Programação
ESTGV 24/25
