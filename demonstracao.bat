@echo off
chcp 65001 >nul
title Apresentação - Demonstração Automática do Almoxarifado
mode con: cols=95 lines=35
cls

echo ===============================================================
echo   DEMONSTRAÇÃO AO VIVO: SISTEMA DE ALMOXARIFADO (C + JSON)
echo ===============================================================
echo.
echo Pressione qualquer tecla para iniciar a demonstracao...
pause >nul

cls
echo ===============================================================
echo   PASSO 1: Listar estoque atual
echo ===============================================================
"%~dp0almox.exe" listar
echo.
pause

cls
echo ===============================================================
echo   PASSO 2: Adicionar novo item (Fita Isolante 3M)
echo ===============================================================
"%~dp0almox.exe" adicionar "Fita Isolante 3M" 25 8.50
echo.
pause

cls
echo ===============================================================
echo   PASSO 3: Dar baixa no estoque (10 unidades de Parafuso)
echo ===============================================================
"%~dp0almox.exe" remover Parafuso 10
echo.
pause

cls
echo ===============================================================
echo   PASSO 4: Gerar Relatorio Financeiro e Balanco Patrimonial
echo ===============================================================
"%~dp0almox.exe" relatorio
echo.
pause

cls
echo ===============================================================
echo   PASSO 5: Consultar item especifico (busca parcial)
echo ===============================================================
"%~dp0almox.exe" consultar fita
echo.
pause

cls
echo ===============================================================
echo   PASSO 6: Visualizar Historico de Auditoria
echo ===============================================================
"%~dp0almox.exe" historico 5
echo.
echo ===============================================================
echo   Fim da demonstracao automatica!
echo   Abrindo agora o modo interativo...
echo ===============================================================
pause
cls
"%~dp0almox.exe"

