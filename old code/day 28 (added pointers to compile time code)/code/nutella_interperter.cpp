/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

internal i32* GetInt(virtual_reg* Reg)
{
    i32* Result = (i32*)Reg->DataPtr;
    return Result;
}

internal f32* GetFloat(virtual_reg* Reg)
{
    f32* Result = (f32*)Reg->DataPtr;
    return Result;
}

internal i32** GetIntPtr(virtual_reg* Reg)
{
    i32** Result = (i32**)Reg->DataPtr;
    return Result;
}

internal f32** GetFloatPtr(virtual_reg* Reg)
{
    f32** Result = (f32**)Reg->DataPtr;
    return Result;
}

inline virtual_reg* AllocateReg(mem_arena* Arena, virtual_reg* Reg)
{
    if (Reg->Flags & RegFlag_IsConstVal)
    {
        if (AreSameType(Reg->Type, GetBasicType(TypeId_Int)))
        {
            i32* Val = PushStruct(Arena, i32);
            *Val = ConvertStringToInt(Reg->Name);

            Reg->DataPtr = Val;
        }
        else if (AreSameType(Reg->Type, GetBasicType(TypeId_Int)))
        {
            f32* Val = PushStruct(Arena, f32);
            *Val = ConvertStringToFloat(Reg->Name);

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

internal virtual_reg* RecursiveExecuteRun(mem_arena* Arena, syntax_node* Root);
internal virtual_reg* ExecuteFuncCall(mem_arena* Arena, syntax_node* Root,
                                      virtual_reg** Args, u32 NumArgs)
{
    virtual_reg* Result = {};
    syntax_node* CurrNode = Root;

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
                    virtual_reg* ArgReg = CurrNode->Children[0]->Reg;
                    ArgReg = AllocateReg(Arena, ArgReg);

                    // NOTE: We know that both types are of equal size
                    memcpy(ArgReg->DataPtr, Args[ArgIndex]->DataPtr, ArgReg->Type->SizeInBytes);

                    CurrNode = CurrNode->Next;
                }
            } break;
            
            case NodeId_Return:
            {
                Result = RecursiveExecuteRun(Arena, CurrNode->Children[0]);
                return Result;
            } break;

            case NodeId_Equal:
            {
                // TODO: Support array indexing and pointer dereferencing
                virtual_reg* Left;
                virtual_reg* Right = RecursiveExecuteRun(Arena, CurrNode->Children[1]);
                if (CurrNode->Children[0]->Id == NodeId_Dereference ||
                    CurrNode->Children[0]->Id == NodeId_GetArrayIndex)
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
                else
                {
                    Left = RecursiveExecuteRun(Arena, CurrNode->Children[0]);

                    // NOTE: We know that both types are of equal size
                    // TODO: Make SizeInBytes of pointers defined
                    memcpy(Left->DataPtr, Right->DataPtr, Left->Type->SizeInBytes);
                }
                
                CurrNode = CurrNode->Next;       
            } break;
            
            default:
            {
                RecursiveExecuteRun(Arena, CurrNode);
                CurrNode = CurrNode->Next;
            } break;
        }
    }

    return Result;
}

internal virtual_reg* RecursiveExecuteRun(mem_arena* Arena, syntax_node* Root)
{
    if (Root->NumArgs == 0)
    {
        return AllocateReg(Arena, Root->Reg);
    }

    virtual_reg* Result = 0;
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
            virtual_reg* Ptr = RecursiveExecuteRun(Arena, Root->Children[0]);
            Result = AllocateReg(Arena, Root->Reg);
            memcpy(Result->DataPtr, Ptr->DataPtr, Result->Type->PointerType->SizeInBytes);
        } break;

        case NodeId_AddressOff:
        {
            virtual_reg* Val = RecursiveExecuteRun(Arena, Root->Children[0]);
            Result = AllocateReg(Arena, Root->Reg);
            i32 MemAddress = (i32)Val->DataPtr;
            *GetInt(Result) = MemAddress;
        } break;
        
        case NodeId_BinaryEqual:
        {
            virtual_reg* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            virtual_reg* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateReg(Arena, Root->Reg);
            
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
            virtual_reg* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            virtual_reg* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateReg(Arena, Root->Reg);

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

        case NodeId_Add:
        {
            virtual_reg* Left = RecursiveExecuteRun(Arena, Root->Children[0]);
            virtual_reg* Right = RecursiveExecuteRun(Arena, Root->Children[1]);
            Result = AllocateReg(Arena, Root->Reg);

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

        case NodeId_FuncCall:
        {
            function* Func = Root->Func;

            // NOTE: Set the arguments
            // TODO: Memory here
            virtual_reg** Regs = (virtual_reg**)malloc(sizeof(virtual_reg*)*Root->NumArgs);
            for (u32 ArgIndex = 0; ArgIndex < Root->NumArgs; ++ArgIndex)
            {
                virtual_reg* Arg = Root->Children[ArgIndex]->Reg;
                Arg = AllocateReg(Arena, Arg);
                Regs[ArgIndex] = Arg;
            }
            
            Result = ExecuteFuncCall(Arena, Func->StartNode, Regs, Root->NumArgs);
            free(Regs);
        } break;
        
    }

    return Result;
}

internal void ExecuteRunStatement(mem_arena* Arena, syntax_node* Root)
{
    syntax_node* Node = CreateSyntaxNode(NodeId_Reg, 0);
    virtual_reg* Val = RecursiveExecuteRun(Arena, Root->Children[0]);
    if (AreSameType(Val->Type, GetBasicType(TypeId_Int)))
    {
        Node->Reg = CreateConstIntRegister(*GetInt(Val));
        Node->Reg->Type = GetBasicType(TypeId_Int);
    }
    else if (AreSameType(Val->Type, GetBasicType(TypeId_Float)))
    {
        Node->Reg = CreateConstFloatRegister(*GetFloat(Val));
        Node->Reg->Type = GetBasicType(TypeId_Float);
    }
    // TODO: Add pointers
    // TODO: Clear the other nodes that where below the run statement?

    Root->Children[0] = Node;
}
