@echo off
cls
cmd /c "C:\Program Files\CodeBlocks\MinGW\bin\mingw32-g++.exe" -c -Wall header\configurator.cpp -o configurator_linux.o
cmd /c "C:\Program Files\CodeBlocks\MinGW\bin\mingw32-g++.exe" -c -Wall .\main.cpp -o .\main.o\
cmd /c "C:\Program Files\CodeBlocks\MinGW\bin\mingw32-g++.exe" .\main.o .\configurator_windows.o -o .\petri_dish.exe -lpdcurses
del /q .\main.o
pause
::echo ================================================================================
cmd /c .\petri_dish.exe
pause