/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

internal void IfStartToIR(virtual_reg* BoolReg, string ExitLocalHeader)
{
    PushClearRegsInstr();
    PushJumpCondInstr(BoolReg, RegFor0_Int->Reg, ExitLocalHeader);
}

internal void IfEndToIR(string ExitLocalHeader, string ExitTotalHeader)
{
    PushJumpInstr(ExitTotalHeader);
    PushJumpHeaderInstr(ExitLocalHeader);
}

internal void ElseStartToIR()
{
    PushClearRegsInstr();        
}

internal void ElseEndToIR(string ExitTotalHeader)
{
    PushJumpInstr(ExitTotalHeader);
    PushJumpHeaderInstr(ExitTotalHeader);    
}

internal void WhileEndToIR(string StartHeader, string EndHeader)
{
    PushJumpInstr(StartHeader);
    PushJumpHeaderInstr(EndHeader);
}

internal void FuncDeclToIR(function* Func)
{
    PushFuncDeclInstr(Func);
}

internal void FuncEndToIR(function* Func)
{
    PushFuncEndInstr(Func);
}

internal typed_reg* SyntaxTreeToIR(syntax_node* Root, function* Func = 0)
{
    typed_reg* Result = 0;
    
    // NOTE: We just have a constant or variable
    if (Root->Id == TokenId_Var && Root->NumArgs == 0)
    {
        if (!IsTokenConstant(Root->Token) && Root->Token.Id == TokenId_Var)
        {
            Result = PushStruct(&TempArena, typed_reg);
            Result->Reg = GetOrCreateRegister(&RegHashTable, Root->Token.Var->Str);
            Result->Type = Root->Token.Var->Type;
        }

        if (IsTokenConstant(Root->Token))
        {
            Result = PushStruct(&TempArena, typed_reg);
            Result->Reg = GetTempRegister(&RegHashTable);
            Result->Reg->Flags = RegFlag_StoresConstant | RegFlag_IsConstVal;
            Result->Type = Root->Token.Var->Type;
            Result->Reg->Name = Root->Token.Var->Str;

            if (AreSameType(GetTokenType(Root->Token), GetBasicType(TypeId_Float)))
            {
                // NOTE: We convert the string of this var to hex
                f32 Val = ConvertStringToFloat(GetTokenString(Root->Token));
                Result->Reg->Name = ConvertIntToString(&TempArena, *(u32*)&Val);
            }
        }

        if (Root->Token.Id == ScannerId_Identifier)
        {
            // TODO: Check this and also, make registers handle struct members
            // NOTE: This case can happen when we are dealing with struct members
            //Root->Token.Flags = TokenFlag_IsLValue;
        }
        else if (IsTypePointer(GetTokenType(Root->Token)))
        {
            Result->Reg->Flags |= RegFlag_IsLValue;
        }

        // TODO: Other cases? Check these cases better
        
        return Result;
    }

    // TODO: We have lots of temporary variables being created. Create accumulators to optimize
    // NOTE: This variable is the returned value from all our expressions
    if (Root->Id != OpId_Define && Root->Id != OpId_Zero &&
        Root->Id != OpId_Equal && Root->Id != OpId_Run &&
        Root->Id != ScannerId_Return)
    {
        // TODO: We defs need to manage memory here
        Result = PushStruct(&TempArena, typed_reg);
        Result->Reg = GetTempRegister(&RegHashTable);
    }
    
    // NOTE: The current node is a operator
    switch (Root->Id)
    {
        case OpId_Define:
        {
            Assert(Root->NumArgs == 1);
            
            Result = SyntaxTreeToIR(Root->Children[0]);
            Result->Reg->Flags |= RegFlag_IsLValue;
            PushDefineInstr(Result->Reg);

            // TODO: For structs, break them up into members and set them all to 0
        } break;

        case OpId_Zero:
        {
            Assert(Root->NumArgs == 1);
            Result = SyntaxTreeToIR(Root->Children[0]);
            
            if (IsTypePointer(Result->Type) || AreSameType(Result->Type, GetBasicType(TypeId_Int)))
            {
                PushAssignInstr(Result->Reg, RegFor0_Int->Reg);
            }
            else if (AreSameType(Result->Type, GetBasicType(TypeId_Float)))
            {
                PushAssignInstr(Result->Reg, RegFor0_Flt->Reg);
            }
        } break;
        
        case OpId_Dereference:
        {
            // NOTE: The dereferences happening here are when we use the actual value of
            // a dereference. If we are assigning a val to a deref, thats handled in Equal
            Assert(Root->NumArgs == 1);
            typed_reg* Ptr = SyntaxTreeToIR(Root->Children[0]);
            
            if (Ptr->Reg->Flags & RegFlag_StoresConstant || !IsTypePointer(Ptr->Type))
            {
                OutputError("Error: Cannot dereference a non pointer type.\n");
            }
            if ((Ptr->Reg->Flags & RegFlag_IsLValue) == 0)
            {
                OutputError("Error: We can only dereference a l-value.\n");
            }

            Result->Type = ChangeRefCountToPtr(Ptr->Type, -1);
            if (IsTypePointer(Result->Type))
            {
                Result->Reg->Flags |= RegFlag_IsLValue;
            }
            
            PushUniOpInstr(IR_LoadFromMem, Ptr->Reg, Result->Reg);
        } break;

        case OpId_AddressOff:
        {
            Assert(Root->NumArgs == 1);
            typed_reg* Val = SyntaxTreeToIR(Root->Children[0]);

            if (Val->Reg->Flags & RegFlag_StoresConstant)
            {
                OutputError("Error: Cannot take address of a constant.\n");
            }
                    
            Result->Type = ChangeRefCountToPtr(Val->Type, 1);
            PushUniOpInstr(IR_AddressOff, Val->Reg, Result->Reg);
        } break;

        case OpId_Negate:
        {
            Assert(Root->NumArgs == 1);
            typed_reg* Arg = SyntaxTreeToIR(Root->Children[0]);
            
            if (!(Arg->Type == GetBasicType(TypeId_Int) ||
                  Arg->Type == GetBasicType(TypeId_Float)))
            {
                OutputError("Error: Can only negate a variable of type int or float.\n");
            }

            // TODO: Create a get constant instr for floats
            // NOTE: Get a 0 constant
            typed_reg* RegFor0 = 0;
            if (Arg->Type == GetBasicType(TypeId_Int))
            {
                RegFor0 = RegFor0_Int;
                PushMathInstr(IR_SubIntInt, RegFor0->Reg, Arg->Reg, Result->Reg);
            }
            else if (Arg->Type == GetBasicType(TypeId_Float))
            {
                RegFor0 = RegFor0_Flt;
                PushMathInstr(IR_SubFltFlt, RegFor0->Reg, Arg->Reg, Result->Reg);
            }            
            Result->Type = Arg->Type;
        } break;
        
        case OpId_Run:
        {
            Assert(Root->NumArgs == 1);
            Result = SyntaxTreeToIR(Root->Children[0]);
            
            // TODO: Currently this doesnt work properly
            // TODO: We need to properly implement the compile time execution here
            // TODO: We probs don't need the constant flag if we just spit out a constant instead
            //Result.Flags |= RegFlag_IsConstant;
        } break;
        
        case OpId_BinaryEqual:
        case OpId_BinaryNotEqual:
        case OpId_Less:
        case OpId_LessEqual:
        case OpId_Greater:
        case OpId_GreaterEqual:            
        {
            u32 IRCode = 0;
            if (Root->Id == OpId_BinaryEqual) {IRCode = IR_Equal;}
            if (Root->Id == OpId_BinaryNotEqual) {IRCode = IR_NotEqual;}
            if (Root->Id == OpId_Less) {IRCode = IR_Less;}
            if (Root->Id == OpId_LessEqual) {IRCode = IR_LessEqual;}
            if (Root->Id == OpId_Greater) {IRCode = IR_Greater;}
            if (Root->Id == OpId_GreaterEqual) {IRCode = IR_GreaterEqual;}
            
            Assert(Root->NumArgs == 2);
            typed_reg* Left = SyntaxTreeToIR(Root->Children[0]);
            typed_reg* Right = SyntaxTreeToIR(Root->Children[1]);
            
            if (!(Left->Type == GetBasicType(TypeId_Int) &&
                  Right->Type == GetBasicType(TypeId_Int)))
            {
                OutputError("Error: No operator exists for given types.\n");
            }

            virtual_reg* TempReg = GetTempRegister(&RegHashTable);
            PushAssignInstr(TempReg, Left->Reg);
            
            Result->Type = GetBasicType(TypeId_Int);
            if (Left->Reg->Flags & RegFlag_StoresConstant &&
                Right->Reg->Flags & RegFlag_StoresConstant)
            {
                Result->Reg->Flags |= RegFlag_StoresConstant;
            }

            PushCmpInstr(IRCode, TempReg, Right->Reg, Result->Reg);
        } break;

        case OpId_Add:
        case OpId_Sub:
        {
            Assert(Root->NumArgs == 2);
            typed_reg* Left = SyntaxTreeToIR(Root->Children[0]);
            typed_reg* Right = SyntaxTreeToIR(Root->Children[1]);
            
            u32 IRCode = 0;
            if (Root->Id == OpId_Add)
            {
                if (Left->Type == GetBasicType(TypeId_Int) &&
                    Right->Type == GetBasicType(TypeId_Int))
                {
                    IRCode = IR_AddIntInt;
                    Result->Type = GetBasicType(TypeId_Int);
                }
                else if (Left->Type == GetBasicType(TypeId_Float) &&
                    Right->Type == GetBasicType(TypeId_Int))
                {
                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;
                    
                    IRCode = IR_AddFltInt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (Right->Type == GetBasicType(TypeId_Float) &&
                    Left->Type == GetBasicType(TypeId_Int))
                {
                    typed_reg* Temp = Left;
                    Left = Right;
                    Right = Temp;

                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;
                    
                    IRCode = IR_AddFltInt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (Left->Type == GetBasicType(TypeId_Float) &&
                    Right->Type == GetBasicType(TypeId_Float))
                {
                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;
                    
                    IRCode = IR_AddFltFlt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (IsTypePointer(Left->Type) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    IRCode = IR_AddIntInt;
                    Result->Reg->Flags |= RegFlag_IsLValue;
                    Result->Type = Left->Type;
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                         IsTypePointer(Right->Type))
                {
                    IRCode = IR_AddIntInt;
                    Result->Reg->Flags |= RegFlag_IsLValue;
                    Result->Type = Right->Type;
                }
                else
                {
                    OutputError("Error: Unsupported type in addition statement.\n");
                }
            }
            else if (Root->Id == OpId_Sub)
            {
                if (Left->Type == GetBasicType(TypeId_Int) &&
                    Right->Type == GetBasicType(TypeId_Int))
                {
                    IRCode = IR_SubIntInt;
                    Result->Type = GetBasicType(TypeId_Int);
                }
                else if (Left->Type == GetBasicType(TypeId_Float) &&
                    Right->Type == GetBasicType(TypeId_Int))
                {
                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;

                    IRCode = IR_SubFltInt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (Right->Type == GetBasicType(TypeId_Float) &&
                    Left->Type == GetBasicType(TypeId_Int))
                {
                    typed_reg* Temp = Left;
                    Left = Right;
                    Right = Temp;

                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;

                    IRCode = IR_SubFltInt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (Left->Type == GetBasicType(TypeId_Float) &&
                    Right->Type == GetBasicType(TypeId_Float))
                {
                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;

                    IRCode = IR_SubFltFlt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (IsTypePointer(Left->Type) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    IRCode = IR_SubIntInt;
                    Result->Reg->Flags |= RegFlag_IsLValue;
                    Result->Type = Left->Type;
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                         IsTypePointer(Right->Type))
                {
                    IRCode = IR_SubIntInt;
                    Result->Reg->Flags |= RegFlag_IsLValue;
                    Result->Type = Right->Type;;
                }
                else
                {
                    OutputError("Error: Unsupported type in subtraction statement.\n");
                }
            }
            
            if (Left->Reg->Flags & RegFlag_StoresConstant &&
                Right->Reg->Flags & RegFlag_StoresConstant)
            {
                Result->Reg->Flags |= RegFlag_StoresConstant;
            }
      
            PushMathInstr(IRCode, Left->Reg, Right->Reg, Result->Reg);            
        } break;
        
        case OpId_Mul:
        case OpId_Div:
        {
            Assert(Root->NumArgs == 2);
            typed_reg* Left = SyntaxTreeToIR(Root->Children[0]);
            typed_reg* Right = SyntaxTreeToIR(Root->Children[1]);
            
            u32 IRCode = 0;
            if (Root->Id == OpId_Mul)
            {
                if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                    AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    IRCode = IR_MulIntInt;
                    Result->Type = GetBasicType(TypeId_Int);
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Int)))
                {
                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;

                    IRCode = IR_MulFltInt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (AreSameType(Right->Type, GetBasicType(TypeId_Float)) &&
                         AreSameType(Left->Type, GetBasicType(TypeId_Int)))
                {
                    typed_reg* Temp = Left;
                    Left = Right;
                    Right = Temp;

                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;

                    IRCode = IR_MulFltInt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Float)))
                {
                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;

                    IRCode = IR_MulFltFlt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else
                {
                    OutputError("Error: Unsupported type in addition statement.\n");
                }
            }
            else if (Root->Id == OpId_Div)
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
                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;

                    IRCode = IR_DivFltInt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (AreSameType(Right->Type, GetBasicType(TypeId_Int)) &&
                         AreSameType(Left->Type, GetBasicType(TypeId_Float)))
                {
                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;

                    IRCode = IR_DivIntFlt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                         AreSameType(Right->Type, GetBasicType(TypeId_Float)))
                {
                    virtual_reg* Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Left->Reg);
                    Left->Reg = Reg;

                    Reg = GetTempRegister(&RegHashTable);
                    PushAssignInstr(Reg, Right->Reg);
                    Right->Reg = Reg;

                    IRCode = IR_DivFltFlt;
                    Result->Type = GetBasicType(TypeId_Float);
                }
                else
                {
                    OutputError("Error: Unsupported type in addition statement.\n");
                }
            }
            
            if (Left->Reg->Flags & RegFlag_StoresConstant &&
                Right->Reg->Flags & RegFlag_StoresConstant)
            {
                Result->Reg->Flags |= RegFlag_StoresConstant;
            }
      
            PushMathInstr(IRCode, Left->Reg, Right->Reg, Result->Reg);
        } break;

        case OpId_GetMember:
        {
            Assert(Root->NumArgs == 2);
            typed_reg* Left = SyntaxTreeToIR(Root->Children[0]);
            typed_reg* Right = SyntaxTreeToIR(Root->Children[1]);
            
#if 0
            type* StructType = GetTokenType(Left);
            if (IsTypePointer(StructType))
            {
                StructType = StructType->PointerType;
            }
            
            if (!IsTypeStruct(StructType))
            {
                OutputError("Error: Cannot access member from a non struct type.\n");
            }

            // NOTE: We get the type of our member and make sure it exists
            type* ArgType = 0;
            u8* CurrByte = StructType->Members;
            for (u32 MemberIndex = 0; MemberIndex < StructType->NumMembers; ++MemberIndex)
            {
                type* CurrArgType = *((type**)CurrByte);
                CurrByte += sizeof(type*);

                string* ArgName = (string*)CurrByte;
                CurrByte += sizeof(string);
                if (StringCompare(GetTokenString(Right), *ArgName))
                {
                    ArgType = CurrArgType;
                    break;
                }
            }

            if (!ArgType)
            {
                OutputError("Error: Member doesn't exist in the struct.\n");
            }
            
            ResultVarName.Var->Type = ArgType;
            ResultVarName.Flags = TokenFlag_IsLValue;
            
            string LeftStr = GetTokenString(Left);
            string RightStr = GetTokenString(Right);        
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushBinOpInstr(IR_GetMember, Left.Var, Right.Var, ResultVarName.Var);
#endif
        } break;
        
        case OpId_Equal:
        {            
            Assert(Root->NumArgs == 2);

            // TODO: Add array indexing here
            // NOTE: If we have a dereference, we need to write to that location in memory
            typed_reg* Left;
            typed_reg* Right = SyntaxTreeToIR(Root->Children[1]);
            if (Root->Children[0]->Id == OpId_Dereference ||
                Root->Children[0]->Id == OpId_GetArrayIndex)
            {
                syntax_node* DerefNode = Root->Children[0];

                Assert(DerefNode->NumArgs == 1);
                Left = SyntaxTreeToIR(DerefNode->Children[0]);

                type* LeftType = ChangeRefCountToPtr(Left->Type, -1);
                if (!AreSameType(LeftType, Right->Type))
                {
                    OutputError("Error: Type on the left of '=' must be the same type as on the right.\n");
                }
                PushAssignToMemInstr(Left->Reg, Right->Reg);
            }
            else
            {
                Left = SyntaxTreeToIR(Root->Children[0]);

                if (!AreSameType(Left->Type, Right->Type))
                {
                    OutputError("Error: Type on the left of '=' must be the same type as on the right.\n");
                }
                PushAssignInstr(Left->Reg, Right->Reg);
            }
            
            if (!(Left->Reg->Flags & RegFlag_IsLValue))
            {
                OutputError("Error: Left of '=' must be a l-value.\n");
            }
        } break;

        case OpId_GetArrayIndex:
        {
            Assert(Root->NumArgs == 2);
            typed_reg* Left = SyntaxTreeToIR(Root->Children[0]);
            typed_reg* Right = SyntaxTreeToIR(Root->Children[1]);
            
            if (!IsTypePointer(Left->Type))
            {
                OutputError("Error: We can only index array types.\n");
            }
            if (Right->Type != GetBasicType(TypeId_Int))
            {
                OutputError("Error: Index of an array must be of type intger.\n");
            }

            Result->Type = ChangeRefCountToPtr(Left->Type, -1);
            Result->Reg->Flags |= RegFlag_IsLValue;

            virtual_reg* Temp = GetTempRegister(&RegHashTable);
            virtual_reg* TypeSize =
                GetOrCreateConstIntRegister(&RegHashTable, Result->Type->SizeInBytes);
            
            PushMathInstr(IR_MulIntInt, Right->Reg, TypeSize, Temp);
            PushMathInstr(IR_AddIntInt, Left->Reg, Temp, Result->Reg);
        } break;
        
        case OpId_FuncCall:
        {
            Assert(Root->NumArgs == Root->Func->NumArgs);
            
            temp_mem TempMem = BeginTempMem(&TempArena);
            virtual_reg** ArgRegs = PushArray(&TempArena, virtual_reg*, Root->NumArgs);
            for (u32 ArgIndex = 0; ArgIndex < Root->NumArgs; ++ArgIndex)
            {
                type* RequiredArgType = Root->Func->ArgTypes[ArgIndex];

                typed_reg* ArgReg = SyntaxTreeToIR(Root->Children[ArgIndex]);
                ArgRegs[ArgIndex] = ArgReg->Reg;
                
                if (!AreSameType(RequiredArgType, ArgReg->Type))
                {
                    OutputError("Error: Type doesn't match the required type for the function arg.\n");
                }
            }

            // TODO: If no return value, make result empty
            Result->Type = Root->Func->ReturnType;
            PushFuncCallInstr(Result->Reg, Root->Func, ArgRegs);

            ClearTempMem(TempMem);
        } break;

        case ScannerId_Return:
        {
            if (Root->NumArgs == 0)
            {                
                if (!AreSameType(Func->ReturnType, GetBasicType(TypeId_Void)))
                {
                    OutputError("Error: Return value isn't of the same type as functions return type.\n");
                }
                PushReturnInstr(0, Func);
            }
            else if (Root->NumArgs == 1)
            {
                typed_reg* Reg = SyntaxTreeToIR(Root->Children[0]);
                if (!AreSameType(Func->ReturnType, Reg->Type))
                {
                    OutputError("Error: Return value isn't of the same type as functions return type.\n");
                }
                PushReturnInstr(Reg->Reg, Func);
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
