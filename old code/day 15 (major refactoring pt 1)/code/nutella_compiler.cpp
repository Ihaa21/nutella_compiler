
#include "nutella_compiler.h"

// TODO: Remove this global?
global scope GlobalScope = {};
global scope* GlobalCurrScope;
global type* BasicTypes[2] = {};

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

      - Make sure that if we get a error, don't generate any code at all

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

inline b32 IsTokenOperator(token Token)
{
    b32 Result = Token.Id > OpId_StartOps && Token.Id < OpId_EndOps;

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

// TODO: Somewhere in the code, we ask for the defined type but don't output a error if the type
// isnt found. Same for var so check the code paths
internal type* IsTypeDefined(scope* LocalScope, string MatchStr)
{
    type* Result = 0;
    
    scope* CurrScope = LocalScope;
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

internal var* IsVarDefined(scope* LocalScope, string MatchStr)
{
    var* Result = 0;
    
    scope* CurrScope = LocalScope;
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

inline var* DefineVarInScope(scope* CurrScope, string Name, type* Type)
{
    var* NewVar = PushStruct(&VarArena, var);
    NewVar->Str = Name;
    NewVar->Type = Type;
    AddNodeToList(&CurrScope->VarList, NewVar);

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

inline scope* CreateScope(u32 ScopeId = ScopeId_None)
{
    temp_mem ScopeTempMem = BeginTempMem(&ScopeArena);
        
    scope* NewScope = PushStruct(&ScopeArena, scope);
    *NewScope = {};
    NewScope->TempMem = ScopeTempMem;
    NewScope->PrevScope = GlobalCurrScope;
    NewScope->Id = ScopeId;
    InitList(&NewScope->TypeList);
    InitList(&NewScope->VarList);

    GlobalCurrScope->NextScope = NewScope;
    GlobalCurrScope = NewScope;

    return NewScope;
}

// TODO: We can definintly merge this into our building tree function. Is it worth it?
internal void RelabelExpressionTokens(token* Tokens, u32 NumTokens, scope* CurrScope)
{
    token* Token = Tokens;
    for (u32 TokenId = 0; TokenId < NumTokens; ++TokenId, ++Token)
    {
        switch (Token->Id)
        {
            case ScannerId_Identifier:
            {
                type* Type = IsTypeDefined(CurrScope, Token->Str);
                if (Type)
                {
                    Token->Type = Type;
                    Token->Id = TokenId_Type;
                }
                else
                {
                    var* Var = IsVarDefined(CurrScope, Token->Str);
                    if (Var)
                    {
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
    }
}

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

// TODO: Try and clean this up. We can probably get rid of the extra +1's -1's
internal expression_node* ParseExpressionToTree(token* Tokens, i32 NumTokens)
{
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
                type* NewType = TypeRight;
                {
                    if (TypeRight->NumRef == 0)
                    {
                        OutputError("Error: Cannot dereference non pointer value.\n");
                    }
                    else
                    {
                        TypeRight->NumRef -= 1;
                        if (TypeRight->NumRef == 0)
                        {
                            // TODO: Free the pointer type
                            NewType = TypeRight->PointerType;
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
                type* NewType = TypeRight;
                {
                    if (TypeRight->NumRef == 0)
                    {
                        NewType = PushStruct(&TypeArena, type);
                        NewType->NumRef = 1;
                        NewType->PointerType = TypeRight;
                    }
                    else
                    {
                        TypeRight->NumRef += 1;
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

inline type* ParseType(token* CurrToken, token** FinishedToken)
{
    type* Result = IsTypeDefined(GlobalCurrScope, CurrToken->Str);
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

inline token EvaluateBoolExpr(token* CurrToken, token* FinishToken, FILE* OutFile, token** FinishedToken)
{    
    if (CurrToken->Id != OpId_OpenBracket)
    {
        OutputError("Error: If keyword must be followed by a statement in brackets.\n");
    }
    ++CurrToken;
    
    token* StartToken = CurrToken;
    u32 BoolExprCount = 0;
    for (; StartToken != FinishToken; ++BoolExprCount, ++CurrToken)
    {
        if (CurrToken->Id == OpId_CloseBracket)
        {
            break;
        }
    }
    ++CurrToken;

    // NOTE: We need the caller to know what the next token after our bool expression is
    *FinishedToken = CurrToken;
    
    RelabelExpressionTokens(StartToken, BoolExprCount, GlobalCurrScope);
    expression_node* ExprTree = ParseExpressionToTree(StartToken, BoolExprCount);
    token BoolExpr = ExpressionTreeToIR(ExprTree, OutFile);

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

internal void ParseTokens(scope* Scope, token* StartToken, token* LastToken, FILE* OutFile);

// TODO: We need to sometimes parse a little even if not allowed to see if we might have an error
// by declaring something in a scope that doesn't allow it
inline b32 TryParseTypeStatement(u32 Flags, scope* Scope, token* CurrToken, token* LastToken,
                                 FILE* OutFile)
{
    if (CurrToken->Id != ScannerId_Identifier)
    {
        return false;
    }
    
    type* Type = ParseType(CurrToken, &CurrToken);
    if (Type)
    {
        if (!(CurrToken->Id == ScannerId_Identifier))
        {
            OutputError("Error: type not followed by a variable or function name\n");
        }

        string NameStr = CurrToken->Str;            
        ++CurrToken;
            
        if (Flags & VarDefn_Allowed && CurrToken->Id == ScannerId_EndLine)
        {
            // NOTE: Here we know we are looking at a variable definition
            var* VarName = IsVarDefined(Scope, NameStr);
            if (VarName)
            {
                OutputError("Error: Variable redefined.\n");
            }

            ++CurrToken;
                
            // NOTE: Define after we evaluate LHS and RHS
            VarName = DefineVarInScope(Scope, NameStr, Type);
            PushDefineInstr(VarName->Type, VarName->Str);
                
            return true;
        }
        else if (Flags & FuncDecl_Allowed && CurrToken->Id == OpId_OpenBracket)
        {
            // NOTE: Here we know we are looking at a function decleration
            function* Func = IsFuncDefined(NameStr);
            if (Func)
            {
                OutputError("Error: Function redefined.\n");
            }

            Func = CreateFunction(NameStr, Type);
            scope* FuncScope = CreateScope(ScopeId_Function);
                
            ++CurrToken;

            u32 CurrentState = 0;
            type* PrevType = 0;
            while (CurrToken != LastToken)
            {
                // TODO: All the types we parse here we should parse using ParseType to allow
                // pointers
                if (CurrentState == ArgParseState_Type)
                {
                    if (CurrToken->Id != ScannerId_Identifier)
                    {
                        OutputError("Error: Expected type before var name for function arg decleration\n");
                    }

                    PrevType = IsTypeDefined(Scope, CurrToken->Str);
                    if (!PrevType)
                    {
                        OutputError("Error: Type for argument not defined. \n");
                    }

                    AddArgToFunction(Func, PrevType);

                    CurrentState = ArgParseState_VarName;
                }
                else if (CurrentState == ArgParseState_VarName)
                {
                    if (CurrToken->Id != ScannerId_Identifier)
                    {
                        OutputError("Error: Expected argument name after type for function decleration.\n");
                    }

                    // NOTE: This is checked in our current scope with argument names as well
                    var* Arg = IsVarDefined(Scope, CurrToken->Str);
                    if (Arg)
                    {
                        OutputError("Error: Argument name already defined.\n");
                    }

                    // TODO: I dont like how we do this process
                    Arg = DefineVarInScope(Scope, CurrToken->Str, PrevType);
                    PushDefineInstr(Arg->Type, Arg->Str);
                        
                    CurrentState = ArgParseState_Comma;
                }
                else if (CurrentState == ArgParseState_Comma)
                {
                    if (CurrToken->Id == OpId_CloseBracket)
                    {
                        break;
                    }
                    else if (CurrToken->Id == ScannerId_Comma)
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

                ++CurrToken;
            }
                    
            ++CurrToken;

            if (CurrentState != ArgParseState_Comma)
            {
                // TODO: Fix this output error
                OutputError("Error: Arguments not done properly.\n");
            }

                
            PushFuncInstr(Func);
            PushScopeInstr(IR_StartScope);
                
            ++CurrToken;
            ParseTokens(FuncScope, CurrToken, LastToken, OutFile);

            return true;
        }
        else if (Flags & VarDecl_Allowed && CurrToken->Id == OpId_Equals)
        {
            // NOTE: Here we know we are looking at a variable decleration
            var* VarName = IsVarDefined(GlobalCurrScope, NameStr);
            if (VarName)
            {
                OutputError("Error: Variable redefined.\n");
            }

            ++CurrToken;

            u32 NewNumTokens = NumElementsBetweenIndexes(LastToken, CurrToken);
            RelabelExpressionTokens(CurrToken, NewNumTokens, GlobalCurrScope);
            expression_node* ExprTree = ParseExpressionToTree(CurrToken, NewNumTokens);
            token RHS = ExpressionTreeToIR(ExprTree, OutFile);

            // NOTE: Define after we evaluate LHS and RHS
            VarName = DefineVarInScope(GlobalCurrScope, NameStr, Type);

            PushDefineInstr(VarName->Type, VarName->Str);

            if (!AreSameType(*VarName, RHS))
            {
                OutputError("Error: Type on LHS and RHS don't match.\n");
            }

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
    
    var* LHS_Var = IsVarDefined(GlobalCurrScope, CurrToken->Str);
    if (Flags & VarManip_Allowed && LHS_Var)
    {            
        ++CurrToken;
            
        if (!(CurrToken->Id == OpId_Equals))
        {
            OutputError("Error: Variable statement without equals operator.\n");
        }

        ++CurrToken;
            
        // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
        u32 NewNumTokens = NumElementsBetweenIndexes(LastToken, CurrToken);
        RelabelExpressionTokens(CurrToken, NewNumTokens, GlobalCurrScope);
        expression_node* ExprTree = ParseExpressionToTree(CurrToken, NewNumTokens);
        token RHS = ExpressionTreeToIR(ExprTree, OutFile);
            
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

inline b32 TryParseIf(token* CurrToken, token* LastToken, FILE* OutFile)
{
    if (CurrToken->Id != ControlId_If)
    {
        return false;
    }
    ++CurrToken;
        
    token BoolExpr = EvaluateBoolExpr(CurrToken, LastToken, OutFile, &CurrToken);
    if (CurrToken->Id != ScannerId_OpenScope)
    {
        OutputError("Error: If statement must be followed by a scope.\n");
    }

    // NOTE: Create a scope
    scope* IfScope = CreateScope(ScopeId_If);

    PushIfInstr(GetTokenString(BoolExpr));
    PushScopeInstr(IR_StartScope);

    ++CurrToken;
    //ParseTokens(CurrToken, NumElementsBetweenIndexes(TokenArray + NumTokens, CurrToken), OutFile);

    return true;
}

inline b32 TryParseElse(token* CurrToken, token* LastToken, FILE* OutFile)
{
    if (CurrToken->Id != ControlId_Else)
    {
        return false;
    }
    ++CurrToken;

    u32 NumTokensToSkip = 0;
    if (CurrToken->Id == ControlId_If)
    {
        ++CurrToken;

        token BoolExpr = EvaluateBoolExpr(CurrToken, LastToken, OutFile, &CurrToken);
            
        PushElseIfInstr(GetTokenString(BoolExpr));
        CreateScope(ScopeId_ElseIf);
    }
    else
    {
        PushElseInstr();
        CreateScope(ScopeId_Else);
    }

    if (!(CurrToken->Id == ScannerId_OpenScope))
    {
        OutputError("Error: Else/Else If statement must be followed by a scope.\n");
    }

    PushScopeInstr(IR_StartScope);
        
    ++CurrToken;
    //ParseTokens(CurrToken, NumElementsBetweenIndexes(TokenArray + NumTokens, CurrToken), OutFile);

    return true;
}

inline b32 TryParseWhile(token* CurrToken, token* LastToken, FILE* OutFile)
{
    if (CurrToken->Id != ControlId_While)
    {
        return false;
    }
    ++CurrToken;
        
    if (CurrToken->Id != OpId_OpenBracket)
    {
        OutputError("Error: If keyword must be followed by a statement in brackets.\n");
    }

    token BoolExpr = EvaluateBoolExpr(CurrToken, LastToken, OutFile, &CurrToken);
    PushWhileInstr(GetTokenString(BoolExpr));

    if (!(CurrToken->Id == ScannerId_OpenScope))
    {
        OutputError("Error: While statement must be followed by a scope.\n");
    }
        
    CreateScope(ScopeId_WhileLoop);
    PushScopeInstr(IR_StartScope);
            
    ++CurrToken;
    //ParseTokens(CurrToken, NumElementsBetweenIndexes(TokenArray + NumTokens, CurrToken), OutFile);

    return true;
}

inline b32 TryParseStruct(token* CurrToken, token* LastToken, FILE* OutFile)
{
    if (CurrToken->Id != ScannerId_Struct)
    {
        return false;
    }
    
    ++CurrToken;

    if (!(CurrToken->Id == ScannerId_OpenScope))
    {
        OutputError("Error: Struct statement must be followed by a scope.\n");
    }
        
    CreateScope(ScopeId_Struct);
    PushScopeInstr(IR_StartScope);
                    
    ++CurrToken;
    //ParseTokens(CurrToken, NumElementsBetweenIndexes(TokenArray + NumTokens, CurrToken), OutFile);

    return true;
}

inline b32 TryParseOpenScope(token* CurrToken, token* LastToken, FILE* OutFile)
{
    if (CurrToken->Id != ScannerId_OpenScope)
    {
        return false;
    }
    
    scope* IfScope = CreateScope();
    PushScopeInstr(IR_StartScope);
    //ParseTokens(TokenArray+1, NumTokens-1, OutFile);

    return true;
}

inline b32 TryParseCloseScope(scope* Scope, token* CurrToken, token* LastToken, FILE* OutFile)
{
    if (CurrToken->Id != ScannerId_CloseScope)
    {
        return false;
    }
    
    temp_mem ScopeTempMem = GlobalCurrScope->TempMem;
    GlobalCurrScope = GlobalCurrScope->PrevScope;
    ClearTempMem(ScopeTempMem);

    PushScopeInstr(IR_EndScope);
    ParseTokens(TokenArray+1, NumTokens-1, OutFile);

    return true;
}

internal void ParseTokens(parser_state* State)
{
    if (Scope->Id == ScopeId_Struct)
    {
        if (TryParseTypeStatement(VarDefn_Allowed, Scope, StartToken, LastToken, OutFile))
        {
            return;
        }
        if (TryParseCloseScope(StartToken, LastToken, OutFile))
        {
            return;
        }
        
        OutputError("");
    }
    else if (Scope->Id == ScopeId_Global)
    {
        if (TryParseTypeStatement(VarDefn_Allowed | VarDecl_Allowed | FuncDecl_Allowed,
                                  Scope, StartToken, LastToken, OutFile))
        {
            return;
        }
        if (TryParseStruct(StartToken, LastToken, OutFile))
        {
            return;
        }
        if (TryParseOpenScope(StartToken, LastToken, OutFile))
        {
            return;
        }
        if (TryParseCloseScope(StartToken, LastToken, OutFile))
        {
            return;
        }
        //if (TryParseLoadFile())
        {
            //    return;
        }
        
        OutputError("");
    }
    else if (IsScopeCodeBlock(Scope->Id))
    {
        if (TryParseTypeStatement(VarDefn_Allowed | VarDecl_Allowed | VarManip_Allowed |
                                  FuncDecl_Allowed, Scope, StartToken, LastToken, OutFile))
        {
            return;
        }
        if (TryParseIf(StartToken, LastToken, OutFile))
        {
            return;
        }
        if (TryParseElse(StartToken, LastToken, OutFile))
        {
            return;
        }
        if (TryParseWhile(StartToken, LastToken, OutFile))
        {
            return;
        }
        if (TryParseOpenScope(StartToken, LastToken, OutFile))
        {
            return;
        }
        if (TryParseCloseScope(StartToken, LastToken, OutFile))
        {
            return;
        }

        OutputError("");
    }
    else
    {
        InvalidCodePath;
    }
}

// TODO: We can simplify the creation of scopes, and make one ir instr to push if/elseif/else/while/struct
// scopes. We also can remove probably some functions we where using to "simplify" scope stuff
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

internal void ScanFileToTokens(char* File, u32 NumChars, FILE* OutCFile)
{
    char* CurrChar = File;
    u32 CurrCharCount = 0;
    
    // NOTE: We use the token array to store one line of tokens which we then parse. Thus, our
    // memory requirments fall to only storing a single line of tokens

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
            parser_state State = {};
            // TODO: Remove this global
            State.Scope = GlobalCurrScope;
            State.StartToken = (token*)TokenArena.Mem;
            State.LastToken = State.StartToken + NumTokens;
            State.OutFile = OutCFile;

            ParseTokens(State);
            NumTokens = 0;
            ClearArena(&TokenArena);
        }
        else
        {
            ++NumTokens;
            ++CurrToken;
        }
    }

    parser_state State = {};
    // TODO: Remove this global
    State.Scope = GlobalCurrScope;
    State.StartToken = (token*)TokenArena.Mem;
    State.LastToken = State.StartToken + NumTokens;
    State.OutFile = OutCFile;
            
    ParseTokens(State);
    if (GlobalCurrScope != &GlobalScope)
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
    InitList(&GlobalScope.TypeList);

    // TODO: This is a ugly way of including a node to our list. Figure out a nicer way to do this
    type IntType = {};
    {
        char IntText[] = "int";
        IntType.Str = InitStr(IntText, 3);
        AddNodeToList(&GlobalScope.TypeList, &IntType);
    }

    type FloatType = {};
    {
        char FloatText[] = "float";
        FloatType.Str = InitStr(FloatText, 5);
        AddNodeToList(&GlobalScope.TypeList, &FloatType);
    }

    type VoidType = {};
    {
        char VoidText[] = "void";
        VoidType.Str = InitStr(VoidText, 4);
        AddNodeToList(&GlobalScope.TypeList, &VoidType);
    }

    // TODO: We might need to handle pointers better
    type VoidPtrType = {};
    {
        VoidPtrType.NumRef = 1;
        VoidPtrType.PointerType = &VoidType;
        AddNodeToList(&GlobalScope.TypeList, &VoidPtrType);
    }

    InitList(&GlobalScope.VarList);

    // NOTE: This is for fast access in our get token type call and for constants
    BasicTypes[TypeId_Int - TypeId_StartTypes - 1] = &IntType;
    BasicTypes[TypeId_Float - TypeId_StartTypes - 1] = &FloatType;
    BasicTypes[TypeId_Void - TypeId_StartTypes - 1] = &VoidType;
    BasicTypes[TypeId_PointerType - TypeId_StartTypes - 1] = &VoidPtrType;
    
    // TODO: Remove this
    GlobalScope.Id = ScopeId_Global;
    GlobalCurrScope = &GlobalScope;
    
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), OutCFile);
    ConvertIrToC(OutCFile);
    
    fclose(OutCFile);
    fclose(NutellaFile);

    return 1;
}
