
internal virtual_reg* RecursiveTypeCheckSyntaxTree(parser_state* State, syntax_node* Root)
{
    virtual_reg* Result = 0;
    
    if (Root->Id == NodeId_Token)
    {
        // NOTE: We just have a constant or variable
        Assert(Root->NumArgs == 0);
        if (IsTokenConstant(*Root->Token))
        {
            Result = GetOrCreateRegForToken(Root->Token);
            Result->Name = GetTokenString(*Root->Token);
        }
        else
        {
            string Str = GetTokenString(*Root->Token);
            Result = GetOrCreateRegForToken(Root->Token);
            if (State->Scope->Id == ScopeId_Global)
            {
                Result->Name = Str;
            }
        }
        
        if (Root->Token->Id == ScannerId_Identifier)
        {
            InvalidCodePath;
            // TODO: Check this and also, make registers handle struct members
            // NOTE: This case can happen when we are dealing with struct members
            //Root->Token.Flags = TokenFlag_IsLValue;
        }
        else if (Root->Token->Flags & TokenFlag_IsArg)
        {
            Result->Flags |= RegFlag_IsArg;
        }
        else if (IsTypePointer(GetTokenType(*Root->Token)))
        {
            Result->Flags |= RegFlag_IsLValue;
        }
        
        Root->Reg = Result;
        Root->Id = NodeId_Reg;

        return Result;
    }
    else if (Root->Id == NodeId_Reg)
    {
        return Root->Reg;
    }

    // TODO: This setup doesn't make sense anymore, put everything in the case statement and have
    // each case create a temp register as required
    // TODO: We have lots of temporary variables being created. Create accumulators to optimize
    // NOTE: This variable is the returned value from all our expressions
    if (Root->Id != NodeId_Define && Root->Id != NodeId_Zero &&
        Root->Id != NodeId_Equal && Root->Id != NodeId_Run &&
        Root->Id != ScannerId_Return && Root->Id != NodeId_If &&
        Root->Id != NodeId_Else && Root->Id != NodeId_While &&
        Root->Id != NodeId_FuncDef && Root->Id != NodeId_FuncCall)
    {
        Result = GetTempRegister();
        Root->Reg = Result;
    }
    
    // NOTE: The current node is a operator
    switch (Root->Id)
    {
        case NodeId_Define:
        {
            Assert(Root->NumArgs == 1);
            
            Result = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
            Result->Flags |= RegFlag_IsLValue;
        } break;

        case NodeId_Zero:
        {
            Assert(Root->NumArgs == 1);
            Result = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
        } break;
        
        case NodeId_Dereference:
        {
            // NOTE: The dereferences happening here are when we use the actual value of
            // a dereference. If we are assigning a val to a deref, thats handled in Equal
            Assert(Root->NumArgs == 1);
            virtual_reg* Ptr = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
            
            if (Ptr->Flags & RegFlag_StoresConstant || !IsTypePointer(Ptr->Type))
            {
                OutputError("Error: Cannot dereference a non pointer type.\n");
            }
            if ((Ptr->Flags & RegFlag_IsLValue) == 0)
            {
                OutputError("Error: We can only dereference a l-value.\n");
            }

            Result->Type = ChangeRefCountToPtr(Ptr->Type, -1);
            if (IsTypePointer(Result->Type))
            {
                Result->Flags |= RegFlag_IsLValue;
            }
        } break;

        case NodeId_AddressOff:
        {
            Assert(Root->NumArgs == 1);
            virtual_reg* Val = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);

            if (Val->Flags & RegFlag_StoresConstant)
            {
                OutputError("Error: Cannot take address of a constant.\n");
            }
            Result->Type = ChangeRefCountToPtr(Val->Type, 1);
        } break;

        case NodeId_Negate:
        {
            Assert(Root->NumArgs == 1);
            virtual_reg* Arg = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
            
            if (!(Arg->Type == GetBasicType(TypeId_Int) ||
                  Arg->Type == GetBasicType(TypeId_Float)))
            {
                OutputError("Error: Can only negate a variable of type int or float.\n");
            }
            Result->Type = Arg->Type;
        } break;
        
        case NodeId_Run:
        {
            Assert(Root->NumArgs == 1);
            Result = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);

            run_node_list* Node = PushStruct(&TempArena, run_node_list);
            Node->Node = Root;
            if (!State->StartList)
            {
                State->StartList = Node;
                State->EndList = Node;
            }
            else
            {
                State->EndList->Next = Node;
                State->EndList = Node;
            }
            
            // TODO: Allocate all nodes in this runs children on the temp arena to clear later?
            //Result.Flags |= RegFlag_IsConstant;
        } break;
        
        case NodeId_BinaryEqual:
        case NodeId_BinaryNotEqual:
        case NodeId_Less:
        case NodeId_LessEqual:
        case NodeId_Greater:
        case NodeId_GreaterEqual:            
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* Left = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
            virtual_reg* Right = RecursiveTypeCheckSyntaxTree(State, Root->Children[1]);
            
            if (!(Left->Type == GetBasicType(TypeId_Int) &&
                  Right->Type == GetBasicType(TypeId_Int)))
            {
                OutputError("Error: No operator exists for given types.\n");
            }
            
            Result->Type = GetBasicType(TypeId_Int);
            if (Left->Flags & RegFlag_StoresConstant &&
                Right->Flags & RegFlag_StoresConstant)
            {
                Result->Flags |= RegFlag_StoresConstant;
            }
        } break;

        case NodeId_Add:
        case NodeId_Sub:
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* Left = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
            virtual_reg* Right = RecursiveTypeCheckSyntaxTree(State, Root->Children[1]);
            
            if (Left->Type == GetBasicType(TypeId_Int) &&
                Right->Type == GetBasicType(TypeId_Int))
            {
                Result->Type = GetBasicType(TypeId_Int);
            }
            else if (Left->Type == GetBasicType(TypeId_Float) &&
                     Right->Type == GetBasicType(TypeId_Int))
            {
                Result->Type = GetBasicType(TypeId_Float);
            }
            else if (Right->Type == GetBasicType(TypeId_Float) &&
                     Left->Type == GetBasicType(TypeId_Int))
            {
                Result->Type = GetBasicType(TypeId_Float);
            }
            else if (Left->Type == GetBasicType(TypeId_Float) &&
                     Right->Type == GetBasicType(TypeId_Float))
            {
                Result->Type = GetBasicType(TypeId_Float);
            }
            else if (IsTypePointer(Left->Type) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                Result->Flags |= RegFlag_IsLValue;
                Result->Type = Left->Type;
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     IsTypePointer(Right->Type))
            {
                Result->Flags |= RegFlag_IsLValue;
                Result->Type = Right->Type;
            }
            else
            {
                OutputError("Error: Unsupported type in addition/subtraction statement.\n");
            }
            
            if (Left->Flags & RegFlag_StoresConstant &&
                Right->Flags & RegFlag_StoresConstant)
            {
                Result->Flags |= RegFlag_StoresConstant;
            }          
        } break;
        
        case NodeId_Mul:
        case NodeId_Div:
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* Left = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
            virtual_reg* Right = RecursiveTypeCheckSyntaxTree(State, Root->Children[1]);
            
            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                Result->Type = GetBasicType(TypeId_Int);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                Result->Type = GetBasicType(TypeId_Float);
            }
            else if (AreSameType(Right->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Left->Type, GetBasicType(TypeId_Int)))
            {
                Result->Type = GetBasicType(TypeId_Float);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                Result->Type = GetBasicType(TypeId_Float);
            }
            else
            {
                OutputError("Error: Unsupported type in addition statement.\n");
            }
                        
            if (Left->Flags & RegFlag_StoresConstant &&
                Right->Flags & RegFlag_StoresConstant)
            {
                Result->Flags |= RegFlag_StoresConstant;
            }
        } break;

        case NodeId_GetMember:
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* Left = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
            token* RightToken = Root->Children[1]->Token; 

            #if 0
            if (IsTypePointer(Left->Type))
            {
                Result->Type = Left->Type->PointerType;
            }            
            if (!IsTypeStruct(Left->Type))
            {
                OutputError("Error: Cannot access member from a non struct type.\n");
            }
            if (Root->Children[1]->Id != NodeId_Var)
            {
                OutputError("Error: Struct member access not followed by a variable.\n");
            }
            if (!(RightToken->Flags & TokenFlag_IsMember))
            {
                OutputError("Error: Struct member access not followed by valid member.\n");
            }
     
            // NOTE: We get the type of our member and make sure it exists
            type* ArgType = 0;
            u8* CurrByte = Left->Type->Members;
            for (u32 MemberIndex = 0; MemberIndex < Left->Type->NumMembers; ++MemberIndex)
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
#endif
            
            //ResultVarName.Var->Type = ArgType;
            //ResultVarName.Flags = TokenFlag_IsLValue;
            
            //string LeftStr = GetTokenString(Left);
            //string RightStr = GetTokenString(Right);        
            //PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            //PushBinOpInstr(IR_GetMember, Left.Var, Right.Var, ResultVarName.Var);
        } break;
        
        case NodeId_Equal:
        {            
            Assert(Root->NumArgs == 2);
                        
            virtual_reg* Left = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
            virtual_reg* Right = RecursiveTypeCheckSyntaxTree(State, Root->Children[1]);

            if (!AreSameType(Left->Type, Right->Type))
            {
                OutputError("Error: Type on the left of '=' must be the same type as on the right.\n");
            }            
            if (!(Left->Flags & RegFlag_IsLValue) && IsTypePointer(Left->Type))
            {
                OutputError("Error: Left of '=' must be a l-value.\n");
            }

            // NOTE: If our left node is a define and we are in global scope,
            // we want to precompute our global computation
            if (State->Scope->Id == ScopeId_Global && Root->Children[0]->Id == NodeId_Define)
            {
                run_node_list* Node = PushStruct(&TempArena, run_node_list);
                Node->Node = Root->Children[1];
                if (!State->StartList)
                {
                    State->StartList = Node;
                    State->EndList = Node;
                }
                else
                {
                    State->EndList->Next = Node;
                    State->EndList = Node;
                }
            }
        } break;

        case NodeId_GetArrayIndex:
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* Left = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
            virtual_reg* Right = RecursiveTypeCheckSyntaxTree(State, Root->Children[1]);
            
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
        } break;

        case NodeId_If:
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* BoolExpr = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);                
            if (BoolExpr->Type != GetBasicType(TypeId_Int))
            {
                OutputError("Error: Expression inside brackets must evaluate to an int.\n");
            }
        } break;

        case NodeId_ElseIf:
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* BoolExpr = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);                
            if (BoolExpr->Type != GetBasicType(TypeId_Int))
            {
                OutputError("Error: Expression inside brackets must evaluate to an int.\n");
            }
        } break;

        case NodeId_Else:
        {
            Assert(Root->NumArgs == 1);
        } break;

        case NodeId_While:
        {
            Assert(Root->NumArgs == 2);
            virtual_reg* BoolExpr = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);                
            if (BoolExpr->Type != GetBasicType(TypeId_Int))
            {
                OutputError("Error: Expression inside brackets must evaluate to an int.\n");
            }
        } break;

        case NodeId_FuncDef:
        {
            Assert(Root->NumArgs == 1);
        } break;
        
        case NodeId_FuncCall:
        {
            Assert(Root->NumArgs == Root->Func->NumArgs);

            Result = GetTempRegister();
            for (u32 ArgIndex = 0; ArgIndex < Root->NumArgs; ++ArgIndex)
            {
                type* RequiredArgType = Root->Func->ArgTypes[ArgIndex];
                virtual_reg* ArgReg = RecursiveTypeCheckSyntaxTree(State, Root->Children[ArgIndex]);
                
                if (!AreSameType(RequiredArgType, ArgReg->Type))
                {
                    OutputError("Error: Type doesn't match the required type for the function arg.\n");
                }
            }

            // TODO: If no return value, make result empty
            Result->Type = Root->Func->ReturnType;
            Root->Reg = Result;
        } break;

        case NodeId_Return:
        {
            if (Root->NumArgs == 0)
            {
            }
            else if (Root->NumArgs == 1)
            {
                Result = RecursiveTypeCheckSyntaxTree(State, Root->Children[0]);
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

internal void TypeCheckSyntaxTree(parser_state* State, syntax_node* Root)
{
    scope* Scope = State->Scope;

    if (State->Scope->ExitCodeBlock)
    {
        *State->Scope->ExitCodeBlock = Root;
        State->Scope->ExitCodeBlock = 0;
        // NOTE: Lets the parser know now to link the next node with prev 
        State->PrevNode = 0;
    }
    else if (Scope->Id == ScopeId_If || Scope->Id == ScopeId_ElseIf ||
             Scope->Id == ScopeId_Else || Scope->Id == ScopeId_WhileLoop ||
             Scope->Id == ScopeId_Function)
    {
        if (!(*Scope->StartCodeBlock))
        {
            // NOTE: We haven't set the start code block so we need to set that
            *Scope->StartCodeBlock = Root;
            // NOTE: Lets the parser know now to link the next node with prev 
            State->PrevNode = 0;
        }
    }
    
    if (State->PrevNode)
    {
        // NOTE: Link up our syntax nodes
        State->PrevNode->Next = Root;
    }
    State->PrevNode = Root;
    
    RecursiveTypeCheckSyntaxTree(State, Root);
}
