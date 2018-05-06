.486
.MODEL FLAT
.STACK
.DATA
.CODE
PUBLIC _RunPtrs
_RunPtrs PROC
push ebp
mov ebp, esp
sub esp, 8
mov EAX, [ebp - 4]
mov EAX, [ebp - 4]
mov EAX, 3
mov dword ptr [ebp - 4], EAX
mov EAX, [ebp - 8]
mov EBX, [ebp - 4]
mov ECX, [ebp - 12]
lea ECX, [ebp - 4]
mov dword ptr [ebp - 12], ECX
mov EAX, [ebp - 8]
mov ECX, [ebp - 12]
mov EAX, ECX
mov dword ptr [ebp - 8], EAX
mov EAX, [ebp - 8]
mov dword ptr [EAX], 4
mov EBX, [ebp - 4]
mov EDX, [ebp - 16]
mov EDX, EBX
add EDX, 3
mov dword ptr [ebp - 16], EDX
mov EBX, [ebp - 4]
mov EDX, [ebp - 16]
mov EBX, EDX
mov dword ptr [ebp - 4], EBX
mov EBX, [ebp - 4]
mov EAX, EBX
mov dword ptr [ebp - 4], EBX
jmp RunPtrs_End
RunPtrs_End:
mov esp, ebp
pop ebp
ret

mov dword ptr [ebp - 8], EAX
mov dword ptr [ebp - 12], ECX
mov dword ptr [ebp - 16], EDX
_RunPtrs ENDP

PUBLIC _WinMainCRTStartup
_WinMainCRTStartup PROC
push ebp
mov ebp, esp
sub esp, 4
mov EAX, [ebp - 4]
mov EAX, [ebp - 4]
mov EAX, 7
mov dword ptr [ebp - 4], EAX
WinMainCRTStartup_End:
mov esp, ebp
pop ebp
ret

_WinMainCRTStartup ENDP

END _WinMainCRTStartup
