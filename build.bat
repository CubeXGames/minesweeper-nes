@echo off
set name="main"
set title="minesweeper"
set path=.\bin\
set CC65_HOME=.\

cc65 -O -Oirs %name%.c --add-source
ca65 crt0.s
ca65 %name%.s -g
ld65 -C nrom_32k_vert.cfg -o %title%.nes crt0.o %name%.o nes.lib -Ln labels.txt
pause