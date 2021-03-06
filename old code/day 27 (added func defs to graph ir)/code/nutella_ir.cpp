
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

internal void PushClearRegsInstr()
{
    ir_instr Instr = {};
    Instr.Id = IR_ClearRegs;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
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

internal void PushJumpCondInstr(virtual_reg* LHS, virtual_reg* RHS, string JumpHeader)
{
    ir_instr Instr = {};
    Instr.Id = IR_JumpCond;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
     
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &LHS, sizeof(virtual_reg*));

    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &RHS, sizeof(virtual_reg*));
     
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

internal void PushCmpInstr(u32 Op, virtual_reg* Arg1, virtual_reg* Arg2, virtual_reg* Ret)
{
    //Assert((Op > IR_BinaryOpStart && Op < IR_BinaryOpEnd) || Op == IR_GetMember);
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

internal void PushMathInstr(u32 Op, virtual_reg* Arg1, virtual_reg* Arg2, virtual_reg* Ret)
{
    //Assert((Op > IR_BinaryOpStart && Op < IR_BinaryOpEnd) || Op == IR_GetMember);
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

internal void PushFuncDeclInstr(function* Func)
{
    ir_instr Instr = {};
    Instr.Id = IR_FuncDecl;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
            
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Func->Name, sizeof(string));    

    u32 StackSize = 0;
    CurrByte = PushStruct(&IR_Arena, u32);
    memcpy(CurrByte, &StackSize, sizeof(u32));
}

internal void PushFuncEndInstr(function* Func)
{
    ir_instr Instr = {};
    Instr.Id = IR_FuncEnd;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, function*);
    memcpy(CurrByte, &Func, sizeof(function*));    
}

internal void PushFuncCallInstr(virtual_reg* Result, function* Func, virtual_reg** Args)
{
    ir_instr Instr = {};
    Instr.Id = IR_FuncCall;
        
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));

    b32 HasReturnType = !(AreSameType(Func->ReturnType, GetBasicType(TypeId_Void)));
    CurrByte = PushStruct(&IR_Arena, b32);
    memcpy(CurrByte, &HasReturnType, sizeof(b32));
    
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Result, sizeof(virtual_reg*));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Func->Name, sizeof(string));

    CurrByte = PushStruct(&IR_Arena, u32);
    memcpy(CurrByte, &Func->NumArgs, sizeof(u32));

    for (u32 ArgIndex = 0; ArgIndex < Func->NumArgs; ++ArgIndex)
    {
        void* CurrByte = PushStruct(&IR_Arena, virtual_reg*);
        memcpy(CurrByte, Args + ArgIndex, sizeof(virtual_reg*));
    }
}

internal void PushReturnInstr(virtual_reg* Reg, function* Func)
{
    ir_instr Instr = {};
    Instr.Id = IR_Return;
        
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, virtual_reg*);
    memcpy(CurrByte, &Reg, sizeof(virtual_reg*));
    
    CurrByte = PushStruct(&IR_Arena, function*);
    memcpy(CurrByte, &Func, sizeof(function*));
}

internal void StartWhileInstr(ir_state* IrState)
{
    ir_instr Instr = {};
    //Instr.Id = IR_While;
        
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));

    // NOTE: This is a pointer to the begining of the loop
    CurrByte = PushStruct(&IR_Arena, ir_instr*);
    memcpy(CurrByte, &CurrByte, sizeof(ir_instr*));

    // NOTE: This is a pointer for exiting the loop, we set this later
    //IrState->ExitWhilePtr = PushStruct(&IR_Arena, ir_instr*);
}
