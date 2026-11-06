@echo off
chcp 65001 >nul
title Sistema de Vendas e Estoque em C
mode con: cols=95 lines=32
cls

if exist "%~dp0vendas.exe" (
    "%~dp0vendas.exe"
) else (
    "%~dp0almox.exe"
)

echo.
echo ===============================================================
echo   Sessão finalizada. Pressione qualquer tecla para fechar.
echo ===============================================================
pause >nul

