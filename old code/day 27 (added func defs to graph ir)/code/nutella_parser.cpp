
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
    b32 WasPrevIndex = false;
    while (State->StartToken != LastToken)
    {
        if (State->StartToken->Id == OpId_GetMember)
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

            syntax_node* NewResult = CreateSyntaxNode(NodeId_GetMember, 2);
            NewResult->Children[0] = Result;
            NewResult->Children[1] = CreateSyntaxNode(NodeId_Var, 0);
            NewResult->Children[1]->Token = *State->StartToken;
            Result = NewResult;
            
            ++State->StartToken;
            WasPrevIndex = false;
        }
        else if ((!WasPrevIndex) && State->StartToken->Id == OpId_OpenSqBracket)
        {
            ++State->StartToken;
            token* EndToken = FindMatchingOpenSq(State->StartToken, LastToken);
            if (!EndToken)
            {
                OutputError("Error: Couldn't find matching close bracket.\n");
            }

            syntax_node* IndexNode = ParseExpr(State, EndToken);
            State->StartToken = EndToken + 1;

            syntax_node* NewResult = CreateSyntaxNode(NodeId_GetArrayIndex, 2);
            NewResult->Children[0] = Result;
            NewResult->Children[1] = IndexNode;
            Result = NewResult;
            
            WasPrevIndex = true;
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
            if (OpId == OpId_Mul) {OpId = NodeId_Dereference;}
            if (OpId == OpId_Sub) {OpId = NodeId_Negate;}

            *CurrNode = CreateSyntaxNode(OpId, 1);
            CurrNode = &((*CurrNode)->Children[0]);
        }
        else if (State->StartToken->Id == OpId_Run)
        {
            ++State->StartToken;

            *CurrNode = CreateSyntaxNode(NodeId_Run, 1);
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

            *CurrNode = CreateSyntaxNode(NodeId_FuncCall, Func->NumArgs);
            (*CurrNode)->Func = Func;

            // NOTE: Parse the arguments in our brackets
            token* EndBracketToken = FindMatchingOpen(State->StartToken, LastToken);
            if (!EndBracketToken)
            {
                OutputError("Error: Mismatched '(' brackets.\n");
            }

            u32 ArgIndex = 0;
            token* CurrToken = State->StartToken;
            while (CurrToken != EndBracketToken)
            {
                if (CurrToken->Id == OpId_OpenBracket)
                {
                    ++CurrToken;
                    CurrToken = FindMatchingOpen(CurrToken, EndBracketToken);
                    if (!CurrToken)
                    {
                        OutputError("Error: Mismatched brackets.\n");
                    }
                }
                else if (CurrToken->Id == ScannerId_Comma)
                {
                    (*CurrNode)->Children[ArgIndex++] = ParseExpr(State, CurrToken);
                    ++State->StartToken;
                }
                
                ++CurrToken;

                if (CurrToken == EndBracketToken)
                {                    
                    (*CurrNode)->Children[ArgIndex++] = ParseExpr(State, CurrToken);
                }
            }

            if (ArgIndex < Func->NumArgs)
            {
                OutputError("Error: Not enough arguments provided for function call.\n");
            }
            if (ArgIndex > Func->NumArgs)
            {
                OutputError("Error: Too many arguments provided for function call.\n");
            }

            FoundTerm = true;
        }
        else if (State->StartToken->Id == TokenId_Var)
        {
            *CurrNode = CreateSyntaxNode(NodeId_Var, 0);
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

            syntax_node* NewResult = CreateSyntaxNode(OpId, 2);
            NewResult->Children[0] = Result;

            syntax_node* TermNode = TryParseTerm(State, LastToken);
            if (State->StartToken != State->LastToken &&
                IsTokenBinOp(*State->StartToken) && State->StartToken->Id > OpId_Div)
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
            // NOTE: Make sure that the operator is handled by a higher level rule
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

            syntax_node* NewResult = CreateSyntaxNode(OpId, 2);
            NewResult->Children[0] = Result;

            syntax_node* TermNode = TryParseTerm(State, LastToken);
            if (State->StartToken != State->LastToken &&
                IsTokenBinOp(*State->StartToken) && State->StartToken->Id > OpId_Sub)
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
            // NOTE: We need to use either a higher or lower level rule to parse the
            // next statement
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

            syntax_node* NewResult = CreateSyntaxNode(OpId, 2);
            NewResult->Children[0] = Result;

            syntax_node* TermNode = TryParseTerm(State, LastToken);
            if (IsTokenBinOp(*State->StartToken) || State->StartToken->Id > OpId_GreaterEqual)
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

            syntax_node* NewResult = CreateSyntaxNode(OpId, 2);
            NewResult->Children[0] = Result;

            syntax_node* TermNode = TryParseTerm(State, State->LastToken);
            if (State->StartToken != State->LastToken &&
                IsTokenBinOp(*State->StartToken) && State->StartToken->Id > OpId_Equal)
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
    VarDefn_Allowed = 1 << 0,
    VarDecl_Allowed = 1 << 1,
    FuncDecl_Allowed = 1 << 2,
    AddToStruct = 1 << 3,
    AllowReturn = 1 << 4,
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
            // NOTE: This is where we create all our ptr types
            Result = PushStruct(&PtrTypeArena, type);
            Result->Flags |= TypeFlag_IsTypePointer;
            Result->NumRef = NumRef;
            Result->PointerType = TempType;

            if (Flags & AddToStruct)
            {
                // NOTE: We need to store a ref to this member in the struct we are definining
                type** TypeRef = PushStruct(&TypeArena, type*);
                *TypeRef = Result;
            }
        }
        else
        {
            if (Flags & AddToStruct)
            {
                // NOTE: We need to store a ref to this member in the struct we are definining
                type** TypeRef = PushStruct(&TypeArena, type*);
                *TypeRef = Result;
            }
        }
    }

    return Result;
}

inline syntax_node* EvaluateBoolExpr(parser_state* State)
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
    ++State->StartToken;

    return ExprTree;
}

inline b32 TryParseGeneralStatement(parser_state* State)
{
    syntax_node* ExprTree = ParseEqualExpr(State);
    SyntaxTreeToIR(State, ExprTree);

    State->IsPrevIf = false;
    Assert(State->StartToken == State->LastToken);
    return true;
}

inline b32 TryParseTypeStatement(u32 Flags, parser_state* State)
{
    if (State->StartToken->Id == TokenId_Type)
    {
        type* Type = ParseType(State, Flags);
        token* NameToken = State->StartToken;
        ++State->StartToken;

        if (Flags & VarDefn_Allowed && State->StartToken->Id == ScannerId_EndLine)
        {
            // NOTE: Here we know we are looking at a variable definition
            if (NameToken->Id == TokenId_Var)
            {
                OutputError("Error: Variable redefined.\n");
            }
            if (IsTokenConstant(*NameToken))
            {
                OutputError("Error: Variable name cannot be a constant.\n");
            }
            if (Type == GetBasicType(TypeId_Void))
            {
                OutputError("Error: Cannot define a variable to be of type void.\n");
            }
            if (NameToken->Id != ScannerId_Identifier)
            {
                OutputError("Error: Variable name either already in use or contains invalid characters.\n");
            }
            string NameStr = GetTokenString(*NameToken);
            
            // NOTE: Here we know we have a variable definition
            if ((Flags & AddToStruct) != 0)
            {
                // NOTE: We are in the scope of a struct so add the var to our struct instead
                // NOTE: ParseType already adds the type onto our type arena as required
                string* MemberStr = PushStruct(&TypeArena, string);
                *MemberStr = NameStr;
                ++State->Scope->StructType->NumMembers;
                State->Scope->StructType->SizeInBytes += Type->SizeInBytes;
            }
            else
            {
                syntax_node* VarNode = CreateSyntaxNode(NodeId_Var, 0);
                VarNode->Token.Id = TokenId_Var;
                VarNode->Token.Var = DefineVarInScope(State, NameStr, Type);
                
                syntax_node* DefineNode = CreateSyntaxNode(NodeId_Define, 1);
                DefineNode->Children[0] = VarNode;

                SyntaxTreeToIR(State, DefineNode);
                
                syntax_node* SetTo0Node = CreateSyntaxNode(NodeId_Zero, 1);
                SetTo0Node->Children[0] = VarNode;

                SyntaxTreeToIR(State, SetTo0Node);
            }

            State->IsPrevIf = false;
            Assert(State->StartToken == State->LastToken);                        
            return true;
        }
        else if (Flags & FuncDecl_Allowed && State->StartToken->Id == OpId_OpenBracket)
        {
            // NOTE: Here we know we are looking at a function decleration
            string NameStr = GetTokenString(*NameToken);
            if (NameToken->Id != ScannerId_Identifier)
            {
                OutputError("Error: Variable name either already in use or contains invalid characters.\n");
            }
            function* Func = IsFuncDefined(NameStr);
            if (Func)
            {
                OutputError("Error: Function redefined.\n");
            }
            Func = CreateFunction(NameStr, Type);
            ++State->StartToken;

            // NOTE: Parse the arguments in our brackets
            token* EndBracketToken = FindMatchingOpen(State->StartToken, State->LastToken);
            if (!EndBracketToken)
            {
                OutputError("Error: Mismatched '(' brackets.\n");
            }

            // NOTE: Create our start node for the function
            Func->StartNode = CreateSyntaxNode(NodeId_FuncDef, 1);
            Func->StartNode->Func = Func;
            SyntaxTreeToIR(State, Func->StartNode);
            State->Scope->ExitCodeBlock = &Func->StartNode->Next;
            AddScope(State, ScopeId_Function);
            State->Scope->StartCodeBlock = &Func->StartNode->Children[0];
            
            var* ArgArray = (var*)(ScopeArena.Mem + ScopeArena.Used);
            type* PrevType = 0;
            while (State->StartToken != EndBracketToken)
            {
                type* ArgType = ParseType(State, Flags);
                if (!ArgType)
                {
                    OutputError("Error: Expected a type for a argument definition.\n");
                }
                AddArgToFunction(Func, ArgType);
                
                if (State->StartToken == EndBracketToken)
                {
                    OutputError("Error: Missing argument name.\n");
                }
                else if (State->StartToken->Id == TokenId_Var)
                {
                    OutputError("Error: Argument name already in use.\n");
                }

                var* Arg = DefineVarInScope(State, GetTokenString(*State->StartToken), ArgType);
                ++State->StartToken;

                syntax_node* VarNode = CreateSyntaxNode(NodeId_Var, 0);
                VarNode->Token.Id = TokenId_Var;
                VarNode->Token.Var = Arg;
                
                syntax_node* DefineNode = CreateSyntaxNode(NodeId_Define, 1);
                DefineNode->Children[0] = VarNode;

                SyntaxTreeToIR(State, DefineNode);
                
                if (State->StartToken == EndBracketToken)
                {
                    break;
                }
                if (State->StartToken->Id != ScannerId_Comma)
                {
                    OutputError("Expected a comma after argument definition.\n");
                }

                ++State->StartToken;
            }
            
            ++State->StartToken;
            State->IsPrevIf = false;
            ++State->StartToken;
            Assert(State->StartToken == State->LastToken);
            
            return true;
        }        
        else if (Flags & VarDecl_Allowed && State->StartToken->Id == OpId_Equal)
        {
            // NOTE: Here we know we are looking at a variable decleration
            if (NameToken->Id == TokenId_Var)
            {
                OutputError("Error: Variable redefined.\n");
            }
            if (IsTokenConstant(*NameToken))
            {
                OutputError("Error: Variable name cannot be a constant.\n");
            }
            if (Type == GetBasicType(TypeId_Void))
            {
                OutputError("Error: Cannot define a variable to be of type void.\n");
            }
            if (NameToken->Id != ScannerId_Identifier)
            {
                OutputError("Error: Variable name either already in use or contains invalid characters.\n");
            }            
            
            string NameStr = GetTokenString(*NameToken);
            ++State->StartToken;
                    
            syntax_node* ExprTree = ParseExpr(State);

            syntax_node* VarNode = CreateSyntaxNode(NodeId_Var, 0);
            VarNode->Token.Id = TokenId_Var;
            VarNode->Token.Var = DefineVarInScope(State, NameStr, Type);
            
            syntax_node* DefineNode = CreateSyntaxNode(NodeId_Define, 1);
            DefineNode->Children[0] = VarNode;
            
            syntax_node* EqualNode = CreateSyntaxNode(NodeId_Equal, 2);
            EqualNode->Children[0] = DefineNode;
            EqualNode->Children[1] = ExprTree;

            SyntaxTreeToIR(State, EqualNode);
            State->IsPrevIf = false;
            Assert(State->StartToken == State->LastToken);            

            return true;
        }
        else
        {
            OutputError("Error: Variable definition followed by unsupported operations\n");
        }
                                     
        return false;
    }

    if ((Flags & AllowReturn) != 0 && State->StartToken->Id == ScannerId_Return)
    {
        ++State->StartToken;

        // NOTE: Here we handle return statements. We allow empty return statements.
        syntax_node* ReturnNode = 0;
        if (State->StartToken == State->LastToken)
        {        
            ReturnNode = CreateSyntaxNode(NodeId_Return, 0);            
        }
        else
        {        
            ReturnNode = CreateSyntaxNode(NodeId_Return, 1);
            ReturnNode->Children[0] = ParseExpr(State);
        }

        State->IsPrevIf = false;
        //SyntaxTreeToIR(ReturnNode, State->Scope->Func);            
        Assert(State->StartToken == State->LastToken);            

        return true;
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

    syntax_node* IfNode = CreateSyntaxNode(NodeId_If, 2);
    IfNode->Children[0] = EvaluateBoolExpr(State);
    SyntaxTreeToIR(State, IfNode);
    
    if (State->StartToken->Id != ScannerId_OpenScope)
    {
        OutputError("Error: If statement must be followed by a scope.\n");
    }

    State->Scope->ExitCodeBlock = &IfNode->Next;
    scope* Scope = AddScope(State, ScopeId_If);
    Scope->StartCodeBlock = &IfNode->Children[1];
    
    State->IsPrevIf = false;
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

        syntax_node* ElseIfNode = CreateSyntaxNode(NodeId_ElseIf, 2);
        ElseIfNode->Children[0] = EvaluateBoolExpr(State);
        SyntaxTreeToIR(State, ElseIfNode);
    
        if (State->StartToken->Id != ScannerId_OpenScope)
        {
            OutputError("Error: Else If statement must be followed by a scope.\n");
        }

        State->Scope->ExitCodeBlock = &ElseIfNode->Next;
        scope* Scope = AddScope(State, ScopeId_ElseIf);
        Scope->StartCodeBlock = &ElseIfNode->Children[1];
    }
    else
    {
        syntax_node* ElseNode = CreateSyntaxNode(NodeId_Else, 1);
        SyntaxTreeToIR(State, ElseNode);
    
        if (State->StartToken->Id != ScannerId_OpenScope)
        {
            OutputError("Error: Else If statement must be followed by a scope.\n");
        }

        State->Scope->ExitCodeBlock = &ElseNode->Next;
        scope* Scope = AddScope(State, ScopeId_Else);
        Scope->StartCodeBlock = &ElseNode->Children[0];
    }

    if (!(State->StartToken->Id == ScannerId_OpenScope))
    {
        OutputError("Error: Else/Else If statement must be followed by a scope.\n");
    }

    State->IsPrevIf = false;
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

    syntax_node* WhileNode = CreateSyntaxNode(NodeId_While, 3);
    WhileNode->Children[0] = EvaluateBoolExpr(State);
    SyntaxTreeToIR(State, WhileNode);
    
    if (State->StartToken->Id != ScannerId_OpenScope)
    {
        OutputError("Error: While statement must be followed by a scope.\n");
    }

    State->Scope->ExitCodeBlock = &WhileNode->Children[2];
    scope* Scope = AddScope(State, ScopeId_WhileLoop);
    Scope->StartCodeBlock = &WhileNode->Children[1];
    
    State->IsPrevIf = false;
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

    if (State->StartToken->Id == TokenId_Type)
    {
        OutputError("Error: Struct name already exists.\n");
    }
    
    type* StructType = PushStruct(&TypeArena, type);
    StructType->Flags |= TypeFlag_IsTypeStruct;
    StructType->Str = GetTokenString(*State->StartToken);
    StructType->Members = (u8*)(StructType + 1);
    
    ++State->StartToken;
    
    if (!(State->StartToken->Id == ScannerId_OpenScope))
    {
        OutputError("Error: Struct statement must be followed by a scope.\n");
    }

    AddScope(State, ScopeId_Struct);
    
    // NOTE: We set this value so that we can define the members of our struct
    State->Scope->StructType = StructType;
    State->IsPrevIf = false;
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
    State->IsPrevIf = false;
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

    if (State->Scope->Id == ScopeId_If || State->Scope->Id == ScopeId_ElseIf ||
        State->Scope->Id == ScopeId_Else)
    {
        State->IsPrevIf = true;
    }
    else if (State->Scope->Id == ScopeId_Function)
    {
        State->PrevNode = State->Scope->Func->StartNode;
    }
    
    // NOTE: We free the scope we allocated
    temp_mem ScopeTempMem = State->Scope->TempMem;
    State->Scope = State->Scope->PrevScope;
    ClearTempMem(ScopeTempMem);    
            
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
        
        OutputError("Error: Encountered unrecognized expression.\n");
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
        
        OutputError("Error: Encountered unrecognized expression.\n");
    }
    else if (IsScopeCodeBlock(State->Scope->Id))
    {
        if (TryParseIf(State))
        {
            return State;
        }
        if (State->IsPrevIf && TryParseElse(State))
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
        if (TryParseTypeStatement(VarDefn_Allowed | VarDecl_Allowed | FuncDecl_Allowed | AllowReturn,
                                  State))
        {
            return State;
        }
        if (TryParseGeneralStatement(State))
        {
            return State;
        }

        OutputError("Error: Encountered unrecognized expression.\n");
    }
    else
    {
        InvalidCodePath;
    }
    
    // NOTE: This never happens, VS just complains about not all paths returning tho
    return 0;
}
