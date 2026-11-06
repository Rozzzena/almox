-- ============================================================
-- SISTEMA DE VENDAS E ESTOQUE
-- Script 01: Criação do banco de dados e tabelas
-- Banco: SQL Server (T-SQL)
-- Equivalência acadêmica ao schema JSON vendas.json
-- ============================================================

-- Cria o banco de dados (execute separado se necessário)
IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'SistemaVendas')
BEGIN
    CREATE DATABASE SistemaVendas;
END
GO

USE SistemaVendas;
GO

-- ============================================================
-- TABELA: Produtos
-- Equivale ao array "itens" do JSON vendas.json
-- ============================================================
IF OBJECT_ID('dbo.Produtos', 'U') IS NOT NULL
    DROP TABLE dbo.Produtos;
GO

CREATE TABLE dbo.Produtos (
    codigo        INT           NOT NULL IDENTITY(1,1),  -- PK auto-incremento
    nome          NVARCHAR(100) NOT NULL,                -- Nome do produto
    dataEntrada   DATE          NOT NULL DEFAULT GETDATE(), -- Data de cadastro
    valorUnitario DECIMAL(10,2) NOT NULL DEFAULT 0.00,   -- Preço unitário (R$)
    quantidade    INT           NOT NULL DEFAULT 0,       -- Saldo em estoque
    ativo         BIT           NOT NULL DEFAULT 1,       -- 1 = ativo, 0 = desativado

    CONSTRAINT PK_Produtos PRIMARY KEY (codigo),
    CONSTRAINT CK_Produtos_quantidade CHECK (quantidade >= 0),
    CONSTRAINT CK_Produtos_valor      CHECK (valorUnitario >= 0),
    CONSTRAINT UQ_Produtos_nome       UNIQUE (nome)
);
GO

-- ============================================================
-- TABELA: Movimentacoes
-- Equivale ao array "historico" do JSON vendas.json
-- Registra toda compra, venda e cadastro de produto
-- ============================================================
IF OBJECT_ID('dbo.Movimentacoes', 'U') IS NOT NULL
    DROP TABLE dbo.Movimentacoes;
GO

CREATE TABLE dbo.Movimentacoes (
    id             INT           NOT NULL IDENTITY(1,1),
    dataHora       DATETIME      NOT NULL DEFAULT GETDATE(),
    operacao       VARCHAR(10)   NOT NULL,   -- 'CADASTRO', 'COMPRA', 'VENDA', 'DESATIVAR'
    nomeProduto    NVARCHAR(100) NOT NULL,
    quantidade     INT           NOT NULL,
    saldoAnterior  INT           NOT NULL,
    saldoNovo      INT           NOT NULL,

    CONSTRAINT PK_Movimentacoes PRIMARY KEY (id),
    CONSTRAINT CK_Movimentacoes_op CHECK (
        operacao IN ('CADASTRO', 'COMPRA', 'VENDA', 'DESATIVAR', 'ADICIONAR', 'REMOVER')
    ),
    CONSTRAINT CK_Movimentacoes_qtd CHECK (quantidade > 0)
);
GO

PRINT 'Schema criado com sucesso: tabelas Produtos e Movimentacoes.';
GO
