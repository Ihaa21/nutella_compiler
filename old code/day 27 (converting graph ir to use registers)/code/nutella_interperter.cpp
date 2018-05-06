/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

struct typed_val
{
    u32 Type;

    union
    {
        f32 Float;
        i32 Int;
        u32 Ptr;
    };
};

internal typed_val RecursiveExecuteRun(syntax_node* Root)
{
    typed_val Result = {};

    if (Root->NumArgs == 0)
    {
        virtual_reg* CurrReg = Root->Reg;
        if (CurrReg->Flags & RegFlag_IsConstVal)
        {
            if (AreSameType(CurrReg->Type, GetBasicType(TypeId_Int)))
            {
                Result.Type = TypeId_Int;
                Result.Int = ConvertStringToInt(CurrReg->Name);
            }
            else if (AreSameType(CurrReg->Type, GetBasicType(TypeId_Int)))
            {
                Result.Type = TypeId_Float;
                Result.Float = ConvertStringToFloat(CurrReg->Name);
            }
            else
            {
                InvalidCodePath;
            }
            
            return Result;
        }
        else
        {
            // NOTE: Check if we had a value assigned to this var
        }
    }

    switch (Root->Id)
    {
        case NodeId_Add:
        {
            typed_val Left = RecursiveExecuteRun(Root->Children[0]);
            typed_val Right = RecursiveExecuteRun(Root->Children[1]);

            if (Left.Type == TypeId_Int && Right.Type == TypeId_Int)
            {
                Result.Type = TypeId_Int;
                Result.Int = Left.Int + Right.Int;
            }
            else if (Left.Type == TypeId_Int && Right.Type == TypeId_Float)
            {
                Result.Type = TypeId_Float;
                Result.Float = (f32)(Left.Int) + Right.Float;
            }
            else if (Left.Type == TypeId_Float && Right.Type == TypeId_Int)
            {
                Result.Type = TypeId_Float;
                Result.Float = Left.Float + (f32)(Right.Int);
            }
            else if (Left.Type == TypeId_Float && Right.Type == TypeId_Float)
            {
                Result.Type = TypeId_Float;
                Result.Float = Left.Float + Right.Float;
            }
        } break;
    }

    return Result;
}

internal void ExecuteRunStatement(syntax_node* Root)
{
    syntax_node* Node = CreateSyntaxNode(NodeId_Reg, 0);
    //Node->Token.Id = NodeId_Reg;
    //Node->Token.Flags = TokenFlag_IsConstant;

    typed_val Val = RecursiveExecuteRun(Root->Children[0]);
    if (Val.Type == TypeId_Int)
    {
        //Node->Token.Str = ConvertIntToString(&SyntaxTreeArena, Val.Int);
    }
    else if (Val.Type == TypeId_Float)
    {
        //Node->Token.Str = ConvertFloatToString(&SyntaxTreeArena, Val.Float);
    }
    // TODO: Add pointers
    // TODO: Clear the other nodes that where below the run statement?

    Root->Children[0] = Node;
}
