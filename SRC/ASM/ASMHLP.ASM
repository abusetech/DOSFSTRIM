;Helper funtions for 32bit assembly instructions
;Assemble with nasm -f obj
;void far cdecl outportl(uint16_t addr, uint32_t dword);
[BITS 16] 

GLOBAL _outportl32

section .text
_outportl32:
push bp
push dx
push eax
mov bp, sp
mov dx, [bp+12]    ;addr
mov eax, [bp+16]   ;dword
out dx, eax        ;output word in eax to address in dx
pop eax
pop dx
pop bp             ;restore bp
retf

;uint32_t far cdecl inportl(uint16_t addr)
GLOBAL _inportl32

_inportl32:
push bp
mov bp, sp
mov dx,[bp+8]   ;move addr into DX 
in eax, dx      ;read 32bit port at address DX into EAX
mov edx, eax    ;copy the result to edx
shr edx, 16     ;shift the bits in edx so that dx contains the upper word
pop bp          ;skip setting SP = BP as we didn't push anything else to the stack
retf            ;return

