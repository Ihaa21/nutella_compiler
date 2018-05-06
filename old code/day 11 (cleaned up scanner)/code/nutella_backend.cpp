
inline b32 IsOpUniary(u32 OpId)
{
    b32 Result = OpId >= IR_UniaryOpStart && (OpId < IR_UniaryOpCount);
    return Result;
}

inline b32 IsOpBinary(u32 OpId)
{
    b32 Result = OpId > IR_BinaryOpStart && (OpId < IR_BinaryOpCount);
    return Result;
}

// NOTE: We decompose int x = 5 to int x; x = 5; so that we don't need to vary the number
// of arguments/tokens passed
inline void PushDefineInstr(type* Type, string NewVar)
{
    ir_instr Instr = {};
    Instr.Id = IR_Define;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
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

    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &TypeString, sizeof(string));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &NewVar, sizeof(string));
}

inline void PushAssignInstr(string Left, string Right)
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

inline void PushUniOpInstr(u32 Op, string Arg, string Ret)
{
    ir_instr Instr = {};
    Instr.Id = Op;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Arg, sizeof(string));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Ret, sizeof(string));
}

inline void PushBinOpInstr(u32 Op, string Arg1, string Arg2, string Ret)
{
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
        }
    }

    ClearArena(&IR_Arena);
}
