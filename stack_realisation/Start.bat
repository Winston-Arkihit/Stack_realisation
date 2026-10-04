@echo off
cd /d "%~dp0"

g++ -DSTACK_FULL_PROTECTION -I "." -I "..\my_lib_func" -x c "code.c" -x c "stack.c" -o "code.exe"

code.exe
pause
