
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

internal void PushDefineInstr(var* Reg)
{
    ir_instr Instr = {};
    Instr.Id = IR_Define;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));

    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Reg, sizeof(var*));
}

internal void PushAssignInstr(var* Left, var* Right)
{
    ir_instr Instr = {};
    Instr.Id = IR_Assign;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Left, sizeof(var*));
    
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Right, sizeof(var*));
}

internal void PushAssignToMemInstr(var* Left, var* Right)
{
    ir_instr Instr = {};
    Instr.Id = IR_AssignToMem;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Left, sizeof(var*));
    
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Right, sizeof(var*));
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

internal void PushJumpCondInstr(var* LHS, var* RHS, string JumpHeader)
{
    ir_instr Instr = {};
    Instr.Id = IR_JumpCond;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
     
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &LHS, sizeof(var*));

    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &RHS, sizeof(var*));
     
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &JumpHeader, sizeof(string));    
}

internal void PushUniOpInstr(u32 Op, var* Arg, var* Ret)
{
    Assert(Op > IR_UniaryOpStart && Op < IR_UniaryOpEnd);
    ir_instr Instr = {};
    Instr.Id = Op;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Arg, sizeof(var*));
    
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Ret, sizeof(var*));
}

internal void PushCmpInstr(u32 Op, var* Arg1, var* Arg2, var* Ret)
{
    //Assert((Op > IR_BinaryOpStart && Op < IR_BinaryOpEnd) || Op == IR_GetMember);
    ir_instr Instr = {};
    Instr.Id = Op;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Arg1, sizeof(var*));
    
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Arg2, sizeof(var*));
        
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Ret, sizeof(var*));    
}

internal void PushMathInstr(u32 Op, var* Arg1, var* Arg2, var* Ret)
{
    //Assert((Op > IR_BinaryOpStart && Op < IR_BinaryOpEnd) || Op == IR_GetMember);
    ir_instr Instr = {};
    Instr.Id = Op;

    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
        
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Arg1, sizeof(var*));
    
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Arg2, sizeof(var*));
        
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Ret, sizeof(var*));
}

internal void PushFuncDeclInstr(function* Func)
{
    ir_instr Instr = {};
    Instr.Id = IR_FuncDecl;
    
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
            
    CurrByte = PushStruct(&IR_Arena, function*);
    memcpy(CurrByte, &Func, sizeof(function*));    

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

internal void PushFuncCallInstr(var* Result, function* Func, var** Args)
{
    ir_instr Instr = {};
    Instr.Id = IR_FuncCall;
        
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));

    b32 HasReturnType = !(AreSameType(Func->ReturnType, GetBasicType(TypeId_Void)));
    CurrByte = PushStruct(&IR_Arena, b32);
    memcpy(CurrByte, &HasReturnType, sizeof(b32));
    
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Result, sizeof(var*));
    
    CurrByte = PushStruct(&IR_Arena, string);
    memcpy(CurrByte, &Func->Name, sizeof(string));

    CurrByte = PushStruct(&IR_Arena, u32);
    memcpy(CurrByte, &Func->NumArgs, sizeof(u32));

    for (u32 ArgIndex = 0; ArgIndex < Func->NumArgs; ++ArgIndex)
    {
        void* CurrByte = PushStruct(&IR_Arena, var*);
        memcpy(CurrByte, Args + ArgIndex, sizeof(var*));
    }
}

internal void PushReturnInstr(var* Reg, function* Func)
{
    ir_instr Instr = {};
    Instr.Id = IR_Return;
        
    void* CurrByte = PushStruct(&IR_Arena, ir_instr);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    
    CurrByte = PushStruct(&IR_Arena, var*);
    memcpy(CurrByte, &Reg, sizeof(var*));
    
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

internal void RecursiveDefineVar(var* Var)
{    
    if (!IsTypeStruct(Var->Type))
    {
        PushDefineInstr(Var);
    }
    else
    {
        for (u32 MemberIndex = 0; MemberIndex < Var->Type->NumMembers; ++MemberIndex)
        {
            RecursiveDefineVar(Var->Members + MemberIndex);
        }
    }
}

internal void RecursiveZeroVar(var* Var)
{    
    if (IsTypePointer(Var->Type) || AreSameType(Var->Type, GetBasicType(TypeId_Int)))
    {
        PushAssignInstr(Var, CreateConstIntVar(0));
    }
    else if (AreSameType(Var->Type, GetBasicType(TypeId_Float)))
    {
        PushAssignInstr(Var, CreateConstFloatVar(0.0));
    }
    else if (IsTypeStruct(Var->Type))
    {
        for (u32 MemberIndex = 0; MemberIndex < Var->Type->NumMembers; ++MemberIndex)
        {
            RecursiveZeroVar(Var->Members + MemberIndex);
        }
    }
}

internal void RecursiveCopy(var* Dest, var* Source)
{
    Assert(AreSameType(Dest->Type, Source->Type));
    
    if (!IsTypeStruct(Source->Type))
    {
        PushAssignInstr(Dest, Source);
    }
    else
    {
        for (u32 MemberIndex = 0; MemberIndex < Dest->Type->NumMembers; ++MemberIndex)
        {
            RecursiveCopy(Dest->Members + MemberIndex, Source->Members + MemberIndex);
        }
    }
}

internal void RecursiveDeref(var* Dest, var* Source)
{
    if (!IsTypeStruct(Dest->Type))
    {
        PushUniOpInstr(IR_LoadFromMem, Source, Dest);
    }
    else
    {
        for (u32 MemberIndex = 0; MemberIndex < Dest->Type->NumMembers; ++MemberIndex)
        {
            var* DestMember = Dest->Members + MemberIndex;            
            var* MemberOffset = CreateConstIntVar(GetStructMemOffset(Dest->Type, DestMember));
            
            PushMathInstr(IR_SubIntInt, Source, MemberOffset, DestMember);
            RecursiveDeref(DestMember, DestMember);
        }
    }
}

internal var* RecursiveSyntaxTreeToIR(function* Func, syntax_node* Root)
{
    var* Result = Root->Var;
    switch (Root->Id)
    {
        case NodeId_Var:
        {
            Result = Root->Var;
            
            // NOTE: If we have a float, we convert its value to hex for our string
            if (Root->Var->Flags & RegFlag_IsConstVal &&
                AreSameType(Result->Type, GetBasicType(TypeId_Float)))
            {
                f32 Val = ConvertStringToFloat(Result->Str);
                Result->Str = ConvertIntToString(&TempArena, *(u32*)&Val);
            }
        } break;
        
        case NodeId_Define:
        {
            Assert(Root->NumArgs == 1);

            Result = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            RecursiveDefineVar(Result);
        } break;
        
        case NodeId_Zero:
        {
            Assert(Root->NumArgs == 1);
            Result = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            RecursiveZeroVar(Result);
        } break;
        
        case NodeId_Dereference:
        {
            // NOTE: The dereferences happening here are when we use the actual value of
            // a dereference. If we are assigning a val to a deref, thats handled in Equal
            Assert(Root->NumArgs == 1);
            var* Ptr = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            RecursiveDefineVar(Result);
            RecursiveDeref(Result, Ptr);
        } break;

        case NodeId_AddressOff:
        {
            Assert(Root->NumArgs == 1);
            var* Val = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);

            if (IsTypeStruct(Val->Type))
            {
                PushUniOpInstr(IR_AddressOff, Val->Members, Result);
            }
            else
            {            
                PushUniOpInstr(IR_AddressOff, Val, Result);
            }
        } break;

        case NodeId_Negate:
        {
            Assert(Root->NumArgs == 1);
            var* Arg = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            
            if (Arg->Type == GetBasicType(TypeId_Int))
            {
                PushMathInstr(IR_SubIntInt, CreateConstIntVar(0), Arg, Result);
            }
            else if (Arg->Type == GetBasicType(TypeId_Float))
            {
                PushMathInstr(IR_SubFltFlt, CreateConstFloatVar(0.0), Arg, Result);
            }
        } break;
        
        case NodeId_Run:
        {
            Assert(Root->NumArgs == 1);
            Result = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
        } break;
        
        case NodeId_BinaryEqual:
        case NodeId_BinaryNotEqual:
        case NodeId_Less:
        case NodeId_LessEqual:
        case NodeId_Greater:
        case NodeId_GreaterEqual:            
        {
            u32 IRCode = 0;
            if (Root->Id == NodeId_BinaryEqual) {IRCode = IR_Equal;}
            if (Root->Id == NodeId_BinaryNotEqual) {IRCode = IR_NotEqual;}
            if (Root->Id == NodeId_Less) {IRCode = IR_Less;}
            if (Root->Id == NodeId_LessEqual) {IRCode = IR_LessEqual;}
            if (Root->Id == NodeId_Greater) {IRCode = IR_Greater;}
            if (Root->Id == NodeId_GreaterEqual) {IRCode = IR_GreaterEqual;}
            
            Assert(Root->NumArgs == 2);
            var* Left = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);
            
            PushAssignInstr(Result, Left);
            PushCmpInstr(IRCode, Result, Right, Result);
        } break;

        case NodeId_Add:
        case NodeId_Sub:
        {
            Assert(Root->NumArgs == 2);
            var* Left = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);
            
            u32 IRCode = 0;
            if (Root->Id == NodeId_Add)
            {
                if (Left->Type == GetBasicType(TypeId_Int) &&
                    Right->Type == GetBasicType(TypeId_Int))
                {
                    IRCode = IR_AddIntInt;
                }
                else if (Left->Type == GetBasicType(TypeId_Float) &&
                    Right->Type == GetBasicType(TypeId_Int))
                {
                    IRCode = IR_AddFltInt;
                }
                else if (Right->Type == GetBasicType(TypeId_Float) &&
                    Left->Type == GetBasicType(TypeId_Int))
                {
                    var* Temp = Left;
                    Left = Right;
                    Right = Temp;
                    
                    IRCode = IR_AddFltInt;
                }
                else if (Left->Type == GetBasicType(TypeId_Float) &&
                    Right->Type == GetBasicType(TypeId_Float))
                {
                    IRCode = IR_AddFltFlt;
                }
                else if (IsTypePointer(Left->Type) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    IRCode = IR_AddIntInt;
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                         IsTypePointer(Right->Type))
                {
                    IRCode = IR_AddIntInt;
                }
                else
                {
                    OutputError("Error: Unsupported type in addition statement.\n");
                }
            }
            else if (Root->Id == NodeId_Sub)
            {
                if (Left->Type == GetBasicType(TypeId_Int) &&
                    Right->Type == GetBasicType(TypeId_Int))
                {
                    IRCode = IR_SubIntInt;
                }
                else if (Left->Type == GetBasicType(TypeId_Float) &&
                    Right->Type == GetBasicType(TypeId_Int))
                {
                    IRCode = IR_SubFltInt;
                }
                else if (Right->Type == GetBasicType(TypeId_Float) &&
                    Left->Type == GetBasicType(TypeId_Int))
                {
                    var* Temp = Left;
                    Left = Right;
                    Right = Temp;

                    IRCode = IR_SubFltInt;
                }
                else if (Left->Type == GetBasicType(TypeId_Float) &&
                    Right->Type == GetBasicType(TypeId_Float))
                {
                    IRCode = IR_SubFltFlt;
                }
                else if (IsTypePointer(Left->Type) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    IRCode = IR_SubIntInt;
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                         IsTypePointer(Right->Type))
                {
                    IRCode = IR_SubIntInt;
                }
                else
                {
                    OutputError("Error: Unsupported type in subtraction statement.\n");
                }
            }
                  
            PushMathInstr(IRCode, Left, Right, Result);            
        } break;
        
        case NodeId_Mul:
        case NodeId_Div:
        {
            Assert(Root->NumArgs == 2);
            var* Left = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);
            
            u32 IRCode = 0;
            if (Root->Id == NodeId_Mul)
            {
                if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                    AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    IRCode = IR_MulIntInt;
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    IRCode = IR_MulFltInt;
                }
                else if (AreSameType(Right->Type, GetBasicType(TypeId_Float)) &&
                         AreSameType(Left->Type, GetBasicType(TypeId_Int)))
                {
                    var* Temp = Left;
                    Left = Right;
                    Right = Temp;

                    IRCode = IR_MulFltInt;
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Float)))
                {
                    IRCode = IR_MulFltFlt;
                }
                else
                {
                    OutputError("Error: Unsupported type in addition statement.\n");
                }
            }
            else if (Root->Id == NodeId_Div)
            {
                if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                    AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    IRCode = IR_DivIntInt;
                    Result->Type = GetBasicType(TypeId_Int);
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    IRCode = IR_DivFltInt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (AreSameType(Right->Type, GetBasicType(TypeId_Int)) &&
                         AreSameType(Left->Type, GetBasicType(TypeId_Float)))
                {
                    IRCode = IR_DivIntFlt;
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Float)))
                {
                    IRCode = IR_DivFltFlt;
                }
                else
                {
                    OutputError("Error: Unsupported type in addition statement.\n");
                }
            }
      
            PushMathInstr(IRCode, Left, Right, Result);
        } break;

        case NodeId_GetMember:
        {
            Assert(Root->NumArgs == 2);
            var* Left = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);

            Result = Right;
            if (IsTypePointer(Left->Type))
            {
                PushDefineInstr(Result);
                PushMathInstr(IR_SubIntInt, Left,
                              CreateConstIntVar(GetStructMemOffset(Left->Type->PointerType, Right)),
                              Result);
                PushUniOpInstr(IR_LoadFromMem, Result, Result);
            }
        } break;
        
        case NodeId_Equal:
        {            
            Assert(Root->NumArgs == 2);
            
            // NOTE: If we have a dereference, we need to write to that location in memory
            syntax_node* LeftNode = Root->Children[0];
            if (LeftNode->Id == NodeId_Dereference)
            {
                // NOTE: We have a ptr deref so we need our right value to write to mem
                var* Left = RecursiveSyntaxTreeToIR(Func, LeftNode->Children[0]);
                var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);
                PushAssignToMemInstr(Left, Right);
            }
            else if (LeftNode->Id == NodeId_GetArrayIndex)
            {
                var* Left = RecursiveSyntaxTreeToIR(Func, LeftNode->Children[0]);
                var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);
                var* Index = RecursiveSyntaxTreeToIR(Func, LeftNode->Children[1]);
                PushMathInstr(IR_SubIntInt, Left, Index, Left);
                PushAssignToMemInstr(Left, Right);
            }
            else if (LeftNode->Id == NodeId_GetMember &&
                     IsTypePointer(LeftNode->Children[0]->Var->Type))
            {
                // NOTE: We have a pointer struct member access so we need to copy right into mem
                var* Struct = RecursiveSyntaxTreeToIR(Func, LeftNode->Children[0]);
                var* Member = RecursiveSyntaxTreeToIR(Func, LeftNode->Children[1]);
                var* MemberOffset = CreateConstIntVar(
                    GetStructMemOffset(Struct->Type->PointerType, Member));
                var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);
                var* MemberAddr = GetTempVar();
                MemberAddr->Type = GetBasicType(TypeId_Int);
                
                RecursiveDefineVar(MemberAddr);
                PushMathInstr(IR_SubIntInt, Struct, MemberOffset, MemberAddr);
                PushAssignToMemInstr(MemberAddr, Right);
            }
            else
            {
                var* Left = RecursiveSyntaxTreeToIR(Func, LeftNode);
                var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);
                RecursiveCopy(Left, Right);
            }
        } break;

        case NodeId_GetArrayIndex:
        {
            Assert(Root->NumArgs == 2);
            var* Left = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);
            var* TypeSize = CreateConstIntVar(Result->Type->SizeInBytes);
            
            PushMathInstr(IR_MulIntInt, Right, TypeSize, Result);
            PushMathInstr(IR_AddIntInt, Left, Result, Result);
        } break;
        
        case NodeId_FuncCall:
        {
            Assert(Root->NumArgs == Root->Func->NumArgs);
            
            temp_mem TempMem = BeginTempMem(&TempArena);
            var** ArgRegs = PushArray(&TempArena, var*, Root->NumArgs);
            for (u32 ArgIndex = 0; ArgIndex < Root->NumArgs; ++ArgIndex)
            {
                var* ArgReg = RecursiveSyntaxTreeToIR(Func, Root->Children[ArgIndex]);
                ArgRegs[ArgIndex] = ArgReg;
            }

            // TODO: If no return value, make result empty
            PushFuncCallInstr(Result, Root->Func, ArgRegs);
            ClearTempMem(TempMem);
        } break;

        case NodeId_Return:
        {
            if (Root->NumArgs == 0)
            {            
                if (!AreSameType(Func->ReturnType, GetBasicType(TypeId_Void)))
                {
                    OutputError("Error: Return value isn't of the same type as functions return type.\n");
                }
                PushReturnInstr(0, Root->Func);
            }
            else if (Root->NumArgs == 1)
            {
                Result = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
                if (!AreSameType(Func->ReturnType, Result->Type))
                {
                    OutputError("Error: Return value isn't of the same type as functions return type.\n");
                }
                PushReturnInstr(Result, Root->Func);
            }
            else
            {
                InvalidCodePath;
            }
        } break;
        
        default:
        {
            InvalidCodePath;
        } break;
    }
        
    return Result;
}

internal void SyntaxTreeToIR(name_state* NameState, syntax_node* Root, function* Func = 0)
{
    syntax_node* CurrNode = Root;
    while (CurrNode)
    {
        switch (CurrNode->Id)
        {
            case NodeId_If:
            {
                Assert(CurrNode->NumArgs == 2);

                string ExitHeader = GetExitIfHeader(NameState);
                
                do
                {
                    string ExitIfHeader = GetExitIfHeader(NameState);
                    var* BoolReg = RecursiveSyntaxTreeToIR(Func, CurrNode->Children[0]);
                    
                    PushClearRegsInstr();
                    PushJumpCondInstr(BoolReg, CreateConstIntVar(0), ExitIfHeader);
                    SyntaxTreeToIR(NameState, CurrNode->Children[1], Func);
                    PushJumpInstr(ExitHeader);
                    PushJumpHeaderInstr(ExitIfHeader);

                    CurrNode = CurrNode->Next;
                }
                while (CurrNode->Id == NodeId_ElseIf);

                if (CurrNode->Id == NodeId_Else)
                {
                    Assert(CurrNode->NumArgs == 1);
                    PushClearRegsInstr();
                    SyntaxTreeToIR(NameState, CurrNode->Children[0], Func);
                    CurrNode = CurrNode->Next;
                }
                    
                PushJumpHeaderInstr(ExitHeader);
            } break;

            case NodeId_While:
            {
                Assert(CurrNode->NumArgs == 2);

                string WhileStartHeader = GetWhileStartHeader(NameState);
                string WhileExitHeader = GetWhileExitHeader(NameState);

                PushClearRegsInstr();
                PushJumpHeaderInstr(WhileStartHeader);
                var* BoolReg = RecursiveSyntaxTreeToIR(Func, CurrNode->Children[0]);
                PushJumpCondInstr(BoolReg, CreateConstIntVar(0), WhileExitHeader);
                SyntaxTreeToIR(NameState, CurrNode->Children[1], Func);
                PushJumpInstr(WhileStartHeader);
                PushJumpHeaderInstr(WhileExitHeader);
                PushClearRegsInstr();

                CurrNode = CurrNode->Next;
            } break;
            
            case NodeId_FuncDef:
            {
                Func = CurrNode->Func;
                
                PushFuncDeclInstr(Func);
                SyntaxTreeToIR(NameState, CurrNode->Children[0], Func);
                PushFuncEndInstr(Func);
                Func = 0;
                
                CurrNode = CurrNode->Next;
            } break;

            default:
            {
                RecursiveSyntaxTreeToIR(Func, CurrNode);
                CurrNode = CurrNode->Next;
            } break;
        }
    }
}
