# 📦 Sistema de Almoxarifado em Linha de Comando (C)

Aplicação desenvolvida em **Linguagem C** para controle e registro de estoque de almoxarifado via terminal. Suporta comandos diretos e modo shell interativo contínuo, com persistência automática em arquivo **JSON**.

---

## 🚀 Funcionalidades

- **Soma automática de quantidades (`adicionar`)**:
  - Se o item já existir no estoque, soma a nova quantidade ao saldo anterior.
  - Se for um item novo, cadastra-o automaticamente.
- **Baixa no estoque (`remover`)**:
  - Subtrai a quantidade especificada.
  - Proteção contra estoque negativo (alerta se a quantidade solicitada for maior que o saldo disponível).
- **Tabela formatada de estoque (`listar`)**:
  - Exibe todos os itens, quantidades e indicadores de status (`OK`, `BAIXO`, `ZERADO`).
  - Total de itens cadastrados e contagem total de unidades em estoque.
- **Busca rápida (`consultar`)**:
  - Localiza itens pelo nome ou busca parcial (insensível a maiúsculas/minúsculas).
- **Histórico completo de movimentações (`historico`)**:
  - Registra data, hora, tipo de operação (`ADICIONAR` ou `REMOVER`), quantidade, saldo anterior e novo saldo.
- **Persistência em JSON (`almoxarifado.json`)**:
  - Todos os dados são salvos imediatamente a cada operação.
- **Suporte completo a nomes com espaços**:
  - Suporta tanto aspas (`adicionar "Cabo de Rede Cat6" 100`) quanto sem aspas (`adicionar Cabo de Rede Cat6 100`).
- **Cores ANSI e suporte UTF-8**:
  - Visual moderno e legível no PowerShell, Windows Terminal e CMD.

---

## 🛠️ Como Compilar

### Opção 1: Usando o script `build.bat` (Mais rápido no Windows)
Basta clicar duas vezes no arquivo `build.bat` ou executar no terminal:
```cmd
build.bat
```

### Opção 2: Usando o GCC diretamente
```bash
gcc -std=c99 -Wall -Wextra -O2 main.c almoxarifado.c -o almox.exe
```

### Opção 3: Usando Make
```bash
make
```

---

## 💻 Como Usar

A aplicação funciona em **dois modos**:

### 1. Modo Interativo (Shell Contínuo)
Ao executar sem argumentos, o terminal abre o prompt `almox> `:
```cmd
.\almox.exe
```

Dentro do prompt interativo, digite os comandos:
```text
almox> adicionar Parafuso Sextavado 100
almox> adicionar Porca M8 200
almox> adicionar Parafuso Sextavado 50
almox> remover Porca M8 30
almox> listar
almox> consultar parafuso
almox> historico
almox> sair
```

### 2. Modo Linha de Comando Direta (CLI)
Você também pode executar comandos diretamente em scripts ou no terminal:

```cmd
# Adicionar unidades
.\almox.exe adicionar Parafuso 100
.\almox.exe adicionar "Cabo de Rede Cat6" 50

# Remover unidades
.\almox.exe remover Parafuso 20
.\almox.exe remover "Cabo de Rede Cat6" 10

# Listar todo o estoque
.\almox.exe listar

# Consultar um item específico
.\almox.exe consultar Parafuso

# Visualizar o histórico de movimentações
.\almox.exe historico
.\almox.exe historico 10

# Ajuda
.\almox.exe ajuda
```

---

## 📁 Estrutura de Arquivos

```
almox/
├── almoxarifado.h      # Definições de estruturas e protótipos de funções
├── almoxarifado.c      # Lógica de negócio, serialização JSON e tabelas
├── main.c              # Ponto de entrada (CLI e Shell interativo)
├── almoxarifado.json   # Arquivo gerado automaticamente com os dados
├── build.bat           # Script de compilação automatizado para Windows
├── Makefile            # Makefile padrão para GCC / MinGW
└── README.md           # Documentação completa
```

---

## 📄 Exemplo do Arquivo JSON (`almoxarifado.json`)

```json
{
  "itens": [
    {
      "nome": "Parafuso Sextavado",
      "quantidade": 150
    },
    {
      "nome": "Porca M8",
      "quantidade": 170
    }
  ],
  "historico": [
    {
      "data_hora": "2026-10-13 21:55:00",
      "operacao": "ADICIONAR",
      "nome": "Parafuso Sextavado",
      "quantidade": 100,
      "saldo_anterior": 0,
      "saldo_novo": 100
    },
    {
      "data_hora": "2026-10-13 21:55:10",
      "operacao": "ADICIONAR",
      "nome": "Parafuso Sextavado",
      "quantidade": 50,
      "saldo_anterior": 100,
      "saldo_novo": 150
    }
  ]
}
```
