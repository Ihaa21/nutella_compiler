.486
.MODEL FLAT
.STACK
.DATA
.CODE
PUBLIC _WinMainCRTStartup
_WinMainCRTStartup PROC
push ebp
mov ebp, esp
sub esp, 36
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
mov EAX, [ebp - 12]
mov EBX, [ebp - 4]
mov ECX, [ebp - 40]
lea ECX, [ebp - 4]
mov dword ptr [ebp - 40], ECX
mov EAX, [ebp - 12]
mov ECX, [ebp - 40]
mov EAX, ECX
mov dword ptr [ebp - 12], EAX
mov EAX, [ebp - 44]
mov EDX, [ebp - 12]
mov EAX, [ebp - 48]
mov EAX, EDX
sub EAX, 4
mov dword ptr [ebp - 48], EAX
mov EAX, [ebp - 48]
mov dword ptr [EAX], 3
mov ESI, [ebp - 20]
mov EDI, [ebp - 24]
mov dword ptr [ebp - 4], EBX
mov EBX, [ebp - 28]
mov dword ptr [ebp - 40], ECX
mov ECX, [ebp - 32]
mov EDX, [ebp - 12]
mov EBX, [ebp - 28]
mov EBX, EDX
sub EBX, 0
mov dword ptr [ebp - 28], EBX
mov EBX, [ebp - 28]
mov EBX, [ebp - 28]
mov EBX, [EBX]
mov dword ptr [ebp - 28], EBX
mov EDX, [ebp - 12]
mov ECX, [ebp - 32]
mov ECX, EDX
sub ECX, 4
mov dword ptr [ebp - 32], ECX
mov EBX, [ebp - 32]
mov EBX, [ebp - 32]
mov EBX, [EBX]
mov dword ptr [ebp - 32], EBX
mov ESI, [ebp - 20]
mov EBX, [ebp - 28]
mov ESI, EBX
mov dword ptr [ebp - 20], ESI
mov EDI, [ebp - 24]
mov ECX, [ebp - 32]
mov EDI, ECX
mov dword ptr [ebp - 24], EDI
mov ESI, [ebp - 36]
mov EDI, [ebp - 20]
mov dword ptr [ebp - 48], EAX
mov EAX, [ebp - 24]
mov dword ptr [ebp - 12], EDX
mov EDX, [ebp - 52]
mov EDX, EDI
add EDX, EAX
mov dword ptr [ebp - 52], EDX
mov ESI, [ebp - 36]
mov EDX, [ebp - 52]
mov ESI, EDX
mov dword ptr [ebp - 36], ESI
WinMainCRTStartup_End:
mov esp, ebp
pop ebp
ret

mov dword ptr [ebp - 48], EAX
mov dword ptr [ebp - 28], EBX
mov dword ptr [ebp - 32], ECX
mov dword ptr [ebp - 52], EDX
mov dword ptr [ebp - 20], EDI
_WinMainCRTStartup ENDP

END _WinMainCRTStartup
