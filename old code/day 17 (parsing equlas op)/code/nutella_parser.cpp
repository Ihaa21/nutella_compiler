/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

inline token* FindMatchingOpen(token* CurrToken, token* LastToken)
{
    token* Result = 0;
    u32 NumOpenLeft = 0;

    while (CurrToken != LastToken)
    {
        if (CurrToken->Id == OpId_CloseBracket)
        {
            if (NumOpenLeft == 0)
            {
                Result = CurrToken;
                break;
            }
            else
            {
                --NumOpenLeft;
            }
        }
        else if (CurrToken->Id == OpId_OpenBracket)
        {
            ++NumOpenLeft;
        }

        ++CurrToken;
    }

    return Result;
}

internal expression_node* ParseExpr(parser_state* State, token* LastToken = 0,
                                    expression_node* FirstTerm = 0);

inline expression_node* TryParseTerm(parser_state* State, token* LastToken)
{
    b32 FoundTerm = false;
    expression_node* Result = 0;
    expression_node** CurrNode = &Result;

    while ((!FoundTerm) && State->StartToken != LastToken)
    {
        // TODO: Add support for negate in ir
        if (State->StartToken->Id == OpId_AddressOff || State->StartToken->Id == OpId_Mul ||
            State->StartToken->Id == OpId_Sub)
        {
            u32 OpId = State->StartToken->Id;
            if (OpId == OpId_Mul) {OpId = OpId_Dereference;}

            *CurrNode = CreateSyntaxNode(1);
            (*CurrNode)->OperatorId = OpId;
            CurrNode = &((*CurrNode)->Children[0]);
        }
        else if (State->StartToken->Id == OpId_Run)
        {
            ++State->StartToken;

            *CurrNode = CreateSyntaxNode(1);
            (*CurrNode)->OperatorId = OpId_Run;
            (*CurrNode)->Children[0] = ParseExpr(State, LastToken);
            State->StartToken = LastToken - 1; // NOTE: This is set so that we examine all tokens provided
            FoundTerm = true;
        }
        else if (State->StartToken->Id == OpId_OpenBracket)
        {
            ++State->StartToken;            
            token* CloseBracket = FindMatchingOpen(State->StartToken, LastToken);
            if (!CloseBracket)
            {
                OutputError("Error: No matching open bracket found for closing bracket.\n");
            }

            *CurrNode = ParseExpr(State, CloseBracket);
            State->StartToken = CloseBracket;
            FoundTerm = true;
        }
        else if (State->StartToken->Id == OpId_CloseBracket)
        {
            OutputError("Error: Mismatched open/close brackets.\n");
        }
        else if (State->StartToken->Id == TokenId_Func)
        {
            function* Func = State->StartToken->Func;
            ++State->StartToken;

            if (State->StartToken->Id != OpId_OpenBracket)
            {
                OutputError("Error: Expected brackets after function call.\n");
            }
            ++State->StartToken;

            *CurrNode = CreateSyntaxNode(Func->NumArgs);
            (*CurrNode)->Func = Func;

            // TODO: Check for multiple args (prob fails)
            u32 NumOpenLeft = 0;
            u32 ArgIndex = 0;
            token* CurrToken = State->StartToken;
            while (CurrToken != LastToken)
            {
                if (CurrToken->Id == ScannerId_Comma)
                {
                    (*CurrNode)->Children[ArgIndex++] = ParseExpr(State, CurrToken);
                    ++State->StartToken;
                }
                else if (CurrToken->Id == OpId_OpenBracket)
                {
                    ++NumOpenLeft;
                }
                else if (CurrToken->Id == OpId_CloseBracket)
                {
                    if (NumOpenLeft > 0)
                    {
                        --NumOpenLeft;
                    }
                    else
                    {
                        (*CurrNode)->Children[ArgIndex++] = ParseExpr(State, CurrToken);
                        if (ArgIndex < Func->NumArgs)
                        {
                            OutputError("Error: Not enough arguments provided for function call.\n");
                        }
                        if (ArgIndex > Func->NumArgs)
                        {
                            OutputError("Error: Too many arguments provided for function call.\n");
                        }
                    
                        break;
                    }
                }

                ++State->StartToken;
            }

            FoundTerm = true;
        }
        else if (State->StartToken->Id == TokenId_Var || IsTokenConstant(*State->StartToken))
        {
            *CurrNode = CreateSyntaxNode(0);
            (*CurrNode)->Token = *State->StartToken;
            FoundTerm = true;
        }
        else
        {
            OutputError("Error: Expected variable, constant, or uniary operator.\n");
        }
        
        ++State->StartToken;
    }

    if (!FoundTerm)
    {
        OutputError("Error: Expected to find a variable or constant but couldn't find one.\n");
    }
    
    return Result;
}

internal expression_node* TryParseMulDivExpr(parser_state* State, token* LastToken,
                                             expression_node* FirstTerm)
{
    if (State->StartToken >= LastToken)
    {
        OutputError("Error: Expression is empty.\n");
    }
    
    expression_node* Result = FirstTerm;
    while (State->StartToken != LastToken)
    {
        if (State->StartToken->Id == OpId_Mul || State->StartToken->Id == OpId_Div)
        {
            u32 OpId = State->StartToken->Id;
            ++State->StartToken;

            expression_node* NewResult = CreateSyntaxNode(2);
            NewResult->OperatorId = OpId;
            NewResult->Children[0] = Result;

            expression_node* TermNode = TryParseTerm(State, LastToken);
            if (IsTokenBinOp(*State->StartToken) && State->StartToken->Id > OpId)
            {
                // NOTE: This is the lowest rule so we don't have any rules to recurse to
                InvalidCodePath;
            }
            else
            {
                NewResult->Children[1] = TermNode;
            }
            
            Result = NewResult;            
        }
        else if (IsTokenBinOp(*State->StartToken))
        {
            // NOTE: This is the lowest rule so we don't have any rules to recurse to
            Assert(State->StartToken->Id <= OpId_Div);
            break; 
        }
        else
        {
            OutputError("Error: Encountered unexpected token.\n");
        }
    }
    
    return Result;
}

internal expression_node* TryParseAddSubExpr(parser_state* State, token* LastToken ,
                                             expression_node* FirstTerm)
{
    if (State->StartToken >= LastToken)
    {
        OutputError("Error: Expression is empty.\n");
    }
    
    expression_node* Result = FirstTerm;
    while (State->StartToken != LastToken)
    {
        if (State->StartToken->Id == OpId_Add || State->StartToken->Id == OpId_Sub)
        {
            u32 OpId = State->StartToken->Id;
            ++State->StartToken;

            expression_node* NewResult = CreateSyntaxNode(2);
            NewResult->OperatorId = OpId;
            NewResult->Children[0] = Result;

            expression_node* TermNode = TryParseTerm(State, LastToken);
            if (IsTokenBinOp(*State->StartToken) || State->StartToken->Id > OpId)
            {
                // NOTE: We have a operator of higher priority so we need to evaluate it
                NewResult->Children[1] = TryParseMulDivExpr(State, LastToken, TermNode);
            }
            else
            {
                NewResult->Children[1] = TermNode;
            }
            
            Result = NewResult;            
        }
        else if (IsTokenBinOp(*State->StartToken))
        {
            if (State->StartToken->Id > OpId_Sub)
            {
                Result = TryParseMulDivExpr(State, LastToken, Result);
            }
            else
            {
                break;
            } 
        }
        else
        {
            OutputError("Error: Encountered unexpected token.\n");
        }
    }
    
    return Result;
}

internal expression_node* ParseExpr(parser_state* State, token* LastToken, expression_node* FirstTerm)
{
    if (!LastToken)
    {
        LastToken = State->LastToken;
    }
    
    if (State->StartToken >= LastToken)
    {
        OutputError("Error: Expression is empty.\n");
    }

    expression_node* Result = 0;
    if (FirstTerm)
    {
        Result = FirstTerm;
    }
    else
    {
        Result = TryParseTerm(State, LastToken);
    }
    
    while (State->StartToken != LastToken)
    {
        if (State->StartToken->Id == OpId_BinaryEqual || State->StartToken->Id == OpId_BinaryNotEqual ||
            State->StartToken->Id == OpId_Less || State->StartToken->Id == OpId_LessEqual ||
            State->StartToken->Id == OpId_Greater || State->StartToken->Id == OpId_GreaterEqual)
        {
            u32 OpId = State->StartToken->Id;
            ++State->StartToken;

            expression_node* NewResult = CreateSyntaxNode(2);
            NewResult->OperatorId = OpId;
            NewResult->Children[0] = Result;

            expression_node* TermNode = TryParseTerm(State, LastToken);
            if (IsTokenBinOp(*State->StartToken) || State->StartToken->Id > OpId)
            {
                // NOTE: We have a op handled by lower rules ahead so we need to evaluate it
                NewResult->Children[1] = TryParseAddSubExpr(State, LastToken, TermNode);
            }
            else
            {
                // NOTE: We either have the ops handled by this func or ops handled by calling funcs
                // so we add the term 
                NewResult->Children[1] = TermNode;
            }
            
            Result = NewResult;
        }
        else if (IsTokenBinOp(*State->StartToken))
        {
            if (State->StartToken->Id > OpId_BinaryNotEqual)
            {
                // NOTE: When we have brackets, then if we find a op we don't understand that needs to
                // be handled by lower rules, we descend here to those rules.
                Result = TryParseAddSubExpr(State, LastToken, Result);
            }
            else
            {
                if (FirstTerm)
                {
                    // NOTE: We have one higher call to do (handle equals operator)
                    break;
                }
                else
                {
                    OutputError("Error: Encountered unexpected tokens.\n");
                }
            }
        }
        else
        {
            OutputError("Error: Encountered unexpected token.\n");
        }
    }

    return Result;
}


internal expression_node* ParseEqualExpr(parser_state* State)
{
#if 0
    // TODO: We probs dont want this here for empty statements like ;
    if (State->StartToken >= LastToken)
    {
        OutputError("Error: Expression is empty.\n");
    }
#endif
    
    expression_node* Result = TryParseTerm(State, State->LastToken);
    while (State->StartToken != State->LastToken)
    {
        if (State->StartToken->Id == OpId_Equal)
        {
            u32 OpId = State->StartToken->Id;
            ++State->StartToken;

            expression_node* NewResult = CreateSyntaxNode(2);
            NewResult->OperatorId = OpId;
            NewResult->Children[0] = Result;

            expression_node* TermNode = TryParseTerm(State, State->LastToken);
            if (IsTokenBinOp(*State->StartToken) || State->StartToken->Id > OpId)
            {
                NewResult->Children[1] = TryParseAddSubExpr(State, State->LastToken, TermNode);
            }
            else
            {
                NewResult->Children[1] = TermNode;
            }
            
            Result = NewResult;
        }
        else if (IsTokenBinOp(*State->StartToken))
        {
            if (State->StartToken->Id > OpId_Equal)
            {
                Result = ParseExpr(State, State->LastToken, Result);
            }
            else
            {
                OutputError("Error: Encountered unexpected token.\n");
            }
        }
        else
        {
            OutputError("Error: Encountered unexpected token.\n");
        }
    }

    return Result;
}

inline type* ParseType(parser_state* State)
{
    type* Result = 0;
    if (State->StartToken->Id == TokenId_Type)
    {
        Result = State->StartToken->Type;
    }
    else if (State->StartToken->Id == ScannerId_Identifier)
    {
        Result = IsTypeDefined(State, State->StartToken->Str);
    }
    else
    {
        OutputError("Error: Expected a type but found unexpected token.\n");
    }
    
    if (Result)
    {            
        ++State->StartToken;
            
        // NOTE: If pointer type, get the number of references
        u32 NumRef = 0;
        while(State->StartToken->Id == OpId_Mul)
        {
            ++NumRef;
            ++State->StartToken;
        }

        type* TempType = Result;
        if (NumRef > 0)
        {
            Result = PushStruct(&TypeArena, type);
            Result->NumRef = NumRef;
            Result->PointerType = TempType;
        }
    }

    return Result;
}

inline token EvaluateBoolExpr(parser_state* State)
{
    if (State->StartToken->Id != OpId_OpenBracket)
    {
        OutputError("Error: If keyword must be followed by a statement in brackets.\n");
    }
    ++State->StartToken;

    token* CurrNode = State->StartToken;
    while (CurrNode != State->LastToken)
    {
        if (CurrNode->Id == OpId_CloseBracket)
        {
            break;
        }
        ++CurrNode;
    }
    
    expression_node* ExprTree = ParseExpr(State, CurrNode, 0);
    token BoolExpr = ExpressionTreeToIR(ExprTree, State->OutFile);
    ++State->StartToken;
    
    if (GetTokenType(BoolExpr) != GetBasicType(TypeId_Int))
    {
        OutputError("Error: Expression inside if statement must evaluate to an int.\n");
    }

    return BoolExpr;
}

enum ParseDecleration
{
    VarDefn_Allowed = 0x1,
    VarDecl_Allowed = 0x2,
    VarManip_Allowed = 0x4,
    FuncDecl_Allowed = 0x8,
};

// TODO: We need to sometimes parse a little even if not allowed to see if we might have an error
// by declaring something in a scope that doesn't allow it
inline b32 TryParseTypeStatement(u32 Flags, parser_state* State)
{
    // TODO: Do we need this?
    //if (State->StartToken->Id != ScannerId_)
    {
        //    return false;
    }

    // TODO: Potentially simplify this functions args
    if (State->StartToken->Id == TokenId_Type)
    {
        type* Type = ParseType(State);
        if (!(State->StartToken->Id == ScannerId_Identifier))
        {
            OutputError("Error: type not followed by a variable or function name\n");
        }

        string NameStr = State->StartToken->Str;            
        ++State->StartToken;
            
        if (Flags & VarDefn_Allowed && State->StartToken->Id == ScannerId_EndLine)
        {
            // NOTE: Here we know we are looking at a variable definition
            if (State->StartToken->Id == TokenId_Var)
            {
                OutputError("Error: Variable redefined.\n");
            }
                
            // NOTE: Define after we evaluate LHS and RHS
            var* VarName = DefineVarInScope(State, NameStr, Type);
            PushDefineInstr(VarName->Type, VarName->Str);
                
            return true;
        }
        else if (Flags & FuncDecl_Allowed && State->StartToken->Id == OpId_OpenBracket)
        {
            // NOTE: Here we know we are looking at a function decleration
            function* Func = IsFuncDefined(NameStr);
            if (Func)
            {
                OutputError("Error: Function redefined.\n");
            }

            Func = CreateFunction(NameStr, Type);
            AddScope(State, ScopeId_Function);
                
            ++State->StartToken;

            u32 CurrentState = ArgParseState_Type;
            type* PrevType = 0;
            while (State->StartToken != State->LastToken)
            {
                if (CurrentState == ArgParseState_Type)
                {
                    PrevType = ParseType(State);
                    if (!PrevType)
                    {
                        OutputError("Error: Type for argument not defined. \n");
                    }

                    AddArgToFunction(Func, PrevType);
                    CurrentState = ArgParseState_VarName;
                }
                else if (CurrentState == ArgParseState_VarName)
                {
                    if (State->StartToken->Id == TokenId_Var)
                    {
                        OutputError("Error: Argument name already defined.\n");
                    }
                    if (State->StartToken->Id != ScannerId_Identifier)
                    {
                        OutputError("Error: Expected argument name after type for function decleration.\n");
                    }

                    var* Arg = DefineVarInScope(State, State->StartToken->Str, PrevType);
                    PushDefineInstr(Arg->Type, Arg->Str);
                        
                    CurrentState = ArgParseState_Comma;
                    ++State->StartToken;
                }
                else if (CurrentState == ArgParseState_Comma)
                {
                    if (State->StartToken->Id == OpId_CloseBracket)
                    {
                        break;
                    }
                    else if (State->StartToken->Id == ScannerId_Comma)
                    {
                        CurrentState = ArgParseState_Type;
                        ++State->StartToken;
                    }
                    else
                    {
                        OutputError("Error: Expected a comma or bracket after argument decleration.\n");
                    }
                }
                else
                {
                    InvalidCodePath;
                }
            }
                    
            ++State->StartToken;

            if (CurrentState != ArgParseState_Comma)
            {
                OutputError("Error: Expected a comma or bracket after argument decleration.\n");
            }

            PushFuncInstr(Func);
            PushScopeInstr(IR_StartScope);
            ++State->StartToken;
            Assert(State->StartToken == State->LastToken);
            
            return true;
        }
        else if (Flags & VarDecl_Allowed && State->StartToken->Id == OpId_Equal)
        {
            // NOTE: Here we know we are looking at a variable decleration
            if (State->StartToken->Id == TokenId_Var)
            {
                OutputError("Error: Variable redefined.\n");
            }

            ++State->StartToken;

            expression_node* ExprTree = ParseExpr(State);
            token RHS = ExpressionTreeToIR(ExprTree, State->OutFile);

            // TODO: We should pass the state to ParseExpr
            State->StartToken = State->LastToken;
            
            // NOTE: Define after we evaluate LHS and RHS
            var* VarName = DefineVarInScope(State, NameStr, Type);
            PushDefineInstr(VarName->Type, VarName->Str);
            if (!AreSameType(*VarName, RHS))
            {
                OutputError("Error: Type on LHS and RHS don't match.\n");
            }

            string RHS_Str = GetTokenString(RHS);
            PushAssignInstr(VarName->Str, RHS_Str);

            // TODO: Add these everywhere so that we can assert that we always check every token
            
            return true;
        }
        else
        {
            OutputError("Error: Variable definition followed by unsupported operations\n");
        }
                                     
        return false;
    }
    
    if (Flags & VarManip_Allowed && State->StartToken->Id == TokenId_Var)
    {
        var* LHS_Var = State->StartToken->Var;    
        ++State->StartToken;
            
        if (!(State->StartToken->Id == OpId_Equal))
        {
            OutputError("Error: Variable statement without equals operator.\n");
        }

        ++State->StartToken;
            
        // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
        expression_node* ExprTree = ParseExpr(State);
        token RHS = ExpressionTreeToIR(ExprTree, State->OutFile);
            
        // NOTE: We set this for debugging to make sure we always parse every token
        State->StartToken = State->LastToken;

        // NOTE: Do a type check here
        if (!AreSameType(*LHS_Var, RHS))
        {
            OutputError("Error: Type on LHS and RHS don't match.\n");
        }

        string RHS_Str = GetTokenString(RHS);
        PushAssignInstr(LHS_Var->Str, RHS_Str);
            
        return true;
    }
    else if ((!(Flags & VarManip_Allowed)) && State->StartToken->Id == TokenId_Var)
    {
        OutputError("Error: Declared variable in a scope which does not allow it.\n");
    }

    // TODO: Figure out the return values here
    return false;
}

inline b32 TryParseIf(parser_state* State)
{
    if (State->StartToken->Id != ControlId_If)
    {
        return false;
    }
    ++State->StartToken;
        
    token BoolExpr = EvaluateBoolExpr(State);
    if (State->StartToken->Id != ScannerId_OpenScope)
    {
        OutputError("Error: If statement must be followed by a scope.\n");
    }

    AddScope(State, ScopeId_If);
    PushIfInstr(GetTokenString(BoolExpr));
    PushScopeInstr(IR_StartScope);
    ++State->StartToken;
    Assert(State->StartToken == State->LastToken);
    return true;
}

inline b32 TryParseElse(parser_state* State)
{
    if (State->StartToken->Id != ControlId_Else)
    {
        return false;
    }
    ++State->StartToken;

    u32 NumTokensToSkip = 0;
    if (State->StartToken->Id == ControlId_If)
    {
        ++State->StartToken;

        token BoolExpr = EvaluateBoolExpr(State);
            
        PushElseIfInstr(GetTokenString(BoolExpr));
        AddScope(State, ScopeId_ElseIf);
    }
    else
    {
        PushElseInstr();
        AddScope(State, ScopeId_Else);
    }

    if (!(State->StartToken->Id == ScannerId_OpenScope))
    {
        OutputError("Error: Else/Else If statement must be followed by a scope.\n");
    }

    PushScopeInstr(IR_StartScope);
    ++State->StartToken;
    Assert(State->StartToken == State->LastToken);
    return true;
}

inline b32 TryParseWhile(parser_state* State)
{
    if (State->StartToken->Id != ControlId_While)
    {
        return false;
    }
    ++State->StartToken;
        
    if (State->StartToken->Id != OpId_OpenBracket)
    {
        OutputError("Error: If keyword must be followed by a statement in brackets.\n");
    }

    token BoolExpr = EvaluateBoolExpr(State);
    PushWhileInstr(GetTokenString(BoolExpr));

    if (!(State->StartToken->Id == ScannerId_OpenScope))
    {
        OutputError("Error: While statement must be followed by a scope.\n");
    }
        
    AddScope(State, ScopeId_WhileLoop);
    PushScopeInstr(IR_StartScope);
    ++State->StartToken;
    Assert(State->StartToken == State->LastToken);
    return true;
}

inline b32 TryParseStruct(parser_state* State)
{
    if (State->StartToken->Id != ScannerId_Struct)
    {
        return false;
    }
    ++State->StartToken;

    // TODO: Add to our type table (here we have the name token)
    ++State->StartToken;
    
    if (!(State->StartToken->Id == ScannerId_OpenScope))
    {
        OutputError("Error: Struct statement must be followed by a scope.\n");
    }
        
    AddScope(State, ScopeId_Struct);
    PushScopeInstr(IR_StartScope);
    ++State->StartToken;
    Assert(State->StartToken == State->LastToken);
    return true;
}

inline b32 TryParseOpenScope(parser_state* State)
{
    if (State->StartToken->Id != ScannerId_OpenScope)
    {
        return false;
    }
    
    AddScope(State);
    PushScopeInstr(IR_StartScope);

    ++State->StartToken;
    Assert(State->StartToken == State->LastToken);
    return true;
}

inline b32 TryParseCloseScope(parser_state* State)
{
    if (State->StartToken->Id != ScannerId_CloseScope)
    {
        return false;
    }
    
    temp_mem ScopeTempMem = State->Scope->TempMem;
    State->Scope = State->Scope->PrevScope;
    ClearTempMem(ScopeTempMem);
    PushScopeInstr(IR_EndScope);

    ++State->StartToken;
    Assert(State->StartToken == State->LastToken);
    return true;
}

internal parser_state* ParseTokens(parser_state* State)
{
    if (State->StartToken == State->LastToken)
    {
        return State;
    }
    
    if (State->Scope->Id == ScopeId_Struct)
    {
        if (TryParseTypeStatement(VarDefn_Allowed, State))
        {
            return State;
        }
        if (TryParseCloseScope(State))
        {
            return State;
        }
        
        OutputError("");
    }
    else if (State->Scope->Id == ScopeId_Global)
    {
        if (TryParseTypeStatement(VarDefn_Allowed | VarDecl_Allowed | FuncDecl_Allowed, State))
        {
            return State;
        }
        if (TryParseStruct(State))
        {
            return State;
        }
        if (TryParseOpenScope(State))
        {
            return State;
        }
        if (TryParseCloseScope(State))
        {
            return State;
        }
        //if (TryParseLoadFile())
        {
            //    return;
        }
        
        OutputError("");
    }
    else if (IsScopeCodeBlock(State->Scope->Id))
    {
        if (TryParseTypeStatement(VarDefn_Allowed | VarDecl_Allowed | VarManip_Allowed |
                                  FuncDecl_Allowed, State))
        {
            return State;
        }
        if (TryParseIf(State))
        {
            return State;
        }
        if (TryParseElse(State))
        {
            return State;
        }
        if (TryParseWhile(State))
        {
            return State;
        }
        if (TryParseOpenScope(State))
        {
            return State;
        }
        if (TryParseCloseScope(State))
        {
            return State;
        }

        OutputError("");
    }
    else
    {
        InvalidCodePath;
    }
    
    // NOTE: This never happens, VS just complains about not all paths returning tho
    return 0;
}
