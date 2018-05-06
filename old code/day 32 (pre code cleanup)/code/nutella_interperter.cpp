/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

internal i32* GetInt(var* Reg)
{
    i32* Result = (i32*)Reg->DataPtr;
    return Result;
}

internal f32* GetFloat(var* Reg)
{
    f32* Result = (f32*)Reg->DataPtr;
    return Result;
}

internal i32** GetIntPtr(var* Reg)
{
    i32** Result = (i32**)Reg->DataPtr;
    return Result;
}

internal f32** GetFloatPtr(var* Reg)
{
    f32** Result = (f32**)Reg->DataPtr;
    return Result;
}

inline var* AllocateVar(mem_arena* Arena, var* Reg)
{
    if (Reg->Flags & RegFlag_IsConstVal)
    {
        if (AreSameType(Reg->Type, GetBasicType(TypeId_Int)))
        {
            i32* Val = PushStruct(Arena, i32);
            *Val = ConvertStringToInt(Reg->Str);

            Reg->DataPtr = Val;
        }
        else if (AreSameType(Reg->Type, GetBasicType(TypeId_Float)))
        {
            f32* Val = PushStruct(Arena, f32);
            *Val = ConvertStringToFloat(Reg->Str);

            Reg->DataPtr = Val;
        }
        else
        {
            InvalidCodePath;
        }
    }
    else
    {
        // NOTE: Check if we had a value assigned to this var
        if (Reg->DataPtr == 0)
        {
            Reg->DataPtr = PushSize(Arena, Reg->Type->SizeInBytes);
        }
    }

    return Reg;
}

internal var* RecursiveExecuteRun(mem_arena* Arena, syntax_node* Root);
internal var* ExecuteFuncCall(mem_arena* Arena, syntax_node* Root,
                                      var** Args, u32 NumArgs)
{
    var* Result = {};
    syntax_node* CurrNode = Root;
    b32 WasIfEvaluated = false;
    
    while (CurrNode)
    {
        u32 temp = NodeId_Define;
        switch (CurrNode->Id)
        {
            case NodeId_FuncDef:
            {
                CurrNode = CurrNode->Children[0];
                
                // NOTE: Copy the values to the arguments
                for (u32 ArgIndex = 0; ArgIndex < NumArgs; ++ArgIndex)
                {
                    Assert(CurrNode->Id == NodeId_Define);
                    var* ArgVar = CurrNode->Children[0]->Var;
                    ArgVar = AllocateVar(Arena, ArgVar);

                    // NOTE: We know that both types are of equal size
                    memcpy(ArgVar->DataPtr, Args[ArgIndex]->DataPtr, ArgVar->Type->SizeInBytes);

                    CurrNode = CurrNode->Next;
                }
            } break;
            
            case NodeId_Return:
            {
                Result = RecursiveExecuteRun(Arena, CurrNode->Children[0]);
                return Result;
            } break;

            case NodeId_If:
            {
                var* BoolExpr = RecursiveExecuteRun(Arena, CurrNode->Children[0]);
                if (*GetInt(BoolExpr) == 1)
                {
                    Result = ExecuteFuncCall(Arena, CurrNode->Children[1], 0, 0);
                    if (Result)
                    {
                        return Result;
                    }
                    WasIfEvaluated = true;
                }

                CurrNode = CurrNode->Next;
            } break;

            case NodeId_ElseIf:
            {
                if (WasIfEvaluated)
                {
                    CurrNode = CurrNode->Next;
                }
                else
                {
                    var* BoolExpr = RecursiveExecuteRun(Arena, CurrNode->Children[0]);
                    if (*GetInt(BoolExpr) == 1)
                    {
                        Result = ExecuteFuncCall(Arena, CurrNode->Children[1], 0, 0);
                        if (Result)
                        {
                            return Result;
                        }
                        WasIfEvaluated = true;
                    }

                    CurrNode = CurrNode->Next;
                }
            } break;

            case NodeId_Else:
            {
                if (WasIfEvaluated)
                {
                    CurrNode = CurrNode->Next;
                }
                else
                {
                    Result = ExecuteFuncCall(Arena, CurrNode->Children[0], 0, 0);
                    if (Result)
                    {
                        return Result;
                    }
                    CurrNode = CurrNode->Next;
                }
            } break;

            case NodeId_While:
            {
                while (true)
                {
                    var* BoolExpr = RecursiveExecuteRun(Arena, CurrNode->Children[0]);
                    if (*GetInt(BoolExpr) == 1)
                    {
                        ExecuteFuncCall(Arena, CurrNode->Children[1], 0, 0);
                    }
                    else
                    {
                        CurrNode = CurrNode->Next;
                        break;
                    }
                }
            } break;
            
            case NodeId_Equal:
            {
                WasIfEvaluated = false;
                
                var* Left;
                var* Right = RecursiveExecuteRun(Arena, CurrNode->Children[1]);
                if (CurrNode->Children[0]->Id == NodeId_Dereference)
                {
                    syntax_node* DerefNode = CurrNode->Children[0];
                    Left = RecursiveExecuteRun(Arena, DerefNode->Children[0]);

                    if (AreSameType(Left->Type->PointerType, GetBasicType(TypeId_Int)))
                    {
                        i32* Ptr = (i32*)Left->DataPtr;
                        i32* PtrVal = (i32*)(*Ptr);
                        *PtrVal = *GetInt(Right);
                    }
                    else if (AreSameType(Left->Type->PointerType, GetBasicType(TypeId_Float)))
                    {
                        i32* Ptr = (i32*)Left->DataPtr;
                        f32* PtrVal = (f32*)(*Ptr);
                        *PtrVal = *GetFloat(Right);
                    }
                }
                else if (CurrNode->Children[0]->Id == NodeId_GetArrayIndex)
                {
                    syntax_node* ArrayNode = CurrNode->Children[0];
                    
                    var* Ptr = RecursiveExecuteRun(Arena, ArrayNode->Children[0]);
                    var* Index = RecursiveExecuteRun(Arena, ArrayNode->Children[1]);
                    
                    i32* MemAddress = *GetIntPtr(Ptr) + *GetInt(Index);
                    memcpy(MemAddress, Right->DataPtr, Right->Type->SizeInBytes);
                }
                else
                {
                    Left = RecursiveExecuteRun(Arena, CurrNode->Children[0]);

                    // NOTE: We know that both types are of equal size
                    memcpy(Left->DataPtr, Right->DataPtr, Left->Type->SizeInBytes);
                }
                
                CurrNode = CurrNode->Next;       
            } break;
            
            default:
            {
                WasIfEvaluated = false;

                RecursiveExecuteRun(Arena, CurrNode);
                CurrNode = CurrNode->Next;
            } break;
        }
    }

    return Result;
}

internal var* RecursiveExecuteRun(mem_arena* Arena, syntax_node* Root)
{
    if (Root->NumArgs == 0 && Root->Id == NodeId_Var)
    {
        return AllocateVar(Arena, Root->Var);
    }

    var* Result = 0;
    switch (Root->Id)
    {
        case NodeId_Define:
        {
            Result = RecursiveExecuteRun(Arena, Root->Children[0]);
        } break;

        case NodeId_Zero:
        {
            Result = RecursiveExecuteRun(Arena, Root->Children[0]);
            memset(Result->DataPtr, 0, Result->Type->SizeInBytes);
        } break;

        case NodeId_Dereference:
        {
            var* Ptr = RecursiveExecuteRun(Arena, Root->Children[0]);
            Result = AllocateVar(Arena, Root->Var);
            memcpy(Result->DataPtr, Ptr->DataPtr, Result->Type->PointerType->SizeInBytes);
        } break;

        case NodeId_GetArrayIndex:
        {
            var* Ptr = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Index = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);

            i32* MemAddress = *GetIntPtr(Ptr) + *GetInt(Index);
            memcpy(Result->DataPtr, MemAddress, Result->Type->PointerType->SizeInBytes);
        } break;
        
        case NodeId_AddressOff:
        {
            var* Val = RecursiveExecuteRun(Arena, Root->Children[0]);
            Result = AllocateVar(Arena, Root->Var);
            i32 MemAddress = (i32)Val->DataPtr;
            *GetInt(Result) = MemAddress;
        } break;

        case NodeId_Negate:
        {
            var* Val = RecursiveExecuteRun(Arena, Root->Children[0]);
            Result = AllocateVar(Arena, Root->Var);
            if (AreSameType(Val->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = -(*GetInt(Val));
            }
            else if (AreSameType(Val->Type, GetBasicType(TypeId_Float)))
            {
                *GetFloat(Result) = -(*GetFloat(Val));
            }
        } break;

        case NodeId_Run:
        {
            OutputError("Error: Cannot have nested #run statements.\n");
        } break;
        
        case NodeId_BinaryEqual:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);
            
            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) == *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetInt(Left) == *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetFloat(Left) == *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetFloat(Left) == *GetFloat(Right);
            }
        } break;

        case NodeId_BinaryNotEqual:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);

            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) != *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetInt(Left) != *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetFloat(Left) != *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetFloat(Left) != *GetFloat(Right);
            }
        } break;

        case NodeId_Less:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);
            
            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) < *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetInt(Left) < *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetFloat(Left) < *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetFloat(Left) < *GetFloat(Right);
            }
        } break;

        case NodeId_LessEqual:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);
            
            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) <= *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetInt(Left) <= *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetFloat(Left) <= *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetFloat(Left) <= *GetFloat(Right);
            }
        } break;

        case NodeId_Greater:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);
            
            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) > *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetInt(Left) > *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetFloat(Left) > *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetFloat(Left) > *GetFloat(Right);
            }
        } break;

        case NodeId_GreaterEqual:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);
            
            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) >= *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetInt(Left) >= *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetFloat(Left) >= *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetInt(Result) = *GetFloat(Left) >= *GetFloat(Right);
            }
        } break;

        case NodeId_Add:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);

            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) + *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetFloat(Result) = *GetInt(Left) + *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetFloat(Result) = *GetFloat(Left) + *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetFloat(Result) = *GetFloat(Left) + *GetFloat(Right);
            }
            else if (IsTypePointer(Left->Type) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) + *GetInt(Right);
            }
            else if (AreSameType(Right->Type, GetBasicType(TypeId_Int)) &&
                     IsTypePointer(Left->Type))
            {
                *GetInt(Result) = *GetInt(Left) + *GetInt(Right);
            }
        } break;

        case NodeId_Sub:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);

            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) - *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetFloat(Result) = *GetInt(Left) - *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetFloat(Result) = *GetFloat(Left) - *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetFloat(Result) = *GetFloat(Left) - *GetFloat(Right);
            }
            else if (IsTypePointer(Left->Type) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) - *GetInt(Right);
            }
            else if (AreSameType(Right->Type, GetBasicType(TypeId_Int)) &&
                     IsTypePointer(Left->Type))
            {
                *GetInt(Result) = *GetInt(Left) - *GetInt(Right);
            }
        } break;

        case NodeId_Mul:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);

            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) * *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetFloat(Result) = *GetInt(Left) * *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetFloat(Result) = *GetFloat(Left) * *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetFloat(Result) = *GetFloat(Left) * *GetFloat(Right);
            }
        } break;

        case NodeId_Div:
        {
            var* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            var* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateVar(Arena, Root->Var);

            if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetInt(Result) = *GetInt(Left) / *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Int)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetFloat(Result) = *GetInt(Left) / *GetFloat(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Int)))
            {
                *GetFloat(Result) = *GetFloat(Left) / *GetInt(Right);
            }
            else if (AreSameType(Left->Type, GetBasicType(TypeId_Float)) &&
                     AreSameType(Right->Type, GetBasicType(TypeId_Float)))
            {
                *GetFloat(Result) = *GetFloat(Left) / *GetFloat(Right);
            }
        } break;

        case NodeId_FuncCall:
        {
            function* Func = Root->Func;

            // NOTE: Set the arguments
            // TODO: Memory here
            var** Vars = (var**)malloc(sizeof(var*)*Root->NumArgs);
            for (u32 ArgIndex = 0; ArgIndex < Root->NumArgs; ++ArgIndex)
            {
                var* Arg = Root->Children[ArgIndex]->Var;
                Arg = AllocateVar(Arena, Arg);
                Vars[ArgIndex] = Arg;
            }
            
            Result = ExecuteFuncCall(Arena, Func->StartNode, Vars, Root->NumArgs);
            free(Vars);
        } break;
        
    }

    return Result;
}

internal void ExecuteRunStatement(mem_arena* Arena, syntax_node* Root)
{
    syntax_node* Node = CreateSyntaxNode(NodeId_Var, 0, 0);
    var* Val = 0;
    try
    {
        if (Root->Id == NodeId_Run)
        {
            Val = RecursiveExecuteRun(Arena, Root->Children[0]);
        }
        else
        {
            Val = RecursiveExecuteRun(Arena, Root);
        }
    }
    catch (...)
    {
        OutputError("Error: Error during compile time execution.\n");
    }
    
    if (AreSameType(Val->Type, GetBasicType(TypeId_Int)))
    {
        Node->Var = CreateConstIntVar(*GetInt(Val));
        Node->Var->Type = GetBasicType(TypeId_Int);
    }
    else if (AreSameType(Val->Type, GetBasicType(TypeId_Float)))
    {
        Node->Var = CreateConstFloatVar(*GetFloat(Val));
        Node->Var->Type = GetBasicType(TypeId_Float);
    }
    
    if (Root->Id == NodeId_Run)
    {
        Root->Children[0] = Node;
    }
    else
    {
        *Root = *Node;
    }
}
