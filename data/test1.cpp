.486
.MODEL FLAT
.STACK
.DATA
.CODE
PUBLIC _DotProduct
_DotProduct PROC
push ebp
mov ebp, esp
sub esp, 4
mov EAX, [ebp + 8]
mov EBX, [ebp + 12]
mov ECX, [ebp + 16]
mov EDX, [ebp + 20]
mov ESI, [ebp - 4]
mov EAX, [ebp + 8]
mov ECX, [ebp + 16]
mov EDI, [ebp - 8]
mov EDI, EAX
imul EDI, ECX
mov dword ptr [ebp - 8], EDI
mov EBX, [ebp + 12]
mov EDX, [ebp + 20]
mov EDI, [ebp - 12]
mov EDI, EBX
imul EDI, EDX
mov dword ptr [ebp - 12], EDI
mov EDI, [ebp - 8]
mov dword ptr [ebp - 4], ESI
mov ESI, [ebp - 12]
mov dword ptr [ebp + 8], EAX
mov EAX, [ebp - 16]
mov EAX, EDI
add EAX, ESI
mov dword ptr [ebp - 16], EAX
mov EAX, [ebp - 4]
mov dword ptr [ebp + 16], ECX
mov ECX, [ebp - 16]
mov EAX, ECX
mov dword ptr [ebp - 4], EAX
mov EAX, [ebp - 4]
mov EAX, EAX
mov dword ptr [ebp - 4], EAX
jmp DotProduct_End
DotProduct_End:
mov esp, ebp
pop ebp
ret

mov dword ptr [ebp + 12], EBX
mov dword ptr [ebp + 16], ECX
mov dword ptr [ebp + 20], EDX
mov dword ptr [ebp - 4], EAX
mov dword ptr [ebp - 8], EDI
_DotProduct ENDP

PUBLIC _DotProductPtr
_DotProductPtr PROC
push ebp
mov ebp, esp
sub esp, 20
mov EAX, [ebp + 8]
mov EBX, [ebp + 12]
mov EDX, [ebp - 4]
mov EDI, [ebp - 8]
mov EAX, [ebp + 8]
mov EDI, [ebp - 8]
mov EDI, EAX
sub EDI, 0
mov dword ptr [ebp - 8], EDI
mov EDI, [ebp - 8]
mov EDI, [ebp - 8]
mov EDI, [EDI]
mov dword ptr [ebp - 8], EDI
mov EDI, [ebp - 12]
mov EBX, [ebp + 12]
mov EDI, [ebp - 12]
mov EDI, EBX
sub EDI, 0
mov dword ptr [ebp - 12], EDI
mov EDI, [ebp - 12]
mov EDI, [ebp - 12]
mov EDI, [EDI]
mov dword ptr [ebp - 12], EDI
mov EDI, [ebp - 8]
mov dword ptr [ebp - 12], ESI
mov ESI, [ebp - 12]
mov dword ptr [ebp - 16], ECX
mov ECX, [ebp - 24]
mov ECX, EDI
imul ECX, ESI
mov dword ptr [ebp - 24], ECX
mov ECX, [ebp - 16]
mov EAX, [ebp + 8]
mov ECX, [ebp - 16]
mov ECX, EAX
sub ECX, 4
mov dword ptr [ebp - 16], ECX
mov ECX, [ebp - 16]
mov ECX, [ebp - 16]
mov ECX, [ECX]
mov dword ptr [ebp - 16], ECX
mov ECX, [ebp - 20]
mov EBX, [ebp + 12]
mov ECX, [ebp - 20]
mov ECX, EBX
sub ECX, 4
mov dword ptr [ebp - 20], ECX
mov ECX, [ebp - 20]
mov ECX, [ebp - 20]
mov ECX, [ECX]
mov dword ptr [ebp - 20], ECX
mov ECX, [ebp - 16]
mov dword ptr [ebp - 4], EDX
mov EDX, [ebp - 20]
mov dword ptr [ebp - 8], EDI
mov EDI, [ebp - 28]
mov EDI, ECX
imul EDI, EDX
mov dword ptr [ebp - 28], EDI
mov EDI, [ebp - 24]
mov dword ptr [ebp - 12], ESI
mov ESI, [ebp - 28]
mov dword ptr [ebp + 8], EAX
mov EAX, [ebp - 32]
mov EAX, EDI
add EAX, ESI
mov dword ptr [ebp - 32], EAX
mov EAX, [ebp - 4]
mov dword ptr [ebp + 12], EBX
mov EBX, [ebp - 32]
mov EAX, EBX
mov dword ptr [ebp - 4], EAX
mov EAX, [ebp - 4]
mov EAX, EAX
mov dword ptr [ebp - 4], EAX
jmp DotProductPtr_End
DotProductPtr_End:
mov esp, ebp
pop ebp
ret

mov dword ptr [ebp + 12], EBX
mov dword ptr [ebp - 16], ECX
mov dword ptr [ebp - 4], EAX
mov dword ptr [ebp - 4], EAX
mov dword ptr [ebp - 24], EDI
_DotProductPtr ENDP

PUBLIC _WinMainCRTStartup
_WinMainCRTStartup PROC
push ebp
mov ebp, esp
sub esp, 24
mov EAX, [ebp - 4]
mov ECX, [ebp - 8]
mov EAX, [ebp - 4]
mov EAX, 0
mov dword ptr [ebp - 4], EAX
mov ECX, [ebp - 8]
mov ECX, 0
mov dword ptr [ebp - 8], ECX
mov EAX, [ebp - 4]
mov EAX, 5
mov dword ptr [ebp - 4], EAX
mov EAX, [ebp - 8]
mov EAX, 6
mov dword ptr [ebp - 8], EAX
mov EAX, [ebp - 12]
mov ECX, [ebp - 16]
mov EAX, [ebp - 12]
mov EAX, 0
mov dword ptr [ebp - 12], EAX
mov ECX, [ebp - 16]
mov ECX, 0
mov dword ptr [ebp - 16], ECX
mov EAX, [ebp - 12]
mov EAX, 10
mov dword ptr [ebp - 12], EAX
mov EAX, [ebp - 16]
mov EAX, 15
mov dword ptr [ebp - 16], EAX
mov EAX, [ebp - 20]
mov dword ptr [ebp - 20], EAX
mov dword ptr [ebp + 12], EBX
mov dword ptr [ebp - 4], EAX
mov dword ptr [ebp - 4], EAX
mov EAX, [ebp - 4]
push EAX
mov ECX, [ebp - 12]
push ECX
mov EDI, [ebp - 20]
push EDI
mov dword ptr [ebp - 20], EDX
mov EDX, [ebp - 28]
push EDX
call _DotProduct
add esp, 16
mov dword ptr [ebp - 28], ESI
mov ESI, [ebp - 28]
mov ESI, EAX
mov dword ptr [ebp - 28], ESI
mov EAX, [ebp - 20]
mov dword ptr [ebp - 32], EBX
mov EBX, [ebp - 28]
mov EAX, EBX
mov dword ptr [ebp - 20], EAX
mov EAX, [ebp - 24]
mov EAX, [ebp - 4]
mov dword ptr [ebp - 8], ECX
mov ECX, [ebp - 32]
lea ECX, [ebp - 4]
mov dword ptr [ebp - 32], ECX
mov EDI, [ebp - 12]
mov ECX, [ebp - 36]
lea ECX, [ebp - 12]
mov dword ptr [ebp - 36], ECX
mov dword ptr [ebp - 24], EAX
mov dword ptr [ebp + 12], EBX
mov dword ptr [ebp - 4], EAX
mov dword ptr [ebp - 12], EDI
mov EAX, [ebp - 32]
push EAX
mov ECX, [ebp - 40]
push ECX
call _DotProductPtr
add esp, 8
mov ESI, [ebp - 40]
mov ESI, EAX
mov dword ptr [ebp - 40], ESI
mov EAX, [ebp - 24]
mov dword ptr [ebp - 16], EDX
mov EDX, [ebp - 40]
mov EAX, EDX
mov dword ptr [ebp - 24], EAX
WinMainCRTStartup_End:
mov esp, ebp
pop ebp
ret

mov dword ptr [ebp + 12], EBX
mov dword ptr [ebp - 36], ECX
mov dword ptr [ebp - 4], EAX
_WinMainCRTStartup ENDP

END _WinMainCRTStartup
