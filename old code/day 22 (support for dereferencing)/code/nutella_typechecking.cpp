/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

inline type* ChangeRefCountToPtr(type* Type, i32 Change)
{
    type* Result = 0;
    if (IsTypePointer(Type))
    {
        if (Type->NumRef + Change == 0)
        {
            // NOTE: We don't need to free the pointer type here since it will be cleared
            // once we exit the scope
            Result = Type->PointerType;
        }
        else
        {
            // NOTE: Ptr types created here we put on the scope arena to clear them later    
            Result = PushStruct(&ScopeArena, type);
            *Result = *Type;
            Result->NumRef += Change;
        }
    }
    else
    {
        // NOTE: Ptr types created here we put on the scope arena to clear them later
        Assert(Change >= 0);

        Result = PushStruct(&ScopeArena, type);
        Result->Flags = TypeFlag_IsTypePointer;
        Result->NumRef = Change;
        Result->PointerType = Type;
    }

    return Result;
}

internal virtual_reg* SyntaxTreeToIR(syntax_node* Root, FILE* OutFile, type* ReturnType = 0)
{
    virtual_reg* Result = 0;
    
    // NOTE: We just have a constant or variable
    if (Root->Id == TokenId_Var && Root->NumArgs == 0)
    {
        if (!IsTokenConstant(Root->Token) && Root->Token.Id == TokenId_Var)
        {
            Result = GetOrCreateRegisterForVar(&RegHashTable, Root->Token.Var);
        }

        if (IsTokenConstant(Root->Token))
        {
            Result = GetTempRegister(&RegHashTable);
            Result->Flags = RegFlag_IsConstant;
            Result->Type = Root->Token.Var->Type;
            Result->Const = Root->Token.Var->Str;
        }

        if (Root->Token.Id == ScannerId_Identifier)
        {
            // TODO: Check this and also, make registers handle struct members
            // NOTE: This case can happen when we are dealing with struct members
            //Root->Token.Flags = TokenFlag_IsLValue;
        }
        else if (IsTypePointer(GetTokenType(Root->Token)))
        {
            // TODO: Get registers to handle pointer types
            Result->Flags |= RegFlag_IsLValue;
        }

        // TODO: Other cases? Check these cases better
        
        return Result;
    }

    // TODO: We have lots of temporary variables being created. Figure out how to remove the
    // unnecessairy ones
    // NOTE: This variable is the returned value from all our expressions
    if (Root->Id != OpId_Define && Root->Id != OpId_Equal && Root->Id != OpId_Run &&
        Root->Id != ScannerId_Return)
    {
        Result = GetTempRegister(&RegHashTable);
    }
    
    // NOTE: The current node is a operator
    switch (Root->Id)
    {
        case OpId_Define:
        {
            Assert(Root->NumArgs == 1);
            
            Result = SyntaxTreeToIR(Root->Children[0], OutFile);
            Result->Flags |= RegFlag_IsLValue;
            PushDefineInstr(Result);
        } break;
        
        case OpId_Dereference:
        {
            // NOTE: The dereferences happening here are when we use the actual value of
            // a dereference. If we are assigning a val to a deref, thats handled in Equal
            Assert(Root->NumArgs == 1);
            virtual_reg* Ptr = SyntaxTreeToIR(Root->Children[0], OutFile);
            
            if (Ptr->Flags & RegFlag_IsConstant || !IsTypePointer(Ptr->Type))
            {
                OutputError("Error: Cannot dereference a non pointer type.\n");
            }
            if ((Ptr->Flags & RegFlag_IsLValue) == 0)
            {
                OutputError("Error: We can only dereference a l-value.\n");
            }

            Result->Type = ChangeRefCountToPtr(Ptr->Type, -1);            
            Result->Flags |= RegFlag_IsLValue;
            
            // TODO: just load from the address
            PushDefineInstr(Result);
            PushUniOpInstr(IR_LoadFromMem, Ptr, Result);
        } break;

        case OpId_AddressOff:
        {
            Assert(Root->NumArgs == 1);
            Result = SyntaxTreeToIR(Root->Children[0], OutFile);

            if (Result->Flags & RegFlag_IsConstant)
            {
                OutputError("Error: Cannot take address of a constant.\n");
            }
            if (Result->Flags & RegFlag_IsLValue)
            {
                OutputError("Error: We can only take the address of a l-value.\n");
            }
        
            Result->Type = ChangeRefCountToPtr(Result->Type, 1);
            
            // TODO: just load from the address
            // TODO: This is ambiguous as to what happens (equals or access the value?)
            //PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            //PushUniOpInstr(IR_Dereference, VarStr, ResultVarName.Var->Str);
        } break;

        case OpId_Negate:
        {
            Assert(Root->NumArgs == 1);
            virtual_reg* Arg = SyntaxTreeToIR(Root->Children[0], OutFile);
            
            if (!(Arg->Type == GetBasicType(TypeId_Int) ||
                  Arg->Type == GetBasicType(TypeId_Float)))
            {
                OutputError("Error: Can only negate a variable of type int or float.\n");
            }

            // NOTE: Get a 0 constant
            virtual_reg* RegFor0 = 0;
            if (Arg->Type == GetBasicType(TypeId_Int))
            {
                RegFor0 = RegFor0_Int;
            }
            else if (Arg->Type == GetBasicType(TypeId_Float))
            {
                RegFor0 = RegFor0_Flt;
            }
            
            Result->Type = Arg->Type;
            PushDefineInstr(Result);
            PushBinOpInstr(IR_Sub, RegFor0, Arg, Result);
        } break;
        
        case OpId_Run:
        {
            Assert(Root->NumArgs == 1);
            Result = SyntaxTreeToIR(Root->Children[0], OutFile);
            
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
            virtual_reg* Left = SyntaxTreeToIR(Root->Children[0], OutFile);
            virtual_reg* Right = SyntaxTreeToIR(Root->Children[1], OutFile);
            
            if (!(Left->Type == GetBasicType(TypeId_Int) &&
                  Right->Type == GetBasicType(TypeId_Int)))
            {
                OutputError("Error: No operator exists for given types.\n");
            }

            Result->Type = GetBasicType(TypeId_Int);
            if (Left->Flags & RegFlag_IsConstant && Right->Flags & RegFlag_IsConstant)
            {
                Result->Flags |= RegFlag_IsConstant;
            }

            PushDefineInstr(Result);
            PushBinOpInstr(IRCode, Left, Right, Result);
        } break;

        case OpId_Add:
        case OpId_Sub:
        case OpId_Mul:
        case OpId_Div:
        {
            // TODO: Make ptr types not defined for mul and div
            u32 IRCode = 0;
            if (Root->Id == OpId_Add) {IRCode = IR_Add;}
            if (Root->Id == OpId_Sub) {IRCode = IR_Sub;}
            if (Root->Id == OpId_Mul) {IRCode = IR_Mul;}
            if (Root->Id == OpId_Div) {IRCode = IR_Div;}

            Assert(Root->NumArgs == 2);
            virtual_reg* Left = SyntaxTreeToIR(Root->Children[0], OutFile);
            virtual_reg* Right = SyntaxTreeToIR(Root->Children[1], OutFile);

            if (Left->Type == GetBasicType(TypeId_Int) &&
                Right->Type == GetBasicType(TypeId_Int))
            {
                Result->Type = Left->Type;
            }
            else if (Left->Type == GetBasicType(TypeId_Int) &&
                     Right->Type == GetBasicType(TypeId_Float))
            {
                Result->Type = Right->Type;
            }
            else if (Left->Type == GetBasicType(TypeId_Float) &&
                     Right->Type == GetBasicType(TypeId_Int))
            {
                Result->Type = Left->Type;
            }
            else if (Left->Type == GetBasicType(TypeId_Float) &&
                     Right->Type == GetBasicType(TypeId_Float))
            {
                Result->Type = Left->Type;
            }
            else if (IsTypePointer(Left->Type) &&
                     Right->Type == GetBasicType(TypeId_Int))
            {
                Result->Flags |= RegFlag_IsLValue;
                Result->Type = Left->Type;
            }
            else if (IsTypePointer(Left->Type) && IsTypePointer(Right->Type) &&
                     Left->Type == Right->Type)
            {
                Result->Flags |= RegFlag_IsLValue;
                Result->Type = Left->Type;
            }
            else
            {
                OutputError("Error: No operator exists for given types.\n");
            }
            
            if (Left->Flags & RegFlag_IsConstant && Right->Flags & RegFlag_IsConstant)
            {
                Result->Flags |= RegFlag_IsConstant;
            }
      
            PushDefineInstr(Result);
            PushBinOpInstr(IRCode, Left, Right, Result);
        } break;

        case OpId_GetMember:
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* Left = SyntaxTreeToIR(Root->Children[0], OutFile);
            virtual_reg* Right = SyntaxTreeToIR(Root->Children[1], OutFile);

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

            // NOTE: If we have a dereference, we need to write to that location in memory
            virtual_reg* Left;
            virtual_reg* Right = SyntaxTreeToIR(Root->Children[1], OutFile);
            if (Root->Children[0]->Id == OpId_Dereference)
            {
                syntax_node* DerefNode = Root->Children[0];

                Assert(DerefNode->NumArgs == 1);
                Left = SyntaxTreeToIR(DerefNode->Children[0], OutFile);

                type* LeftType = ChangeRefCountToPtr(Left->Type, -1);
                if (!AreSameType(LeftType, Right->Type))
                {
                    // TODO: Do the proper type checking here
                    OutputError("Error: Type on the left of '=' must be the same type as on the right.\n");
                }
                PushAssignToMemInstr(Left, Right);
            }
            else
            {
                Left = SyntaxTreeToIR(Root->Children[0], OutFile);

                if (!AreSameType(Left->Type, Right->Type))
                {
                    OutputError("Error: Type on the left of '=' must be the same type as on the right.\n");
                }
                PushAssignInstr(Left, Right);
            }
            
            if (!(Left->Flags & RegFlag_IsLValue))
            {
                OutputError("Error: Left of '=' must be a l-value.\n");
            }
        } break;

        case OpId_GetArrayIndex:
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* Left = SyntaxTreeToIR(Root->Children[0], OutFile);
            virtual_reg* Right = SyntaxTreeToIR(Root->Children[1], OutFile);
            
            if (!IsTypePointer(Left->Type))
            {
                OutputError("Error: We can only index array types.\n");
            }
            if (Right->Type != GetBasicType(TypeId_Int))
            {
                OutputError("Error: Index of an array must be of type intger.\n");
            }

            Result->Type = ChangeRefCountToPtr(Left->Type, -1);
            Result->Flags |= RegFlag_IsLValue;

            PushDefineInstr(Result);
            //PushGetArrayIndexInstr(Left, Right, Result);
        } break;
        
        case OpId_FuncCall:
        {
#if 0
            // TODO: Get rid of this allocation (make it happen on a temp arena)
            token* ArgTokens = (token*)malloc(sizeof(token)*Root->NumArgs);
            for (u32 ArgIndex = 0; ArgIndex < Root->NumArgs; ++ArgIndex)
            {
                type* RequiredArgType = Root->Func->ArgTypes[ArgIndex];
                token ArgToken = SyntaxTreeToIR(Root->Children[ArgIndex], OutFile);
                type* ArgTokenType = GetTokenType(ArgToken);
                ArgTokens[ArgIndex] = ArgToken;
                
                if (!AreSameType(RequiredArgType, ArgTokenType))
                {
                    OutputError("Error: Type doesn't match the required type for the function arg.\n");
                }
            }

            ResultVarName.Var->Type = Root->Func->ReturnType;
            PushFuncCallInstr(ResultVarName.Var->Str, Root->Func->Name, Root->NumArgs, ArgTokens);

            free(ArgTokens);
#endif            
        } break;

        case ScannerId_Return:
        {
#if 0
            if (Root->NumArgs == 0)
            {                
                if (ReturnType != GetBasicType(TypeId_Void))
                {
                    OutputError("Error: Return value isn't of the same type as functions return type.\n");
                }
                PushReturnInstr();
            }
            else if (Root->NumArgs == 1)
            {
                token Token = SyntaxTreeToIR(Root->Children[0], OutFile);
                if (!AreSameType(ReturnType, GetTokenType(Token)))
                {
                    OutputError("Error: Return value isn't of the same type as functions return type.\n");
                }
                PushReturnInstr(GetTokenString(Token));
            }
            else
            {
                InvalidCodePath;
            }
#endif
        } break;
        
        default:
        {
            InvalidCodePath;
        } break;
    }
        
    return Result;
}
