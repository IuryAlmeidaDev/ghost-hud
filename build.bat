@echo off
echo ========================================================
echo           GhostHUD - Compilador Win32 em C Puro         
echo ========================================================

set TCC_BIN=C:\Users\iuryc\tcc\tcc\tcc.exe

if exist "%TCC_BIN%" (
    echo [OK] Compilando com Tiny C Compiler (TCC)...
    "%TCC_BIN%" src\main.c src\hardware.c src\ui.c -o GhostHUD.exe -lgdi32 -luser32 -lkernel32
) else (
    echo [INFO] TCC local nao encontrado. Tentando gcc no PATH...
    gcc -O3 src\main.c src\hardware.c src\ui.c -o GhostHUD.exe -lgdi32 -luser32 -lkernel32 -mwindows
)

if %ERRORLEVEL% equ 0 (
    echo.
    echo [SUCESSO] GhostHUD.exe compilado com exito!
    dir GhostHUD.exe | findstr GhostHUD.exe
) else (
    echo.
    echo [ERRO] Falha na compilacao.
)

pause
