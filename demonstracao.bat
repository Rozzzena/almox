@echo off
chcp 65001 >nul
title Apresentação - Demonstração Automática do Sistema de Vendas
mode con: cols=95 lines=35
cls

set "EXE_CMD=%~dp0vendas.exe"
if not exist "%EXE_CMD%" (
    echo [ERRO] vendas.exe nao encontrado. Execute build.bat para compilar.
    pause
    exit /b 1
)

echo ===============================================================
echo   DEMONSTRAÇÃO AO VIVO: SISTEMA DE VENDAS E ESTOQUE (C + JSON)
echo ===============================================================
echo.
echo Pressione qualquer tecla para iniciar a demonstracao...
pause >nul

cls
echo ===============================================================
echo   PASSO 1: Listar estoque atual de produtos
echo ===============================================================
"%EXE_CMD%" listar
echo.
pause

cls
echo ===============================================================
echo   PASSO 2: Comprar / entrada de novo produto (Fita Isolante 3M)
echo ===============================================================
"%EXE_CMD%" comprar "Fita Isolante 3M" 25 8.50
echo.
pause

cls
echo ===============================================================
echo   PASSO 3: Realizar venda de produto (10 unidades de Parafuso)
echo ===============================================================
"%EXE_CMD%" vender Parafuso 10
echo.
pause

cls
echo ===============================================================
echo   PASSO 4: Gerar Relatório de Vendas e Balanço Patrimonial
echo ===============================================================
"%EXE_CMD%" relatorio
echo.
pause

cls
echo ===============================================================
echo   PASSO 5: Consultar produto específico (busca parcial)
echo ===============================================================
"%EXE_CMD%" consultar fita
echo.
pause

cls
echo ===============================================================
echo   PASSO 6: Visualizar Histórico de Movimentações
echo ===============================================================
"%EXE_CMD%" historico 5
echo.
echo ===============================================================
echo   Fim da demonstracao automatica!
echo   Abrindo agora o modo interativo...
echo ===============================================================
pause
cls
"%EXE_CMD%"


