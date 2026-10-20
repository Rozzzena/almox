# 📦 Sistema de Almoxarifado em Linha de Comando (C)

Aplicação desenvolvida em **Linguagem C (C99)** para controle e registro de estoque de almoxarifado via terminal. Implementa persistência em **JSON** com modelagem e operações 100% espelhadas no schema **MongoDB** (referência acadêmica/PIM). Suporta comandos diretos (CLI) e modo shell interativo contínuo.

---

## 🎯 Alinhamento com MongoDB (PIM / Banco NoSQL)

As operações da aplicação em C correspondem diretamente às queries do script `almoxarifadopim.mongodb.js`:

| Operação no Sistema (C) | Operação MongoDB Equivalente | Descrição |
| :--- | :--- | :--- |
| `adicionar <item> <qtd> [valor]` | `db.produtos.insertOne(...)` ou `updateOne(..., {$inc: {quantidade: N}})` | Cadastra item novo ou incrementa saldo |
| `remover <item> <qtd>` | `db.produtos.updateOne({quantidade: {$gte: N}}, {$inc: {quantidade: -N}})` | Baixa com trava de estoque negativo |
| `desativar <item>` | `db.produtos.updateOne(..., {$set: {ativo: false}})` | Exclusão lógica (soft delete) |
| `adicionar <item_inativo> <qtd>` | `db.produtos.updateOne(..., {$set: {ativo: true}})` | Reativação automática do item |
| `listar` | `db.produtos.find({ativo: true})` | Exibe apenas itens ativos com subtotais |
| `listar tudo` | `db.produtos.find({})` | Exibe todos os itens, inclusive inativos |
| `consultar <termo>` | `db.produtos.find({ativo: true, nome: /termo/i})` | Busca parcial case-insensitive |
| `relatorio` | `db.produtos.aggregate([{$match: {ativo: true}}, {$group: ...}])` | Soma do valor total e balanço patrimonial |
| `historico [limite]` | Collection de auditoria / log | Rastreabilidade de movimentações |

---

## 🚀 Funcionalidades

- **Soma automática de quantidades (`adicionar`)**:
  - Se o item já existir no estoque, soma a nova quantidade ao saldo anterior.
  - Se for um item novo, cadastra-o automaticamente com código sequencial, data e valor unitário.
  - Se o item estiver inativo, ele é reativado automaticamente.
- **Baixa no estoque (`remover`)**:
  - Subtrai a quantidade especificada.
  - Proteção contra estoque negativo (alerta se a quantidade solicitada for maior que o saldo disponível).
- **Exclusão Lógica (`desativar`)**:
  - Marca o item como `ativo = false`, mantendo o histórico e os dados íntegros.
- **Tabela formatada de estoque (`listar` / `listar tudo`)**:
  - Exibe código, descrição, quantidade, valor unitário, valor subtotal e status (`OK`, `BAIXO`, `ZERADO`, `INATIVO`).
  - Total de unidades e valor patrimonial total em estoque.
- **Relatório Financeiro e Quantitativo (`relatorio`)**:
  - Sumário com itens ativos, inativos, zerados, em ponto de reposição e valor financeiro total.
- **Busca rápida (`consultar`)**:
  - Localiza itens pelo nome ou busca parcial (insensível a maiúsculas/minúsculas).
- **Histórico completo de movimentações (`historico`)**:
  - Registra data, hora, tipo de operação (`ADICIONAR`, `REMOVER` ou `DESATIVAR`), quantidade, saldo anterior e novo saldo.
- **Persistência em JSON (`almoxarifado.json`)**:
  - Todos os dados são salvos imediatamente a cada operação, sem necessidade de drivers ou bibliotecas externas.
- **Cores ANSI e suporte UTF-8**:
  - Visual moderno e legível no PowerShell, Windows Terminal e CMD.

---

## 🛠️ Como Compilar

### Opção 1: Usando o script `build.bat` (Recomendado no Windows)
Execute no terminal ou dê duplo-clique:
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
almox> adicionar Parafuso 100 0.50
almox> adicionar "Cabo de Rede Cat6" 50 4.20
almox> remover Parafuso 25
almox> desativar "Cabo de Rede Cat6"
almox> listar
almox> listar tudo
almox> relatorio
almox> consultar parafuso
almox> historico
almox> sair
```

### 2. Modo Linha de Comando Direta (CLI)
Você também pode executar comandos diretamente em scripts ou no terminal:

```cmd
# Adicionar unidades (com ou sem valor unitário)
.\almox.exe adicionar Parafuso 100 0.50
.\almox.exe adicionar "Luva Nitrilica" 40 15.50

# Remover unidades
.\almox.exe remover Parafuso 20

# Desativar item (exclusão lógica)
.\almox.exe desativar Parafuso

# Listar itens ativos
.\almox.exe listar

# Listar todos os itens (inclusive desativados)
.\almox.exe listar tudo

# Relatório patrimonial ($aggregate)
.\almox.exe relatorio

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
├── almoxarifado.h      # Definições das structs (ItemEstoque com schema Mongo) e protótipos
├── almoxarifado.c      # Lógica de negócio, serialização JSON e relatórios
├── main.c              # Ponto de entrada (CLI e Shell interativo)
├── almoxarifado.json   # Base de dados em formato JSON (auto-gerada)
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
      "codigo": 101,
      "nome": "Parafuso M6",
      "dataEntrada": "2026-09-10",
      "valorUnitario": 0.50,
      "quantidade": 500,
      "ativo": true
    },
    {
      "codigo": 102,
      "nome": "Cabo de Rede Cat6",
      "dataEntrada": "2026-10-15",
      "valorUnitario": 4.20,
      "quantidade": 80,
      "ativo": false
    }
  ],
  "historico": [
    {
      "data_hora": "2026-10-20 01:04:41",
      "operacao": "ADICIONAR",
      "nome": "Parafuso M6",
      "quantidade": 500,
      "saldo_anterior": 0,
      "saldo_novo": 500
    }
  ]
}
```
