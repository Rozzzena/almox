@echo off
setlocal

echo ========================================================
echo   Compilando Sistema de Almoxarifado em C...
echo ========================================================

where gcc >nul 2>&1
if %ERRORLEVEL% neq 0 (
    if exist "%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\gcc.exe" (
        set "PATH=%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin;%PATH%"
    )
)

gcc -std=c99 -Wall -Wextra -O2 main.c almoxarifado.c -o almox.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo [SUCESSO] Compilacao concluida com sucesso!
    echo Executavel gerado: almox.exe
    echo.
    echo Para executar no modo interativo:
    echo   almox.exe
    echo.
    echo Para executar via comandos diretos:
    echo   almox.exe adicionar Parafuso 100
    echo   almox.exe remover Parafuso 20
    echo   almox.exe listar
    echo ========================================================
) else (
    echo.
    echo [ERRO] Falha na compilacao. Codigo de saida: %ERRORLEVEL%
)

endlocal
