-- ============================================================
-- SISTEMA DE VENDAS E ESTOQUE
-- Script 02: Dados iniciais (seed) com os produtos do vendas.json
-- ============================================================

USE SistemaVendas;
GO

-- Limpa dados existentes antes de reinserir
DELETE FROM dbo.Movimentacoes;
DELETE FROM dbo.Produtos;
DBCC CHECKIDENT ('dbo.Produtos',      RESEED, 0);
DBCC CHECKIDENT ('dbo.Movimentacoes', RESEED, 0);
GO

-- ============================================================
-- INSERÇÃO DE PRODUTOS (espelho do array "itens" do JSON)
-- Equivale a: INSERT INTO Produtos / almox_adicionar()
-- ============================================================
INSERT INTO dbo.Produtos (nome, dataEntrada, valorUnitario, quantidade, ativo)
VALUES
    ('Parafuso',            '2026-10-13', 0.85,   170, 1),
    ('Cabo de Rede Cat6',   '2026-10-13', 0.00,    50, 1),
    ('Porca',               '2026-10-13', 0.00,   200, 1),
    ('Luva Nitrilica',      '2026-10-20', 15.50,   40, 1),
    ('Teclado Mecanico RGB','2026-11-05', 249.90,  13, 1),
    ('Mouse Sem Fio',       '2026-11-05', 49.90,   20, 1);
GO

-- ============================================================
-- INSERÇÃO DO HISTÓRICO (espelho do array "historico" do JSON)
-- ============================================================
INSERT INTO dbo.Movimentacoes (dataHora, operacao, nomeProduto, quantidade, saldoAnterior, saldoNovo)
VALUES
    ('2026-10-13 22:01:29', 'CADASTRO',  'Parafuso',             100,   0, 100),
    ('2026-10-13 22:01:29', 'CADASTRO',  'Cabo de Rede Cat6',     50,   0,  50),
    ('2026-10-13 22:01:29', 'COMPRA',    'Parafuso',              50, 100, 150),
    ('2026-10-13 22:01:38', 'VENDA',     'Parafuso',              30, 150, 120),
    ('2026-10-13 22:06:23', 'CADASTRO',  'Porca',                200,   0, 200),
    ('2026-10-20 01:04:46', 'CADASTRO',  'Luva Nitrilica',        40,   0,  40),
    ('2026-10-20 01:06:11', 'DESATIVAR', 'Parafuso',             120, 120,   0),
    ('2026-10-20 01:08:53', 'COMPRA',    'Parafuso',              10, 120, 130),
    ('2026-11-05 22:55:17', 'CADASTRO',  'Teclado Mecanico RGB',  15,   0,  15),
    ('2026-11-05 22:55:27', 'CADASTRO',  'Mouse Sem Fio',         20,   0,  20),
    ('2026-11-05 23:02:26', 'VENDA',     'Parafuso',              10, 130, 120),
    ('2026-11-05 23:02:26', 'COMPRA',    'Parafuso',              50, 120, 170),
    ('2026-11-05 23:02:26', 'VENDA',     'Teclado Mecanico RGB',   2,  15,  13);
GO

PRINT 'Dados inseridos com sucesso.';

-- Verificação rápida
SELECT 'Produtos cadastrados' AS info, COUNT(*) AS total FROM dbo.Produtos
UNION ALL
SELECT 'Movimentações registradas', COUNT(*) FROM dbo.Movimentacoes;
GO
