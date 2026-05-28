align 4
[bits 16]
	;; first 1MB memory
	;; 0x00000 ~ 0x003FF : interupt vector
	;; 0x00400 ~ 0x004FF : BIOS data
	;; 0x00500 ~ 0x07BFF : free
	;; 0x07C00 ~ 0x07DFF : loader
	;; 0x07E00 ~ 0x9FFFF : free
	;; 0xA0000 ~ 0xBFFFF : vedio
	;; 0xC0000 ~ 0xFFFFF : bios interupt precedure

jmp setup

[global setup]
setup:
	mov ax, setupseg
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0xffff

	;; loading message
	mov si, setup_msg
	call print_str

read_info:
	mov ax, 0x9000
	mov ds, ax

	;; Now get memory size and save at 0x90002
	;; This may just report up to 64M.
	;; It only reports contiguous (usable) RAM.
	mov ah, 0x88
	int 0x15
	mov [2], ax

	mov ax, setupseg
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0xffff

	mov ax, setupseg
	mov es, ax
	mov bx, setupoffset+setupsize ; put kernel at here now
	;0,1 is for boot, setupsize/512 for setup.bin
	mov ch, 0
	mov dh, 0
	mov cl, 3
	mov si, systemsize/512
.readfloppy:
	mov ax, 0x0201
	and dx, 0xff00
	int 0x13
	jc .read_error
	add bx, 512
	inc cl
	cmp cl, 19
	jne .read_next
	mov cl, 1
	inc dh
	cmp dh, 2
	jne .read_next
	mov dh, 0
	inc ch
.read_next:
	dec si
	jnz .readfloppy
	jmp .read_done
.read_error:
	xor ax, ax
	and dx, 0xff00
	int 0x13
	jmp .readfloppy
.read_done:

	;; move system to 0x00000
	;; this is OK for our kernel.bin is small
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

	;; enable A20
	call empty_8042
	mov al, 0xd1
	out 0x64, al
	call empty_8042
	mov al, 0xdf
	out 0x60, al
	call empty_8042
	;; enter pretect-mode
	mov eax, cr0
	or  eax, 1
	mov cr0, eax

	;; jump into head, which at 0x00000
	jmp dword 0x8:0x0

empty_8042:
	in al, 0x64
	test al, 0x2
	jnz  empty_8042
	ret

;interupt show message
print_str:
	mov ah, 0x0E
.next:
	lodsb
	or al, al
	jz .done
	int 0x10
	jmp .next
.done:
	ret

setupseg 	equ 	0x9000
setupoffset	equ 	0x0100
setupsize 	equ 	512

systemseg       equ     0x0000
systemoffset	equ 	0x0000
systemsize      equ 	1024*36 ; this will bigger than kernel.bin
				;;; bochs seems has error when read more than 1024*36

setup_msg db "Setuping Panda OS"
	db 13, 10, 0

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
	;; 64 MB
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
times 512-($-$$) db 0
