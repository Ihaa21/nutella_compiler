
inline void x86_WriteAdd(virtual_reg* Reg1, virtual_reg* Reg2, backend_state* State)
{
    string InstrName = {};
    
    if (AreSameType(Reg1->Type, GetBasicType(TypeId_Int)) &&
        AreSameType(Reg2->Type, GetBasicType(TypeId_Int)))
    {
        InstrName = InitStr("add ", 4);
    }
    else if (AreSameType(Reg1->Type, GetBasicType(TypeId_Int)) &&
             AreSameType(Reg2->Type, GetBasicType(TypeId_Float)))
    {
        InstrName = InitStr("fiadd  ", 6);
    }
    else if (AreSameType(Reg1->Type, GetBasicType(TypeId_Float)) &&
             AreSameType(Reg2->Type, GetBasicType(TypeId_Int)))
    {
        InstrName = InitStr("fiadd  ", 6);
    }
    else if (AreSameType(Reg1->Type, GetBasicType(TypeId_Float)) &&
             AreSameType(Reg2->Type, GetBasicType(TypeId_Float)))
    {
        InstrName = InitStr("fadd  ", 5);
    }
    else
    {
        InvalidCodePath;
    }

    string Temp4 = StringConcat(InstrName, Reg1->Name);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2->Name);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteSub(virtual_reg* Reg1, virtual_reg* Reg2, backend_state* State)
{
    string InstrName = {};
    
    if (AreSameType(Reg1->Type, GetBasicType(TypeId_Int)) &&
        AreSameType(Reg2->Type, GetBasicType(TypeId_Int)))
    {
        InstrName = InitStr("sub ", 4);
    }
    else if (AreSameType(Reg1->Type, GetBasicType(TypeId_Int)) &&
             AreSameType(Reg2->Type, GetBasicType(TypeId_Float)))
    {
        InstrName = InitStr("fisub  ", 6);
    }
    else if (AreSameType(Reg1->Type, GetBasicType(TypeId_Float)) &&
             AreSameType(Reg2->Type, GetBasicType(TypeId_Int)))
    {
        InstrName = InitStr("fisub  ", 6);
    }
    else if (AreSameType(Reg1->Type, GetBasicType(TypeId_Float)) &&
             AreSameType(Reg2->Type, GetBasicType(TypeId_Float)))
    {
        InstrName = InitStr("fsub  ", 5);
    }
    else
    {
        InvalidCodePath;
    } 

    string Temp4 = StringConcat(InstrName, Reg1->Name);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2->Name);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteMov(string Reg1Name, string Reg2Name, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here
    
    string Temp4 = StringConcat(InitStr("mov ", 4), Reg1Name);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2Name);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);
    
    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteMovFromMem(string Reg1Name, string Reg2Name, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    string Temp = StringConcat(InitStr("[", 1), Reg2Name);
    Reg2Name = StringConcat(Temp, InitStr("]", 1));
    
    string Temp4 = StringConcat(InitStr("mov ", 4), Reg1Name);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2Name);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp.Text);
    free(Reg2Name.Text);
    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteMovToMem(string Reg1Name, string Reg2Name, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    string Temp = StringConcat(InitStr("[", 1), Reg1Name);
    Reg1Name = StringConcat(Temp, InitStr("]", 1));
    
    string Temp4 = StringConcat(InitStr("mov ", 4), Reg1Name);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2Name);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp.Text);
    free(Reg1Name.Text);
    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteMovFromStack(string Reg1Name, u32 Offset, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    // TODO: MEMORY MANAGING
    string StrOffset = ConvertIntToString(&TempArena, Offset);
    string Temp = StringConcat(InitStr("[esp + ", 7), StrOffset);
    string Reg2Name = StringConcat(Temp, InitStr("]", 1));
    
    string Temp4 = StringConcat(InitStr("mov ", 4), Reg1Name);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2Name);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp.Text);
    free(Reg2Name.Text);
    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteCmp(string Reg1Name, string Reg2Name, backend_state* State)
{
    // TODO: What about floats?
    string Temp4 = StringConcat(InitStr("cmp ", 4), Reg1Name);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2Name);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteHeader(string HeaderName, backend_state* State)
{    
    string OutputText = StringConcat(HeaderName, InitStr(":\n", 2));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);
}

inline void x86_WriteJmp(string HeaderName, backend_state* State)
{
    string Temp4 = StringConcat(InitStr("jmp ", 4), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
}

inline void x86_WriteJe(string HeaderName, backend_state* State)
{
    string Temp4 = StringConcat(InitStr("je ", 3), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
}

inline void x86_WriteJne(string HeaderName, backend_state* State)
{
    string Temp4 = StringConcat(InitStr("jne ", 4), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
}

inline void x86_WriteJg(string HeaderName, backend_state* State)
{
    string Temp4 = StringConcat(InitStr("jg ", 3), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
}

inline void x86_WriteJge(string HeaderName, backend_state* State)
{
    string Temp4 = StringConcat(InitStr("jge ", 4), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
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
    if (State->CurrRegCount == ArrayCount(State->LoadedRegisters))
    {
        // TODO: Use LRU to swap out virtual registers
        InvalidCodePath;
    }
    else
    {
        ++State->CurrRegCount;
        State->LoadedRegisters[State->CurrRegCount] = Result;
        Result->Name = State->CPU_RegNames[State->CurrRegCount];
    }

    // NOTE: Load from stack
    Result->Flags |= RegFlag_InCPU;

    if ((Result->Flags & RegFlag_IsTemp) == 0)
    {
        // TODO: Make this times type size, not 4
        Result->MemoryIndex = Result->MemoryIndex*4;
        State->StackOffset += 4;
        //StackOffset += TypeSize;
        x86_WriteMovFromStack(Result->Name, Result->MemoryIndex, State);
    }
    
    return Result;
}

internal void ConvertIrToAssembly(mem_arena* IR_Arena, FILE* OutFile)
{
    backend_state State = {};
    State.CurrByte = IR_Arena->Mem;
    State.OutFile = OutFile;

    // NOTE: Name our CPU registers
    for (u32 CurrReg = 0; CurrReg < ArrayCount(State.CPU_RegNames); ++CurrReg)
    {
        State.CPU_RegNames[CurrReg] = GetRegisterName(&RegArena, CurrReg);                
    }
    
    while (State.CurrByte < ((u8*)IR_Arena->Mem + IR_Arena->Used))
    {
        ir_instr* Instr = (ir_instr*)State.CurrByte;
        State.CurrByte += sizeof(ir_instr);

        switch(Instr->Id)
        {
            case IR_Define:
            {
                virtual_reg* Register = GetRegisterFromIR(&State);
                // NOTE: By default, we init values to 0
                x86_WriteMov(Register->Name, InitStr("0", 1), &State);
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
                string* RHS = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                string* HeaderName = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);

                x86_WriteCmp(LHS->Name, *RHS, &State);
                x86_WriteJe(*HeaderName, &State);
            } break;
            
            case IR_Assign:
            {
                virtual_reg* LHS = GetRegisterFromIR(&State);
                virtual_reg* RHS = GetRegisterFromIR(&State);
                
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
                string* Arg = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                string* Return = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = "&";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, InitStr(Temp2, 1));
                string Temp6 = StringConcat(Temp5, *Arg);
                string OutputText = StringConcat(Temp6, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State.OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(OutputText.Text);
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
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 1, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 0, 1), &State);
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
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 0, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 1, 1), &State);
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
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 1, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 0, 1), &State);
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
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 1, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 0, 1), &State);
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
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 0, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 1, 1), &State);
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
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 0, 1), &State);
                x86_WriteJmp(ExitHeader, &State);
                x86_WriteHeader(NotEqualHeader, &State);
                x86_WriteMov(Return->Name, GetStringFromConstInt(&TempArena, 1, 1), &State);
                x86_WriteHeader(ExitHeader, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;

            case IR_Add:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                x86_WriteAdd(Return, Arg2, &State);

                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;
            
            case IR_Sub:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                x86_WriteSub(Return, Arg2, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;
            
            case IR_Mul:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;
            
            case IR_Div:
            {
                virtual_reg* Arg1 = GetRegisterFromIR(&State);
                virtual_reg* Arg2 = GetRegisterFromIR(&State);
                virtual_reg* Return = GetRegisterFromIR(&State);
                
                x86_WriteMov(Return->Name, Arg1->Name, &State);
                //x86_WriteDiv(Return, Arg2, &State);
                
                Arg1->Flags |= RegFlag_LoadToMem;
                Arg2->Flags |= RegFlag_LoadToMem;
                Return->Flags |= RegFlag_LoadToMem;
            } break;
             
            case IR_GetArrayIndex:
            {
                string* Arg1 = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                string* Arg2 = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                string* Return = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = "[";
                char Temp3[] = "];\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 1));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 3));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State.OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;

            case IR_FuncDecl:
            {
                string* Return = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);

                string* Name = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);

                u32* NumArgs = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);

                char Temp1[] = " ";
                char Temp2[] = "(";
                char TempComma[] = ",";

                string Temp3 = StringConcat(*Return, InitStr(Temp1, 1));
                string Temp4 = StringConcat(Temp3, *Name);
                string Temp5 = StringConcat(Temp4, InitStr(Temp2, 1));
                
                for (u32 ArgIndex = 0; ArgIndex < (*NumArgs); ++ArgIndex)
                {
                    string* ArgType = (string*)State.CurrByte;
                    State.CurrByte += sizeof(string);

                    string Temp = StringConcat(Temp5, *ArgType);
                    free(Temp5.Text);
                    Temp5 = StringConcat(Temp, InitStr(TempComma, 1));
                    free(Temp.Text);
                }

                char TempEndBracket[] = ")\n\n";
                string OutputText = StringConcat(Temp5, InitStr(TempEndBracket, 3));
                free(Temp5.Text);
                
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State.OutFile);
                free(Temp3.Text);
                free(Temp4.Text);
                free(OutputText.Text);
            } break;

            case IR_FuncCall:
            {
                string* ResultVar = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);

                string* Name = (string*)State.CurrByte;
                State.CurrByte += sizeof(string);

                u32* NumArgs = (u32*)State.CurrByte;
                State.CurrByte += sizeof(u32);

                char Equals[] = " = ";
                char Temp1[] = "(";
                char Temp2[] = ",";

                string Equals1 = StringConcat(*ResultVar, InitStr(Equals, 3));
                string Equals2 = StringConcat(Equals1, *Name);
                string Temp3 = StringConcat(Equals2, InitStr(Temp1, 1));

                for (u32 ArgIndex = 0; ArgIndex < (*NumArgs); ++ArgIndex)
                {
                    string* ArgName = (string*)State.CurrByte;
                    State.CurrByte += sizeof(string);

                    string Temp4 = StringConcat(Temp3, *ArgName);
                    free(Temp3.Text);
                    Temp3 = StringConcat(Temp4, InitStr(Temp2, 1));
                    free(Temp4.Text);
                }

                char TempEnd[] = ");\n";
                
                string OutputText = StringConcat(Temp3, InitStr(TempEnd, 3));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State.OutFile);

                free(Equals1.Text);
                free(Equals2.Text);
                free(Temp3.Text);
                free(OutputText.Text);
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
