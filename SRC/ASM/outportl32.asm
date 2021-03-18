;void far cdecl outportl(uint16_t addr, uint32_t dword);
[BITS 16] 
GLOBAL _outportl32
section .text
_outportl32:
push bp
push dx
push eax
mov bp, sp
mov dx, [bp+16]    ;addr
mov eax, [bp+12]   ;dword
out dx, eax        ;output word in eax to address in dx
pop eax
pop dx
pop bp             ;restore bp
retf
