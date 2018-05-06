/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

struct type;
inline b32 AreSameType(type* Type1, type* Type2);

inline string ConvertTypeToString(type* Type)
{
    string TypeString = Type->Str;
    if (IsTypePointer(Type))
    {
        string PointerStr = AllocStr(&TempArena, Type->NumRef);
        for (u32 CharIndex = 0; CharIndex < PointerStr.NumChars; ++CharIndex)
        {
            PointerStr.Text[CharIndex] = '*';
        }

        TypeString = StringConcat(Type->PointerType->Str, PointerStr);
    }

    return TypeString;
}

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

internal void PushDefineInstr(virtual_reg* Reg)
{
    ir_instr Instr = {};
    Instr.Id = IR_Define;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));

    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Reg, sizeof(virtual_reg*));
}

internal void PushAssignInstr(virtual_reg* Left, virtual_reg* Right)
{
    ir_instr Instr = {};
    Instr.Id = IR_Assign;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Left, sizeof(virtual_reg*));
    
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Right, sizeof(virtual_reg*));
}

internal void PushAssignToMemInstr(virtual_reg* Left, virtual_reg* Right)
{
    ir_instr Instr = {};
    Instr.Id = IR_AssignToMem;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Left, sizeof(virtual_reg*));
    
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Right, sizeof(virtual_reg*));
}

internal void PushJumpHeaderInstr(string Name)
{
    ir_instr Instr = {};
    Instr.Id = IR_JumpHeader;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Name, sizeof(string));    
}

internal void PushJumpInstr(string JumpHeader)
{
    ir_instr Instr = {};
    Instr.Id = IR_Jump;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &JumpHeader, sizeof(string));    
}

internal void PushJumpCondInstr(string LHS, string RHS, string JumpHeader)
{
    ir_instr Instr = {};
    Instr.Id = IR_JumpCond;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
     
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &LHS, sizeof(string));

    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &RHS, sizeof(string));
     
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &JumpHeader, sizeof(string));    
}

internal void PushUniOpInstr(u32 Op, virtual_reg* Arg, virtual_reg* Ret)
{
    Assert(Op > IR_UniaryOpStart && Op < IR_UniaryOpEnd);
    ir_instr Instr = {};
    Instr.Id = Op;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Arg, sizeof(virtual_reg*));
    
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Ret, sizeof(virtual_reg*));
}

internal void PushBinOpInstr(u32 Op, virtual_reg* Arg1, virtual_reg* Arg2, virtual_reg* Ret)
{
    Assert((Op > IR_BinaryOpStart && Op < IR_BinaryOpEnd) || Op == IR_GetMember);
    ir_instr Instr = {};
    Instr.Id = Op;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Arg1, sizeof(virtual_reg*));
    
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Arg2, sizeof(virtual_reg*));
        
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Ret, sizeof(virtual_reg*));
}

internal void PushGetArrayIndexInstr(string Array, string Index, string Result)
{
    ir_instr Instr = {};
    Instr.Id = IR_GetArrayIndex;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
            
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Array, sizeof(string));    
            
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Index, sizeof(string));    
            
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Result, sizeof(string));    
}

internal void PushFuncDeclInstr(function* Func)
{
    ir_instr Instr = {};
    Instr.Id = IR_FuncDecl;
    
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
        void* CurrByte = PushStruct(&IR_Arena, string);
        memcpy(CurrByte, &ArgStr, sizeof(string));        
    }
}

internal void PushFuncCallInstr(string ResultVar, string FuncName, u32 NumArgs, token* Args)
{
    ir_instr Instr = {};
    Instr.Id = IR_FuncCall;
        
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &ResultVar, sizeof(string));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &FuncName, sizeof(string));

    CurrByte = PushStruct(&IR_Arena, u32);
    memcpy(CurrByte, &NumArgs, sizeof(u32));

    for (u32 ArgIndex = 0; ArgIndex < NumArgs; ++ArgIndex)
    {
        void* CurrByte = PushStruct(&IR_Arena, string);
        string TempArgStr = GetTokenString(Args[ArgIndex]);
        memcpy(CurrByte, &TempArgStr, sizeof(string));
    }
}

internal void PushReturnInstr(string ReturnVal = {})
{
    ir_instr Instr = {};
    Instr.Id = IR_Return;
        
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));

    if (ReturnVal.NumChars == 0)
    {
        u32 NumArgs = 0;
        CurrByte = PushStruct(&IR_Arena, u32);
        memcpy(CurrByte, &NumArgs, sizeof(u32));
    }
    else
    {
        u32 NumArgs = 1;
        CurrByte = PushStruct(&IR_Arena, u32);
        memcpy(CurrByte, &NumArgs, sizeof(u32));

        CurrByte = PushStruct(&IR_Arena, string);
        memcpy(CurrByte, &ReturnVal, sizeof(string));        
    }
}
