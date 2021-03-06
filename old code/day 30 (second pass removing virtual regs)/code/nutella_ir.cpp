
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

            if (IsTypeStruct(Result->Type))
            {
                // TODO: Change the struct type since most of the info is being stored in vars 
                for (u32 MemberIndex = 0; MemberIndex < Result->Type->NumMembers; ++MemberIndex)
                {
                    PushDefineInstr(Result->Members + MemberIndex);
                }
            }
            else
            {
                PushDefineInstr(Result);
            }
        } break;
        
        case NodeId_Zero:
        {
            Assert(Root->NumArgs == 1);
            Result = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            
            if (IsTypePointer(Result->Type) || AreSameType(Result->Type, GetBasicType(TypeId_Int)))
            {
                PushAssignInstr(Result, CreateConstIntVar(0));
            }
            else if (AreSameType(Result->Type, GetBasicType(TypeId_Float)))
            {
                PushAssignInstr(Result, CreateConstFloatVar(0.0));
            }
        } break;
        
        case NodeId_Dereference:
        {
            // NOTE: The dereferences happening here are when we use the actual value of
            // a dereference. If we are assigning a val to a deref, thats handled in Equal
            Assert(Root->NumArgs == 1);
            var* Ptr = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
            
            PushUniOpInstr(IR_LoadFromMem, Ptr, Result);
        } break;

        case NodeId_AddressOff:
        {
            Assert(Root->NumArgs == 1);
            var* Val = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
                    
            PushUniOpInstr(IR_AddressOff, Val, Result);
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
        } break;
        
        case NodeId_Equal:
        {            
            Assert(Root->NumArgs == 2);

            // TODO: Add array indexing here
            // NOTE: If we have a dereference, we need to write to that location in memory
            var* Left;
            var* Right = RecursiveSyntaxTreeToIR(Func, Root->Children[1]);
            if (Root->Children[0]->Id == NodeId_Dereference ||
                Root->Children[0]->Id == NodeId_GetArrayIndex)
            {
                syntax_node* DerefNode = Root->Children[0];
                Left = RecursiveSyntaxTreeToIR(Func, DerefNode->Children[0]);
                PushAssignToMemInstr(Left, Right);
            }
            else
            {
                Left = RecursiveSyntaxTreeToIR(Func, Root->Children[0]);
                PushAssignInstr(Left, Right);
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
                Assert(CurrNode->NumArgs == 1);

                string ExitHeader = GetExitIfHeader(NameState);
                
                do
                {
                    var* BoolReg = RecursiveSyntaxTreeToIR(Func, CurrNode->Children[0]);
                    PushClearRegsInstr();
                    PushJumpCondInstr(BoolReg, CreateConstIntVar(0), ExitHeader);
                    SyntaxTreeToIR(NameState, CurrNode->Children[1], Func);
                    PushJumpInstr(ExitHeader);

                    CurrNode = CurrNode->Next;
                }
                while (CurrNode->Id == NodeId_ElseIf);

                if (CurrNode->Id == NodeId_Else)
                {
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
