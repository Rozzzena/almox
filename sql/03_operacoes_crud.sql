-- ============================================================
-- SISTEMA DE VENDAS E ESTOQUE
-- Script 03: Operações CRUD — equivalência com o sistema C
-- Cada bloco abaixo espelha uma função do almoxarifado.c
-- ============================================================

USE SistemaVendas;
GO

-- ============================================================
-- CADASTRAR NOVO PRODUTO
-- Equivale a: almox_adicionar() com produto novo
-- Equivale a: vendas> cadastrar
-- ============================================================
-- INSERT INTO dbo.Produtos (nome, valorUnitario, quantidade)
-- VALUES ('Cadeira Gamer', 850.00, 5);
-- GO


-- ============================================================
-- COMPRAR (entrada de estoque de produto existente)
-- Equivale a: almox_adicionar() em produto já cadastrado
-- Equivale a: vendas> comprar <produto> <qtd>
-- ============================================================
-- Incrementa quantidade e registra movimentação:

-- DECLARE @nome     NVARCHAR(100) = 'Parafuso';
-- DECLARE @qtd      INT           = 50;
-- DECLARE @saldoAnt INT;
--
-- SELECT @saldoAnt = quantidade FROM dbo.Produtos WHERE nome = @nome;
--
-- UPDATE dbo.Produtos
--    SET quantidade = quantidade + @qtd
--  WHERE nome = @nome AND ativo = 1;
--
-- INSERT INTO dbo.Movimentacoes (operacao, nomeProduto, quantidade, saldoAnterior, saldoNovo)
-- VALUES ('COMPRA', @nome, @qtd, @saldoAnt, @saldoAnt + @qtd);
-- GO


-- ============================================================
-- VENDER (saída de estoque — trava de saldo negativo)
-- Equivale a: almox_remover()
-- Equivale a: vendas> vender <produto> <qtd>
-- ============================================================

-- DECLARE @nome     NVARCHAR(100) = 'Parafuso';
-- DECLARE @qtd      INT           = 10;
-- DECLARE @saldoAnt INT;
--
-- SELECT @saldoAnt = quantidade FROM dbo.Produtos
--  WHERE nome = @nome AND ativo = 1;
--
-- IF @saldoAnt IS NULL
--     RAISERROR ('Produto não encontrado ou inativo.', 16, 1);
-- ELSE IF @saldoAnt < @qtd
--     RAISERROR ('Saldo insuficiente para realizar a venda.', 16, 1);
-- ELSE
-- BEGIN
--     UPDATE dbo.Produtos
--        SET quantidade = quantidade - @qtd
--      WHERE nome = @nome AND ativo = 1;
--
--     INSERT INTO dbo.Movimentacoes (operacao, nomeProduto, quantidade, saldoAnterior, saldoNovo)
--     VALUES ('VENDA', @nome, @qtd, @saldoAnt, @saldoAnt - @qtd);
-- END
-- GO


-- ============================================================
-- DESATIVAR PRODUTO (exclusão lógica — soft delete)
-- Equivale a: almox_desativar()
-- Equivale a: vendas> desativar <produto>
-- ============================================================

-- UPDATE dbo.Produtos SET ativo = 0 WHERE nome = 'Parafuso';
--
-- INSERT INTO dbo.Movimentacoes (operacao, nomeProduto, quantidade, saldoAnterior, saldoNovo)
-- SELECT 'DESATIVAR', nome, quantidade, quantidade, 0
--   FROM dbo.Produtos WHERE nome = 'Parafuso';
-- GO


-- ============================================================
-- REATIVAR PRODUTO (via compra no produto inativo)
-- Equivale ao comportamento de reativação do almox_adicionar()
-- ============================================================

-- UPDATE dbo.Produtos SET ativo = 1 WHERE nome = 'Parafuso' AND ativo = 0;
-- GO


-- ============================================================
-- LISTAR PRODUTOS ATIVOS
-- Equivale a: almox_listar(almox, 0)
-- Equivale a: vendas> listar
-- ============================================================
SELECT
    codigo,
    nome,
    FORMAT(dataEntrada, 'dd/MM/yyyy')             AS dataEntrada,
    FORMAT(valorUnitario, 'N2', 'pt-BR')          AS [precoUnit (R$)],
    quantidade,
    CAST(valorUnitario * quantidade AS DECIMAL(12,2)) AS [totalEstoque (R$)],
    CASE
        WHEN quantidade = 0  THEN 'ZERADO'
        WHEN quantidade <= 5 THEN 'BAIXO'
        ELSE 'OK'
    END AS status
FROM dbo.Produtos
WHERE ativo = 1
ORDER BY codigo;
GO


-- ============================================================
-- LISTAR TODOS OS PRODUTOS (incluindo inativos)
-- Equivale a: almox_listar(almox, 1)
-- Equivale a: vendas> listar tudo
-- ============================================================
SELECT
    codigo,
    nome,
    quantidade,
    valorUnitario,
    CASE WHEN ativo = 1 THEN 'ATIVO' ELSE 'INATIVO' END AS status
FROM dbo.Produtos
ORDER BY ativo DESC, codigo;
GO


-- ============================================================
-- CONSULTAR PRODUTO POR NOME (busca parcial)
-- Equivale a: almox_consultar()
-- Equivale a: vendas> consultar <termo>
-- ============================================================
SELECT codigo, nome, quantidade, valorUnitario
  FROM dbo.Produtos
 WHERE ativo = 1
   AND nome LIKE '%Parafuso%';   -- <-- substitua pelo termo buscado
GO


-- ============================================================
-- HISTÓRICO DE MOVIMENTAÇÕES
-- Equivale a: almox_exibir_historico()
-- Equivale a: vendas> historico [limite]
-- ============================================================
SELECT TOP 20                            -- TOP 20 = limite padrão do sistema C
    FORMAT(dataHora, 'yyyy-MM-dd HH:mm:ss') AS dataHora,
    operacao,
    nomeProduto,
    quantidade,
    saldoAnterior,
    saldoNovo
FROM dbo.Movimentacoes
ORDER BY dataHora DESC;
GO
