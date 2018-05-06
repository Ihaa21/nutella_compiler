
inline void x86_WriteAdd(var* Reg1, var* Reg2, FILE* OutFile)
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

    string Temp4 = StringConcat(InstrName, Reg1->Str);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2->Str);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteSub(var* Reg1, var* Reg2, FILE* OutFile)
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

    string Temp4 = StringConcat(InstrName, Reg1->Str);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2->Str);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteMov(string Reg1Name, string Reg2Name, FILE* OutFile)
{
    // NOTE: We don't need to convert types since the type checking guarentees
    // that we are equating values if they are of the same type
    // NOTE: All our data sizes are 32 bit so we always load 32 bits here
    string Temp4 = StringConcat(InitStr("mov ", 4), Reg1Name);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2Name);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteCmp(string Reg1Name, string Reg2Name, FILE* OutFile)
{
    // TODO: What about floats?
    string Temp4 = StringConcat(InitStr("cmp ", 4), Reg1Name);
    string Temp5 = StringConcat(InitStr(", ", 2), Reg2Name);
    string Temp6 = StringConcat(Temp5, InitStr("\n", 1));
    string OutputText = StringConcat(Temp4, Temp6);
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

    free(Temp4.Text);
    free(Temp5.Text);
    free(Temp6.Text);
    free(OutputText.Text);
}

inline void x86_WriteHeader(string HeaderName, FILE* OutFile)
{    
    string OutputText = StringConcat(HeaderName, InitStr(":\n", 2));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);
}

inline void x86_WriteJmp(string HeaderName, FILE* OutFile)
{
    string Temp4 = StringConcat(InitStr("jmp ", 4), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
}

inline void x86_WriteJe(string HeaderName, FILE* OutFile)
{
    string Temp4 = StringConcat(InitStr("je ", 3), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
}

inline void x86_WriteJne(string HeaderName, FILE* OutFile)
{
    string Temp4 = StringConcat(InitStr("jne ", 4), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
}

inline void x86_WriteJg(string HeaderName, FILE* OutFile)
{
    string Temp4 = StringConcat(InitStr("jg ", 3), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
}

inline void x86_WriteJge(string HeaderName, FILE* OutFile)
{
    string Temp4 = StringConcat(InitStr("jge ", 4), HeaderName);
    string OutputText = StringConcat(Temp4, InitStr("\n", 1));
    fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

    free(Temp4.Text);
    free(OutputText.Text);
}

internal void ConvertIrToAssembly(FILE* OutFile)
{
    u8* CurrByte = IR_Arena.Mem;
    
    while (CurrByte < ((u8*)IR_Arena.Mem + IR_Arena.Used))
    {
        ir_instr* Instr = (ir_instr*)CurrByte;
        CurrByte += sizeof(ir_instr);

        switch(Instr->Id)
        {
            case IR_Define:
            {
                string* Type = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* VarName = (string*)CurrByte;
                CurrByte += sizeof(string);

                char Temp1[] = " ";
                char Temp2[] = ";\n";

                string Temp3 = StringConcat(*Type, InitStr(Temp1, 1));
                string Temp4 = StringConcat(Temp3, *VarName);
                string OutputText = StringConcat(Temp4, InitStr(Temp2, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);
                
                free(Temp3.Text);
                free(Temp4.Text);
                free(OutputText.Text);

                // NOTE: By default, we init values to 0
                x86_WriteMov(*VarName, InitStr("0", 1), OutFile);
            } break;
            
            case IR_JumpHeader:
            {
                string* HeaderName = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                x86_WriteHeader(*HeaderName, OutFile);
            } break;

            case IR_Jump:
            {
                string* HeaderName = (string*)CurrByte;
                CurrByte += sizeof(string);

                x86_WriteJmp(*HeaderName, OutFile);
            } break;

            case IR_JumpCond:
            {
                string* LHS = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* RHS = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* HeaderName = (string*)CurrByte;
                CurrByte += sizeof(string);

                x86_WriteCmp(*LHS, *RHS, OutFile);
                x86_WriteJe(*HeaderName, OutFile);
            } break;
            
            case IR_Assign:
            {
                string* LHS = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* RHS = (string*)CurrByte;
                CurrByte += sizeof(string);

                x86_WriteMov(*LHS, *RHS, OutFile);
            } break;

            case IR_AddressOff:
            {
                string* Arg = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = "&";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, InitStr(Temp2, 1));
                string Temp6 = StringConcat(Temp5, *Arg);
                string OutputText = StringConcat(Temp6, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(OutputText.Text);
            } break;

            case IR_Dereference:
            {
                string* Arg = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = "*";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, InitStr(Temp2, 1));
                string Temp6 = StringConcat(Temp5, *Arg);
                string OutputText = StringConcat(Temp6, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(OutputText.Text);
            } break;

            case IR_GetMember:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = ".";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 1));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;

            case IR_Equal:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);

                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);
                
                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Str, Arg2->Str, OutFile);
                x86_WriteJne(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 1, 1), OutFile);
                x86_WriteJmp(ExitHeader, OutFile);
                x86_WriteHeader(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 0, 1), OutFile);
                x86_WriteHeader(ExitHeader, OutFile);
            } break;

            case IR_NotEqual:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);

                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Str, Arg2->Str, OutFile);
                x86_WriteJne(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 0, 1), OutFile);
                x86_WriteJmp(ExitHeader, OutFile);
                x86_WriteHeader(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 1, 1), OutFile);
                x86_WriteHeader(ExitHeader, OutFile);
            } break;

            case IR_Less:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);

                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Str, Arg2->Str, OutFile);
                x86_WriteJge(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 1, 1), OutFile);
                x86_WriteJmp(ExitHeader, OutFile);
                x86_WriteHeader(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 0, 1), OutFile);
                x86_WriteHeader(ExitHeader, OutFile);                
            } break;

            case IR_LessEqual:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);

                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Str, Arg2->Str, OutFile);
                x86_WriteJg(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 1, 1), OutFile);
                x86_WriteJmp(ExitHeader, OutFile);
                x86_WriteHeader(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 0, 1), OutFile);
                x86_WriteHeader(ExitHeader, OutFile);                
            } break;

            case IR_Greater:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);

                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Str, Arg2->Str, OutFile);
                x86_WriteJge(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 0, 1), OutFile);
                x86_WriteJmp(ExitHeader, OutFile);
                x86_WriteHeader(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 1, 1), OutFile);
                x86_WriteHeader(ExitHeader, OutFile);                
            } break;

            case IR_GreaterEqual:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);

                string NotEqualHeader = GetTempHeader(&TempArena, &NameState);
                string ExitHeader = GetTempHeader(&TempArena, &NameState);

                // TODO: Memory management here plz
                x86_WriteCmp(Arg1->Str, Arg2->Str, OutFile);
                x86_WriteJge(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 0, 1), OutFile);
                x86_WriteJmp(ExitHeader, OutFile);
                x86_WriteHeader(NotEqualHeader, OutFile);
                x86_WriteMov(Return->Str, GetStringFromConstInt(&TempArena, 1, 1), OutFile);
                x86_WriteHeader(ExitHeader, OutFile);                
            } break;

            case IR_Add:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);

                x86_WriteMov(Return->Str, Arg1->Str, OutFile);
                x86_WriteAdd(Return, Arg2, OutFile);
            } break;
            
            case IR_Sub:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);
                
                x86_WriteMov(Return->Str, Arg1->Str, OutFile);
                x86_WriteSub(Return, Arg2, OutFile);
            } break;
            
            case IR_Mul:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);
                
                x86_WriteMov(Return->Str, Arg1->Str, OutFile);
            } break;
            
            case IR_Div:
            {
                var* Arg1 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Arg2 = (var*)CurrByte;
                CurrByte += sizeof(var);
                var* Return = (var*)CurrByte;
                CurrByte += sizeof(var);
                
                x86_WriteMov(Return->Str, Arg1->Str, OutFile);
                //x86_WriteDiv(Return, Arg2, OutFile);
            } break;
             
            case IR_GetArrayIndex:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = "[";
                char Temp3[] = "];\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 1));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 3));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;

            case IR_FuncDecl:
            {
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);

                string* Name = (string*)CurrByte;
                CurrByte += sizeof(string);

                u32* NumArgs = (u32*)CurrByte;
                CurrByte += sizeof(u32);

                char Temp1[] = " ";
                char Temp2[] = "(";
                char TempComma[] = ",";

                string Temp3 = StringConcat(*Return, InitStr(Temp1, 1));
                string Temp4 = StringConcat(Temp3, *Name);
                string Temp5 = StringConcat(Temp4, InitStr(Temp2, 1));
                
                for (u32 ArgIndex = 0; ArgIndex < (*NumArgs); ++ArgIndex)
                {
                    string* ArgType = (string*)CurrByte;
                    CurrByte += sizeof(string);

                    string Temp = StringConcat(Temp5, *ArgType);
                    free(Temp5.Text);
                    Temp5 = StringConcat(Temp, InitStr(TempComma, 1));
                    free(Temp.Text);
                }

                char TempEndBracket[] = ")\n\n";
                string OutputText = StringConcat(Temp5, InitStr(TempEndBracket, 3));
                free(Temp5.Text);
                
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);
                free(Temp3.Text);
                free(Temp4.Text);
                free(OutputText.Text);
            } break;

            case IR_FuncCall:
            {
                string* ResultVar = (string*)CurrByte;
                CurrByte += sizeof(string);

                string* Name = (string*)CurrByte;
                CurrByte += sizeof(string);

                u32* NumArgs = (u32*)CurrByte;
                CurrByte += sizeof(u32);

                char Equals[] = " = ";
                char Temp1[] = "(";
                char Temp2[] = ",";

                string Equals1 = StringConcat(*ResultVar, InitStr(Equals, 3));
                string Equals2 = StringConcat(Equals1, *Name);
                string Temp3 = StringConcat(Equals2, InitStr(Temp1, 1));

                for (u32 ArgIndex = 0; ArgIndex < (*NumArgs); ++ArgIndex)
                {
                    string* ArgName = (string*)CurrByte;
                    CurrByte += sizeof(string);

                    string Temp4 = StringConcat(Temp3, *ArgName);
                    free(Temp3.Text);
                    Temp3 = StringConcat(Temp4, InitStr(Temp2, 1));
                    free(Temp4.Text);
                }

                char TempEnd[] = ");\n";
                
                string OutputText = StringConcat(Temp3, InitStr(TempEnd, 3));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Equals1.Text);
                free(Equals2.Text);
                free(Temp3.Text);
                free(OutputText.Text);
            } break;

            case IR_Return:
            {
                u32* NumArgs = (u32*)CurrByte;
                CurrByte += sizeof(u32);

                char Temp[] = "return ";
                char Temp1[] = ";\n";

                string OutputText = InitStr(Temp, 7);
                
                if (*NumArgs == 1)
                {                    
                    string* ReturnVal = (string*)CurrByte;
                    CurrByte += sizeof(string);

                    string TempStr = StringConcat(OutputText, *ReturnVal);
                    OutputText = TempStr;
                }

                string TempStr = StringConcat(OutputText, InitStr(Temp1, 2));
                if (*NumArgs == 1)
                {
                    free(OutputText.Text);
                }
                OutputText = TempStr;

                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);
                free(OutputText.Text);
            } break;
            
            default:
            {
                InvalidCodePath;
            } break;
        }
    }

    ClearArena(&IR_Arena);
}
