[bits 32]
[SECTION .text]

[global read_eip]
read_eip:
    pop eax                     ; Get the return address
    jmp eax                     ; Return. Can't use RET because return
                                ; address popped off the stack.

;; return to user mode via an IRET instruction.
;; note:
;;    USER_CS = 0x1B
;;    USER_DS = 0x23
;;
[global enter_user]
enter_user:
    pop dword eax       ;; ignore the returned eip
    pop dword ebx       ;; eip -> ebx
    pop dword ecx       ;; esp3 -> ecx
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push dword 0x23     ;; ss3
    push dword ecx      ;; esp3
    pushf               ;; eflags
    push dword 0x1B     ;; cs
    push dword ebx      ;; eip
    iretd

global gdt_flush
extern gp
gdt_flush:
    lgdt [gp]
;;; set ds/es etc to data seg
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

global move_vedio_mode
move_vedio_mode:
	mov ah, 0x0
	mov al, 0x13
	int 0x10

global move_text_mode
move_text_mode:
	mov ah, 0x0
	mov al, 0x03
	int 0x10

global _do_swtch
_do_swtch:
    mov eax, dword [esp+4]  ;; new
    pop dword [eax]         ;; *old
    mov dword [eax+4], esp
    mov dword [eax+8], ebx
    mov dword [eax+12], ecx
    mov dword [eax+16], edx
    mov dword [eax+20], esi
    mov dword [eax+24], edi
    mov dword [eax+28], ebp

    mov eax, dword [esp+4]

    mov ebp, dword [eax+28]
    mov edi, dword [eax+24]
    mov esi, dword [eax+20]
    mov edx, dword [eax+16]
    mov ecx, dword [eax+12]
    mov ebx, dword [eax+8]
    mov esp, dword [eax+4]
    push dword [eax]
    ret


;;; This macro creates a stub for an ISR which does NOT pass it's own
; error code (adds a dummy errcode byte).
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    cli                         ; Disable interrupts firstly.
    push byte 0                 ; Push a dummy error code.
    push byte %1                ; Push the interrupt number.
    jmp common_stub         ; Go to our common handler code.
%endmacro

; This macro creates a stub for an ISR which passes it's own
; error code.
%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli                         ; Disable interrupts.
    push byte %1                ; Push the interrupt number
    jmp common_stub
%endmacro

; This macro creates a stub for an IRQ - the first parameter is
; the IRQ number, the second is the ISR number it is remapped to.
%macro IRQ 2
  global irq%1
  irq%1:
    cli
    push byte 0
    push byte %2
    jmp common_stub
%endmacro

global isr14
isr14:
	cli                         ; Disable interrupts firstly.
	push byte 14                ; Push the interrupt number.
	jmp  common_stub         ; Go to our common handler code.

;;; system call
global sys_call
sys_call:
	push  dword 0
	push  dword 128
	jmp   common_stub

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ  10,    42
IRQ  11,    43
IRQ  12,    44
IRQ  13,    45
IRQ  14,    46
IRQ  15,    47

;; int idt.c
extern hwint_handler
global stub_ret
common_stub:
    cli
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10   ; Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp   ; Push the stack

    push eax
    mov eax, hwint_handler
    call eax       ; A special call, preserves the 'eip' register
    pop eax

stub_ret:
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!


section .bss
	resb 8192	; 80KB for stack
stack_top:	; top of our stack here
