
#include "nutella_compiler.h"
#include "nutella_string.cpp"
#include "nutella_linked_list.cpp"

// TODO: Remove this global?
global scope GlobalScope = {};
global scope* GlobalCurrScope;
global type* BasicTypes[2] = {};

// TODO: Put this into a global state or maybe its okay to keep here?
global mem_arena VarArena;
global mem_arena TypeArena;
global mem_arena TokenArena;
global mem_arena IR_Arena;

/*
  TODO List:
      - Define more strictly what it means to be a statement and expression (using grammars)

      - Figure out memory management since currently we are leaking and using malloc all over the
        place. Probably better to define arenas which we grow as we need memory for different parts
        of our program.

      - We need a way to define functions and operators. We don't wanna inject a operator function
        for ints and floats into our code but we would rather have some way of just puttng the
        operations directly into the calling place (as if inlined since its just one instruction).
        Rn, I'm thinking of special casing them by checking if the left and right types are both
        ints or floats and then subbing in the instruction. Otherwise, we check for a operator defined
        for our types present and we use that instead. Then, we also have to clearly define return
        types for our operators so that we can finally do proper type checking.

      - For type checking, it makes it easier if constant int tokens just store their types as ints
        since it makes less branchy code (its probably slower). Figure out which path you wann choose.
        
      - Make sure that if we get a error, don't generate any code at all

      - Make a function that returns the argument type list and the return type for our arguments
        so that we can pass it in a token such as a function name token or a opeartor token and
        it will give us the types required for those operations

      - We can have a int for each type and var that says which numbered scope that var is from. Then,
        when we check if a var is defined in a given scope, we just check if the num scope of that var
        is less than or equal to our current scope. We would have to remove all variables and types
        from the list of a given number once we evict that scope to not cause scope num collisions. Take
        a look at how useful this optimization would actually be.

      - For memory, make linked lists of 4 meg or 1 meg blocks of mem where in that block, its just a
        flat array. For other things like types, we can resizeable arrays?
 */

inline type* GetBasicType(u32 Id)
{
    type* Result = BasicTypes[Id - TokenId_ConstInt];
    return Result;
}

inline b32 IsTokenConstant(token Token)
{
    b32 Result = (Token.Id >= TokenId_ConstInt && Token.Id <= TokenId_ConstStr);

    return Result;
}

inline b32 IsTypePointer(type* Type)
{
    b32 Result = Type->NumRef > 0;
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

    if (Token.Id == TokenId_Var)
    {
        Result = Token.Var->Str;
    }
    else if (Token.Id == TokenId_Type)
    {
        Result = Token.Type->Str;
    }
    else if (IsTokenConstant(Token))
    {
        Result = Token.Str;
    }
    else if (Token.Id == TokenId_ScannerName)
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

// TODO: Merge this with our tree generation??? 
internal void RelabelExpressionTokens(token* Tokens, u32 NumTokens, scope* CurrScope)
{
    token* Token = Tokens;
    for (u32 TokenId = 0; TokenId < NumTokens; ++TokenId, ++Token)
    {
        switch (Token->Id)
        {
            case TokenId_ScannerName:
            {
                type* InferType = IsTypeDefined(CurrScope, Token->Str);
                if (InferType)
                {
                    Token->Type = InferType;
                    Token->Id = TokenId_Type;
                }
                else
                {
                    var* InferVar = IsVarDefined(CurrScope, Token->Str);
                    if (InferVar)
                    {
                        Token->Var = InferVar;
                        Token->Id = TokenId_Var;
                    }
                }
            } break;

            case TokenId_Numeric:
            {
                // TODO: When we support floats, infer the type here
                Token->Id = TokenId_ConstInt;
                // TODO: Store the type of int inside the type pointer here
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
        if (CurrToken->Id == TokenId_OpenBracket)
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
        else if (CurrToken->Id == TokenId_CloseBracket)
        {
            ++NumCloseLeft;
        }
    }

    return Result;
}

// TODO: Try and clean this up. We can probably get rid of the extra +1's -1's
internal expression_node* ParseExpressionToTree(token* Tokens, i32 NumTokens)
{
    expression_node* Result = (expression_node*)malloc(sizeof(expression_node));
    memset(Result, 0, sizeof(expression_node));

    // TODO: Add dereference op and address of op and functions (all of these should take highest
    // precedence as ordered from right to left (next to brackets)
    // NOTE: We assume we don't have a end line at the end
    // NOTE: We scan in reverse order of what our order of operations wants since the tree will
    // be traveresed in a reversed order which then would give us the correct result

    i32 OpenBracketPos = -1;
    token* OpenBracketToken = 0;
    i32 CloseBracketPos = -1;
    token* CloseBracketToken = 0;

    i32 AddressOfPos = -1;
    token* AddressOfToken = 0;
    
    i32 MulDivTokenPos = -1;
    token* MulDivToken = 0;

    i32 CurrTokenCount = NumTokens - 1;
    token* CurrToken = Tokens + (NumTokens - 1);

    // NOTE: Base case for when we have just 1 token thats a variable or constant
    if (NumTokens == 1)
    {
        if (!(CurrToken->Id == TokenId_Var ||
              CurrToken->Id == TokenId_ConstInt))
        {
            printf("Error: Expression does not contain a variable name or constant.\n");
            return Result;
        }

        Result->Token = *CurrToken;
        return Result;
    }

    // TODO: Make the error checking code reside once we found tokens and start evaluating the tree,
    // not when we are searching
    b32 FoundAddSub = false;
    while (CurrToken >= Tokens)
    {
        if (CurrToken->Id == TokenId_CloseBracket)
        {
            i32 TokensTillMatchingOpen = FindMatchingOpen(CurrToken, CurrTokenCount);

            if (TokensTillMatchingOpen == -1)
            {
                printf("Error: No matching open bracket found for closing bracket.\n");
                break;
            }
            
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
        else if (CurrToken->Id == TokenId_OpenBracket)
        {
            // NOTE: We should never reach this code since we look for pairs of open/close
            // brackets right away when we see the first close bracket. Thus, this is invalid.

            printf("Error: Mismatching open/close brackets.\n");
            break;
        }
        else if (CurrToken->Id == TokenId_AddressOff)
        {
            AddressOfPos = CurrTokenCount;
            AddressOfToken = CurrToken;
        }
        else if (CurrToken->Id == TokenId_Mul || CurrToken->Id == TokenId_Div)
        {
            if (CurrTokenCount == 0)
            {
                printf("Error: Operators must have a left hand and a right hand side.\n");
                break;
            }

            if (MulDivTokenPos == 0)
            {
                MulDivTokenPos = CurrTokenCount;
                MulDivToken = CurrToken;
            }
        }
        else if (CurrToken->Id == TokenId_Add || CurrToken->Id == TokenId_Sub)
        {
            if (CurrTokenCount == -1)
            {
                printf("Error: Operators must have a left hand and a right hand side.\n");
                break;
            }

            Result->OperatorId = CurrToken->Id;
            Result->Left = ParseExpressionToTree(Tokens, CurrTokenCount);
            Result->Right = ParseExpressionToTree(CurrToken+1, NumTokens - CurrTokenCount - 1);
            
            FoundAddSub = true;
            break;
        }

        --CurrToken;
        --CurrTokenCount;
    }

    // NOTE: We didn't find a add/sub op so we look for next ones based on order of ops
    if (!FoundAddSub)
    {
        if (MulDivTokenPos != -1)
        {
            Result->OperatorId = MulDivToken->Id;
            Result->Left = ParseExpressionToTree(Tokens, MulDivTokenPos);
            Result->Right = ParseExpressionToTree(MulDivToken+1, NumTokens - MulDivTokenPos - 1);
        }
        else if (AddressOfPos != -1)
        {
            // TODO: What error to output if address of isnt the first op in our line?
            if (AddressOfPos == NumTokens || AddressOfPos != 0)
            {
                printf("Error: Need to have variable after address of operator.\n");
                return 0;
            }

            Result->OperatorId = AddressOfToken->Id;
            Result->Right = ParseExpressionToTree(AddressOfToken+1, NumTokens - AddressOfPos - 1);
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
    
    u32 IR_OpId = 0;
    if (Root->OperatorId == TokenId_Add)
    {
        IR_OpId = IR_Add;
    }
    else if (Root->OperatorId == TokenId_Sub)
    {
        IR_OpId = IR_Sub;
    }
    else if (Root->OperatorId == TokenId_Mul)
    {
        IR_OpId = IR_Mul;
    }
    else if (Root->OperatorId == TokenId_Div)
    {
        IR_OpId = IR_Div;
    }
    else if (Root->OperatorId == TokenId_AddressOff)
    {
        IR_OpId = IR_AddressOff;
    }
    else
    {
        printf("Error: Undefined operator in expression.\n");
        return {};
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

        // TODO: Do type checking here
        type* TypeRight = GetTokenType(Right);
        string RightStr = GetTokenString(Right);

        // TODO: Add pointer type here if not present
        ResultVarName.Var->Type = TypeRight;
        
        PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
        PushUniOpInstr(IR_OpId, RightStr, ResultVarName.Var->Str);
    }
    else if (IsOpBinary(IR_OpId))
    {
        // TODO: Figure out pointer addition IR
        Assert(Root->Left && Root->Right);
        token Left = ExpressionTreeToIR(Root->Left, OutCFile);
        token Right = ExpressionTreeToIR(Root->Right, OutCFile);

        // TODO: Make types pointers so that we can store type size and extra info once
        // TODO: Should we be defining int types like this or should they be looked up?
        type* TypeLeft = GetTokenType(Left);
        string LeftStr = GetTokenString(Left);

        type* TypeRight = GetTokenType(Right);
        string RightStr = GetTokenString(Right);
        
        // TODO: Add other const into this assert
        Assert(Left.Id == TokenId_Var || Left.Id == TokenId_ConstInt);
        Assert(Right.Id == TokenId_Var || Right.Id == TokenId_ConstInt);

        // TODO: Definitley fix this type choosing
        if (StringCompare(TypeLeft->Str, TypeRight->Str))
        {
            ResultVarName.Var->Type = GetBasicType(TokenId_ConstInt);
        }
        else
        {
            printf("Error: No defined addition op between type left and type right.\n");
            return ResultVarName;
        }

#if 0
        // NOTE: What i want
        func_operator* Operator = GetOperator(IR_OpId);
        if (!ResolveTypesForOp(Operator, TypeLeft, TypeRight))
        {
            printf("Error: No operator exists for given types.\n");
            return;
        }

        ResultVarName.Var->Type = Operator->ReturnType;

        PushBinOpInstr();
#endif
        
        PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
        PushBinOpInstr(IR_OpId, LeftStr, RightStr, ResultVarName.Var->Str);
    }
    else
    {
        InvalidCodePath;
    }
        
    return ResultVarName;
}

internal void ParseTokens(token* TokenArray, u32 NumTokens, FILE* OutFile)
{
/*
  NOTE:
       We define rulesets(grammar) here and make sure that our token array follows them properly

           Statement - (type) (variable name) (end) |
                       (type) (variable name) (equals) (integer) (end)
                       (var_name) (equals) (numeric) (end) |
                       (var_name) (equals) (var_name) (end)
                       (var_name) = (expression)

           Expression - (var + const) |
                        (var + var) |
                        (const + var) |
                        (const + const)

*/

    if (NumTokens == 0)
    {
        return;
    }
    
    token* CurrToken = TokenArray;
    if (CurrToken->Id == TokenId_ScannerName)
    {
        // NOTE: Loop through types in this scope
        type* DeclareType = IsTypeDefined(GlobalCurrScope, CurrToken->Str);
        if (DeclareType)
        {            
            ++CurrToken;
            
            // NOTE: If pointer type, get the number of references
            u32 NumRef = 0;
            while(CurrToken->Id == TokenId_Mul)
            {
                ++NumRef;
                ++CurrToken;
            }

            // TODO: Organize this somehow in memory
            type* VarType = DeclareType;
            if (NumRef > 0)
            {
                VarType = PushStruct(&TypeArena, type);
                VarType->NumRef = NumRef;
                VarType->PointerType = DeclareType;
            }
            
            if (!(CurrToken->Id == TokenId_ScannerName))
            {
                printf("Error: type not followed by a variable name\n");
                return;
            }

            string VarNameStr = CurrToken->Str;
            var* VarName = IsVarDefined(GlobalCurrScope, CurrToken->Str);
            if (VarName)
            {
                printf("Error: Variable redefined.\n");
                return;
            }
            
            ++CurrToken;
            
            if (CurrToken->Id == TokenId_EndLine)
            {
                ++CurrToken;
                
                // NOTE: Define after we evaluate LHS and RHS
                VarName = DefineVarInScope(GlobalCurrScope, VarNameStr, VarType);
                PushDefineInstr(VarName->Type, VarName->Str);
                
                return;
            }
            else if (CurrToken->Id != TokenId_Equals)
            {
                printf("Error: Variable definition followed by unsupported operations\n");
                return;
            }
            // TODO: Check against the rest of the possible outcomes

            ++CurrToken;
            
            // TODO: We can generalize our expression trees to include equal operators which will
            // allow us to make trees out of all statements instead of just expressions. But then we
            // need a way to make sure that only some operators are present on the left side
            // (dereference pointer or ++ but not addition or subtraction)
            // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
            u32 NewNumTokens = NumTokens - (((u32)CurrToken - (u32)TokenArray) / sizeof(token));
            RelabelExpressionTokens(CurrToken, NewNumTokens, GlobalCurrScope);
            expression_node* ExprTree = ParseExpressionToTree(CurrToken, NewNumTokens);
            token RHS = ExpressionTreeToIR(ExprTree, OutFile);

            // NOTE: Define after we evaluate LHS and RHS
            VarName = DefineVarInScope(GlobalCurrScope, VarNameStr, VarType);

            PushDefineInstr(VarName->Type, VarName->Str);

            if (!AreSameType(*VarName, RHS))
            {
                printf("Error: Type on LHS and RHS don't match.\n");
                return;
            }

            string RHS_Str = GetTokenString(RHS);
            PushAssignInstr(VarName->Str, RHS_Str);
                        
            return;
        }

        // TODO: We should build a syntax tree out of LHS and RHS. Then we could simplify all of this
        // code to just be one code path
        var* LHS_Var = IsVarDefined(GlobalCurrScope, CurrToken->Str);
        if (LHS_Var)
        {            
            ++CurrToken;
            
            if (!(CurrToken->Id == TokenId_Equals))
            {
                printf("Error: Variable statement without equals operator.\n");
                return;
            }

            ++CurrToken;
            
            // TODO: Let Parse Expression tree return # nodes touched
            // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
            u32 NewNumTokens = NumTokens - (((u32)CurrToken - (u32)TokenArray) / sizeof(token));
            RelabelExpressionTokens(CurrToken, NewNumTokens, GlobalCurrScope);
            expression_node* ExprTree = ParseExpressionToTree(CurrToken, NewNumTokens);
            token RHS = ExpressionTreeToIR(ExprTree, OutFile);
            
            // NOTE: Do a type check here
            if (!AreSameType(*LHS_Var, RHS))
            {
                printf("Error: Type on LHS and RHS don't match.\n");
                return;
            }

            string RHS_Str = GetTokenString(RHS);
            PushAssignInstr(LHS_Var->Str, RHS_Str);
            
            return;
        }

        printf("Error: LHS is not a variable name or variable decleration.\n");
        return;
    }
    else if (CurrToken->Id == TokenId_OpenScope)
    {
        scope* NewScope = (scope*)malloc(sizeof(scope));
        *NewScope = {};
        NewScope->PrevScope = GlobalCurrScope;
        InitList(&NewScope->TypeList);
        InitList(&NewScope->VarList);

        GlobalCurrScope->NextScope = NewScope;
        GlobalCurrScope = NewScope;

        ParseTokens(TokenArray+1, NumTokens-1, OutFile);
    }
    else if (CurrToken->Id == TokenId_CloseScope)
    {
        // TODO: Free the memory so we don't leak
        GlobalCurrScope = GlobalCurrScope->PrevScope;

        ParseTokens(TokenArray+1, NumTokens-1, OutFile);
    }
    else if (CurrToken->Id == TokenId_Numeric)
    {
        printf("Error: Can't start statement with a number.\n");
        return;
    }
    else if (CurrToken->Id == TokenId_Equals)
    {
        printf("Error: Can't start statement with eqauls.\n");
        return;
    }
    else if (CurrToken->Id == TokenId_EndLine)
    {
        printf("Warning: Starting statement with endline.\n");
    }
    else
    {
        InvalidCodePath;
    }
}

internal void ScanFileToTokens(char* File, u32 NumChars, FILE* OutCFile)
{
    char* CurrChar = File;
    u32 CurrCharCount = 0;
    
    // NOTE: We use the token array to store one line of tokens which we then parse. Thus, our
    // memory requirments fall to only storing a single line of tokens

    // TODO: We need better error reporting if the tokens we are seeing don't match what we expect
    u32 NumTokens = 0;
    while (CurrChar <= (File + NumChars))
    {
        token* CurrToken = PushStruct(&TokenArena, token);
        CurrCharCount += GetPastWhiteSpace(&CurrChar);

        b32 FinishedLine = false;
        if (StringCompare(CurrChar, ":=", 2))
        {
            CurrToken->Id = TokenId_Equals;
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, "+", 1))
        {
            CurrToken->Id = TokenId_Add;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "-", 1))
        {
            CurrToken->Id = TokenId_Sub;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "*", 1))
        {
            CurrToken->Id = TokenId_Mul;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "&", 1))
        {
            CurrToken->Id = TokenId_AddressOff;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "/", 1))
        {
            CurrToken->Id = TokenId_Div;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "(", 1))
        {
            CurrToken->Id = TokenId_OpenBracket;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, ")", 1))
        {
            CurrToken->Id = TokenId_CloseBracket;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "{", 1))
        {
            CurrToken->Id = TokenId_OpenScope;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "}", 1))
        {
            CurrToken->Id = TokenId_CloseScope;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, ";", 1))
        {
            CurrToken->Id = TokenId_EndLine;
            CurrToken->Str = InitStr(CurrChar, 1);
            FinishedLine = true;
        }
        else if (IsCharNumeric(*CurrChar))
        {
            // NOTE: If the first char is a number, we assume its a integer (no floats yet)
            CurrToken->Id = TokenId_Numeric;
            CurrToken->Str = InitStr(CurrChar, NumCharsToNonInt(CurrChar, CurrCharCount, NumChars));
        }
        else
        {
            CurrToken->Id = TokenId_ScannerName;
            CurrToken->Str = InitStr(CurrChar, NumCharsToNonAlpha(CurrChar, CurrCharCount, NumChars));
        }

        CurrCharCount += CurrToken->Str.NumChars;
        CurrChar += CurrToken->Str.NumChars;
        
        if (FinishedLine)
        {
            ParseTokens((token*)TokenArena.Mem, NumTokens, OutCFile);
            NumTokens = 0;
            ClearArena(&TokenArena);
        }
        else
        {
            ++NumTokens;
            ++CurrToken;
        }
    }

    ParseTokens((token*)TokenArena.Mem, NumTokens, OutCFile);
    if (GlobalCurrScope != &GlobalScope)
    {
        printf("Error: Mismatched {} pairs.\n");
        return;
    }
}

int main(int argc, char** argv)
{
    // NOTE: For now, where just gunna load in one specific file and translate it to C++
    FILE* NutellaFile = fopen("test.nutella", "rb");
    if (!NutellaFile)
    {
        printf("Couldn't find nutella file\n");
        return 0;
    }

    fseek(NutellaFile, 0, SEEK_END);
    u32 FileSizeInBytes = ftell(NutellaFile);
    fseek(NutellaFile, 0, SEEK_SET);

    char* Data = (char*)malloc(FileSizeInBytes + sizeof(char));
    Data[FileSizeInBytes] = 0;
    if (!fread(Data, FileSizeInBytes, 1, NutellaFile))
    {
        printf("Couldn't read nutella file\n");
        return 0;
    }

    // TODO: Figure out sizes and methods of expanding
    VarArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    TypeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    TokenArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    IR_Arena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));

    //u32 NumTokens = 0;
    //token TokenArray[10000] = {};
    
    FILE* OutCFile = fopen("test.cpp", "wb");

    // NOTE: Setup our global scope and basic types array
    InitList(&GlobalScope.TypeList);

    // TODO: This is a ugly way of including a node to our list. Figure out a nicer way to do this
    type IntType = {};
    char IntText[] = "int";
    IntType.Str = InitStr(IntText, 3);
    AddNodeToList(&GlobalScope.TypeList, &IntType);
    
    type StringType = {};
    char StringText[] = "string";
    StringType.Str = InitStr(StringText, 6);
    AddNodeToList(&GlobalScope.TypeList, &StringType);
    
    InitList(&GlobalScope.VarList);

    // NOTE: This is for fast access in our get token type call and for constants
    BasicTypes[TokenId_ConstInt - TokenId_ConstInt] = &IntType;
    BasicTypes[TokenId_ConstStr - TokenId_ConstInt] = &StringType;

    // TODO: Remove this
    GlobalCurrScope = &GlobalScope;
    
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), OutCFile);
    ConvertIrToC(OutCFile);
    
    fclose(OutCFile);
    fclose(NutellaFile);

    return 1;
}
