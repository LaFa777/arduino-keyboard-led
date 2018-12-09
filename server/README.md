# Серверная часть

Команда для сборки:
```
gcc -o keyboard-led main.c arduino-serial/arduino-serial-lib.c -I/usr/include -lX11 -lxkbfile
```