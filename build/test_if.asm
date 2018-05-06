.486
.MODEL FLAT
.STACK
.DATA
.CODE
PUBLIC _RunIf
_RunIf PROC
push ebp
mov ebp, esp
sub esp, 0
mov EAX, [ebp + 8]
mov EBX, [ebp - 4]
mov EAX, [ebp + 8]
mov EBX, EAX
mov dword ptr [ebp - 4], EBX
mov EBX, [ebp - 4]
mov EBX, [ebp - 4]
cmp EBX, 5
jge TEMP0
mov EBX, 1
jmp TEMP1
TEMP0:
mov EBX, 0
TEMP1:
mov dword ptr [ebp - 4], EBX
mov dword ptr [ebp + 8], EAX
mov EAX, [ebp - 4]
cmp EAX, 0
je EXIT_IF2
mov EAX, 0
mov dword ptr [ebp + 0], 0
jmp RunIf_End
jmp EXIT_IF1
EXIT_IF2:
mov dword ptr [ebp - 4], EAX
mov EAX, 0
mov dword ptr [ebp + 0], 0
jmp RunIf_End
EXIT_IF1:
RunIf_End:
mov esp, ebp
pop ebp
ret

_RunIf ENDP

PUBLIC _WinMainCRTStartup
_WinMainCRTStartup PROC
push ebp
mov ebp, esp
sub esp, 4
mov EAX, [ebp - 4]
mov EAX, [ebp - 4]
mov EAX, 0
mov dword ptr [ebp - 4], EAX
WinMainCRTStartup_End:
mov esp, ebp
pop ebp
ret

_WinMainCRTStartup ENDP

END _WinMainCRTStartup
