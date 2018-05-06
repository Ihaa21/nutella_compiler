/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */
 
#if 0
#define TryParseExprCreator(name, BoolExpr, LowerCall)                  \
        internal expression_node* name(token* CurrToken, token* LastToken, expression_node* FirstTerm, token** FinishedToken) \
    {                                                                   \
        if (CurrToken >= LastToken)                                     \
        {                                                               \
            OutputError("Error: Expression is empty.\n");               \
        }                                                               \
        expression_node* Result = FirstTerm;                            \
        while (CurrToken != LastToken)                                  \
        {                                                               \
            if (BoolExpr)                                               \
            {                                                           \
                u32 OpId = CurrToken->Id;                               \
                ++CurrToken;                                            \
                                                                        \
                expression_node* NewResult = PushStruct(&SyntaxTreeArena, expression_node); \
                *NewResult = {};                                        \
                NewResult->OperatorId = OpId;                           \
                NewResult->Left = Result;                               \
                                                                        \
                expression_node* TermNode = TryParseTerm(CurrToken, LastToken, &CurrToken); \
                if (CurrToken->Id > OpId)                               \
                {                                                       \
                    NewResult->Right = LowerCall;                       \
                }                                                       \
                else                                                    \
                {                                                       \
                    NewResult->Right = TermNode;                        \
                }                                                       \
                Result = NewResult;                                     \
            }                                                           \
            else                                                        \
            {                                                           \
                break;                                                  \
            }                                                           \
        }                                                               \
                                                                        \
        *FinishedToken = CurrToken;                                     \
        return 0;                                                       \
    }

TryParseExprCreator(TryParseMulDivExpr,
                    CurrToken->Id == OpId_Mul || CurrToken->Id == OpId_Div,
                    NewResult->Right;InvalidCodePath)
#endif

// TODO: Remove this version
inline i32 FindMatchingOpen(token* StartToken, u32 SearchCount)
{
    i32 Result = -1;
    u32 NumCloseLeft = 0;

    for (u32 TokenIndex = 1; TokenIndex <= SearchCount; ++TokenIndex)
    {
        token* CurrToken = StartToken - TokenIndex;
        if (CurrToken->Id == OpId_OpenBracket)
        {
            if (NumCloseLeft == 0)
            {
                Result = TokenIndex;
                break;
            }
            else
            {
                --NumCloseLeft;
            }
        }
        else if (CurrToken->Id == OpId_CloseBracket)
        {
            ++NumCloseLeft;
        }
    }

    return Result;
}

internal expression_node* ParseExpressionToTree(token* Tokens, i32 NumTokens)
{
    #if 0
    expression_node* Result = PushStruct(&SyntaxTreeArena, expression_node);
    memset(Result, 0, sizeof(expression_node));

    // NOTE: We assume we don't have a end line at the end
    // NOTE: We scan in reverse order of what our order of operations wants since the tree will
    // be traveresed in a reversed order which then would give us the correct result

    i32 OpenBracketPos = -1;
    token* OpenBracketToken = 0;
    i32 CloseBracketPos = -1;
    token* CloseBracketToken = 0;

    i32 AddressOfPos = -1;
    token* AddressOfToken = 0;

    i32 DereferencePos = -1;
    token* DereferenceToken = 0;
    
    i32 MulDivPos = -1;
    token* MulDivToken = 0;
    
    i32 AddSubPos = -1;
    token* AddSubToken = 0;
    
    i32 CurrTokenCount = NumTokens - 1;
    token* CurrToken = Tokens + (NumTokens - 1);

    if (NumTokens == 0)
    {
        OutputError("Error: Expression has no tokens.\n");
    }
    
    // NOTE: Base case for when we have just 1 token thats a variable or constant
    if (NumTokens == 1)
    {
        if (!(CurrToken->Id == TokenId_Var || IsTokenConstant(*CurrToken)))
        {
            OutputError("Error: Expression does not contain a variable name or constant.\n");
        }

        Result->Token = *CurrToken;
        return Result;
    }

    // TODO: This can be made to not have to scan the whole loop but only the nearest op
    b32 FoundCompareOp = false;
    while (CurrToken >= Tokens)
    {
        if (CurrToken->Id == OpId_CloseBracket)
        {
            i32 TokensTillMatchingOpen = FindMatchingOpen(CurrToken, CurrTokenCount);

            if (TokensTillMatchingOpen == -1)
            {
                OutputError("Error: No matching open bracket found for closing bracket.\n");
            }

            // NOTE: Don't scan the inside of the brackets
            if (CloseBracketPos == -1)
            {
                CloseBracketPos = CurrTokenCount;
                CloseBracketToken = CurrToken;

                OpenBracketPos = CurrTokenCount - TokensTillMatchingOpen;
                OpenBracketToken = CurrToken - TokensTillMatchingOpen;
            }

            CurrToken -= TokensTillMatchingOpen;
            CurrTokenCount -= TokensTillMatchingOpen;
        }
        else if (CurrToken->Id == OpId_OpenBracket)
        {
            // NOTE: We should never reach this code since we look for pairs of open/close
            // brackets right away when we see the first close bracket. Thus, this is invalid.

            OutputError("Error: Mismatching open/close brackets.\n");
        }
        else if (CurrToken->Id == OpId_AddressOff)
        {
            if (AddressOfPos == -1)
            {
                AddressOfPos = CurrTokenCount;
                AddressOfToken = CurrToken;
            }
        }
        else if (CurrToken->Id == OpId_Mul)
        {
            if (CurrTokenCount == NumTokens ||
                IsTokenOperator(*(CurrToken-1)))
            {
                // NOTE: This is a dereference
                if (DereferencePos == -1)
                {
                    DereferencePos = CurrTokenCount;
                    DereferenceToken = CurrToken;
                }
            }
            else
            {
                // NOTE: This is a multiplication
                if (CurrTokenCount == 0)
                {
                    OutputError("Error: Operators must have a left hand and a right hand side.\n");
                }

                if (MulDivPos == -1)
                {
                    MulDivPos = CurrTokenCount;
                    MulDivToken = CurrToken;
                }
            }
        }
        else if (CurrToken->Id == OpId_Div)
        {
            if (CurrTokenCount == 0)
            {
                OutputError("Error: Operators must have a left hand and a right hand side.\n");
            }

            if (MulDivPos == -1)
            {
                MulDivPos = CurrTokenCount;
                MulDivToken = CurrToken;
            }
        }
        else if (CurrToken->Id == OpId_Add || CurrToken->Id == OpId_Sub)
        {
            if (CurrTokenCount == -1)
            {
                OutputError("Error: Operators must have a left hand and a right hand side.\n");
            }

            if (AddSubPos == -1)
            {
                AddSubPos = CurrTokenCount;
                AddSubToken = CurrToken;
            }
            
            break;
        }
        else if (CurrToken->Id == OpId_BinaryEqual || CurrToken->Id == OpId_BinaryNotEqual ||
                 CurrToken->Id == OpId_Less || CurrToken->Id == OpId_LessEqual ||
                 CurrToken->Id == OpId_Greater || CurrToken->Id == OpId_GreaterEqual)
        {
            if (CurrTokenCount == 0)
            {
                OutputError("Error: Operator must have a left hand and a right hand side.\n");
            }

            Result->OperatorId = CurrToken->Id;
            Result->Left = ParseExpressionToTree(Tokens, CurrTokenCount);
            Result->Right = ParseExpressionToTree(CurrToken+1, NumTokens - CurrTokenCount - 1);
            
            FoundCompareOp = true;
        }

        --CurrToken;
        --CurrTokenCount;
    }

    // TODO: We can probably simplify this in a case with similar cases and if we move the compare
    // op here too
    // NOTE: We didn't find a add/sub op so we look for next ones based on order of ops
    if (!FoundCompareOp)
    {
        if (AddSubPos != -1)
        {            
            Result->OperatorId = CurrToken->Id;
            Result->Left = ParseExpressionToTree(Tokens, CurrTokenCount);
            Result->Right = ParseExpressionToTree(CurrToken+1, NumTokens - CurrTokenCount - 1);
        }
        else if (MulDivPos != -1)
        {
            Result->OperatorId = MulDivToken->Id;
            Result->Left = ParseExpressionToTree(Tokens, MulDivPos);
            Result->Right = ParseExpressionToTree(MulDivToken+1, NumTokens - MulDivPos - 1);
        }
        else if (AddressOfPos != -1)
        {
            // TODO: What error to output if address of isnt the first op in our line?
            if (AddressOfPos == NumTokens || AddressOfPos != 0)
            {
                OutputError("Error: Need to have variable after address of operator.\n");
            }

            Result->OperatorId = AddressOfToken->Id;
            Result->Right = ParseExpressionToTree(AddressOfToken+1, NumTokens - AddressOfPos - 1);
        }
        else if (DereferencePos != -1)
        {
            // TODO: What error to output if address of isnt the first op in our line?
            if (DereferencePos == NumTokens || DereferencePos != 0)
            {
                OutputError("Error: Need to have variable after address of operator.\n");
            }

            Result->OperatorId = OpId_Dereference;
            Result->Right = ParseExpressionToTree(DereferenceToken+1, NumTokens - DereferencePos - 1);
        }
        else if (CloseBracketPos != -1)
        {
            Result = ParseExpressionToTree(OpenBracketToken + 1, CloseBracketPos - OpenBracketPos - 1);
        }
        else
        {
            InvalidCodePath;
        }
    }

    return Result;
    #endif

    return 0;
}

#if 0
internal void ParseTokens(token* TokenArray, u32 NumTokens, FILE* OutFile)
{
    if (NumTokens == 0)
    {
        return;
    }
    
    token* CurrToken = TokenArray;
    if (CurrToken->Id == ScannerId_Identifier)
    {
    }
    else if (CurrToken->Id == ControlId_If)
    {
    }
    else if (CurrToken->Id == ControlId_Else)
    {
    }
    else if (CurrToken->Id == ControlId_While)
    {
    }
    else if (CurrToken->Id == ScannerId_Struct)
    {
    }
    else if (CurrToken->Id == ScannerId_OpenScope)
    {
    }
    else if (CurrToken->Id == ScannerId_CloseScope)
    {
    }
    else if (CurrToken->Id == TypeId_Int)
    {
        OutputError("Error: Can't start statement with a number.\n");
    }
    else if (CurrToken->Id == TypeId_Float)
    {
        OutputError("Error: Can't start statement with a number.\n");
    }
    else if (CurrToken->Id == OpId_Equals)
    {
        OutputError("Error: Can't start statement with eqauls.\n");
    }
    else if (CurrToken->Id == ScannerId_EndLine)
    {
        // TODO: Add a output warning call
        printf("Warning: Starting statement with endline.\n");
    }
    else
    {
        InvalidCodePath;
    }
}
#endif
