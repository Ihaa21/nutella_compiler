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

inline token* FindMatchingOpenSq(token* CurrToken, token* LastToken)
{
    token* Result = 0;
    u32 NumOpenLeft = 0;

    while (CurrToken != LastToken)
    {
        if (CurrToken->Id == OpId_CloseSqBracket)
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
        else if (CurrToken->Id == OpId_OpenSqBracket)
        {
            ++NumOpenLeft;
        }

        ++CurrToken;
    }

    return Result;
}

internal syntax_node* ParseExpr(parser_state* State, token* LastToken = 0,
                                    syntax_node* FirstTerm = 0);

inline syntax_node* ParseMemberAndArrayAccessing(parser_state* State, token* LastToken,
                                                 syntax_node* Term)
{    
    // NOTE: We check here for accessing members and array indexing
    syntax_node* Result = Term;
    i32 LoopState = -1;
    while (State->StartToken != LastToken)
    {
        if ((LoopState == -1 || LoopState == 0) && State->StartToken->Id == OpId_GetMember)
        {
            // NOTE: Members can have various names which may conflict but are understood
            // from context so we just make sure we are looking at a possible identifier
            ++State->StartToken;
            if (!(State->StartToken->Id == ScannerId_Identifier ||
                  State->StartToken->Id == TokenId_Var ||
                  State->StartToken->Id == TokenId_Type ||
                  State->StartToken->Id == TokenId_Func))
            {
                OutputError("Error: Expected member variable but found unexpected tokens.\n");
            }

            Result = CreateSyntaxNode(2);
            Result->OperatorId = OpId_GetMember;
            Result->Children[0] = Term;
            Result->Children[1] = CreateSyntaxNode(0);
            Result->Children[1]->OperatorId = TokenId_Var;
            Result->Children[1]->Token = *State->StartToken;
            
            ++State->StartToken;
            LoopState = 1;
        }
        else if ((LoopState == -1 || LoopState == 1) && State->StartToken->Id == OpId_OpenSqBracket)
        {
            ++State->StartToken;
            token* EndToken = FindMatchingOpenSq(State->StartToken, LastToken);
            if (!EndToken)
            {
                OutputError("Error: Couldn't find matching close bracket.\n");
            }

            syntax_node* IndexNode = ParseExpr(State, EndToken);
            State->StartToken = EndToken + 1;

            Result = CreateSyntaxNode(2);
            Result->OperatorId = OpId_GetArrayIndex;
            Result->Children[0] = Term;
            Result->Children[1] = IndexNode;
            
            LoopState = 0;
        }
        else
        {
            break;
        }
    }

    return Result;
}

inline syntax_node* TryParseTerm(parser_state* State, token* LastToken)
{
    b32 FoundTerm = false;
    syntax_node* Result = 0;
    syntax_node** CurrNode = &Result;

    while ((!FoundTerm) && State->StartToken != LastToken)
    {
        if (State->StartToken->Id == OpId_AddressOff || State->StartToken->Id == OpId_Mul ||
            State->StartToken->Id == OpId_Sub)
        {
            u32 OpId = State->StartToken->Id;
            if (OpId == OpId_Mul) {OpId = OpId_Dereference;}
            if (OpId == OpId_Sub) {OpId = OpId_Negate;}

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
            (*CurrNode)->OperatorId = OpId_FuncCall;
            (*CurrNode)->Func = Func;

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

                ++CurrToken;
            }

            FoundTerm = true;
        }
        else if (State->StartToken->Id == TokenId_Var)
        {
            *CurrNode = CreateSyntaxNode(0);
            (*CurrNode)->OperatorId = TokenId_Var;
            (*CurrNode)->Token = *State->StartToken;
            ++State->StartToken;
            *CurrNode = ParseMemberAndArrayAccessing(State, LastToken, *CurrNode);
            --State->StartToken;
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

    Result = ParseMemberAndArrayAccessing(State, LastToken, Result);
    
    return Result;
}

internal syntax_node* TryParseMulDivExpr(parser_state* State, token* LastToken,
                                             syntax_node* FirstTerm)
{
    if (State->StartToken >= LastToken)
    {
        OutputError("Error: Expression is empty.\n");
    }
    
    syntax_node* Result = FirstTerm;
    while (State->StartToken != LastToken)
    {
        if (State->StartToken->Id == OpId_Mul || State->StartToken->Id == OpId_Div)
        {
            u32 OpId = State->StartToken->Id;
            ++State->StartToken;

            syntax_node* NewResult = CreateSyntaxNode(2);
            NewResult->OperatorId = OpId;
            NewResult->Children[0] = Result;

            syntax_node* TermNode = TryParseTerm(State, LastToken);
            if (State->StartToken != State->LastToken &&
                IsTokenBinOp(*State->StartToken) && State->StartToken->Id > OpId)
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

internal syntax_node* TryParseAddSubExpr(parser_state* State, token* LastToken ,
                                             syntax_node* FirstTerm)
{
    if (State->StartToken >= LastToken)
    {
        OutputError("Error: Expression is empty.\n");
    }
    
    syntax_node* Result = FirstTerm;
    while (State->StartToken != LastToken)
    {
        if (State->StartToken->Id == OpId_Add || State->StartToken->Id == OpId_Sub)
        {
            u32 OpId = State->StartToken->Id;
            ++State->StartToken;

            syntax_node* NewResult = CreateSyntaxNode(2);
            NewResult->OperatorId = OpId;
            NewResult->Children[0] = Result;

            syntax_node* TermNode = TryParseTerm(State, LastToken);
            if (State->StartToken != State->LastToken &&
                IsTokenBinOp(*State->StartToken) && State->StartToken->Id > OpId)
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

internal syntax_node* ParseExpr(parser_state* State, token* LastToken, syntax_node* FirstTerm)
{
    if (!LastToken)
    {
        LastToken = State->LastToken;
    }
    
    if (State->StartToken >= LastToken)
    {
        OutputError("Error: Expression is empty.\n");
    }

    syntax_node* Result = 0;
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

            syntax_node* NewResult = CreateSyntaxNode(2);
            NewResult->OperatorId = OpId;
            NewResult->Children[0] = Result;

            syntax_node* TermNode = TryParseTerm(State, LastToken);
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

internal syntax_node* ParseEqualExpr(parser_state* State)
{
    b32 EncounteredEquals = false;
    syntax_node* Result = TryParseTerm(State, State->LastToken);
    while (State->StartToken != State->LastToken)
    {
        if (!EncounteredEquals && State->StartToken->Id == OpId_Equal)
        {
            u32 OpId = State->StartToken->Id;
            ++State->StartToken;

            syntax_node* NewResult = CreateSyntaxNode(2);
            NewResult->OperatorId = OpId;
            NewResult->Children[0] = Result;

            syntax_node* TermNode = TryParseTerm(State, State->LastToken);
            if (State->StartToken != State->LastToken &&
                IsTokenBinOp(*State->StartToken) && State->StartToken->Id > OpId)
            {
                NewResult->Children[1] = ParseExpr(State, State->LastToken, TermNode);
            }
            else
            {
                NewResult->Children[1] = TermNode;
            }
            
            Result = NewResult;
            EncounteredEquals = true;
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

enum ParseDecleration
{
    VarDefn_Allowed = 0x1,
    VarDecl_Allowed = 0x2,
    FuncDecl_Allowed = 0x4,
    AddToStruct = 0x8,
};

inline type* ParseType(parser_state* State, u32 Flags)
{
    // NOTE: This function is only for parsing types defined in variable definitions
    type* Result = 0;
    if (State->StartToken->Id == TokenId_Type)
    {
        Result = State->StartToken->Type;
    }
    else if (State->StartToken->Id == ScannerId_Identifier)
    {
        Result = IsTypeDefined(State->StartToken->Str);
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

        if (State->StartToken->Id == OpId_OpenSqBracket)
        {
            ++State->StartToken;
            if (State->StartToken->Id != OpId_CloseSqBracket)
            {
                OutputError("Error: Mismatched open/close square brackets.\n");
            }
            ++State->StartToken;
            ++NumRef;
        }

        type* TempType = Result;
        if (NumRef > 0)
        {
            Result = PushStruct(&ScopeArena, type);
            *Result = {};
            Result->Flags |= TypeFlag_IsTypePointer;
            Result->NumRef = NumRef;
            Result->PointerType = TempType;

            if ((Flags & AddToStruct) != 0)
            {
                // NOTE: We need to store a ref to this member in the struct we are definining
                type** TypeRef = PushStruct(&TypeArena, type*);
                *TypeRef = Result;
            }
        }
        else
        {
            if ((Flags & AddToStruct) != 0)
            {
                // NOTE: We need to store a ref to this member in the struct we are definining
                type** TypeRef = PushStruct(&TypeArena, type*);
                *TypeRef = Result;
            }
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
    
    syntax_node* ExprTree = ParseExpr(State, CurrNode, 0);
    token BoolExpr = SyntaxTreeToIR(ExprTree, State->OutFile);
    ++State->StartToken;
    
    if (GetTokenType(BoolExpr) != GetBasicType(TypeId_Int))
    {
        OutputError("Error: Expression inside if statement must evaluate to an int.\n");
    }

    return BoolExpr;
}

inline b32 TryParseGeneralStatement(parser_state* State)
{    
    syntax_node* ExprTree = ParseEqualExpr(State);
    SyntaxTreeToIR(ExprTree, State->OutFile);
    Assert(State->StartToken == State->LastToken);
            
    return true;
}

inline b32 TryParseTypeStatement(u32 Flags, parser_state* State)
{
    if (State->StartToken->Id == TokenId_Type)
    {
        type* Type = ParseType(State, Flags);
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
            if (IsTokenConstant(*State->StartToken))
            {
                OutputError("Error: Variable name cannot be a constant.\n");
            }
                
            // NOTE: Define after we evaluate LHS and RHS
            Assert(State->StartToken == State->LastToken);

            if ((Flags & AddToStruct) != 0)
            {
                // NOTE: We are in the scope of a struct so add the var to our struct instead
                // NOTE: ParseType already adds the type onto our type arena as required
                string* MemberStr = PushStruct(&TypeArena, string);
                *MemberStr = NameStr;
                ++State->StructType->NumMembers;
            }
            else
            {
                var* VarName = DefineVarInScope(State, NameStr, Type);
                PushDefineInstr(VarName->Type, VarName->Str);
            }
                
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
                    PrevType = ParseType(State, Flags);
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
                    if (IsTokenConstant(*State->StartToken))
                    {
                        OutputError("Error: Variable name cannot be a constant.\n");
                    }
                    if (State->StartToken->Id != ScannerId_Identifier)
                    {
                        OutputError("Error: Expected argument name after type for function decleration.\n");
                    }

                    var* Arg = DefineVarInScope(State, State->StartToken->Str, PrevType);
                    PushDefineInstr(PrevType, Arg->Str);
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

            PushFuncDeclInstr(Func);
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
            if (IsTokenConstant(*State->StartToken))
            {
                OutputError("Error: Variable name cannot be a constant.\n");
            }

            ++State->StartToken;

            syntax_node* ExprTree = ParseExpr(State);
            token RHS = SyntaxTreeToIR(ExprTree, State->OutFile);
            
            // NOTE: Define after we evaluate LHS and RHS
            var* VarName = DefineVarInScope(State, NameStr, Type);
            PushDefineInstr(VarName->Type, VarName->Str);
            if (!AreSameType(VarName->Type, GetTokenType(RHS)))
            {
                OutputError("Error: Type on LHS and RHS don't match.\n");
            }
            
            Assert(State->StartToken == State->LastToken);            
            string RHS_Str = GetTokenString(RHS);
            PushAssignInstr(VarName->Str, RHS_Str);
            
            return true;
        }
        else
        {
            OutputError("Error: Variable definition followed by unsupported operations\n");
        }
                                     
        return false;
    }
    
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

    if (State->StartToken->Id != ScannerId_Identifier)
    {
        OutputError("Error: Expected a name for struct.\n");
    }
    
    type* StructType = PushStruct(&TypeArena, type);
    *StructType = {};
    StructType->Flags |= TypeFlag_IsTypeStruct;
    StructType->Str = GetTokenString(*State->StartToken);
    StructType->Members = (u8*)(StructType + 1);
    State->StructType = StructType;
    
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

    // NOTE: We free the scope we allocated
    temp_mem ScopeTempMem = State->Scope->TempMem;
    State->Scope = State->Scope->PrevScope;
    ClearTempMem(ScopeTempMem);

    PushScopeInstr(IR_EndScope);
    State->StructType = 0;
    
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

    // NOTE: TryParseStatement is always the last statement since its most general and we first
    // want to make sure we don't have a if or other kind of statement
    if (State->Scope->Id == ScopeId_Struct)
    {
        if (TryParseCloseScope(State))
        {
            return State;
        }
        if (TryParseTypeStatement(VarDefn_Allowed | AddToStruct, State))
        {
            return State;
        }
        
        OutputError("");
    }
    else if (State->Scope->Id == ScopeId_Global)
    {
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
        if (TryParseTypeStatement(VarDefn_Allowed | VarDecl_Allowed | FuncDecl_Allowed, State))
        {
            return State;
        }
        
        OutputError("");
    }
    else if (IsScopeCodeBlock(State->Scope->Id))
    {
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
        if (TryParseTypeStatement(VarDefn_Allowed | VarDecl_Allowed | FuncDecl_Allowed, State))
        {
            return State;
        }
        if (TryParseGeneralStatement(State))
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
