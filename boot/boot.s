[bits 16]

;;; [0x7c00] will be seted in boot.ld
[global start]
start:
	;; puts loading
	mov si, msg
	call print

	;; read the setup code from floppy
	;; begin with second, read setup to 0x90100
.readfloppy:
	mov ax, setupseg
	mov es, ax
	mov bx, setupoffset
	mov ah, 2
	mov dl, [bootdriver]
	mov ch, 0
	mov cl, 2
	mov al, setupsize/512
	int 0x13		;interupt to read
	jc  .readfloppy

	;; ok, let finish boot and jump to setup
	;; jmp setupseg:setupoffset
jmp setupseg:setupoffset

print:
	mov ah, 0x0E
.next:
	lodsb
	or al, al
	jz .done
	int 0x10 		;interupt to print
	jmp .next
.done:
	ret

msg	db "Loading Panda OS"	;
	db 13, 10, 0  		;

;;; some constants

setupseg 	equ 0x9000    	;setup address
setupoffset     equ 0x0100	;
setupsize 	equ 512

bootdriver 	db  0
; Magic number for sector
times 510-($-$$) db 0
dw 0xAA55
