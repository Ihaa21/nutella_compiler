/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

// TODO: Move this 
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

struct accum_val
{
    virtual_reg* Reg;
    type* Type;
};

inline void GetRegisterFromToken(token Token, accum_val* Accum)
{
    // NOTE: We set our accum register to the current token we are examining
    if (!IsTokenConstant(Token) && Token.Id == TokenId_Var)
    {
        Accum->Reg = GetOrCreateRegisterForVar(&RegHashTable, Token.Var);
    }
    else if (IsTokenConstant(Token))
    {
        Accum->Reg = GetTempRegister(&RegHashTable);
        Accum->Reg->Flags = RegFlag_StoresConstant | RegFlag_IsConstVal;
        Accum->Reg->Name = Token.Var->Str;
    }
    else
    {
        InvalidCodePath;
    }
    
    if (Token.Id == ScannerId_Identifier)
    {
        // TODO: Check this and also, make registers handle struct members
        // NOTE: This case can happen when we are dealing with struct members
        //Root->Token.Flags = TokenFlag_IsLValue;
    }
    else if (IsTypePointer(GetTokenType(Token)))
    {
        Accum->Reg->Flags |= RegFlag_IsLValue;
    }

    // TODO: Other cases? Check these cases better
}

#if 1
inline void AccumRegisterFromToken(token Token, accum_val* Accum)
{
    // NOTE: We set our accum register to the current token we are examining
    virtual_reg* Reg = 0;
    if (!IsTokenConstant(Token) && Token.Id == TokenId_Var)
    {
        Reg = GetOrCreateRegisterForVar(&RegHashTable, Token.Var);
    }    
    else if (IsTokenConstant(Token))
    {
        Reg = GetTempRegister(&RegHashTable);
        Reg->Flags = RegFlag_StoresConstant | RegFlag_IsConstVal;
        Reg->Name = Token.Var->Str;
    }
    else
    {
        InvalidCodePath;
    }

    Accum->Reg->Flags = Reg->Flags & (!RegFlag_IsConstVal);
    PushAssignInstr(Accum->Reg, Reg);
    
    if (Token.Id == ScannerId_Identifier)
    {
        // TODO: Check this and also, make registers handle struct members
        // NOTE: This case can happen when we are dealing with struct members
        //Root->Token.Flags = TokenFlag_IsLValue;
    }
    else if (IsTypePointer(GetTokenType(Token)))
    {
        Accum->Reg->Flags |= RegFlag_IsLValue;
    }

    // TODO: Other cases? Check these cases better
}
#endif

internal void SyntaxTreeToIR(parser_state* State, syntax_node* Root, accum_val* Accum = 0,
                             type* ReturnType = 0)
{
    if (!Accum)
    {
        // NOTE: Accum wasn't specified so we reserve some space on the stack for it
        accum_val _Accum = {};
        Accum = &_Accum;
        Accum->Reg = GetTempRegister(&RegHashTable);
    }
    
    // NOTE: The current node is a operator
    switch (Root->Id)
    {
        case TokenId_Var:
        {
            // NOTE: We have a constant or var
            Assert(Root->NumArgs == 0);
            AccumRegisterFromToken(Root->Token, Accum);            
        } break;
        
        case OpId_Define:
        {
            Assert(Root->NumArgs == 1);
            
            SyntaxTreeToIR(State, Root->Children[0], Accum);
            Accum->Reg->Flags |= RegFlag_IsLValue;
            Accum->Reg->MemoryIndex = State->CurrNumLocals++;
            PushDefineInstr(Accum->Reg);

            // TODO: For structs, break them up into members and set them all to 0
            // TODO: Only do this if we don't assign a value
            if (IsTypePointer(Accum->Type) || AreSameType(Accum->Type, GetBasicType(TypeId_Int)))
            {
                PushAssignInstr(Accum->Reg, RegFor0_Int);
            }
            else if (AreSameType(Accum->Type, GetBasicType(TypeId_Float)))
            {
                PushAssignInstr(Accum->Reg, RegFor0_Flt);
            }
        } break;
        
        case OpId_Dereference:
        {
            // NOTE: The dereferences happening here are when we use the actual value of
            // a dereference. If we are assigning a val to a deref, thats handled in Equal
            Assert(Root->NumArgs == 1);
            SyntaxTreeToIR(State, Root->Children[0], Accum);

            if (Accum->Reg->Flags & RegFlag_StoresConstant || !IsTypePointer(Accum->Type))
            {
                OutputError("Error: Cannot dereference a non pointer type.\n");
            }
            if ((Accum->Reg->Flags & RegFlag_IsLValue) == 0)
            {
                OutputError("Error: We can only dereference a l-value.\n");
            }

            Accum->Type = ChangeRefCountToPtr(Accum->Type, -1);
            if (IsTypePointer(Accum->Type))
            {
                Accum->Reg->Flags |= RegFlag_IsLValue;
            }
            
            PushUniOpInstr(IR_LoadFromMem, Accum->Reg, Accum->Reg);
        } break;

        case OpId_AddressOff:
        {
            Assert(Root->NumArgs == 1);
            SyntaxTreeToIR(State, Root->Children[0], Accum);

            if (Accum->Reg->Flags & RegFlag_StoresConstant)
            {
                OutputError("Error: Cannot take address of a constant.\n");
            }
                    
            Accum->Type = ChangeRefCountToPtr(Accum->Type, 1);
            PushUniOpInstr(IR_AddressOff, Accum->Reg, Accum->Reg);
        } break;

        case OpId_Negate:
        {
            Assert(Root->NumArgs == 1);
            SyntaxTreeToIR(State, Root->Children[0], Accum);

            // NOTE: Get a 0 constant
            virtual_reg* RegFor0 = 0;
            if (Accum->Type == GetBasicType(TypeId_Int))
            {
                RegFor0 = RegFor0_Int;
            }
            else if (Accum->Type == GetBasicType(TypeId_Float))
            {
                RegFor0 = RegFor0_Flt;
            }
            else
            {
                OutputError("Error: Can only negate a variable of type int or float.\n");
            }
            
            PushBinOpInstr(IR_Sub, RegFor0, Accum->Reg, Accum->Reg);
        } break;
        
        case OpId_Run:
        {
            Assert(Root->NumArgs == 1);
            SyntaxTreeToIR(State, Root->Children[0], Accum);
            
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
            SyntaxTreeToIR(State, Root->Children[0], Accum);

            accum_val Right = {};
            if (Root->Children[1]->Id == TokenId_Var)
            {
                // NOTE: We have a const or var on our right so we just add it directly
                GetRegisterFromToken(Root->Children[1]->Token, &Right);
            }
            else
            {
                // NOTE: We have a more complex expression on our right so we need a temp var
                Right.Reg = GetTempRegister(&RegHashTable);
                SyntaxTreeToIR(State, Root->Children[1], &Right);
            }
            
            if (!(Accum->Type == GetBasicType(TypeId_Int) &&
                  Right.Type == GetBasicType(TypeId_Int)))
            {
                OutputError("Error: No operator exists for given types.\n");
            }

            Accum->Type = GetBasicType(TypeId_Int);
            if (Accum->Reg->Flags & RegFlag_StoresConstant &&
                Right.Reg->Flags & RegFlag_StoresConstant)
            {
                Accum->Reg->Flags |= RegFlag_StoresConstant;
            }

            // TODO: Do we have to keep the old type for accum here?
            PushBinOpInstr(IRCode, Accum->Reg, Right.Reg, Accum->Reg);
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
            SyntaxTreeToIR(State, Root->Children[0], Accum);

            accum_val Right = {};
            if (Root->Children[1]->Id == TokenId_Var)
            {
                // NOTE: We have a const or var on our right so we just add it directly
                GetRegisterFromToken(Root->Children[1]->Token, &Right);
            }
            else
            {
                // NOTE: We have a more complex expression on our right so we need a temp var
                Right.Reg = GetTempRegister(&RegHashTable);
                SyntaxTreeToIR(State, Root->Children[1], &Right);
            }

            if (Accum->Type == GetBasicType(TypeId_Int) &&
                Right.Type == GetBasicType(TypeId_Int))
            {
                Accum->Type = Accum->Type;
            }
            else if (Accum->Type == GetBasicType(TypeId_Int) &&
                     Right.Type == GetBasicType(TypeId_Float))
            {
                Accum->Type = Right.Type;
            }
            else if (Accum->Type == GetBasicType(TypeId_Float) &&
                     Right.Type == GetBasicType(TypeId_Int))
            {
                Accum->Type = Accum->Type;
            }
            else if (Accum->Type == GetBasicType(TypeId_Float) &&
                     Right.Type == GetBasicType(TypeId_Float))
            {
                Accum->Type = Accum->Type;
            }
            else if (IsTypePointer(Accum->Type) &&
                     Right.Type == GetBasicType(TypeId_Int))
            {
                Accum->Reg->Flags |= RegFlag_IsLValue;
                Accum->Type = Accum->Type;
            }
            else if (IsTypePointer(Accum->Type) && IsTypePointer(Right.Type) &&
                     Accum->Type == Right.Type)
            {
                Accum->Reg->Flags |= RegFlag_IsLValue;
                Accum->Type = Accum->Type;
            }
            else
            {
                OutputError("Error: No operator exists for given types.\n");
            }
            
            if (Accum->Reg->Flags & RegFlag_StoresConstant && Right.Reg->Flags & RegFlag_StoresConstant)
            {
                Accum->Reg->Flags |= RegFlag_StoresConstant;
            }
      
            PushBinOpInstr(IRCode, Accum->Reg, Right.Reg, Accum->Reg);
        } break;

        case OpId_GetMember:
        {
            Assert(Root->NumArgs == 2);
            //virtual_reg* Left = SyntaxTreeToIR(State, Root->Children[0]);
            //virtual_reg* Right = SyntaxTreeToIR(State, Root->Children[1]);
            
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

            // TODO: Add a address off here?
            // NOTE: If we have a dereference, we need to write to that location in memory
#if 0
            virtual_reg* Left;
            virtual_reg* Right = SyntaxTreeToIR(State, Root->Children[1]);
            if (Root->Children[0]->Id == OpId_Dereference)
            {
                syntax_node* DerefNode = Root->Children[0];

                Assert(DerefNode->NumArgs == 1);
                Left = SyntaxTreeToIR(State, DerefNode->Children[0]);

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
                Left = SyntaxTreeToIR(State, Root->Children[0]);

                if (!AreSameType(Left->Type, Right->Type))
                {
                    OutputError("Error: Type on the left of '=' must be the same type as on the right.\n");
                }
                PushAssignInstr(Left, Right);
            }
#endif

            SyntaxTreeToIR(State, Root->Children[0], Accum);
            type* LeftType = Accum->Type;
            if (!(Accum->Reg->Flags & RegFlag_IsLValue))
            {
                OutputError("Error: Left of '=' must be a l-value.\n");
            }
            SyntaxTreeToIR(State, Root->Children[1], Accum);
            if (!AreSameType(LeftType, Accum->Type))
            {
                OutputError("Error: Type on the left of '=' must be the same type as on the right.\n");
            }
            //PushAssignInstr(Left, Right);
            
        } break;

        case OpId_GetArrayIndex:
        {
            Assert(Root->NumArgs == 2);
#if 0
            virtual_reg* Left = SyntaxTreeToIR(State, Root->Children[0]);
            virtual_reg* Right = SyntaxTreeToIR(State, Root->Children[1]);
            
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
#endif
        } break;
        
        case OpId_FuncCall:
        {
            // TODO: Get rid of this allocation (make it happen on a instr arena by building)
            virtual_reg** ArgRegs = (virtual_reg**)malloc(sizeof(virtual_reg*)*Root->NumArgs);
            for (u32 ArgIndex = 0; ArgIndex < Root->NumArgs; ++ArgIndex)
            {
                type* RequiredArgType = Root->Func->ArgTypes[ArgIndex];

                accum_val ArgAccum = {};
                SyntaxTreeToIR(State, Root->Children[ArgIndex], &ArgAccum);

                ArgRegs[ArgIndex] = ArgAccum.Reg;
                
                if (!AreSameType(RequiredArgType, ArgAccum.Type))
                {
                    OutputError("Error: Type doesn't match the required type for the function arg.\n");
                }
            }

            // TODO: Do we need a lvalue since being a pointer is basically the requirment for a lval
            Accum->Type = Root->Func->ReturnType;
            PushFuncCallInstr(Accum->Reg, Root->Func->Name, Root->NumArgs, ArgRegs);

            free(ArgRegs);
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
                token Token = SyntaxTreeToIR(State, Root->Children[0]);
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
}
