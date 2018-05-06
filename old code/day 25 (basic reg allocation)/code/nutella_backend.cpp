
#define x86_WriteAddIntInt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "add ", 4, State)
#define x86_WriteAddFltInt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "fiadd ", 6, State)
#define x86_WriteAddFltFlt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "fadd ", 5, State)
#define x86_WriteSubIntInt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "sub ", 4, State)
#define x86_WriteSubFltInt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "fisub ", 6, State)
#define x86_WriteSubFltFlt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "fsub ", 5, State)
#define x86_WriteMulIntInt(Reg1, Reg2, State) x86_WriteBinOpTemplate(Reg1, Reg2, "imul ", 5, State)
inline void x86_WriteBinOpTemplate(virtual_reg* Reg1, virtual_reg* Reg2, char* Instr, u32 NumChars,
                                   backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString(Instr, NumChars, &StrState);
    AddString(Reg1->Name, &StrState);
    AddString(", ", 2, &StrState);
    AddString(Reg2->Name, &StrState);
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

inline void x86_WriteLea(string Reg1Name, string Reg2Name, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("lea ", 4, &StrState);
    AddString(Reg1Name, &StrState);
    AddString(", [", 3, &StrState);
    AddString(Reg2Name, &StrState);
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
    AddString("mov [", 5, &StrState);
    AddString(Reg1Name, &StrState);
    AddString("], ", 3, &StrState);
    AddString(Reg2Name, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteMovFromStack(virtual_reg* Reg1, u32 Offset, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("mov ", 4, &StrState);
    AddString(Reg1->Name, &StrState);
    AddString(", [esp + ", 9, &StrState);
    AddString(Offset, &StrState);
    AddString("]\n", 2, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WriteMovToStack(u32 Offset, virtual_reg* Reg1, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("mov [esp + ", 11, &StrState);
    AddString(Offset, &StrState);
    AddString("], ", 3, &StrState);
    AddString(Reg1->Name, &StrState);
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
#define x86_WriteCallFunc(HeaderName, State) x86_WriteJmpTemplate(HeaderName, "call ", 5, State)
#define x86_WritePush(HeaderName, State) x86_WriteJmpTemplate(HeaderName, "push ", 5, State)
#define x86_WritePop(HeaderName, State) x86_WriteJmpTemplate(HeaderName, "pop ", 4, State)
inline void x86_WriteJmpTemplate(string HeaderName, char* Instr, u32 NumChars, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString(Instr, NumChars, &StrState);
    AddString(HeaderName, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void PushRegisterOntoStack(u32 Index, backend_state* State)
{
    virtual_reg* Reg = State->LoadedRegisters[Index];
    virtual_reg** StackReg = PushStruct(&State->RegAllocStack, virtual_reg*);
    *StackReg = Reg;
    State->LoadedRegisters[Index] = 0;
    x86_WritePush(State->CPU_RegNames[Index], State);
}

inline void PopRegistersFromStack(u32 Index, backend_state* State)
{
    virtual_reg* Reg = (virtual_reg*)(State->RegAllocStack.Mem + State->RegAllocStack.Used -
                        sizeof(virtual_reg*));
    State->RegAllocStack.Used -= sizeof(virtual_reg*);
    State->LoadedRegisters[Index] = Reg;
    x86_WritePop(State->CPU_RegNames[Index], State);
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

inline void AllocateReg(virtual_reg* Reg, backend_state* State)
{
    if (Reg->Flags & RegFlag_IsConstVal)
    {
        // NOTE: We have a const value so we don't need to allocate a register for it
        return;
    }
    else if (Reg->Flags & RegFlag_InCPU)
    {
        // NOTE: The register is already allocated
        UpdateLRU(Reg->CPU_RegIndex, State);
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
                Reg->Name = State->CPU_RegNames[RegIndex];
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
        virtual_reg* RegToSwap = State->LRU.Prev->Reg;        
        if (RegToSwap->Flags & RegFlag_LoadToMem)
        {
            // NOTE: We need to swap out our value
            x86_WriteMovToStack(RegToSwap->MemoryIndex, RegToSwap, State);
        }
        
        Reg->Name = RegToSwap->Name;
        Reg->CPU_RegIndex = RegToSwap->CPU_RegIndex;
        Reg->Flags |= RegFlag_InCPU;
        State->LRU.Prev->Reg = Reg;
        UpdateLRU(Reg->CPU_RegIndex, State);        
        x86_WriteMovFromStack(Reg, Reg->MemoryIndex, State);
                
        RegToSwap->Flags &= !(RegFlag_LoadToMem);
        RegToSwap->Flags &= !(RegFlag_InCPU);
        RegToSwap->CPU_RegIndex = 7;
        
        return;
    }
}

inline virtual_reg* GetRegisterFromIR(backend_state* State)
{
    virtual_reg* Result = *((virtual_reg**)(State->CurrByte));
    State->CurrByte += sizeof(virtual_reg*);
    
    // NOTE: If we have a constant, we don't assign a cpu register to it
    if (Result->Flags & RegFlag_IsConstVal ||
        Result->Flags & RegFlag_InCPU)
    {
        // TODO: Convert true and false here to 0 and 1?
        return Result;
    }
    
    // NOTE: We assign a cpu register to this virtual register
    if (State->NumEmpty > 0)
    {
        // NOTE: Search for our emtpy register
        --State->NumEmpty;
        for (u32 RegIndex = 0; RegIndex < 6; ++RegIndex)
        {
            if (!State->LoadedRegisters[RegIndex])
            {
                State->LoadedRegisters[RegIndex] = Result;
                Result->Name = State->CPU_RegNames[RegIndex];
                break;
            }
        }
    }
    else
    {
        // TODO: Use LRU to swap out virtual registers
        InvalidCodePath;
    }

    // NOTE: Load from stack
    Result->Flags |= RegFlag_InCPU;
    if ((Result->Flags & RegFlag_IsTemp) == 0)
    {
        //x86_WriteMovFromStack(Result->Name, Result->MemoryIndex, State);
    }
    
    return Result;
}

internal void ConvertIrToAssembly(mem_arena* IR_Arena, FILE* OutFile)
{
    backend_state State = {};
    State.NumEmpty = 6;
    State.RegAllocStack = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
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
    
    // NOTE: Name our CPU registers
#if 0
    State.CPU_RegNames[0] = InitStr("EAX", 3);
    State.CPU_RegNames[1] = InitStr("EBX", 3);
    State.CPU_RegNames[2] = InitStr("ECX", 3);
    State.CPU_RegNames[3] = InitStr("EDX", 3);
    State.CPU_RegNames[4] = InitStr("ESI", 3);
    State.CPU_RegNames[5] = InitStr("EDI", 3);
#endif
    
    State.CPU_RegNames[0] = InitStr("r0", 2);
    State.CPU_RegNames[1] = InitStr("r1", 2);
    State.CPU_RegNames[2] = InitStr("r2", 2);
    State.CPU_RegNames[3] = InitStr("r3", 2);
    State.CPU_RegNames[4] = InitStr("r4", 2);
    State.CPU_RegNames[5] = InitStr("r5", 2);
    
    // NOTE: Calculate the stack offsets of our local vars
    u32 CurrStackOffset = 0;
    while (State.CurrByte < ((u8*)IR_Arena->Mem + IR_Arena->Used))
    {
        ir_instr* Instr = (ir_instr*)State.CurrByte;
        State.CurrByte += sizeof(ir_instr);

        switch(Instr->Id)
        {
            case IR_Define:
            {
                virtual_reg* Register = *((virtual_reg**)(State.CurrByte));
                Register->MemoryIndex = CurrStackOffset;
                CurrStackOffset += 4;
                
                State.CurrByte += sizeof(virtual_reg*);
            } break;

            case IR_JumpHeader:
            case IR_Jump:
            {
                State.CurrByte += sizeof(string);
            } break;
            
            case IR_JumpCond:
            {
                State.CurrByte += sizeof(virtual_reg*);
                State.CurrByte += sizeof(string);
                State.CurrByte += sizeof(string);
            } break;

            case IR_Assign:
            case IR_AssignToMem:
            case IR_AddressOff:
            case IR_LoadFromMem:
            {
                State.CurrByte += sizeof(virtual_reg*);
                State.CurrByte += sizeof(virtual_reg*);
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
            {
                State.CurrByte += sizeof(virtual_reg*);
                State.CurrByte += sizeof(virtual_reg*);
                State.CurrByte += sizeof(virtual_reg*);
            } break;

            case IR_FuncDecl:
            {
                State.CurrByte += sizeof(string);
                State.CurrByte += sizeof(u32);
                CurrStackOffset = 0;
            } break;

            case IR_FuncEnd:
            {
            } break;
            
            case IR_FuncCall:
            {
                State.CurrByte += sizeof(virtual_reg*);
                State.CurrByte += sizeof(string);
                u32* NumArgs = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);
                State.CurrByte += sizeof(virtual_reg*)*(*NumArgs);
            } break;

            default:
            {
                InvalidCodePath;
            } break;
        }
    }

    State.CurrByte = IR_Arena->Mem;
    while (State.CurrByte < ((u8*)IR_Arena->Mem + IR_Arena->Used))
    {
        ir_instr* Instr = (ir_instr*)State.CurrByte;
        State.CurrByte += sizeof(ir_instr);

        switch(Instr->Id)
        {
            case IR_Define:
            {
                virtual_reg* Register = *((virtual_reg**)State.CurrByte);
                State.CurrByte += sizeof(virtual_reg*);
                AllocateReg(Register, &State);
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
                virtual_reg* LHS = GetRegisterFromIR(&State);
                virtual_reg* RHS = GetRegisterFromIR(&State);
                string* HeaderName = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);

                x86_WriteCmp(LHS->Name, RHS->Name, &State);
                x86_WriteJe(*HeaderName, &State);
            } break;
            
            case IR_Assign:
            {
                virtual_reg* LHS = *((virtual_reg**)State.CurrByte);
                State.CurrByte += sizeof(virtual_reg*);
                AllocateReg(LHS, &State);
                
                virtual_reg* RHS = *((virtual_reg**)State.CurrByte);
                State.CurrByte += sizeof(virtual_reg*);
                AllocateReg(RHS, &State);

                LHS->Flags |= RegFlag_LoadToMem;
                x86_WriteMov(LHS->Name, RHS->Name, &State);
            } break;

            case IR_AssignToMem:
            {
                virtual_reg* LHS = GetRegisterFromIR(&State);
                virtual_reg* RHS = GetRegisterFromIR(&State);
                
                x86_WriteMovToMem(LHS->Name, RHS->Name, &State);
            } break;

            case IR_AddressOff:
            {
                virtual_reg* Val = GetRegisterFromIR(&State);
                virtual_reg* Result = GetRegisterFromIR(&State);

                x86_WriteLea(Result->Name, Val->Name, &State);
            } break;

            case IR_LoadFromMem:
            {
                virtual_reg* Pointer = GetRegisterFromIR(&State);
                virtual_reg* Result = GetRegisterFromIR(&State);
                
                x86_WriteMovFromMem(Result->Name, Pointer->Name, &State);
            } break;

            case IR_GetMember:
            {
                string* Arg1 = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                string* Arg2 = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                string* Return = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = ".";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 1));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State.OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;

            case IR_Equal:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);
                
                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Name, Arg2->Name, &State);
                x86_WriteJne(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_NotEqual:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Name, Arg2->Name, &State);
                x86_WriteJne(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_Less:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Name, Arg2->Name, &State);
                x86_WriteJge(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_LessEqual:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Name, Arg2->Name, &State);
                x86_WriteJg(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_Greater:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Name, Arg2->Name, &State);
                x86_WriteJge(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_GreaterEqual:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Name, Arg2->Name, &State);
                x86_WriteJge(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 0), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, ConvertIntToString(&TempArena, 1), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_AddIntInt:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                x86_WriteAddIntInt(Return, Arg2, &State);

                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_AddFltInt:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                x86_WriteAddFltInt(Return, Arg2, &State);

                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_AddFltFlt:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                x86_WriteAddFltFlt(Return, Arg2, &State);

                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_SubIntInt:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                x86_WriteSubIntInt(Return, Arg2, &State);

                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_SubFltInt:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                x86_WriteSubFltInt(Return, Arg2, &State);

                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_SubFltFlt:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                x86_WriteSubFltFlt(Return, Arg2, &State);

                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_MulIntInt:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                x86_WriteMulIntInt(Return, Arg2, &State);

                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;


            // TODO: Add all mul and div instructions
            
            case IR_FuncDecl:
            {
                string* Name = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                u32* NumArgs = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);
                
                str_build_state StrState = StartBuildingString(&TempArena);
                AddString(*Name, &StrState);
                AddString(":\n", 2, &StrState);
                // NOTE: Save stack pointer
                AddString("push ebp\n", 9, &StrState);
                AddString("mov ebp, esp\n", 13, &StrState);
                fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State.OutFile);
                // NOTE: Save previous funcs temp registers
                PushRegisterOntoStack(EBX_REG_INDEX, &State);
                PushRegisterOntoStack(EDI_REG_INDEX, &State);
                PushRegisterOntoStack(ESI_REG_INDEX, &State);
            } break;

            case IR_FuncEnd:
            {
                PopRegistersFromStack(ESI_REG_INDEX, &State);
                PopRegistersFromStack(EDI_REG_INDEX, &State);
                PopRegistersFromStack(EBX_REG_INDEX, &State);
                // NOTE: Deallocate our locals
                fwrite("mov esp, ebp\n", 13, sizeof(char), State.OutFile);
                fwrite("pop ebp\n", 8, sizeof(char), State.OutFile);
                fwrite("ret\n", 4, sizeof(char), State.OutFile);
            } break;
            
            case IR_FuncCall:
            {
                virtual_reg* Result = *((virtual_reg**)State.CurrByte);
                b32 HasReturnVal = true;//!AreSameType(Result->Type, GetBasicType(TypeId_Void));
                if (HasReturnVal)
                {
                    virtual_reg* Result = GetRegisterFromIR(&State);
                }
                else
                {
                    // NOTE: We have a void type so we don't have a return value
                    State.CurrByte += sizeof(virtual_reg*);
                }
                string* Name = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                u32* NumArgs = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);

                // NOTE: Save registers onto the stack
                for (i32 ArgIndex = (*NumArgs) - 1; ArgIndex >= 0; --ArgIndex)
                {
                    virtual_reg* ArgName = GetRegisterFromIR(&State);
                    //x86_WritePush(ArgName, &State);
                }

                x86_WriteCallFunc(*Name, &State);

                if (HasReturnVal)
                {
                    // NOTE: We follow convention of taking the EAX register to store the ret val
                    x86_WriteMov(Result->Name, State.CPU_RegNames[EAX_REG_INDEX], &State);
                }
                // TODO: Clear the arguments from the stack
            } break;

            case IR_Return:
            {
                u32* NumArgs = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);

                char Temp[] = "return ";
                char Temp1[] = ";\n";

                string OutputText = InitStr(Temp, 7);
                
                if (*NumArgs == 1)
                {                    
                    string* ReturnVal = (string*)State.CurrByte;
                    State.CurrByte += sizeof(string);

                    string TempStr = StringConcat(OutputText, *ReturnVal);
                    OutputText = TempStr;
                }

                string TempStr = StringConcat(OutputText, InitStr(Temp1, 2));
                if (*NumArgs == 1)
                {
                    free(OutputText.Text);
                }
                OutputText = TempStr;

                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State.OutFile);
                free(OutputText.Text);
            } break;
            
            default:
            {
                InvalidCodePath;
            } break;
        }
    }

    ClearArena(IR_Arena);
}
