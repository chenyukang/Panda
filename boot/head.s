extern kmain
[BITS 32]
	mov esp, stack_top
	mov ax, 0x10
	mov ds, ax
	mov cl, '2'
	mov [0xb8000], cl
	mov cl, 0x04
	mov [0xb8001], cl

        ;; check A20 enable
        xor eax, eax
err:	inc eax
	mov [0x000000], eax
	cmp [0x100000], eax
	je err

	;; Finally, we goto OS's c world
	push esp
	call kmain
    ;; 	jmp $
;------------------------------------------------------------------------

section .bss
	resb 8192	; 80KB for stack
stack_top:	; top of our stack here
