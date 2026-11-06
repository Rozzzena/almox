-- ============================================================
-- SISTEMA DE VENDAS E ESTOQUE
-- Script 05: Stored Procedures
-- Encapsula cada operação do sistema C como procedure T-SQL
-- ============================================================

USE SistemaVendas;
GO

-- ============================================================
-- PROCEDURE: sp_CadastrarProduto
-- Equivale a: almox_adicionar() para produto NOVO
-- Uso: EXEC sp_CadastrarProduto 'Monitor 4K', 5, 1899.90
-- ============================================================
CREATE OR ALTER PROCEDURE sp_CadastrarProduto
    @nome          NVARCHAR(100),
    @quantidade    INT,
    @valorUnitario DECIMAL(10,2) = 0.00
AS
BEGIN
    SET NOCOUNT ON;

    IF EXISTS (SELECT 1 FROM dbo.Produtos WHERE nome = @nome AND ativo = 1)
    BEGIN
        PRINT 'Produto ja existe e esta ativo. Use sp_ComprarProduto para adicionar estoque.';
        RETURN;
    END

    -- Reativa se estava inativo
    IF EXISTS (SELECT 1 FROM dbo.Produtos WHERE nome = @nome AND ativo = 0)
    BEGIN
        UPDATE dbo.Produtos
           SET ativo = 1, quantidade = @quantidade,
               valorUnitario = CASE WHEN @valorUnitario > 0 THEN @valorUnitario ELSE valorUnitario END
         WHERE nome = @nome;

        INSERT INTO dbo.Movimentacoes (operacao, nomeProduto, quantidade, saldoAnterior, saldoNovo)
        VALUES ('CADASTRO', @nome, @quantidade, 0, @quantidade);

        PRINT 'Produto reativado: ' + @nome;
        RETURN;
    END

    INSERT INTO dbo.Produtos (nome, valorUnitario, quantidade)
    VALUES (@nome, @valorUnitario, @quantidade);

    INSERT INTO dbo.Movimentacoes (operacao, nomeProduto, quantidade, saldoAnterior, saldoNovo)
    VALUES ('CADASTRO', @nome, @quantidade, 0, @quantidade);

    PRINT 'Produto cadastrado com sucesso: ' + @nome;
END;
GO


-- ============================================================
-- PROCEDURE: sp_ComprarProduto
-- Equivale a: almox_adicionar() em produto EXISTENTE (compra/reposição)
-- Uso: EXEC sp_ComprarProduto 'Parafuso', 100, 0.85
-- ============================================================
CREATE OR ALTER PROCEDURE sp_ComprarProduto
    @nome          NVARCHAR(100),
    @quantidade    INT,
    @valorUnitario DECIMAL(10,2) = 0.00
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @saldoAnt INT;
    SELECT @saldoAnt = quantidade FROM dbo.Produtos WHERE nome = @nome AND ativo = 1;

    IF @saldoAnt IS NULL
    BEGIN
        PRINT 'Produto nao encontrado ou inativo. Use sp_CadastrarProduto primeiro.';
        RETURN;
    END

    UPDATE dbo.Produtos
       SET quantidade     = quantidade + @quantidade,
           valorUnitario  = CASE WHEN @valorUnitario > 0 THEN @valorUnitario ELSE valorUnitario END
     WHERE nome = @nome AND ativo = 1;

    INSERT INTO dbo.Movimentacoes (operacao, nomeProduto, quantidade, saldoAnterior, saldoNovo)
    VALUES ('COMPRA', @nome, @quantidade, @saldoAnt, @saldoAnt + @quantidade);

    PRINT 'Compra registrada. Novo saldo de ' + @nome + ': ' + CAST(@saldoAnt + @quantidade AS VARCHAR);
END;
GO


-- ============================================================
-- PROCEDURE: sp_VenderProduto
-- Equivale a: almox_remover() — com trava de saldo negativo
-- Uso: EXEC sp_VenderProduto 'Parafuso', 10
-- ============================================================
CREATE OR ALTER PROCEDURE sp_VenderProduto
    @nome       NVARCHAR(100),
    @quantidade INT
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @saldoAnt INT;
    SELECT @saldoAnt = quantidade FROM dbo.Produtos WHERE nome = @nome AND ativo = 1;

    IF @saldoAnt IS NULL
    BEGIN
        RAISERROR('Produto ''%s'' nao encontrado ou inativo.', 16, 1, @nome);
        RETURN;
    END

    IF @saldoAnt < @quantidade
    BEGIN
        RAISERROR('Saldo insuficiente: %s tem apenas %d unidade(s) disponivel(eis).', 16, 1, @nome, @saldoAnt);
        RETURN;
    END

    UPDATE dbo.Produtos
       SET quantidade = quantidade - @quantidade
     WHERE nome = @nome AND ativo = 1;

    INSERT INTO dbo.Movimentacoes (operacao, nomeProduto, quantidade, saldoAnterior, saldoNovo)
    VALUES ('VENDA', @nome, @quantidade, @saldoAnt, @saldoAnt - @quantidade);

    DECLARE @totalVenda DECIMAL(10,2);
    SELECT @totalVenda = valorUnitario * @quantidade FROM dbo.Produtos WHERE nome = @nome;

    PRINT 'Venda realizada: ' + CAST(@quantidade AS VARCHAR) + 'x ' + @nome
        + ' | Total: R$ ' + FORMAT(@totalVenda, 'N2', 'pt-BR');
END;
GO


-- ============================================================
-- PROCEDURE: sp_DesativarProduto
-- Equivale a: almox_desativar() — exclusão lógica
-- Uso: EXEC sp_DesativarProduto 'Parafuso'
-- ============================================================
CREATE OR ALTER PROCEDURE sp_DesativarProduto
    @nome NVARCHAR(100)
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @saldo INT;
    SELECT @saldo = quantidade FROM dbo.Produtos WHERE nome = @nome AND ativo = 1;

    IF @saldo IS NULL
    BEGIN
        PRINT 'Produto nao encontrado ou ja inativo.';
        RETURN;
    END

    UPDATE dbo.Produtos SET ativo = 0 WHERE nome = @nome;

    INSERT INTO dbo.Movimentacoes (operacao, nomeProduto, quantidade, saldoAnterior, saldoNovo)
    VALUES ('DESATIVAR', @nome, @saldo, @saldo, 0);

    PRINT 'Produto desativado: ' + @nome;
END;
GO


PRINT 'Stored Procedures criadas com sucesso.';
GO

-- ============================================================
-- EXEMPLOS DE USO:
-- ============================================================
-- EXEC sp_CadastrarProduto  'Monitor 4K', 5, 1899.90;
-- EXEC sp_ComprarProduto    'Parafuso', 100, 0.85;
-- EXEC sp_VenderProduto     'Parafuso', 10;
-- EXEC sp_DesativarProduto  'Porca';
