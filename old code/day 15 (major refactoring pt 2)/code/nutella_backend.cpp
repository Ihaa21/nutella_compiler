
inline b32 IsOpUniary(u32 OpId)
{
    b32 Result = OpId >= IR_UniaryOpStart && (OpId < IR_UniaryOpEnd);
    return Result;
}

inline b32 IsOpBinary(u32 OpId)
{
    b32 Result = OpId > IR_BinaryOpStart && (OpId < IR_BinaryOpEnd);
    return Result;
}

inline string ConvertTypeToString(type* Type)
{
    string TypeString = Type->Str;
    if (IsTypePointer(Type))
    {
        // TODO: This leaks, make a temp mem for string stuff like this
        string PointerStr = AllocStr(Type->NumRef);
        for (u32 CharIndex = 0; CharIndex < PointerStr.NumChars; ++CharIndex)
        {
            PointerStr.Text[CharIndex] = '*';
        }

        TypeString = StringConcat(Type->PointerType->Str, PointerStr);
    }

    return TypeString;
}

// NOTE: We decompose int x = 5 to int x; x = 5; so that we don't need to vary the number
// of arguments/tokens passed
internal void PushDefineInstr(type* Type, string NewVar)
{
    ir_instr Instr = {};
    Instr.Id = IR_Define;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));

    string TypeStr = ConvertTypeToString(Type);
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &TypeStr, sizeof(string));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &NewVar, sizeof(string));
}

internal void PushAssignInstr(string Left, string Right)
{
    ir_instr Instr = {};
    Instr.Id = IR_Assign;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Left, sizeof(string));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Right, sizeof(string));
}

internal void PushUniOpInstr(u32 Op, string Arg, string Ret)
{
    Assert(Op > IR_UniaryOpStart && Op < IR_UniaryOpEnd);
    ir_instr Instr = {};
    Instr.Id = Op;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Arg, sizeof(string));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Ret, sizeof(string));
}

internal void PushBinOpInstr(u32 Op, string Arg1, string Arg2, string Ret)
{
    Assert(Op > IR_BinaryOpStart && Op < IR_BinaryOpEnd);
    ir_instr Instr = {};
    Instr.Id = Op;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Arg1, sizeof(string));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Arg2, sizeof(string));
        
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Ret, sizeof(string));
}

internal void PushScopeInstr(u32 Op)
{
    Assert(Op == IR_StartScope || Op == IR_EndScope);
    ir_instr Instr = {};
    Instr.Id = Op;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
}

internal void PushIfInstr(string Expr)
{
    ir_instr Instr = {};
    Instr.Id = IR_If;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Expr, sizeof(string));    
}

internal void PushElseIfInstr(string Expr)
{
    ir_instr Instr = {};
    Instr.Id = IR_ElseIf;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Expr, sizeof(string));    
}

internal void PushElseInstr()
{
    ir_instr Instr = {};
    Instr.Id = IR_Else;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
}

internal void PushWhileInstr(string Expr)
{
    ir_instr Instr = {};
    Instr.Id = IR_While;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
            
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Expr, sizeof(string));    
}

internal void PushFuncInstr(function* Func)
{
    ir_instr Instr = {};
    Instr.Id = IR_Function;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));

    string ReturnStr = ConvertTypeToString(Func->ReturnType);
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &ReturnStr, sizeof(string));    
            
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Func->Name, sizeof(string));    
            
    CurrByte = PushStruct(&IR_Arena, u32);
    memcpy(CurrByte, &Func->NumArgs, sizeof(u32));    

    for (u32 ArgIndex = 0; ArgIndex < Func->NumArgs; ++ArgIndex)
    {
        string ArgStr = ConvertTypeToString(Func->ArgTypes[ArgIndex]);
        CurrByte = PushStruct(&IR_Arena, string);
        memcpy(CurrByte, &ArgStr, sizeof(string));            
    }
}

internal void ConvertIrToC(FILE* OutFile)
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
                // TODO: Add default values to standard types (and
                // eventually to structs when we implement them)
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
            } break;

            case IR_Assign:
            {
                string* LHS = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* RHS = (string*)CurrByte;
                CurrByte += sizeof(string);

                char Temp1[] = " = ";
                char Temp2[] = ";\n";

                string Temp3 = StringConcat(*LHS, InitStr(Temp1, 3));
                string Temp4 = StringConcat(Temp3, *RHS);
                string OutputText = StringConcat(Temp4, InitStr(Temp2, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp3.Text);
                free(OutputText.Text);
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

            case IR_Equals:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " == ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 4));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;

            case IR_NotEquals:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " != ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 4));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;

            case IR_Less:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " < ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;

            case IR_LessEqual:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " <= ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 4));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;

            case IR_Greater:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " > ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;

            case IR_GreaterEqual:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " >= ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 4));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Add:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " + ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Sub:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " - ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Mul:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " * ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Div:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " / ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
             
            case IR_StartScope:
            {
                string OutputText = InitStr("{\n", 2);
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);
            } break;
                         
            case IR_EndScope:
            {
                string OutputText = InitStr("}\n", 2);
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);
            } break;

            case IR_If:
            {
                string* Expr = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = "if (";
                char Temp2[] = ")";
                char Temp3[] = "\n";

                string Temp4 = StringConcat(InitStr(Temp1, 4), *Expr);
                string Temp5 = StringConcat(Temp4, InitStr(Temp2, 1));
                string OutputText = StringConcat(Temp5, InitStr(Temp3, 1));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(OutputText.Text);
            } break;
            
            case IR_ElseIf:
            {
                string* Expr = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = "else if (";
                char Temp2[] = ")";
                char Temp3[] = "\n";

                string Temp4 = StringConcat(InitStr(Temp1, 9), *Expr);
                string Temp5 = StringConcat(Temp4, InitStr(Temp2, 1));
                string OutputText = StringConcat(Temp5, InitStr(Temp3, 1));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Else:
            {
                char Temp1[] = "else";
                char Temp2[] = "\n";

                string OutputText = StringConcat(InitStr(Temp1, 4), InitStr(Temp2, 1));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(OutputText.Text);
            } break;
            
            case IR_While:
            {
                string* Expr = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = "while (";
                char Temp2[] = ")";
                char Temp3[] = "\n";

                string Temp4 = StringConcat(InitStr(Temp1, 7), *Expr);
                string Temp5 = StringConcat(Temp4, InitStr(Temp2, 1));
                string OutputText = StringConcat(Temp5, InitStr(Temp3, 1));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(OutputText.Text);
            } break;

            case IR_Function:
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
                string OutputText = Temp5;
                
                for (u32 ArgIndex = 0; ArgIndex < *NumArgs; ++ArgIndex)
                {
                    string* ArgType = (string*)CurrByte;
                    CurrByte += sizeof(string);

                    string Temp = StringConcat(OutputText, *ArgType);
                    free(OutputText.Text);
                    OutputText = StringConcat(Temp, InitStr(TempComma, 1));
                    free(Temp.Text);
                }

                char TempEndBracket[] = ")";
                string LastTemp = StringConcat(OutputText, InitStr(TempEndBracket, 1));
                free(OutputText.Text);
                OutputText = LastTemp;

                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);
                free(Temp3.Text);
                free(Temp4.Text);
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
