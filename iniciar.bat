@echo off
chcp 65001 >nul
title Sistema de Almoxarifado em C - Apresentação PIM
mode con: cols=95 lines=32
cls

"%~dp0almox.exe"

echo.
echo ===============================================================
echo   Sessão finalizada. Pressione qualquer tecla para fechar.
echo ===============================================================
pause >nul

