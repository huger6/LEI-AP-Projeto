# 📚 GESTÃO UNIVERSITÁRIA - Sistema de Informação (Versão A)

**Nota obtida:** 19 valores  
**Curso:** Engenharia Informática<br>
**UC:** <a href="https://github.com/huger6/LEI-AP" target="_blank">Algoritmos e Programação – ESTGV 2024/2025</a>

> ℹ️ Observação: A estrutura dos ficheiros não está idealmente organizada, dado tratar-se do **primeiro projeto universitário** dos autores.

---

## 📖 Descrição
Sistema integrado para gestão de dados académicos, desenvolvido em **C**.  
O software permite gerir informações de estudantes universitários, incluindo **dados pessoais** e **situação escolar**.

---

## 🚀 Funcionalidades Principais

### 1. Gestão de Estudantes
- Inserir novos estudantes  
- Eliminar estudantes  
- Pesquisar por nome/código  
- Listar por diversos critérios  

### 2. Dados Académicos
- Registo de matrículas  
- Controlo de ECTS  
- Cálculo de médias e anos curriculares  
- Identificação de finalistas  
- Monitorização de risco de prescrição  

### 3. Estatísticas
- Médias por nacionalidade  
- Distribuição por escalões etários  
- Análise de performance académica  
- Número de finalistas  
- Estudantes em risco  

### 4. Gestão de Ficheiros
- Suporte para **TXT** e **CSV**  
- Backup automático  
- Autosave configurável  
- Log de erros detalhado  

---

## ⚙️ Requisitos Técnicos
- **Sistemas operativos suportados:** Windows / Unix  
- **Compilador C compatível:** Preferência para **C23** (GCC 13 ou superior)  
- **Espaço em disco:** ≥ 1MB  
- **Suporte a UTF-8**

### 🔧 Compilação
- **Windows 11 Home 23H2 (64 bits):**  
  ```bash
  gcc -Wall -Wextra -g -O0 -std=c23 -o (FILENAME) main.c funcoes.c
  ```
- **Linux (Ubuntu 20.04.6 LTS)**:
  ```bash
  gcc -std=c2x -Wall -Wextra -o (FILENAME) main.c funcoes.c -D_XOPEN_SOURCE=700
  ```

---

# 📂 Ficheiros do Sistema
- `dados.txt` → Informações pessoais  
- `situacao_Escolar_Estudantes.txt` → Dados académicos  
- `erros.txt` → Log de erros  
- `config.txt` → Configurações  
- `logs.bin` → Dados binários  
- `*_backup.*` → Ficheiros de backup  

---

## 📝 Notas de Utilização
1. Na **primeira execução**, os dados são carregados a partir dos ficheiros `.txt`  
2. Nas execuções seguintes, utilizam-se os ficheiros binários  
3. Backups são criados automaticamente  
4. Erros de carregamento ficam registados em `erros.txt`  
5. Interface baseada em menus simples e intuitivos  

---

## 👥 Autores
- **Hugo Afonso** (30032)  
- **Mateus Silva** (29989)  
