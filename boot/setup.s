
align 4
[bits 16]
	
jmp setup

	
[global setup]
setup:	
	mov ax, setupseg
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0xffff
	
	;; puts loading
	mov si, setup_msg
	call set_print_str		;

	.readfloppy:
	mov ax, setupseg
	mov es, ax
	mov bx, setupoffset+setupsize
	mov ah, 2
	mov dl, [0]
	mov ch, 0
	mov cl, 1+1+setupsize/512
	mov al, systemsize/512
	int 0x13
	jc .readfloppy


	;; move system to 0x0000
	cld
	mov si, setupoffset+setupsize
	mov ax, systemseg
	mov es, ax
	mov di, systemoffset
	mov cx, systemsize/4
	rep movsd

	;;
	cli
	lgdt [gdt_addr]
	
	;; A20
	call empty_8042
	mov al, 0xd1
	out 0x64, al
	call empty_8042
	mov al, 0xdf
	out 0x60, al
	call empty_8042

	mov eax, cr0
	or  eax, 1
	mov cr0, eax

	jmp dword 0x8:0x0
	jmp $

set_print_str:
	mov ah, 0x0E
.next:
	lodsb
	or al, al
	jz .done
	int 0x10 		;interupt to print
	jmp .next
.done:
	ret

empty_8042:
	in al, 0x64
	test al, 0x2
	jnz  empty_8042
	ret

[bits 32]
system:
	;设置寄存器
	mov ax, 0x10
	mov ds , ax
	mov cl , '1'
	mov [0xb8000] , cl
	mov cl , 0x04
	mov [0xb8001] , cl
	jmp $

setupseg 	equ 	0x9000
setupoffset	equ 	0x0100
setupsize 	equ 	1024

systemseg 	equ	0x0000
systemoffset	equ 	0x0000
systemsize 	equ 	1024*30 ; this will bigger than kernel.bin
	
	
setup_msg db "Setup Panda OS"	;
	db 13, 10, 0  		;


gdt_addr:
	dw 0x7fff
	dw gdt
	dw 0x009

gdt:
gdt_null:
	dw 0x0000
	dw 0x0000
	dw 0x0000
	dw 0x0000
gdt_system_code:
	dw 0x3fff
	dw 0x0000
	dw 0x9a00
	dw 0x00c0
gdt_syste_data:
	dw 0x3fff
	dw 0x0000
	dw 0x9200
	dw 0x00c0
	
	
; Magic number for sector
times 1024-($-$$) db 0
