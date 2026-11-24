@echo off
nasm -f bin mario.asm -o MARIO.COM -l mario.lst
if errorlevel 1 goto failed
echo Build OK: MARIO.COM
goto done
:failed
echo Build FAILED
:done
