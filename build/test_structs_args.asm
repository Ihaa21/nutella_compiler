.486
.MODEL FLAT
.STACK
.DATA
.CODE
PUBLIC _VecOp
_VecOp PROC
push ebp
mov ebp, esp
sub esp, 0
mov EAX, [ebp + 8]
mov EBX, [ebp + 12]
mov EAX, [ebp + 8]
mov EBX, [ebp + 12]
mov ECX, [ebp - 4]
mov ECX, EAX
add ECX, EBX
mov dword ptr [ebp - 4], ECX
mov ECX, [ebp - 4]
mov EAX, ECX
mov dword ptr [ebp - 4], ECX
jmp VecOp_End
VecOp_End:
mov esp, ebp
pop ebp
ret

mov dword ptr [ebp + 8], EAX
mov dword ptr [ebp + 12], EBX
_VecOp ENDP

PUBLIC _WinMainCRTStartup
_WinMainCRTStartup PROC
push ebp
mov ebp, esp
sub esp, 12
mov EAX, [ebp - 4]
mov EBX, [ebp - 8]
mov EAX, [ebp - 4]
mov EAX, 0
mov dword ptr [ebp - 4], EAX
mov EBX, [ebp - 8]
mov EBX, 0
mov dword ptr [ebp - 8], EBX
mov EAX, [ebp - 4]
mov EAX, 1
mov dword ptr [ebp - 4], EAX
mov EAX, [ebp - 8]
mov EAX, 3
mov dword ptr [ebp - 8], EAX
mov EAX, [ebp - 12]
mov dword ptr [ebp - 12], EAX
mov EAX, [ebp - 4]
push EAX
mov EBX, [ebp - 12]
push EBX
call _VecOp
add esp, 8
mov ECX, [ebp - 16]
mov ECX, EAX
mov dword ptr [ebp - 16], ECX
mov EAX, [ebp - 12]
mov ECX, [ebp - 16]
mov EAX, ECX
mov dword ptr [ebp - 12], EAX
WinMainCRTStartup_End:
mov esp, ebp
pop ebp
ret

mov dword ptr [ebp - 8], EBX
mov dword ptr [ebp - 16], ECX
_WinMainCRTStartup ENDP

END _WinMainCRTStartup
