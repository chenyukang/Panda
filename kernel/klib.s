
[SECTION .text]

; 导出函数
global	port_read
global	port_write

port_read:
	mov	edx, [esp + 4]		; port
	mov	edi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1
	cld
	rep	insw
	ret

; ========================================================================
;                  void port_write(u16 port, void* buf, int n);
; ========================================================================
port_write:
	mov	edx, [esp + 4]		; port
	mov	esi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1
	cld
	rep	outsw
	ret

[global read_eip]
read_eip:
    pop eax                     ; Get the return address
    jmp eax                     ; Return. Can't use RET because return
                                ; address popped off the stack. 

;;
;; retu(uint eip, uint esp3)
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
