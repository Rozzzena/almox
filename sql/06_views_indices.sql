-- ============================================================
-- SISTEMA DE VENDAS E ESTOQUE
-- Script 06: View e índices de desempenho
-- ============================================================

USE SistemaVendas;
GO

-- ============================================================
-- VIEW: vw_EstoqueAtivo
-- Equivale ao: vendas> listar  (somente ativos)
-- ============================================================
CREATE OR ALTER VIEW dbo.vw_EstoqueAtivo AS
SELECT
    p.codigo,
    p.nome,
    p.dataEntrada,
    p.valorUnitario                             AS [precoUnit],
    p.quantidade,
    p.valorUnitario * p.quantidade              AS [subtotal],
    CASE
        WHEN p.quantidade = 0  THEN 'ZERADO'
        WHEN p.quantidade <= 5 THEN 'BAIXO'
        ELSE 'OK'
    END                                         AS status
FROM dbo.Produtos p
WHERE p.ativo = 1;
GO

-- Uso: SELECT * FROM dbo.vw_EstoqueAtivo ORDER BY codigo;


-- ============================================================
-- VIEW: vw_ResumoVendas
-- Equivale ao relatório de vendas por produto
-- ============================================================
CREATE OR ALTER VIEW dbo.vw_ResumoVendas AS
SELECT
    m.nomeProduto,
    SUM(m.quantidade)                           AS totalVendido,
    p.valorUnitario,
    SUM(m.quantidade) * p.valorUnitario         AS receitaGerada
FROM dbo.Movimentacoes m
JOIN dbo.Produtos p ON p.nome = m.nomeProduto
WHERE m.operacao = 'VENDA'
GROUP BY m.nomeProduto, p.valorUnitario;
GO

-- Uso: SELECT * FROM dbo.vw_ResumoVendas ORDER BY receitaGerada DESC;


-- ============================================================
-- VIEW: vw_FluxoCaixa
-- Compras vs Vendas — balanço geral
-- ============================================================
CREATE OR ALTER VIEW dbo.vw_FluxoCaixa AS
SELECT
    operacao,
    COUNT(*)                                    AS totalOperacoes,
    SUM(m.quantidade)                           AS totalUnidades,
    SUM(m.quantidade * p.valorUnitario)         AS valorTotal
FROM dbo.Movimentacoes m
JOIN dbo.Produtos p ON p.nome = m.nomeProduto
WHERE m.operacao IN ('COMPRA', 'VENDA')
GROUP BY operacao;
GO

-- Uso: SELECT * FROM dbo.vw_FluxoCaixa;


-- ============================================================
-- ÍNDICES de desempenho
-- ============================================================

-- Índice em nome de produto (buscas frequentes por nome)
IF NOT EXISTS (
    SELECT 1 FROM sys.indexes WHERE name = 'IX_Produtos_nome' AND object_id = OBJECT_ID('dbo.Produtos')
)
    CREATE INDEX IX_Produtos_nome ON dbo.Produtos (nome);
GO

-- Índice em data das movimentações (relatórios por período)
IF NOT EXISTS (
    SELECT 1 FROM sys.indexes WHERE name = 'IX_Movimentacoes_dataHora' AND object_id = OBJECT_ID('dbo.Movimentacoes')
)
    CREATE INDEX IX_Movimentacoes_dataHora ON dbo.Movimentacoes (dataHora DESC);
GO

-- Índice em operação + produto (relatórios de compra/venda)
IF NOT EXISTS (
    SELECT 1 FROM sys.indexes WHERE name = 'IX_Movimentacoes_operacao_produto' AND object_id = OBJECT_ID('dbo.Movimentacoes')
)
    CREATE INDEX IX_Movimentacoes_operacao_produto ON dbo.Movimentacoes (operacao, nomeProduto);
GO

PRINT 'Views e indices criados com sucesso.';
GO
