
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

inline void x86_WriteMovFromStack(string Reg1Name, u32 Offset, backend_state* State)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here

    str_build_state StrState = StartBuildingString(&TempArena);
    AddString("mov ", 4, &StrState);
    AddString(Reg1Name, &StrState);
    AddString(", [esp + ", 9, &StrState);
    AddString(Offset, &StrState);
    AddString("]\n", 2, &StrState);
    
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
inline void x86_WriteJmpTemplate(string HeaderName, char* Instr, u32 NumChars, backend_state* State)
{
    str_build_state StrState = StartBuildingString(&TempArena);
    AddString(Instr, NumChars, &StrState);
    AddString(HeaderName, &StrState);
    AddString("\n", 1, &StrState);
    
    fwrite(StrState.Str.Text, StrState.Str.NumChars, sizeof(char), State->OutFile);
    EndBuildingString(&StrState);
}

inline void x86_WritePush(virtual_reg* Arg, backend_state* State)
{
    string Temp = StringConcat(InitStr("push ", 5), Arg->Name);
    string OutputText = StringConcat(Temp, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), State->OutFile);

    free(Temp.Text);
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
        
    while (State.CurrByte < ((u8*)IR_Arena->Mem + IR_Arena->Used))
    {
        ir_instr* Instr = (ir_instr*)State.CurrByte;
        State.CurrByte += sizeof(ir_instr);

        switch(Instr->Id)
        {
            case IR_Define:
            {
                virtual_reg* Register = GetRegisterFromIR(&State);
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

                for (i32 ArgIndex = (*NumArgs) - 1; ArgIndex >= 0; --ArgIndex)
                {
                    virtual_reg* ArgName = GetRegisterFromIR(&State);
                    x86_WritePush(ArgName, &State);
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
