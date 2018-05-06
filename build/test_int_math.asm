.486
.MODEL FLAT
.STACK
.DATA
.CODE
PUBLIC _RunMath
_RunMath PROC
push ebp
mov ebp, esp
sub esp, 0
mov EAX, [ebp + 8]
mov EBX, [ebp + 12]
mov EAX, [ebp + 8]
mov ECX, [ebp - 4]
mov ECX, EAX
imul ECX, 3
mov dword ptr [ebp - 4], ECX
mov ECX, [ebp - 4]
mov EBX, [ebp + 12]
mov EDX, [ebp - 8]
mov EDX, ECX
imul EDX, EBX
mov dword ptr [ebp - 8], EDX
mov EDX, [ebp - 8]
mov ESI, [ebp - 12]
mov ESI, EDX
sub ESI, 4
mov dword ptr [ebp - 12], ESI
mov ESI, [ebp - 12]
mov EDI, [ebp - 16]
mov EDI, ESI
add EDI, 4
mov dword ptr [ebp - 16], EDI
mov EDI, [ebp - 16]
mov EAX, EDI
mov dword ptr [ebp - 16], EDI
jmp RunMath_End
RunMath_End:
mov esp, ebp
pop ebp
ret

mov dword ptr [ebp + 8], EAX
mov dword ptr [ebp + 12], EBX
mov dword ptr [ebp - 4], ECX
mov dword ptr [ebp - 8], EDX
mov dword ptr [ebp - 12], ESI
_RunMath ENDP

PUBLIC _WinMainCRTStartup
_WinMainCRTStartup PROC
push ebp
mov ebp, esp
sub esp, 4
mov EAX, [ebp - 4]
mov EAX, [ebp - 4]
mov EAX, 36
mov dword ptr [ebp - 4], EAX
WinMainCRTStartup_End:
mov esp, ebp
pop ebp
ret

_WinMainCRTStartup ENDP

END _WinMainCRTStartup
