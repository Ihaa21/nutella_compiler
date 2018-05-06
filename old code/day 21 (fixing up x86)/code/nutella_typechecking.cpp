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

internal token SyntaxTreeToIR(syntax_node* Root, FILE* OutCFile, type* ReturnType = 0)
{
    // NOTE: We just have a constant or variable
    if (Root->Id == TokenId_Var && Root->NumArgs == 0)
    {
        if (Root->Token.Id == ScannerId_Identifier)
        {
            // NOTE: This case can happen when we are dealing with struct members
            Root->Token.Flags = TokenFlag_IsLValue;
        }
        else if (IsTypePointer(GetTokenType(Root->Token)))
        {
            Root->Token.Flags = TokenFlag_IsLValue;
        }

        if (!IsTokenConstant(Root->Token) && Root->Token.Id == TokenId_Var)
        {
            Root->Token.Var->Str = GetRegisterNameForVar(&RegHashTable, Root->Token.Str);
        }
        
        return Root->Token;
    }

    // TODO: We have lots of temporary variables being created. Figure out how to remove the
    // unnecessairy ones
    // NOTE: This variable is the returned value from all our expressions
    token ResultVarName = {};
    if (Root->Id != OpId_Define && Root->Id != OpId_Equal && Root->Id != OpId_Run &&
        Root->Id != ScannerId_Return)
    {
        ResultVarName.Id = TokenId_Var;
        ResultVarName.Var = PushStruct(&ScopeArena, var);
        ResultVarName.Var->Str = GetTempRegisterName(&RegHashTable);
    }
    
    // NOTE: The current node is a operator
    switch (Root->Id)
    {
        case OpId_Define:
        {
            Assert(Root->NumArgs == 1);
            
            token Var = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeVar = GetTokenType(Var);
            string VarStr = GetTokenString(Var);

            ResultVarName = Var;
            
            PushDefineInstr(TypeVar, VarStr);
        } break;
        
        case OpId_Dereference:
        {
            Assert(Root->NumArgs == 1);
            
            token Var = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeVar = GetTokenType(Var);
            string VarStr = GetTokenString(Var);
            
            if (IsTokenConstant(Var) || !IsTypePointer(TypeVar))
            {
                OutputError("Error: Cannot dereference a non pointer type.\n");
            }
            if (Var.Flags != TokenFlag_IsLValue)
            {
                OutputError("Error: We can only dereference a l-value.\n");
            }

            type* NewType = ChangeRefCountToPtr(TypeVar, -1);            
            ResultVarName.Flags = TokenFlag_IsLValue;
            ResultVarName.Var->Type = NewType;

            // TODO: just load from the address
            // TODO: This is ambiguous as to what happens (equals or access the value?)
            //PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            //PushUniOpInstr(IR_Dereference, VarStr, ResultVarName.Var->Str);
        } break;

        case OpId_AddressOff:
        {
            Assert(Root->NumArgs == 1);
            
            token Var = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeVar = GetTokenType(Var);
            string VarStr = GetTokenString(Var);

            if (IsTokenConstant(Var))
            {
                OutputError("Error: Cannot take address of a constant.\n");
            }
            if (Var.Flags != TokenFlag_IsLValue)
            {
                OutputError("Error: We can only take the address of a l-value.\n");
            }
        
            type* NewType = ChangeRefCountToPtr(TypeVar, 1);            
            ResultVarName.Flags = TokenFlag_IsLValue;
            ResultVarName.Var->Type = NewType;

            // TODO: just load from the address
            // TODO: This is ambiguous as to what happens (equals or access the value?)
            //PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            //PushUniOpInstr(IR_Dereference, VarStr, ResultVarName.Var->Str);
        } break;

        case OpId_Negate:
        {
            Assert(Root->NumArgs == 1);
            
            token Var = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeVar = GetTokenType(Var);
            string VarStr = GetTokenString(Var);

            if (!(TypeVar == GetBasicType(TypeId_Int) || TypeVar == GetBasicType(TypeId_Float)))
            {
                OutputError("Error: Can only negate a variable of type int or float.\n");
            }

            char CharFor0 = '0';
            var VarFor0 = {};
            VarFor0.Str.NumChars = 1;
            VarFor0.Str.Text = &CharFor0;
            VarFor0.Type = TypeVar;
            
            ResultVarName.Var->Type = TypeVar;
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushBinOpInstr(IR_Sub, &VarFor0, Var.Var, ResultVarName.Var);
        } break;
        
        case OpId_Run:
        {
            Assert(Root->NumArgs == 1);

            // TODO: Currently this doesnt work properly
            // TODO: We need to properly implement the compile time execution here
            token Var = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeVar = GetTokenType(Var);

            ResultVarName.Flags = TokenFlag_IsConstant;
            ResultVarName.Var->Type = TypeVar;
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
            
            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);
            
            Assert(Root->NumArgs == 2);
            Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
            Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));

            if (!(GetTokenType(Left) == GetBasicType(TypeId_Int) &&
                  GetTokenType(Right) == GetBasicType(TypeId_Int)))
            {
                OutputError("Error: No operator exists for given types.\n");
            }

            ResultVarName.Var->Type = GetBasicType(TypeId_Int);
            if (IsTokenConstant(Left) && IsTokenConstant(Right))
            {
                ResultVarName.Flags = TokenFlag_IsConstant;
            }
        
            string LeftStr = GetTokenString(Left);
            string RightStr = GetTokenString(Right);        
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushBinOpInstr(IRCode, Left.Var, Right.Var, ResultVarName.Var);
        } break;

        case OpId_Add:
        case OpId_Sub:
        case OpId_Mul:
        case OpId_Div:
        {
            u32 IRCode = 0;
            if (Root->Id == OpId_Add) {IRCode = IR_Add;}
            if (Root->Id == OpId_Sub) {IRCode = IR_Sub;}
            if (Root->Id == OpId_Mul) {IRCode = IR_Mul;}
            if (Root->Id == OpId_Div) {IRCode = IR_Div;}

            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeLeft = GetTokenType(Left);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);
            type* TypeRight = GetTokenType(Right);
            
            Assert(Root->NumArgs == 2);
            Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
            Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));

            if (TypeLeft == GetBasicType(TypeId_Int) &&
                TypeRight == GetBasicType(TypeId_Int))
            {
                ResultVarName.Var->Type = TypeLeft;
            }
            else if (TypeLeft == GetBasicType(TypeId_Int) &&
                     TypeRight == GetBasicType(TypeId_Float))
            {
                ResultVarName.Var->Type = TypeRight;
            }
            else if (TypeLeft == GetBasicType(TypeId_Float) &&
                     TypeRight == GetBasicType(TypeId_Int))
            {
                ResultVarName.Var->Type = TypeLeft;
            }
            else if (TypeLeft == GetBasicType(TypeId_Float) &&
                     TypeRight == GetBasicType(TypeId_Float))
            {
                ResultVarName.Var->Type = TypeLeft;
            }
            else if (IsTypePointer(TypeLeft) &&
                     TypeRight == GetBasicType(TypeId_Int))
            {
                ResultVarName.Flags = TokenFlag_IsLValue;
                ResultVarName.Var->Type = TypeLeft;
            }
            else if (IsTypePointer(TypeLeft) && IsTypePointer(TypeRight) &&
                     TypeLeft == TypeRight)
            {
                ResultVarName.Flags = TokenFlag_IsLValue;
                ResultVarName.Var->Type = TypeLeft;
            }
            else
            {
                OutputError("Error: No operator exists for given types.\n");
            }
            
            if (IsTokenConstant(Left) && IsTokenConstant(Right))
            {
                ResultVarName.Flags = TokenFlag_IsConstant;
            }

            string LeftStr = GetTokenString(Left);
            string RightStr = GetTokenString(Right);        
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushBinOpInstr(IRCode, Left.Var, Right.Var, ResultVarName.Var);
        } break;

        case OpId_GetMember:
        {
            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);

            Assert(Root->NumArgs == 2);
            
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
        } break;
        
        case OpId_Equal:
        {            
            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);

            Assert(Root->NumArgs == 2);
            Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
            Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));

            if (!IsTokenLValue(Left))
            {
                OutputError("Error: Left of '=' must be a l-value.\n");
            }
            if (!AreSameType(GetTokenType(Left), GetTokenType(Right)))
            {
                OutputError("Error: Type on the left of '=' must be the same type as on the right.\n");
            }

            PushAssignInstr(GetTokenString(Left), GetTokenString(Right));
        } break;

        case OpId_GetArrayIndex:
        {
            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);

            Assert(Root->NumArgs == 2);
            Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
            Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));

            if (!IsTypePointer(GetTokenType(Left)))
            {
                OutputError("Error: We can only index array types.\n");
            }
            if (GetTokenType(Right) != GetBasicType(TypeId_Int))
            {
                OutputError("Error: Index of an array must be of type intger.\n");
            }

            ResultVarName.Var->Type = ChangeRefCountToPtr(GetTokenType(Left), -1);
            ResultVarName.Flags = TokenFlag_IsLValue;

            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushGetArrayIndexInstr(GetTokenString(Left), GetTokenString(Right), ResultVarName.Var->Str);
        } break;
        
        case OpId_FuncCall:
        {
            // TODO: Get rid of this allocation (make it happen on a temp arena)
            token* ArgTokens = (token*)malloc(sizeof(token)*Root->NumArgs);
            for (u32 ArgIndex = 0; ArgIndex < Root->NumArgs; ++ArgIndex)
            {
                type* RequiredArgType = Root->Func->ArgTypes[ArgIndex];
                token ArgToken = SyntaxTreeToIR(Root->Children[ArgIndex], OutCFile);
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
        } break;

        case ScannerId_Return:
        {
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
                token Token = SyntaxTreeToIR(Root->Children[0], OutCFile);
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
            
            break;
        }
        
        default:
        {
            InvalidCodePath;
        } break;
    }
        
    return ResultVarName;
}
