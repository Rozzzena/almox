# 📦 Sistema de Vendas e Estoque em Linha de Comando (C)

Aplicação desenvolvida em **Linguagem C (C99)** para gestão de compras, vendas e controle de estoque de produtos via terminal. Implementa persistência em **JSON** (`vendas.json`) com comandos diretos (CLI) e modo shell interativo contínuo.

---

## 🎯 Comandos do Sistema

| Comando | Sintaxe | Descrição |
| :--- | :--- | :--- |
| `cadastrar` | `cadastrar` | Cadastra novo produto campo a campo (nome, qtd, custo, venda, est. mín) |
| `cadastrar` | `cadastrar <produto> <qtd> [valor]` | Cadastra produto em linha única |
| `comprar` | `comprar` | Registra entrada/compra de estoque (interativo) |
| `comprar` | `comprar <produto> <qtd> [valor]` | Entrada de estoque em linha única |
| `vender` | `vender` | Realiza uma venda (interativo) |
| `vender` | `vender <produto> <qtd>` | Venda em linha única com trava de saldo |
| `editar` | `editar <codigo>` | Atualização guiada de produto existente (campos opcionais) |
| `desativar` | `desativar <produto>` | Exclusão lógica — remove das listagens sem apagar |
| `listar` | `listar` | Exibe apenas produtos ativos com saldos, custo e valores |
| `listar tudo` | `listar tudo` | Exibe todos os produtos, inclusive desativados |
| `exportar` | `exportar [arquivo.csv]` | Exporta dados completos para planilha CSV (padrão: `estoque.csv`) |
| `consultar` | `consultar <termo>` | Busca parcial no nome do produto |
| `relatorio` | `relatorio` | Balanço com custo total, valor de venda e lucro projetado |
| `historico` | `historico [limite]` | Histórico de compras, vendas, cadastros e edições |

---

## 🚀 Funcionalidades

- **Cadastro de produto campo a campo (`cadastrar`)**: Solicita nome, quantidade e preço um por um, com validação e cancelamento seguro.
- **Registro de Compras (`comprar`)**: Entrada de estoque com interativo ou linha única. Reativa produtos inativos automaticamente.
- **Registro de Vendas (`vender`)**: Dá baixa no estoque, mostra valor total da venda e protege contra saldo negativo.
- **Exclusão Lógica (`desativar`)**: Desativa o produto sem apagar histórico. Use `comprar` no mesmo produto para reativar.
- **Tabela formatada de estoque (`listar` / `listar tudo`)**: Exibe código, descrição, quantidade, valor unitário, valor subtotal e status (`OK`, `BAIXO`, `ZERADO`, `INATIVO`).
- **Relatório Financeiro (`relatorio`)**: Sumário com total de produtos, unidades, inativos e valor financeiro em estoque.
- **Busca rápida (`consultar`)**: Localiza produtos pelo nome, busca parcial sem distinção de maiúsculas/minúsculas.
- **Histórico de Movimentações (`historico`)**: Registra data, hora, tipo (`CADASTRO`, `COMPRA`, `VENDA`, `DESATIVAR`), saldo anterior e novo saldo.
- **Persistência em JSON (`vendas.json`)**: Dados salvos imediatamente após cada operação, sem dependências externas.
- **Cores ANSI e suporte UTF-8**: Visual moderno no PowerShell, Windows Terminal e CMD.

---

## 🛠️ Como Compilar

### Opção 1: Usando o script `build.bat` (Recomendado no Windows)
Execute no terminal ou dê duplo-clique:
```cmd
build.bat
```

### Opção 2: Usando o GCC diretamente
```bash
gcc -std=c99 -Wall -Wextra -O2 main.c almoxarifado.c -o vendas.exe
```

### Opção 3: Usando Make
```bash
make
```

---

## 💻 Como Usar

A aplicação funciona em **dois modos**:

### 1. Modo Interativo (Shell Contínuo)
Ao executar sem argumentos, o terminal abre o prompt `vendas> `:
```cmd
.\vendas.exe
```

Dentro do prompt interativo, digite os comandos:
```text
vendas> cadastrar
(solicita nome, quantidade e valor unitário com Enter)

vendas> comprar "Cabo de Rede Cat6" 50 4.20
vendas> vender Parafuso 25
vendas> desativar "Cabo de Rede Cat6"
vendas> listar
vendas> listar tudo
vendas> relatorio
vendas> consultar parafuso
vendas> historico
vendas> sair
```

### 2. Modo Linha de Comando Direta (CLI)
Você também pode executar comandos diretamente em scripts ou no terminal:

```cmd
# Cadastrar novo produto
.\vendas.exe cadastrar "Luva Nitrilica" 40 15.50

# Registrar compra (entrada de estoque)
.\vendas.exe comprar Parafuso 100 0.50

# Registrar venda (baixa de estoque)
.\vendas.exe vender Parafuso 20

# Desativar produto (exclusão lógica)
.\vendas.exe desativar Parafuso

# Listar produtos ativos
.\vendas.exe listar

# Listar todos (inclusive desativados)
.\vendas.exe listar tudo

# Relatório financeiro
.\vendas.exe relatorio

# Consultar um produto específico
.\vendas.exe consultar Parafuso

# Histórico de movimentações
.\vendas.exe historico
.\vendas.exe historico 10
```

---

## 📁 Estrutura de Arquivos

```
almox/
├── almoxarifado.h    # Definições de structs, constantes e protótipos
├── almoxarifado.c    # Lógica de compras, vendas, listagem e relatórios
├── main.c            # CLI, shell interativo e fluxos campo a campo
├── vendas.json       # Banco de dados JSON (gerado automaticamente)
├── build.bat         # Script de compilação para Windows
├── Makefile          # Makefile para GCC / MinGW
├── iniciar.bat       # Atalho de duplo clique para executar
├── demonstracao.bat  # Roteiro de demonstração guiada
└── README.md         # Documentação completa
```

---

## 📄 Exemplo do Arquivo JSON (`vendas.json`)

```json
{
  "itens": [
    {
      "codigo": 1,
      "nome": "Parafuso",
      "dataEntrada": "2026-11-05",
      "valorUnitario": 0.85,
      "quantidade": 170,
      "ativo": true
    }
  ],
  "historico": [
    {
      "data_hora": "2026-11-05 23:02:26",
      "operacao": "COMPRA",
      "nome": "Parafuso",
      "quantidade": 50,
      "saldo_anterior": 120,
      "saldo_novo": 170
    },
    {
      "data_hora": "2026-11-05 23:02:26",
      "operacao": "VENDA",
      "nome": "Parafuso",
      "quantidade": 10,
      "saldo_anterior": 130,
      "saldo_novo": 120
    }
  ]
}
```
