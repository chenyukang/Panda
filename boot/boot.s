
align 4
[bits 16]
	
;;; [0x7c00] will be seted in boot.ld

jmp start

[global start]
start:	
	mov [bootdriver], dl	;the driver number which boot from

	mov ax, bootseg
	mov ds, ax

	;; puts loading
	mov si, msg
	call print_str

	;; read the setup code from floppy
	;; begin with second
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
	jmp setupseg:setupoffset
	
print_str:
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
bootseg 	equ 0x0000	;boot begin address
setupseg 	equ 0x9000    	;setup address
setupoffset 	equ 0x0100	;
setupsize 	equ 1024	;
bootdriver 	db  0		;
; Magic number for sector
times 510-($-$$) db 0
dw 0xAA55
