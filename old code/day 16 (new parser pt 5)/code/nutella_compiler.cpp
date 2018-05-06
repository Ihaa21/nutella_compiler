
#include "nutella_compiler.h"

// TODO: We need a better way to specify this
global type* BasicTypes[4] = {};

// TODO: Put this into a global state or maybe its okay to keep here?
global mem_arena VarArena;
global mem_arena TypeArena;
global mem_arena FuncArena;
global mem_arena ScopeArena;
global mem_arena SyntaxTreeArena;
global mem_arena TokenArena;
global mem_arena IR_Arena;
global mem_arena StringArena;

#include "nutella_string.cpp"
#include "nutella_linked_list.cpp"

/*
  TODO List:

      - Make the ir codes for if statements. Then make while loop ir and parsing. Then add function
        parsing and for this, we need to make sure every statement we write that isn't a defintion or
        constant (we can check for these now!!) are inside function scopes. Maybe have a global bool
        saying that we currently are in a function scope. Add negation
  
      - Figure out memory management since currently we are leaking and using malloc all over the
        place. Probably better to define arenas which we grow as we need memory for different parts
        of our program.

      - We will need to make a function struct to define types for arguments and return type as well as
        the proper ir code to jump to the functions location when called and to return back after we
        finish calling the function. Translating to C kinda falls apart here unless we precode it to
        just have C like function ir codes. Maybe worth.
      
      - We can have a int for each type and var that says which numbered scope that var is from. Then,
        when we check if a var is defined in a given scope, we just check if the num scope of that var
        is less than or equal to our current scope. We would have to remove all variables and types
        from the list of a given number once we evict that scope to not cause scope num collisions. Take
        a look at how useful this optimization would actually be.

      - For memory, make linked lists of 4 meg or 1 meg blocks of mem where in that block, its just a
        flat array. For other things like types, we can resizeable arrays?

      - Check that all code paths (IsVar/TypeDefined calls, etc) have a error returned if the thing
        searched for isn't found.

 */

#define NumElementsBetweenIndexes(Ptr1, Ptr2) (u32)((Ptr1) - (Ptr2))

inline void OutputError(char* ErrorMsg)
{
    printf(ErrorMsg);
    exit(0);
}

inline type* GetBasicType(u32 Id)
{
    Assert(Id > TypeId_StartTypes && Id < TypeId_EndTypes);
    type* Result = BasicTypes[Id - TypeId_StartTypes - 1];
    return Result;
}

inline b32 IsTokenConstant(token Token)
{
    b32 Result = ((Token.Id > TypeId_StartTypes && Token.Id < TypeId_EndTypes) ||
                  (Token.Id == TokenId_ConstVar));

    return Result;
}

inline b32 IsTokenBinOp(token Token)
{
    b32 Result = Token.Id > OpId_StartBinOps && Token.Id < OpId_EndBinOps;

    return Result;
}

inline b32 IsTypePointer(type* Type)
{
    b32 Result = Type->NumRef > 0;
    return Result;
}

inline b32 IsScopeCodeBlock(u32 Id)
{
    b32 Result = (Id < ScopeId_EndCodeScope && Id > ScopeId_StartCodeScope);
    return Result;
}

#include "nutella_backend.cpp"
// TODO: Remove
#include "oldcode.cpp"

inline type* GetTokenType(token Token)
{
    type* Result = 0;

    if (Token.Id == TokenId_Var)
    {
        Result = Token.Var->Type;
    }
    else if (Token.Id == TokenId_ConstVar)
    {
        Result = Token.Var->Type;
    }
    else if (Token.Id == TokenId_Type)
    {
        // TODO: Should we maybe spit a warning during debugging for this?
        Result = Token.Type;
    }
    else if (IsTokenConstant(Token))
    {
        Result = GetBasicType(Token.Id);
    }
    else
    {
        InvalidCodePath;
    }

    return Result;
}

inline string GetTokenString(token Token)
{
    string Result = {};

    if (Token.Id == TokenId_Var || Token.Id == TokenId_ConstVar)
    {
        Result = Token.Var->Str;
    }
    else if (Token.Id == TokenId_Type) // TODO: When does this happen?
    {
        Result = Token.Type->Str;
    }
    else if (IsTokenConstant(Token))
    {
        Result = Token.Str;
    }
    else if (Token.Id == ScannerId_Identifier)
    {
        Result = Token.Str;
    }
    else
    {
        InvalidCodePath;
    }

    return Result;
}

internal type* IsTypeDefined(parser_state* State, string MatchStr)
{
    type* Result = 0;
    
    scope* CurrScope = State->Scope;
    while (CurrScope)
    {        
        linked_list_node* CurrNode = GetFirstNode(&CurrScope->TypeList);
        type* CurrType = 0;
        while (CurrNode)
        {
            CurrType = (type*)CurrNode->Data;
            if (StringCompare(CurrType->Str, MatchStr))
            {
                Result = CurrType;
                break;
            }

            CurrNode = CurrNode->Next;
        }

        if (Result)
        {
            break;
        }
        
        CurrScope = CurrScope->PrevScope;
    }

    return Result;
}

internal var* IsVarDefined(parser_state* State, string MatchStr)
{
    var* Result = 0;
    
    scope* CurrScope = State->Scope;
    while (CurrScope)
    {        
        linked_list_node* CurrNode = GetFirstNode(&CurrScope->VarList);
        var* CurrVar = 0;
        while (CurrNode)
        {
            CurrVar = (var*)CurrNode->Data;
            if (StringCompare(CurrVar->Str, MatchStr))
            {
                Result = CurrVar;
                break;
            }

            CurrNode = CurrNode->Next;
        }

        if (Result)
        {
            break;
        }
        
        CurrScope = CurrScope->PrevScope;
    }

    return Result;
}

inline var* DefineVarInScope(parser_state* State, string Name, type* Type)
{
    var* NewVar = PushStruct(&VarArena, var);
    NewVar->Str = Name;
    NewVar->Type = Type;
    AddNodeToList(&State->Scope->VarList, NewVar);

    return NewVar;
}

inline function* IsFuncDefined(string NameStr)
{
    function* Result = 0;
    
    u8* CurrByte = FuncArena.Mem;
    while (CurrByte < (FuncArena.Mem + FuncArena.Used))
    {
        function* CurrFunc = (function*)CurrByte;
        if (StringCompare(CurrFunc->Name, NameStr))
        {
            Result = CurrFunc;
            break;
        }

        CurrByte += sizeof(type*)*CurrFunc->NumArgs;
    }

    return Result;
}

inline function* CreateFunction(string NameStr, type* ReturnType)
{
    function* Result = PushStruct(&FuncArena, function);
    Result->ReturnType = ReturnType;
    Result->Name = NameStr;
    Result->NumArgs = 0;
    Result->ArgTypes = (type**)(Result + 1);

    return Result;
}

inline void AddArgToFunction(function* Func, type* ArgType)
{
    Func->NumArgs += 1;
    type** NewArg = PushStruct(&FuncArena, type*);
    *NewArg = ArgType;
}

inline b32 AreSameType(var Var, token Token)
{
    // NOTE: We guarentee that tokens of same type share the same type pointer if
    // they aren't pointer types. Pointer types will have the same PointerType but we must check
    // NumRef first. We don't allow pointertype to point to another pointer type.
    type* VarType = Var.Type;
    type* TokenType = GetTokenType(Token);
    b32 Result = ((VarType->NumRef == TokenType->NumRef) &&
                  ((VarType->NumRef > 0 && VarType->PointerType == TokenType->PointerType) ||
                   (VarType->NumRef == 0 && VarType == TokenType)));

    Assert(VarType->NumRef == 0 || (VarType->NumRef > 0 && VarType->PointerType->NumRef == 0));
    Assert(TokenType->NumRef == 0 || (TokenType->NumRef > 0 && TokenType->PointerType->NumRef == 0));
    
    return Result;
}

inline void AddScope(parser_state* State, u32 ScopeId = ScopeId_None)
{
    temp_mem ScopeTempMem = BeginTempMem(&ScopeArena);
        
    scope* NewScope = PushStruct(&ScopeArena, scope);
    *NewScope = {};
    NewScope->TempMem = ScopeTempMem;
    NewScope->PrevScope = State->Scope;
    NewScope->Id = ScopeId;
    InitList(&NewScope->TypeList);
    InitList(&NewScope->VarList);

    State->Scope->NextScope = NewScope;
    State->Scope = NewScope;
}

// TODO: We can definintly merge this into our building tree function. Is it worth it?
internal void RelabelExpressionTokens(parser_state* State, token* Tokens, token* LastToken)
{
    token* Token = Tokens;
    while (Token != LastToken)
    {
        switch (Token->Id)
        {
            case ScannerId_Identifier:
            {
                type* Type = IsTypeDefined(State, Token->Str);
                if (Type)
                {
                    Token->Type = Type;
                    Token->Id = TokenId_Type;
                }
                else
                {
                    var* Var = IsVarDefined(State, Token->Str);
                    if (Var)
                    {
                        // TODO: We got a bug where a variable which has a pointer gets its typed
                        // modified which is incorrect
                        Token->Var = Var;
                        Token->Id = TokenId_Var;
                    }
                    else
                    {
                        OutputError("Error: Identifier not defined as a type or variable.\n");
                    }
                }
            } break;
        }

        ++Token;
    }
}

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

internal expression_node* TryParseAddSubExpr(token* CurrToken, token* LastToken);

inline expression_node* TryParseTerm(token* CurrToken, token* LastToken, token** FinishedToken)
{
    expression_node* Result = PushStruct(&SyntaxTreeArena, expression_node);
    *Result = {};

    b32 FoundTerm = false;
    expression_node* CurrNode = Result;
    while ((!FoundTerm) && CurrToken != LastToken)
    {
        // TODO: Add support for negate in ir
        if (CurrToken->Id == OpId_AddressOff || CurrToken->Id == OpId_Mul || CurrToken->Id == OpId_Sub)
        {
            u32 OpId = CurrToken->Id;
            if (OpId == OpId_Mul) {OpId = OpId_Dereference;}
            
            CurrNode->OperatorId = OpId;
            CurrNode->Right = PushStruct(&SyntaxTreeArena, expression_node);
            *CurrNode->Right = {};

            CurrNode = CurrNode->Right;
        }
        else if (CurrToken->Id == OpId_OpenBracket)
        {
            token* OpenBracket = CurrToken+1;            
            token* CloseBracket = FindMatchingOpen(OpenBracket, LastToken);
            if (!CloseBracket)
            {
                OutputError("Error: No matching open bracket found for closing bracket.\n");
            }

            // TODO: This currently leaks one node
            *CurrNode = *TryParseAddSubExpr(OpenBracket, CloseBracket);
            CurrToken = CloseBracket;
            FoundTerm = true;
        }
        else if (CurrToken->Id == OpId_CloseBracket)
        {
            OutputError("Error: Mismatched open/close brackets.\n");
        }
        else if (CurrToken->Id == TokenId_Var || IsTokenConstant(*CurrToken))
        {
            CurrNode->Token = *CurrToken;
            FoundTerm = true;
        }
        else
        {
            OutputError("Error: Expected variable, constant, or uniary operator.\n");
        }
        
        ++CurrToken;
    }

    if (!FoundTerm)
    {
        OutputError("Error: Expected to find a variable or constant but couldn't find one.\n");
    }
    
    *FinishedToken = CurrToken;
    return Result;
}
 
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

internal expression_node* TryParseMulDivExpr(token* CurrToken, token* LastToken,
                                             expression_node* FirstTerm, token** FinishedToken)
{
    if (CurrToken >= LastToken)
    {
        OutputError("Error: Expression is empty.\n");
    }
    
    expression_node* Result = FirstTerm;
    while (CurrToken != LastToken)
    {
        if (CurrToken->Id == OpId_Mul || CurrToken->Id == OpId_Div)
        {
            u32 OpId = CurrToken->Id;
            ++CurrToken;

            expression_node* NewResult = PushStruct(&SyntaxTreeArena, expression_node);
            *NewResult = {};
            NewResult->OperatorId = OpId;
            NewResult->Left = Result;

            expression_node* TermNode = TryParseTerm(CurrToken, LastToken, &CurrToken);
            if (CurrToken->Id > OpId)
            {
                // NOTE: We have a operator of higher priority so we need to evaluate it
                InvalidCodePath;
                //NewResult->Right = TryParseMulDivExpr(CurrToken, LastToken, &CurrToken);
            }
            else
            {
                NewResult->Right = TermNode;
            }
            
            Result = NewResult;            
        }
        else
        {
            // NOTE: We cannot parse anymore in this ruleset so we return here to the calling function
            break;
        }
    }
    
    *FinishedToken = CurrToken;
    return Result;
}

internal expression_node* TryParseAddSubExpr(token* CurrToken, token* LastToken)
{
    if (CurrToken >= LastToken)
    {
        OutputError("Error: Expression is empty.\n");
    }

    expression_node* Result = TryParseTerm(CurrToken, LastToken, &CurrToken);
    while (CurrToken != LastToken)
    {
        if (CurrToken->Id == OpId_Add || CurrToken->Id == OpId_Sub)
        {
            u32 OpId = CurrToken->Id;
            ++CurrToken;

            expression_node* NewResult = PushStruct(&SyntaxTreeArena, expression_node);
            *NewResult = {};
            NewResult->OperatorId = OpId;
            NewResult->Left = Result;

            expression_node* TermNode = TryParseTerm(CurrToken, LastToken, &CurrToken);
            if (CurrToken->Id > OpId)
            {
                // NOTE: We have a operator of higher priority so we need to evaluate it                
                NewResult->Right = TryParseMulDivExpr(CurrToken, LastToken, TermNode, &CurrToken);
            }
            else
            {
                NewResult->Right = TermNode;
            }
            
            Result = NewResult;
        }
        else if (IsTokenBinOp(*CurrToken))
        {
            // NOTE: We return here to the calling function
            Result = TryParseMulDivExpr(CurrToken, LastToken, Result, &CurrToken);
            //InvalidCodePath;
        }
        else
        {
            OutputError("Error: Encountered unexpected token.\n");
        }
    }

    return Result;
}


// TODO: Remove this!!
// TODO: Make a table which has variable names, types, and functions as they appear in the code
// and then a mapping to dummy names so that we can always be sure that we don't have name conflicts
global u32 UsedCount = 0;
global const char* VarNames[] = {"a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9"};

internal token ExpressionTreeToIR(expression_node* Root, FILE* OutCFile)
{
    // NOTE: We just have a constant or variable
    if (!Root->Left && !Root->Right)
    {
        return Root->Token;
    }
    Assert(Root->Left || Root->Right);

    // TODO: Assign values to our enums so that we don't need to manually reassign here
    u32 IR_OpId = 0;
    if (Root->OperatorId == OpId_Add)
    {
        IR_OpId = IR_Add;
    }
    else if (Root->OperatorId == OpId_Sub)
    {
        IR_OpId = IR_Sub;
    }
    else if (Root->OperatorId == OpId_Mul)
    {
        IR_OpId = IR_Mul;
    }
    else if (Root->OperatorId == OpId_Div)
    {
        IR_OpId = IR_Div;
    }
    else if (Root->OperatorId == OpId_BinaryEqual)
    {
        IR_OpId = IR_Equals;
    }
    else if (Root->OperatorId == OpId_BinaryNotEqual)
    {
        IR_OpId = IR_NotEquals;
    }
    else if (Root->OperatorId == OpId_Less)
    {
        IR_OpId = IR_Less;
    }
    else if (Root->OperatorId == OpId_LessEqual)
    {
        IR_OpId = IR_LessEqual;
    }
    else if (Root->OperatorId == OpId_Greater)
    {
        IR_OpId = IR_Greater;
    }
    else if (Root->OperatorId == OpId_GreaterEqual)
    {
        IR_OpId = IR_GreaterEqual;
    }
    else if (Root->OperatorId == OpId_AddressOff)
    {
        IR_OpId = IR_AddressOff;
    }
    else if (Root->OperatorId == OpId_Dereference)
    {
        IR_OpId = IR_Dereference;
    }
    else
    {
        OutputError("Error: Undefined operator in expression.\n");
    }

    // TODO: Add this variable to our var list? or make a seperate temp var list
    // or change the variable names of all our variables in ir?
    token ResultVarName = {};
    ResultVarName.Id = TokenId_Var;
    // TODO: This leaks right now
    // TODO: Assign the resulting type to this var once we add functions
    ResultVarName.Var = PushStruct(&VarArena, var);
    *ResultVarName.Var = {};
    ResultVarName.Var->Str = InitStr((char*)VarNames[UsedCount], 2);
    ++UsedCount;

    // TODO: Operators are just inlined functions with a diff way to call them. Figure out
    // how to somehow store or write functions for normal ops and then not have to hard code here
    if (IsOpUniary(IR_OpId))
    {
        token Right = ExpressionTreeToIR(Root->Right, OutCFile);
        type* TypeRight = GetTokenType(Right);
        string RightStr = GetTokenString(Right);

        switch (IR_OpId)
        {
            case IR_Dereference:
            {                
                if (IsTokenConstant(Right))
                {
                    OutputError("Error: Cannot take address of a constant.\n");
                }
        
                // NOTE: Create pointer type
                type* NewType = PushStruct(&TypeArena, type);
                *NewType = *TypeRight;
                {
                    if (TypeRight->NumRef == 0)
                    {
                        OutputError("Error: Cannot dereference non pointer value.\n");
                    }
                    else
                    {
                        NewType->NumRef -= 1;
                        if (NewType->NumRef == 0)
                        {
                            // TODO: Free the pointer type
                            NewType = NewType->PointerType;
                        }
                    }
                }

                ResultVarName.Var->Type = NewType;

            } break;

            case IR_AddressOff:
            {                
                if (IsTokenConstant(Right))
                {
                    OutputError("Error: Cannot take address of a constant.\n");
                }
        
                // NOTE: Create pointer type
                type* NewType = PushStruct(&TypeArena, type);
                *NewType = *TypeRight;
                {
                    if (NewType->NumRef == 0)
                    {
                        NewType = PushStruct(&TypeArena, type);
                        NewType->NumRef = 1;
                        NewType->PointerType = TypeRight;
                    }
                    else
                    {
                        NewType->NumRef += 1;
                    }
                }

                ResultVarName.Var->Type = NewType;
            } break;
        }
                
        PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
        PushUniOpInstr(IR_OpId, RightStr, ResultVarName.Var->Str);
    }
    else if (IsOpBinary(IR_OpId))
    {
        // TODO: Figure out pointer addition IR
        Assert(Root->Left && Root->Right);

        token Left = ExpressionTreeToIR(Root->Left, OutCFile);
        token Right = ExpressionTreeToIR(Root->Right, OutCFile);

        Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
        Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));

        type* ReturnType = 0;
        switch (IR_OpId)
        {
            case IR_Equals:
            case IR_NotEquals:
            case IR_Less:
            case IR_LessEqual:
            case IR_Greater:
            case IR_GreaterEqual:            
            {
                if (!(GetTokenType(Left) == GetBasicType(TypeId_Int) &&
                      GetTokenType(Right) == GetBasicType(TypeId_Int)))
                {
                    OutputError("Error: No operator exists for given types.\n");
                }

                ReturnType = GetBasicType(TypeId_Int);
            } break;

            case IR_Add:
            case IR_Sub:
            case IR_Mul:
            case IR_Div:
            {
                // TODO: Allow int float and float int operations
                if (GetTokenType(Left) == GetBasicType(TypeId_Int) &&
                    GetTokenType(Right) == GetBasicType(TypeId_Int))
                {
                    ReturnType = GetBasicType(TypeId_Int);
                }
                else if (GetTokenType(Left) == GetBasicType(TypeId_Float) &&
                         GetTokenType(Right) == GetBasicType(TypeId_Float))
                {
                    ReturnType = GetBasicType(TypeId_Float);
                }
                else
                {
                    OutputError("Error: No operator exists for given types.\n");
                }
            } break;
        }

        ResultVarName.Var->Type = ReturnType;

        if (IsTokenConstant(Left) && IsTokenConstant(Right))
        {
            ResultVarName.Id = TokenId_ConstVar;
        }
        
        string LeftStr = GetTokenString(Left);
        string RightStr = GetTokenString(Right);        
        PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
        PushBinOpInstr(IR_OpId, LeftStr, RightStr, ResultVarName.Var->Str);
    }
    else
    {
        InvalidCodePath;
    }
        
    return ResultVarName;
}

inline type* ParseType(parser_state* State, token* CurrToken, token** FinishedToken)
{
    type* Result = IsTypeDefined(State, CurrToken->Str);
    if (Result)
    {            
        ++CurrToken;
            
        // NOTE: If pointer type, get the number of references
        u32 NumRef = 0;
        while(CurrToken->Id == OpId_Mul)
        {
            ++NumRef;
            ++CurrToken;
        }

        type* TempType = Result;
        if (NumRef > 0)
        {
            Result = PushStruct(&TypeArena, type);
            Result->NumRef = NumRef;
            Result->PointerType = TempType;
        }

        *FinishedToken = CurrToken;
    }

    return Result;
}

inline token EvaluateBoolExpr(parser_state* State, token** FinishedToken)
{
    token* CurrToken = State->StartToken;
    if (CurrToken->Id != OpId_OpenBracket)
    {
        OutputError("Error: If keyword must be followed by a statement in brackets.\n");
    }
    ++CurrToken;
    
    token* StartToken = CurrToken;
    u32 BoolExprCount = 0;
    for (; StartToken != State->LastToken; ++BoolExprCount, ++CurrToken)
    {
        if (CurrToken->Id == OpId_CloseBracket)
        {
            break;
        }
    }
    ++CurrToken;

    // NOTE: We need the caller to know what the next token after our bool expression is
    *FinishedToken = CurrToken;
    
    RelabelExpressionTokens(State, StartToken, StartToken + BoolExprCount);
    expression_node* ExprTree = ParseExpressionToTree(StartToken, BoolExprCount);
    token BoolExpr = ExpressionTreeToIR(ExprTree, State->OutFile);

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

internal parser_state* ParseTokens(parser_state* State);

// TODO: We need to sometimes parse a little even if not allowed to see if we might have an error
// by declaring something in a scope that doesn't allow it
inline b32 TryParseTypeStatement(u32 Flags, parser_state* State)
{
    if (State->StartToken->Id != ScannerId_Identifier)
    {
        return false;
    }

    // TODO: Potentially simplify this functions args
    type* Type = ParseType(State, State->StartToken, &State->StartToken);
    if (Type)
    {
        if (!(State->StartToken->Id == ScannerId_Identifier))
        {
            OutputError("Error: type not followed by a variable or function name\n");
        }

        string NameStr = State->StartToken->Str;            
        ++State->StartToken;
            
        if (Flags & VarDefn_Allowed && State->StartToken->Id == ScannerId_EndLine)
        {
            // NOTE: Here we know we are looking at a variable definition
            var* VarName = IsVarDefined(State, NameStr);
            if (VarName)
            {
                OutputError("Error: Variable redefined.\n");
            }
                
            // NOTE: Define after we evaluate LHS and RHS
            VarName = DefineVarInScope(State, NameStr, Type);
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

            u32 CurrentState = 0;
            type* PrevType = 0;
            while (State->StartToken != State->LastToken)
            {
                // TODO: All the types we parse here we should parse using ParseType to allow
                // pointers
                if (CurrentState == ArgParseState_Type)
                {
                    if (State->StartToken->Id != ScannerId_Identifier)
                    {
                        OutputError("Error: Expected type before var name for function arg decleration\n");
                    }

                    PrevType = IsTypeDefined(State, State->StartToken->Str);
                    if (!PrevType)
                    {
                        OutputError("Error: Type for argument not defined. \n");
                    }

                    AddArgToFunction(Func, PrevType);

                    CurrentState = ArgParseState_VarName;
                }
                else if (CurrentState == ArgParseState_VarName)
                {
                    if (State->StartToken->Id != ScannerId_Identifier)
                    {
                        OutputError("Error: Expected argument name after type for function decleration.\n");
                    }

                    // NOTE: This is checked in our current scope with argument names as well
                    var* Arg = IsVarDefined(State, State->StartToken->Str);
                    if (Arg)
                    {
                        OutputError("Error: Argument name already defined.\n");
                    }

                    // TODO: I dont like how we do this process
                    Arg = DefineVarInScope(State, State->StartToken->Str, PrevType);
                    PushDefineInstr(Arg->Type, Arg->Str);
                        
                    CurrentState = ArgParseState_Comma;
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

                ++State->StartToken;
            }
                    
            ++State->StartToken;

            if (CurrentState != ArgParseState_Comma)
            {
                // TODO: Fix this output error
                OutputError("Error: Arguments not done properly.\n");
            }

            PushFuncInstr(Func);
            PushScopeInstr(IR_StartScope);
                
            ++State->StartToken;
            ParseTokens(State);

            return true;
        }
        else if (Flags & VarDecl_Allowed && State->StartToken->Id == OpId_Equals)
        {
            // NOTE: Here we know we are looking at a variable decleration
            var* VarName = IsVarDefined(State, NameStr);
            if (VarName)
            {
                OutputError("Error: Variable redefined.\n");
            }

            ++State->StartToken;

            u32 NewNumTokens = NumElementsBetweenIndexes(State->LastToken, State->StartToken);
            RelabelExpressionTokens(State, State->StartToken, State->LastToken);
            expression_node* ExprTree = ParseExpressionToTree(State->StartToken, NewNumTokens);
            token RHS = ExpressionTreeToIR(ExprTree, State->OutFile);

            // NOTE: We set this for debugging to make sure we always parse every token
            State->StartToken = State->LastToken;
            
            // NOTE: Define after we evaluate LHS and RHS
            VarName = DefineVarInScope(State, NameStr, Type);
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
    
    var* LHS_Var = IsVarDefined(State, State->StartToken->Str);
    if (Flags & VarManip_Allowed && LHS_Var)
    {            
        ++State->StartToken;
            
        if (!(State->StartToken->Id == OpId_Equals))
        {
            OutputError("Error: Variable statement without equals operator.\n");
        }

        ++State->StartToken;
            
        // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
        u32 NewNumTokens = NumElementsBetweenIndexes(State->LastToken, State->StartToken);
        RelabelExpressionTokens(State, State->StartToken, State->LastToken);
        //expression_node* ExprTree = ParseExpressionToTree(State->StartToken, NewNumTokens);
        expression_node* ExprTree = TryParseAddSubExpr(State->StartToken, State->LastToken);
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
    else if (LHS_Var && (!(Flags & VarManip_Allowed)))
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
        
    token BoolExpr = EvaluateBoolExpr(State, &State->StartToken);
    if (State->StartToken->Id != ScannerId_OpenScope)
    {
        OutputError("Error: If statement must be followed by a scope.\n");
    }

    AddScope(State, ScopeId_If);
    PushIfInstr(GetTokenString(BoolExpr));
    PushScopeInstr(IR_StartScope);

    ++State->StartToken;
    ParseTokens(State);

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

        token BoolExpr = EvaluateBoolExpr(State, &State->StartToken);
            
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
    ParseTokens(State);

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

    token BoolExpr = EvaluateBoolExpr(State, &State->StartToken);
    PushWhileInstr(GetTokenString(BoolExpr));

    if (!(State->StartToken->Id == ScannerId_OpenScope))
    {
        OutputError("Error: While statement must be followed by a scope.\n");
    }
        
    AddScope(State, ScopeId_WhileLoop);
    PushScopeInstr(IR_StartScope);
            
    ++State->StartToken;
    ParseTokens(State);

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
    ParseTokens(State);

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

    State->StartToken += 1;
    ParseTokens(State);

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

    State->StartToken += 1;
    ParseTokens(State);

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

// TODO: We can simplify the creation of scopes, and make one ir instr to push if/elseif/else/while/struct
// scopes. We also can remove probably some functions we where using to "simplify" scope stuff

internal void ScanFileToTokens(char* File, u32 NumChars, scope* GlobalScope, FILE* OutCFile)
{
    char* CurrChar = File;
    u32 CurrCharCount = 0;
    
    // NOTE: We use the token array to store one line of tokens which we then parse. Thus, our
    // memory requirments fall to only storing a single line of tokens

    parser_state State = {};
    State.Scope = GlobalScope;
    State.OutFile = OutCFile;
    
    u32 NumTokens = 0;
    while (CurrChar <= (File + NumChars))
    {
        token* CurrToken = PushStruct(&TokenArena, token);
        CurrCharCount += GetPastWhiteSpace(&CurrChar);

        b32 FinishedLine = false;
        if (StringCompare(CurrChar, "=", 1))
        {
            CurrToken->Id = OpId_Equals;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "==", 2))
        {
            CurrToken->Id = OpId_BinaryEqual;
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, "!=", 2))
        {
            CurrToken->Id = OpId_BinaryNotEqual;
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, "<", 1))
        {
            CurrToken->Id = OpId_Less;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "<=", 2))
        {
            CurrToken->Id = OpId_LessEqual;
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, ">", 1))
        {
            CurrToken->Id = OpId_Greater;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, ">=", 2))
        {
            CurrToken->Id = OpId_GreaterEqual;
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, "+", 1))
        {
            CurrToken->Id = OpId_Add;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "-", 1))
        {
            CurrToken->Id = OpId_Sub;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "*", 1))
        {
            CurrToken->Id = OpId_Mul;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "&", 1))
        {
            CurrToken->Id = OpId_AddressOff;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "/", 1))
        {
            CurrToken->Id = OpId_Div;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "(", 1))
        {
            CurrToken->Id = OpId_OpenBracket;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, ")", 1))
        {
            CurrToken->Id = OpId_CloseBracket;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "{", 1))
        {
            CurrToken->Id = ScannerId_OpenScope;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "}", 1))
        {
            CurrToken->Id = ScannerId_CloseScope;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, ",", 1))
        {
            CurrToken->Id = ScannerId_Comma;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "struct", 1))
        {
            CurrToken->Id = ScannerId_Struct;
            CurrToken->Str = InitStr(CurrChar, 6);
        }
        else if (StringCompare(CurrChar, "if", 2))
        {
            CurrToken->Id = ControlId_If;
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, "else", 4))
        {
            CurrToken->Id = ControlId_Else;
            CurrToken->Str = InitStr(CurrChar, 4);
        }
        else if (StringCompare(CurrChar, "while", 5))
        {
            CurrToken->Id = ControlId_While;
            CurrToken->Str = InitStr(CurrChar, 5);
        }
        else if (StringCompare(CurrChar, ";", 1))
        {
            CurrToken->Id = ScannerId_EndLine;
            CurrToken->Str = InitStr(CurrChar, 1);
            FinishedLine = true;
        }
        else if (IsCharNumeric(*CurrChar))
        {
            u32 NumNumbers = NumCharsToNonInt(CurrChar, CurrCharCount, NumChars);
            CurrToken->Id = TypeId_Int;
            if (*(CurrChar + NumNumbers) == '.')
            {
                NumNumbers += 1;
                u32 NumNumbersAfterDot = NumCharsToNonInt(CurrChar + NumNumbers,
                                                          CurrCharCount + NumNumbers, NumChars);
                NumNumbers += NumNumbersAfterDot;
                CurrToken->Id = TypeId_Float;
            }
            
            CurrToken->Str = InitStr(CurrChar, NumCharsToNonInt(CurrChar, CurrCharCount, NumChars));
        }
        else if (IsCharIdentifier(*CurrChar))
        {
            CurrToken->Id = ScannerId_Identifier;
            CurrToken->Str = InitStr(CurrChar,
                                     NumCharsToNonIdentifier(CurrChar, CurrCharCount, NumChars));
        }
        else
        {
            OutputError("Error: Unrecognized token.\n");
        }

        CurrCharCount += CurrToken->Str.NumChars;
        CurrChar += CurrToken->Str.NumChars;
        
        if (FinishedLine)
        {
            State.StartToken = (token*)TokenArena.Mem;
            State.LastToken = State.StartToken + NumTokens;
    
            State = *ParseTokens(&State);            
            Assert(State.StartToken == State.LastToken);
            NumTokens = 0;
            ClearArena(&TokenArena);
        }
        else
        {
            ++NumTokens;
            ++CurrToken;
        }

        CurrCharCount += GetPastWhiteSpace(&CurrChar);
    }

    State.StartToken = (token*)TokenArena.Mem;
    State.LastToken = State.StartToken + NumTokens;
    State = *ParseTokens(&State);    
    Assert(State.StartToken == State.LastToken);
    if (State.Scope != GlobalScope)
    {
        OutputError("Error: Mismatched {} pairs.\n");
    }
}

int main(int argc, char** argv)
{
    // NOTE: For now, where just gunna load in one specific file and translate it to C++
    FILE* NutellaFile = fopen("test.nutella", "rb");
    if (!NutellaFile)
    {
        OutputError("Couldn't find nutella file\n");
    }

    fseek(NutellaFile, 0, SEEK_END);
    u32 FileSizeInBytes = ftell(NutellaFile);
    fseek(NutellaFile, 0, SEEK_SET);

    char* Data = (char*)malloc(FileSizeInBytes + sizeof(char));
    Data[FileSizeInBytes] = 0;
    if (!fread(Data, FileSizeInBytes, 1, NutellaFile))
    {
        OutputError("Couldn't read nutella file\n");
    }

    // TODO: Figure out sizes and methods of expanding
    VarArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    TypeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    FuncArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    ScopeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    SyntaxTreeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    TokenArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    IR_Arena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    StringArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    
    FILE* OutCFile = fopen("test.cpp", "wb");

    // NOTE: Setup our global scope and basic types array
    // TODO: We should be using a function which standardizes this
    temp_mem ScopeTempMem = BeginTempMem(&ScopeArena);
        
    scope* GlobalScope = PushStruct(&ScopeArena, scope);
    *GlobalScope = {};
    GlobalScope->TempMem = ScopeTempMem;
    GlobalScope->Id = ScopeId_Global;
    
    InitList(&GlobalScope->TypeList);

    // TODO: This is a ugly way of including a node to our list. Figure out a nicer way to do this
    type IntType = {};
    {
        char IntText[] = "int";
        IntType.Str = InitStr(IntText, 3);
        AddNodeToList(&GlobalScope->TypeList, &IntType);
    }

    type FloatType = {};
    {
        char FloatText[] = "float";
        FloatType.Str = InitStr(FloatText, 5);
        AddNodeToList(&GlobalScope->TypeList, &FloatType);
    }

    type VoidType = {};
    {
        char VoidText[] = "void";
        VoidType.Str = InitStr(VoidText, 4);
        AddNodeToList(&GlobalScope->TypeList, &VoidType);
    }

    // TODO: We might need to handle pointers better
    type VoidPtrType = {};
    {
        VoidPtrType.NumRef = 1;
        VoidPtrType.PointerType = &VoidType;
        AddNodeToList(&GlobalScope->TypeList, &VoidPtrType);
    }

    InitList(&GlobalScope->VarList);

    // NOTE: This is for fast access in our get token type call and for constants
    BasicTypes[TypeId_Int - TypeId_StartTypes - 1] = &IntType;
    BasicTypes[TypeId_Float - TypeId_StartTypes - 1] = &FloatType;
    BasicTypes[TypeId_Void - TypeId_StartTypes - 1] = &VoidType;
    BasicTypes[TypeId_PointerType - TypeId_StartTypes - 1] = &VoidPtrType;
    
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), GlobalScope, OutCFile);
    ConvertIrToC(OutCFile);
    
    fclose(OutCFile);
    fclose(NutellaFile);

    return 1;
}
