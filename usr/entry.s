[global _start]
[extern main]
[extern exit]
_start:
    call main
    push eax
    call exit

[section .data]
[global errno]
errno dw 0
