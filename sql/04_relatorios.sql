-- ============================================================
-- SISTEMA DE VENDAS E ESTOQUE
-- Script 04: Relatórios e Consultas Analíticas
-- Equivale a: almox_relatorio() e consultas avançadas
-- ============================================================

USE SistemaVendas;
GO

-- ============================================================
-- RELATÓRIO PRINCIPAL: Balanço do estoque
-- Equivale a: vendas> relatorio
-- ============================================================
SELECT
    COUNT(*)                                              AS totalProdutosAtivos,
    SUM(quantidade)                                       AS totalUnidadesEmEstoque,
    SUM(valorUnitario * quantidade)                       AS [valorTotalEstoque_R$],
    SUM(CASE WHEN quantidade = 0  THEN 1 ELSE 0 END)      AS produtosZerados,
    SUM(CASE WHEN quantidade <= 5 AND quantidade > 0
              THEN 1 ELSE 0 END)                          AS produtosComEstoqueBaixo
FROM dbo.Produtos
WHERE ativo = 1;
GO


-- ============================================================
-- RELATÓRIO: Resumo de Vendas por Produto
-- Total de unidades vendidas e receita gerada por produto
-- ============================================================
SELECT
    m.nomeProduto,
    SUM(m.quantidade)                           AS totalUnidadesVendidas,
    p.valorUnitario,
    SUM(m.quantidade) * p.valorUnitario         AS [receitaTotal_R$]
FROM dbo.Movimentacoes m
JOIN dbo.Produtos p ON p.nome = m.nomeProduto
WHERE m.operacao = 'VENDA'
GROUP BY m.nomeProduto, p.valorUnitario
ORDER BY [receitaTotal_R$] DESC;
GO


-- ============================================================
-- RELATÓRIO: Resumo de Compras por Produto
-- Total de unidades compradas e custo total de estoque
-- ============================================================
SELECT
    m.nomeProduto,
    SUM(m.quantidade)                           AS totalUnidadesCompradas,
    p.valorUnitario,
    SUM(m.quantidade) * p.valorUnitario         AS [custoTotal_R$]
FROM dbo.Movimentacoes m
JOIN dbo.Produtos p ON p.nome = m.nomeProduto
WHERE m.operacao IN ('COMPRA', 'CADASTRO')
GROUP BY m.nomeProduto, p.valorUnitario
ORDER BY [custoTotal_R$] DESC;
GO


-- ============================================================
-- RELATÓRIO: Movimentações por período
-- Filtra compras e vendas num intervalo de datas
-- ============================================================
SELECT
    FORMAT(dataHora, 'yyyy-MM-dd HH:mm:ss') AS dataHora,
    operacao,
    nomeProduto,
    quantidade,
    saldoAnterior,
    saldoNovo
FROM dbo.Movimentacoes
WHERE dataHora BETWEEN '2026-10-01' AND '2026-12-31'
  AND operacao IN ('COMPRA', 'VENDA')
ORDER BY dataHora DESC;
GO


-- ============================================================
-- RELATÓRIO: Produtos inativos (soft-deleted)
-- Equivale a: vendas> listar tudo (mostrando apenas inativos)
-- ============================================================
SELECT
    codigo,
    nome,
    FORMAT(dataEntrada, 'dd/MM/yyyy') AS dataEntrada,
    valorUnitario
FROM dbo.Produtos
WHERE ativo = 0
ORDER BY nome;
GO


-- ============================================================
-- RELATÓRIO: Ranking de produtos mais vendidos
-- ============================================================
SELECT TOP 10
    nomeProduto,
    SUM(quantidade) AS totalVendido
FROM dbo.Movimentacoes
WHERE operacao = 'VENDA'
GROUP BY nomeProduto
ORDER BY totalVendido DESC;
GO


-- ============================================================
-- RELATÓRIO: Fluxo de caixa (Compras vs Vendas)
-- Resumo financeiro por tipo de operação
-- ============================================================
SELECT
    operacao,
    COUNT(*)        AS totalOperacoes,
    SUM(m.quantidade * p.valorUnitario) AS [valorTotal_R$]
FROM dbo.Movimentacoes m
JOIN dbo.Produtos p ON p.nome = m.nomeProduto
WHERE m.operacao IN ('COMPRA', 'VENDA', 'CADASTRO')
GROUP BY operacao
ORDER BY operacao;
GO
