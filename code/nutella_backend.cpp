
#define x86_WriteAddIntInt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "add ", 4, State)
#define x86_WriteSubIntInt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "sub ", 4, State)
#define x86_WriteMulIntInt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "imul ", 5, State)
#define x86_WriteDivIntInt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "idiv ", 5, State)
inline void x86_WriteBinOpTemplate(var* Reg1, var* Reg2, char* Instr, u32 NumChars,
                                   backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString(Instr, NumChars, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString(", ", 2, &StrState);
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteCmp(string Reg1Name, string Reg2Name, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("cmp ", 4, &StrState);
    AddString(Reg1Name, &StrState);
    AddString(", ", 2, &StrState);
    AddString(Reg2Name, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteLea(string Reg1Name, var* Reg2, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("lea ", 4, &StrState);
    AddString(Reg1Name, &StrState);
    AddString(", [ebp ", 7, &StrState);
    if (Reg2->MemoryIndex < 0)
    {
        AddString("- ", 2, &StrState);
        AddString(-Reg2->MemoryIndex, &StrState);
    }
    else
    {
        AddString("+ ", 2, &StrState);
        AddString(Reg2->MemoryIndex, &StrState);
    }
    AddString("]\n", 2, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteMov(string Reg1Name, string Reg2Name, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("mov ", 4, &StrState);
    AddString(Reg1Name, &StrState);
    AddString(", ", 2, &StrState);
    AddString(Reg2Name, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteMovFromMem(string Reg1Name, string Reg2Name, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("mov ", 4, &StrState);
    AddString(Reg1Name, &StrState);
    AddString(", [", 3, &StrState);
    AddString(Reg2Name, &StrState);
    AddString("]\n", 2, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteMovToMem(string Reg1Name, string Reg2Name, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("mov dword ptr [", 15, &StrState);
    AddString(Reg1Name, &StrState);
    AddString("], ", 3, &StrState);
    AddString(Reg2Name, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteMovFromStack(var* Reg1, i32 Offset, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("mov ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString(", [ebp ", 7, &StrState);
    if (Offset < 0)
    {
        AddString("- ", 2, &StrState);
        AddString(-Offset, &StrState);
    }
    else
    {
        AddString("+ ", 2, &StrState);
        AddString(Offset, &StrState);
    }
    AddString("]\n", 2, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteMovToStack(i32 Offset, var* Reg1, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("mov dword ptr [ebp ", 19, &StrState);
    if (Offset < 0)
    {
        AddString("- ", 2, &StrState);
        AddString(-Offset, &StrState);
    }
    else
    {
        AddString("+ ", 2, &StrState);
        AddString(Offset, &StrState);
    }
    AddString("], ", 3, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteHeader(string HeaderName, backend_state* State)
{    
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString(HeaderName, &StrState);
    AddString(":\n", 2, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

#define x86_WriteJmp(HeaderName, State) x86_WriteJmpTemplate(HeaderName, "jmp ", 4, State)
#define x86_WriteJe(HeaderName, State) x86_WriteJmpTemplate(HeaderName, "je ", 3, State)
#define x86_WriteJne(HeaderName, State) x86_WriteJmpTemplate(HeaderName, "jne ", 4, State)
#define x86_WriteJg(HeaderName, State) x86_WriteJmpTemplate(HeaderName, "jg ", 3, State)
#define x86_WriteJge(HeaderName, State) x86_WriteJmpTemplate(HeaderName, "jge ", 4, State)
#define x86_WriteCallFunc(HeaderName, State) x86_WriteJmpTemplate(HeaderName, "call _", 6, State)
inline void x86_WriteJmpTemplate(string HeaderName, char* Instr, u32 NumChars, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString(Instr, NumChars, &StrState);
    AddString(HeaderName, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WritePush(var* Reg, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    if (Reg->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WritePop(var* Reg, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("pop ", 4, &StrState);
    AddString(Reg->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteAddFltInt(var* Reg1, var* Reg2, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("push ", 5, &StrState);
    if (Reg2->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fiadd dword ptr[ebp]\n", 21, &StrState);
    
    AddString("sub ebp, 4\n", 11, &StrState);
    AddString("fst dword ptr [ebp]\n", 20, &StrState);
    
    AddString("pop ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteAddFltFlt(var* Reg1, var* Reg2, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("push ", 5, &StrState);
    if (Reg2->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("fadd ST(0), ST(1)\n", 18, &StrState);
    
    AddString("sub ebp, 4\n", 11, &StrState);
    AddString("fst dword ptr [ebp]\n", 20, &StrState);
    
    AddString("pop ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteSubFltInt(var* Reg1, var* Reg2, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("push ", 5, &StrState);
    if (Reg2->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fisub dword ptr [ebp]\n", 22, &StrState);
    
    AddString("sub ebp, 4\n", 11, &StrState);
    AddString("fst dword ptr [ebp]\n", 20, &StrState);
    
    AddString("pop ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteSubFltFlt(var* Reg1, var* Reg2, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("push ", 5, &StrState);
    if (Reg2->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("fsub ST(0), ST(1)\n", 18, &StrState);
    
    AddString("sub ebp, 4\n", 11, &StrState);
    AddString("fst dword ptr [ebp]\n", 20, &StrState);
    
    AddString("pop ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteMulFltInt(var* Reg1, var* Reg2, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("push ", 5, &StrState);
    if (Reg2->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fimul dword ptr [ebp]\n", 22, &StrState);
    
    AddString("sub ebp, 4\n", 11, &StrState);
    AddString("fst dword ptr [ebp]\n", 20, &StrState);
    
    AddString("pop ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteMulFltFlt(var* Reg1, var* Reg2, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("push ", 5, &StrState);
    if (Reg2->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("fmul ST(0), ST(1)\n", 18, &StrState);
    
    AddString("sub ebp, 4\n", 11, &StrState);
    AddString("fst dword ptr [ebp]\n", 20, &StrState);
    
    AddString("pop ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteDivIntFlt(var* Reg1, var* Reg2, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fild dword ptr [ebp]\n", 21, &StrState);

    AddString("push ", 5, &StrState);
    if (Reg2->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 20, &StrState);
    AddString("fdivp\n", 6, &StrState);
    
    AddString("sub ebp, 4\n", 11, &StrState);
    AddString("fst dword ptr [ebp]\n", 20, &StrState);
    
    AddString("pop ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteDivFltInt(var* Reg1, var* Reg2, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("push ", 5, &StrState);
    if (Reg2->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fild dword ptr [ebp]\n", 21, &StrState);
    AddString("fdivp\n", 6, &StrState);
    
    AddString("sub ebp, 4\n", 11, &StrState);
    AddString("fst dword ptr [ebp]\n", 20, &StrState);
    
    AddString("pop ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteDivFltFlt(var* Reg1, var* Reg2, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("push ", 5, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);

    AddString("push ", 5, &StrState);
    if (Reg2->Flags & RegFlag_IsConstVal)
    {
        AddString("dword ptr ", 10, &StrState);
    }
    AddString(Reg2->Str, &StrState);
    AddString("\n", 1, &StrState);

    AddString("fld dword ptr[ebp]\n", 19, &StrState);
    AddString("fdivp\n", 6, &StrState);
    
    AddString("sub ebp, 4\n", 11, &StrState);
    AddString("fst dword ptr [ebp]\n", 20, &StrState);
    
    AddString("pop ", 4, &StrState);
    AddString(Reg1->Str, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void UpdateLRU(u32 Index, backend_state* State)
{
    reg_list* ListNode = State->ListNodes[Index];
    ListNode->Prev->Next = ListNode->Next;
    ListNode->Next->Prev = ListNode->Prev;

    ListNode->Next = State->LRU.Next;
    ListNode->Prev = &State->LRU;

    State->LRU.Next->Prev = ListNode;
    State->LRU.Next = ListNode;
}

inline void AllocateReg(var* Reg, backend_state* State)
{
    if (Reg->Flags & RegFlag_IsTemp)
    {
        State->TempStackOffset -= 4;
        Reg->MemoryIndex = State->TempStackOffset;
    }
    
    if (Reg->Flags & RegFlag_IsConstVal || Reg->Flags & RegFlag_IsGlobal)
    {
        // NOTE: We have a const value so we don't need to allocate a register for it
        return;
    }
    else if (Reg->Flags & RegFlag_InCPU)
    {
        // NOTE: The register is already allocated
        UpdateLRU(Reg->CPU_RegIndex, State);
        x86_WriteMovFromStack(Reg, Reg->MemoryIndex, State);
        return;
    }
    else if (State->NumEmpty > 0)
    {
        --State->NumEmpty;
        for (u32 RegIndex = 0; RegIndex < 6; ++RegIndex)
        {
            if (!State->LoadedRegisters[RegIndex])
            {
                State->LoadedRegisters[RegIndex] = Reg;
                Reg->Str = State->CPU_RegNames[RegIndex];
                Reg->CPU_RegIndex = RegIndex;
                Reg->Flags |= RegFlag_InCPU;
                
                State->ListNodes[RegIndex]->Reg = Reg;
                UpdateLRU(Reg->CPU_RegIndex, State);

                x86_WriteMovFromStack(Reg, Reg->MemoryIndex, State);
                break;
            }
        }

        return;
    }
    else
    {
        // NOTE: We swap registers with our least recently used register
        var* RegToSwap = State->LRU.Prev->Reg;        
        // NOTE: We need to update our value in memory
        x86_WriteMovToStack(RegToSwap->MemoryIndex, RegToSwap, State);
        
        Reg->Str = RegToSwap->Str;
        Reg->CPU_RegIndex = RegToSwap->CPU_RegIndex;
        Reg->Flags |= RegFlag_InCPU;
        State->LRU.Prev->Reg = Reg;
        UpdateLRU(Reg->CPU_RegIndex, State);        
        x86_WriteMovFromStack(Reg, Reg->MemoryIndex, State);
        
        RegToSwap->Flags &= !(RegFlag_InCPU);
        RegToSwap->CPU_RegIndex = 7;
        
        return;
    }
}

inline void WriteRegToMem(var* Reg, backend_state* State)
{
    if (Reg)
    {
        if (Reg->Flags & RegFlag_InCPU)
        {
            Reg->Flags &= !(RegFlag_InCPU);
            
            State->LoadedRegisters[Reg->CPU_RegIndex] = 0;
            State->ListNodes[Reg->CPU_RegIndex]->Reg = 0;
            ++State->NumEmpty;

            Reg->CPU_RegIndex = 7;
        }
        x86_WriteMovToStack(Reg->MemoryIndex, Reg, State);
    }
}

inline var* GetRegisterFromIR(backend_state* State)
{
    var* Result = *((var**)(State->CurrByte));
    State->CurrByte += sizeof(var*);
    AllocateReg(Result, State);
        
    return Result;
}

internal void ConvertIrToAssembly(mem_arena* IR_Arena, FILE* OutFile)
{
    backend_state State = {};
    State.NumEmpty = 6;
    State.CurrByte = IR_Arena->Mem;
    State.OutFile = OutFile;

    State.LRU.Next = &State.LRU;
    State.LRU.Prev = &State.LRU;
    for (u32 Index = 0; Index < 6; ++Index)
    {
        State.ListNodes[Index] = PushStruct(&TempArena, reg_list);
        State.ListNodes[Index]->Prev = &State.LRU;
        State.ListNodes[Index]->Next = State.LRU.Next;

        State.LRU.Next->Prev = State.ListNodes[Index];
        State.LRU.Next = State.ListNodes[Index];
    }

    fwrite(".486\n.MODEL FLAT\n.STACK\n.DATA\n", 30, sizeof(char), State.OutFile);
    
    // NOTE: Name our CPU registers
#if 1
    State.CPU_RegNames[0] = InitStr("EAX", 3);
    State.CPU_RegNames[1] = InitStr("EBX", 3);
    State.CPU_RegNames[2] = InitStr("ECX", 3);
    State.CPU_RegNames[3] = InitStr("EDX", 3);
    State.CPU_RegNames[4] = InitStr("ESI", 3);
    State.CPU_RegNames[5] = InitStr("EDI", 3);
#else    
    State.CPU_RegNames[0] = InitStr("r0", 2);
    State.CPU_RegNames[1] = InitStr("r1", 2);
    State.CPU_RegNames[2] = InitStr("r2", 2);
    State.CPU_RegNames[3] = InitStr("r3", 2);
    State.CPU_RegNames[4] = InitStr("r4", 2);
    State.CPU_RegNames[5] = InitStr("r5", 2);
#endif

    // NOTE: This pass over our IR calculates the stack offsets of our locals
    // NOTE: Used to know if our var is a global or not
    b32 IsInFunc = false;
    var* PrevGlobal = 0;
    // NOTE: Calculate the stack offsets of our local vars
    i32 CurrStackOffset = -4;
    u32* CurrStackSize = 0;
    i32 CurrArgStackOffset = 8;
    while (State.CurrByte < ((u8*)IR_Arena->Mem + IR_Arena->Used))
    {
        ir_instr* Instr = (ir_instr*)State.CurrByte;
        State.CurrByte += sizeof(ir_instr);

        switch(Instr->Id)
        {
            case IR_Define:
            {
                var* Register = *((var**)(State.CurrByte));

                if (IsInFunc)
                {                    
                    if (Register->MemoryIndex == 0)
                    {
                        if (Register->Flags & RegFlag_IsArg)
                        {
                            Register->MemoryIndex = CurrArgStackOffset;
                            CurrArgStackOffset += 4;
                        }
                        else
                        {
                            Register->MemoryIndex = CurrStackOffset;
                            CurrStackOffset -= 4;
                            *CurrStackSize += 4;
                        }
                    }
                }
                else
                {
                    // NOTE: We know we have a global here
                    Register->Flags |= RegFlag_IsGlobal;
                    PrevGlobal = Register;
                    
                    str_build_state StrState = StartBuildingString(&TempArena);
                    AddString(Register->Str, &StrState);
                    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State.OutFile);
                    EndBuildingString(&StrState);
                    
                    Instr->Id = IR_NoOp;
                    u32* SizeInBytes = (u32*)State.CurrByte;
                    *SizeInBytes = sizeof(var*);
                }
                
                State.CurrByte += sizeof(var*);
            } break;

            case IR_JumpHeader:
            case IR_Jump:
            {
                State.CurrByte += sizeof(string);
            } break;
            
            case IR_JumpCond:
            {
                State.CurrByte += sizeof(var*);
                State.CurrByte += sizeof(var*);
                State.CurrByte += sizeof(string);
            } break;

            case IR_Assign:
            case IR_AssignToMem:
            case IR_AddressOff:
            case IR_LoadFromMem:
            {
                if (PrevGlobal)
                {
                    Assert(Instr->Id == IR_Assign);
                    var* GlobalReg = *((var**)(State.CurrByte));
                    var* AssignReg = *((var**)(State.CurrByte + sizeof(var*)));
                    Assert(PrevGlobal == GlobalReg);
                    
                    str_build_state StrState = StartBuildingString(&TempArena);
                    AddString(" DD ", 4, &StrState);
                    AddString(AssignReg->Str, &StrState);
                    AddString("\n", 1, &StrState);
                    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State.OutFile);
                    EndBuildingString(&StrState);
                    
                    PrevGlobal = 0;

                    Instr->Id = IR_NoOp;
                    u32* SizeInBytes = (u32*)State.CurrByte;
                    *SizeInBytes = 2*sizeof(var*);
                }
                
                State.CurrByte += sizeof(var*);
                State.CurrByte += sizeof(var*);
            } break;

            case IR_Equal:
            case IR_NotEqual:
            case IR_Less:
            case IR_LessEqual:
            case IR_Greater:
            case IR_GreaterEqual:
            case IR_AddIntInt:
            case IR_AddFltInt:
            case IR_AddFltFlt:
            case IR_SubIntInt:
            case IR_SubFltInt:
            case IR_SubFltFlt:
            case IR_MulIntInt:
            case IR_MulFltInt:
            case IR_MulFltFlt:
            case IR_DivIntInt:
            case IR_DivFltInt:
            case IR_DivIntFlt:
            case IR_DivFltFlt:
            {
                State.CurrByte += sizeof(var*);
                State.CurrByte += sizeof(var*);
                State.CurrByte += sizeof(var*);
            } break;

            case IR_FuncDecl:
            {
                State.CurrByte += sizeof(function*);
                CurrStackSize = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);
                CurrStackOffset = -4;
                CurrArgStackOffset = 8;
                IsInFunc = true;
            } break;

            case IR_FuncEnd:
            {
                State.CurrByte += sizeof(function*);
                CurrStackSize = 0;
                IsInFunc = false;
            } break;
            
            case IR_FuncCall:
            {
                State.CurrByte += sizeof(b32);
                State.CurrByte += sizeof(var*);
                State.CurrByte += sizeof(string);
                u32* NumArgs = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);
                State.CurrByte += sizeof(var*)*(*NumArgs);
            } break;

            case IR_Return:
            {
                State.CurrByte += sizeof(var*);
                State.CurrByte += sizeof(function*);
            } break;
            
            case IR_ClearRegs:
            {
            } break;
            
            default:
            {
                InvalidCodePath;
            } break;
        }
    }

    fwrite(".CODE\n", 6, sizeof(char), State.OutFile);
    
    // NOTE: Convert ir to x86
    function* CurrFunc = 0;
    State.CurrByte = IR_Arena->Mem;
    while (State.CurrByte < ((u8*)IR_Arena->Mem + IR_Arena->Used))
    {
        ir_instr* Instr = (ir_instr*)State.CurrByte;
        State.CurrByte += sizeof(ir_instr);

        switch(Instr->Id)
        {
            case IR_NoOp:
            {
                u32* SizeInBytes = (u32*)State.CurrByte;
                State.CurrByte += *SizeInBytes;
            } break;
            
            case IR_ClearRegs:
            {
                // NOTE: This instr clears our CPU regs and loads their values to memory
                for (u32 RegIndex = 0; RegIndex < 6; ++RegIndex)
                {
                    WriteRegToMem(State.LoadedRegisters[RegIndex], &State);
                }
            } break;
            
            case IR_Define:
            {
                var* Register = GetRegisterFromIR(&State);
            } break;
            
            case IR_JumpHeader:
            {
                string* HeaderName = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                
                x86_WriteHeader(*HeaderName, &State);
            } break;

            case IR_Jump:
            {
                string* HeaderName = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);

                x86_WriteJmp(*HeaderName, &State);
            } break;

            case IR_JumpCond:
            {
                var* LHS = GetRegisterFromIR(&State);
                var* RHS = GetRegisterFromIR(&State);
                string* HeaderName = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);

                x86_WriteCmp(LHS->Str, RHS->Str, &State);
                x86_WriteJe(*HeaderName, &State);
            } break;
            
            case IR_Assign:
            {
                var* LHS = GetRegisterFromIR(&State);
                var* RHS = GetRegisterFromIR(&State);

                if (LHS->Flags & RegFlag_IsGlobal &&
                    RHS->Flags & RegFlag_IsGlobal)
                {
                    x86_WritePush(RHS, &State);
                    x86_WritePop(LHS, &State);
                }
                else
                {
                    x86_WriteMov(LHS->Str, RHS->Str, &State);
                    if (!(LHS->Flags & RegFlag_IsGlobal))
                    {
                        WriteRegToMem(LHS, &State);
                    }
                }
            } break;

            case IR_AssignToMem:
            {
                var* LHS = GetRegisterFromIR(&State);
                var* RHS = GetRegisterFromIR(&State);

                if (RHS->Flags & RegFlag_IsGlobal)
                {
                    x86_WritePush(RHS, &State);
                    x86_WritePop(LHS, &State);
                }
                else
                {
                    x86_WriteMovToMem(LHS->Str, RHS->Str, &State);
                }
            } break;

            case IR_AddressOff:
            {
                var* Val = GetRegisterFromIR(&State);
                var* Result = GetRegisterFromIR(&State);

                x86_WriteLea(Result->Str, Val, &State);
                WriteRegToMem(Result, &State);
            } break;

            case IR_LoadFromMem:
            {
                var* Pointer = GetRegisterFromIR(&State);
                var* Result = GetRegisterFromIR(&State);
                
                x86_WriteMovFromMem(Result->Str, Pointer->Str, &State);
                WriteRegToMem(Result, &State);
            } break;

            case IR_Equal:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);
                
                x86_WriteCmp(Arg1->Str, Arg2->Str, &State);
                x86_WriteJne(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                WriteRegToMem(Return, &State);
            } break;

            case IR_NotEqual:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                x86_WriteCmp(Arg1->Str, Arg2->Str, &State);
                x86_WriteJne(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                WriteRegToMem(Return, &State);
            } break;

            case IR_Less:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                x86_WriteCmp(Arg1->Str, Arg2->Str, &State);
                x86_WriteJge(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                WriteRegToMem(Return, &State);
            } break;

            case IR_LessEqual:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                x86_WriteCmp(Arg1->Str, Arg2->Str, &State);
                x86_WriteJg(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                WriteRegToMem(Return, &State);
            } break;

            case IR_Greater:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                x86_WriteCmp(Arg1->Str, Arg2->Str, &State);
                x86_WriteJge(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                WriteRegToMem(Return, &State);
            } break;

            case IR_GreaterEqual:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                x86_WriteCmp(Arg1->Str, Arg2->Str, &State);
                x86_WriteJge(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Str, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                WriteRegToMem(Return, &State);
            } break;

            case IR_AddIntInt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteAddIntInt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_AddFltInt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteAddFltInt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_AddFltFlt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteAddFltFlt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_SubIntInt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteSubIntInt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_SubFltInt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteSubFltInt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_SubFltFlt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteSubFltFlt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_MulIntInt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteMulIntInt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_MulFltInt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteMulFltInt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_MulFltFlt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteMulFltFlt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_DivIntInt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteDivIntInt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_DivFltInt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteDivFltInt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_DivIntFlt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteDivIntFlt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;

            case IR_DivFltFlt:
            {
                var* Arg1 = GetRegisterFromIR(&State);
                var* Arg2 = GetRegisterFromIR(&State);
                var* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Str, Arg1->Str, &State);
                x86_WriteDivFltFlt(Return, Arg2, &State);

                WriteRegToMem(Return, &State);
            } break;
            
            case IR_FuncDecl:
            {
                function* Func = *(function**)State.CurrByte;
                State.CurrByte += sizeof(function*);
                u32* StackSize = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);
                State.TempStackOffset = -(i32)(*StackSize);
                
                CurrFunc = Func;
                
                str_build_state StrState = StartBuildingString(&TempArena);
                AddString("PUBLIC _", 8, &StrState);
                AddString(Func->Name, &StrState);
                AddString("\n", 1, &StrState);
                AddString("_", 1, &StrState);
                AddString(Func->Name, &StrState);
                AddString(" PROC\n", 6, &StrState);
                // NOTE: Save stack pointer
                AddString("push ebp\n", 9, &StrState);
                AddString("mov ebp, esp\n", 13, &StrState);
                AddString("sub esp, ", 9, &StrState);
                AddString(*StackSize, &StrState);
                AddString("\n", 1, &StrState);
                fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State.OutFile);
            } break;

            case IR_FuncEnd:
            {
                function* Func = *((function**)State.CurrByte);
                State.CurrByte += sizeof(function*);
                
                str_build_state StrState = StartBuildingString(&TempArena);
                AddString(Func->Name, &StrState);
                AddString("_End:\n", 6, &StrState);
                fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State.OutFile);
                EndBuildingString(&StrState);

                fwrite("mov esp, ebp\n", 13, sizeof(char), State.OutFile);
                fwrite("pop ebp\n", 8, sizeof(char), State.OutFile);
                
                fwrite("ret\n\n", 5, sizeof(char), State.OutFile);

                for (u32 RegIndex = 0; RegIndex < 6; ++RegIndex)
                {
                    WriteRegToMem(State.LoadedRegisters[RegIndex], &State);
                }

                StrState = StartBuildingString(&TempArena);
                AddString("_", 1, &StrState);
                AddString(Func->Name, &StrState);
                AddString(" ENDP\n\n", 7, &StrState);
                fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State.OutFile);
                EndBuildingString(&StrState);
                
            } break;
            
            case IR_FuncCall:
            {
                b32* HasReturnVal = (b32*)State.CurrByte;
                State.CurrByte += sizeof(b32);
                var* Result = *((var**)State.CurrByte);
                State.CurrByte += sizeof(var*);
                string* Name = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                u32* NumArgs = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);

                // NOTE: Write out our registers to memory
                WriteRegToMem(State.LoadedRegisters[0], &State);
                WriteRegToMem(State.LoadedRegisters[1], &State);
                WriteRegToMem(State.LoadedRegisters[2], &State);
                WriteRegToMem(State.LoadedRegisters[3], &State);
                WriteRegToMem(State.LoadedRegisters[4], &State);
                WriteRegToMem(State.LoadedRegisters[5], &State);
                
                // NOTE: Push arguments onto the stack in reverse order
                u32 StackOffset = 8;
                for (i32 ArgIndex = (*NumArgs) - 1; ArgIndex >= 0; --ArgIndex)
                {
                    // NOTE: Since esp gets changed, we need to adjust the memory locations
                    // of our variables here
                    var* ArgName = *((var**)State.CurrByte);
                    ArgName->MemoryIndex += (ArgIndex - *NumArgs + 1)*4;
                    ArgName = GetRegisterFromIR(&State);
                    x86_WritePush(ArgName, &State);
                    // NOTE: Restore the memory index
                    ArgName->MemoryIndex -= (ArgIndex - *NumArgs + 1)*4;
                }
                x86_WriteCallFunc(*Name, &State);

                // NOTE: Free locals
                str_build_state StrState = StartBuildingString(&TempArena);
                AddString("add esp, ", 9, &StrState);
                AddString((*NumArgs)*4, &StrState);
                AddString("\n", 1, &StrState);
                fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State.OutFile);
                EndBuildingString(&StrState);

                // NOTE: If we have a return value, store it 
                if (HasReturnVal)
                {
                    // NOTE: We follow convention of taking the EAX register to store the ret val
                    // NOTE: We have to fake here that EAX is in use (the registers are empty since
                    // we freed then above)
                    State.LoadedRegisters[EAX_REG_INDEX] = Result + 1;
                    AllocateReg(Result, &State);
                    State.LoadedRegisters[EAX_REG_INDEX] = 0;
                    x86_WriteMov(Result->Str, State.CPU_RegNames[EAX_REG_INDEX], &State);
                    // NOTE: We write this result value to memory incase it gets overwritten
                    // as we pop registers
                    WriteRegToMem(Result, &State);
                }
            } break;

            case IR_Return:
            {
                var* Reg = GetRegisterFromIR(&State);
                function* Func = *((function**)State.CurrByte);
                State.CurrByte += sizeof(function*);

                if (Reg)
                {
                    x86_WriteMov(State.CPU_RegNames[EAX_REG_INDEX], Reg->Str, &State);
                    WriteRegToMem(Reg, &State);
                }
                
                str_build_state StrState = StartBuildingString(&TempArena);
                AddString(CurrFunc->Name, &StrState);
                AddString("_End", 4, &StrState);
                x86_WriteJmp(StrState.Str, &State);
                EndBuildingString(&StrState);
            } break;
            
            default:
            {
                InvalidCodePath;
            } break;
        }
    }

    fwrite("END _WinMainCRTStartup\n", 23, sizeof(char), State.OutFile); 
    
    ClearArena(IR_Arena);
}
