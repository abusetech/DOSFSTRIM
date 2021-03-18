;uint32_t far cdecl inportl(uint16_t addr)
[BITS 16]
section .text
GLOBAL _inportl32
_inportl32:
push bp
mov bp, sp
mov dx,[bp+6]   ;move addr into DX (pushed to SP+6 before call) 
in eax, dx      ;read 32bit port at address DX into EAX
mov edx, eax    ;copy the result to edx
shr edx, 16     ;shift the bits in edx so that dx contains the upper word
pop bp          ;skip setting SP = BP as we didn't push anything else to the stack
retf            ;return
