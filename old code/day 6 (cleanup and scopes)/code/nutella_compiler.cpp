
#include "nutella_compiler.h"
#include "nutella_string.cpp"
#include "nutella_linked_list.cpp"

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
 */

// TODO: Remove this global?
global scope GlobalScope = {};
global scope* GlobalCurrScope;
global type* BasicTypes[2] = {};

// TODO: Remove the globals
global u32 ByteCount = 0;
global void* IR_Mem = 0;

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
    else if (Token.Id >= TokenId_ConstInt && Token.Id <= TokenId_ConstStr)
    {
        Result = BasicTypes[Token.Id - TokenId_ConstInt];
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
    else if (Token.Id >= TokenId_ConstInt && Token.Id <= TokenId_ConstStr)
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
    var* NewVar = (var*)malloc(sizeof(var));
    NewVar->Str = Name;
    NewVar->Type = Type;
    AddNodeToList(&CurrScope->VarList, NewVar);

    return NewVar;
}

// NOTE: We decompose int x = 5 to int x; x = 5; so that we don't need to vary the number
// of arguments/tokens passed
inline void PushDefineInstr(string Type, string NewVar)
{
    ir_instr Instr = {};
    Instr.Id = IR_Define;
    
    void* CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    ByteCount += sizeof(ir_instr);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Type, sizeof(string));
    ByteCount += sizeof(string);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &NewVar, sizeof(string));
    ByteCount += sizeof(string);
}

inline void PushAssignInstr(string Left, string Right)
{
    ir_instr Instr = {};
    Instr.Id = IR_Assign;
    
    void* CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    ByteCount += sizeof(Instr);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Left, sizeof(string));
    ByteCount += sizeof(string);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Right, sizeof(string));
    ByteCount += sizeof(string);    
}

// TODO: Potentially have to rename this to int bin ops since we want diff outputs for floats, etc
inline void PushBinOpInstr(u32 Op, string Arg1, string Arg2, string Ret)
{
    ir_instr Instr = {};
    Instr.Id = Op;

    void* CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    ByteCount += sizeof(Instr);
        
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Arg1, sizeof(string));
    ByteCount += sizeof(string);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Arg2, sizeof(string));
    ByteCount += sizeof(string);
        
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Ret, sizeof(string));
    ByteCount += sizeof(string);
}

internal void ConvertIrToC(FILE* OutFile)
{
    u8* CurrByte = (u8*)IR_Mem;
    
    while (CurrByte < ((u8*)IR_Mem + ByteCount))
    {
        ir_instr* Instr = (ir_instr*)CurrByte;
        CurrByte += sizeof(ir_instr);

        switch(Instr->Id)
        {
            case IR_Define:
            {
                // TODO: Add default values to standard types (and
                // eventually to structs when we implement them)
                string* Type = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* VarName = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = "int ";
                char Temp2[] = ";\n";

                string Temp3 = StringConcat(Temp1, *VarName);
                string OutputText = StringConcat(Temp3, InitStr(Temp2, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp3.Text);
                free(OutputText.Text);
            } break;

            case IR_Assign:
            {
                string* LHS = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* RHS = (string*)CurrByte;
                CurrByte += sizeof(string);

                char Temp1[] = " = ";
                char Temp2[] = ";\n";

                string Temp3 = StringConcat(*LHS, InitStr(Temp1, 3));
                string Temp4 = StringConcat(Temp3, *RHS);
                string OutputText = StringConcat(Temp4, InitStr(Temp2, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp3.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Add:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " + ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Sub:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " - ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Mul:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " * ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Div:
            {
                string* Arg1 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Arg2 = (string*)CurrByte;
                CurrByte += sizeof(string);
                string* Return = (string*)CurrByte;
                CurrByte += sizeof(string);
                
                char Temp1[] = " = ";
                char Temp2[] = " / ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(*Return, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, *Arg1);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, *Arg2);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
            
        }
    }
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
internal expression_node* ParseExpressionToTree(token* Tokens, u32 NumTokens)
{
    expression_node* Result = (expression_node*)malloc(sizeof(expression_node));
    memset(Result, 0, sizeof(expression_node));

    // TODO: Add dereference op and address of op and functions (all of these should take highest
    // precedence as ordered from right to left (next to brackets)
    // NOTE: We assume we don't have a end line at the end
    // NOTE: We scan in reverse order of what our order of operations wants since the tree will
    // be traveresed in a reversed order which then would give us the correct result

    u32 OpenBracketPos = 0;
    token* OpenBracketToken = 0;
    u32 CloseBracketPos = 0;
    token* CloseBracketToken = 0;
    
    u32 MulDivTokenPos = 0;
    token* MulDivToken = 0;

    u32 CurrTokenCount = NumTokens - 1;
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
            
            if (CloseBracketPos == 0)
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
            if (CurrTokenCount == 0)
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
        if (MulDivTokenPos != 0)
        {
            Result->OperatorId = MulDivToken->Id;
            Result->Left = ParseExpressionToTree(Tokens, MulDivTokenPos);
            Result->Right = ParseExpressionToTree(MulDivToken+1, NumTokens - MulDivTokenPos - 1);
        }
        else if (CloseBracketPos != 0)
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
    if (Root->Left)
    {
        // NOTE: If one branch exists, both branches exist
        Assert(Root->Left && Root->Right);
        token Left = ExpressionTreeToIR(Root->Left, OutCFile);
        token Right = ExpressionTreeToIR(Root->Right, OutCFile);

        // TODO: Add this variable to our var list? or make a seperate temp var list
        // or change the variable names of all our variables in ir?
        token ResultVarName = {};
        ResultVarName.Id = TokenId_Var;
        // TODO: This leaks right now
        // TODO: Assign the resulting type to this var once we add functions
        ResultVarName.Var = (var*)malloc(sizeof(var));
        *ResultVarName.Var = {};
        ResultVarName.Var->Str = InitStr((char*)VarNames[UsedCount], 2);
        ++UsedCount;

        // TODO: Make types pointers so that we can store type size and extra info once
        // TODO: Should we be defining int types like this or should they be looked up?
        type* TypeLeft = GetTokenType(Left);
        string LeftStr = GetTokenString(Left);

        type* TypeRight = GetTokenType(Right);
        string RightStr = GetTokenString(Right);
        
        // TODO: Add other const into this assert
        Assert(Left.Id == TokenId_Var || Left.Id == TokenId_ConstInt);
        Assert(Right.Id == TokenId_Var || Right.Id == TokenId_ConstInt);
        
        // TODO: We need to add order of operations now and brackets for changing that order
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
        else
        {
            printf("Error: Undefined operator in expression.\n");
            return ResultVarName;
        }
        
        // TODO: Generalize this to include different ops
        if (StringCompare(TypeLeft->Str, TypeRight->Str))
        {
            ResultVarName.Var->Type = (type*)malloc(sizeof(type));
            *ResultVarName.Var->Type = { InitStr("int", 3) };
        }
        else
        {
            printf("Error: No defined addition op between type left and type right.\n");
            return ResultVarName;
        }
        
        PushDefineInstr(ResultVarName.Var->Type->Str, ResultVarName.Var->Str);
        PushBinOpInstr(IR_OpId, LeftStr, RightStr, ResultVarName.Var->Str);
                
        return ResultVarName;
    }
    else
    {
        return Root->Token;
    }
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
    
    token* CurrToken = TokenArray;
    u32 CurrTokenCount = 0;

    if (CurrToken->Id == TokenId_ScannerName)
    {
        // NOTE: Loop through types in this scope
        type* DeclareType = IsTypeDefined(GlobalCurrScope, CurrToken->Str);
        if (DeclareType)
        {            
            ++CurrToken;
            ++CurrTokenCount;

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
            ++CurrTokenCount;

            if (CurrToken->Id == TokenId_EndLine)
            {
                ++CurrToken;
                ++CurrTokenCount;

                // NOTE: Define after we evaluate LHS and RHS
                VarName = DefineVarInScope(GlobalCurrScope, VarNameStr, DeclareType);

                PushDefineInstr(VarName->Type->Str, VarName->Str);
                
                return;
            }
            else if (CurrToken->Id != TokenId_Equals)
            {
                printf("Error: Variable definition followed by unsupported operations\n");
                return;
            }
            // TODO: Check against the rest of the possible outcomes

            ++CurrToken;
            ++CurrTokenCount;

            // TODO: We can generalize our expression trees to include equal operators which will
            // allow us to make trees out of all statements instead of just expressions. But then we
            // need a way to make sure that only some operators are present on the left side
            // (dereference pointer or ++ but not addition or subtraction)
            // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
            u32 NewNumTokens = NumTokens - CurrTokenCount;
            RelabelExpressionTokens(CurrToken, NewNumTokens, GlobalCurrScope);
            expression_node* ExprTree = ParseExpressionToTree(CurrToken, NewNumTokens);
            token RHS = ExpressionTreeToIR(ExprTree, OutFile);

            // NOTE: Define after we evaluate LHS and RHS
            VarName = DefineVarInScope(GlobalCurrScope, VarNameStr, DeclareType);

            PushDefineInstr(VarName->Type->Str, VarName->Str);

            // NOTE: Do a type check here
            string RHS_Str = GetTokenString(RHS);
            type* RHS_Type = GetTokenType(RHS);
            if (!StringCompare(VarName->Type->Str, RHS_Type->Str))
            {
                printf("Error: Type on LHS and RHS don't match.\n");
                return;
            }

            PushAssignInstr(VarName->Str, RHS_Str);
                        
            return;
        }

        var* LHS_Var = IsVarDefined(GlobalCurrScope, CurrToken->Str);
        if (LHS_Var)
        {            
            ++CurrToken;
            ++CurrTokenCount;

            if (!(CurrToken->Id == TokenId_Equals))
            {
                printf("Error: Variable statement without equals operator.\n");
                return;
            }

            ++CurrToken;
            ++CurrTokenCount;

            // TODO: Let Parse Expression tree return # nodes touched
            // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
            u32 NewNumTokens = NumTokens - CurrTokenCount;
            RelabelExpressionTokens(CurrToken, NewNumTokens, GlobalCurrScope);
            expression_node* ExprTree = ParseExpressionToTree(CurrToken, NewNumTokens);
            token RHS = ExpressionTreeToIR(ExprTree, OutFile);
            
            // NOTE: Do a type check here
            string RHS_Str = GetTokenString(RHS);
            type* RHS_Type = GetTokenType(RHS);
            if (!StringCompare(LHS_Var->Type->Str, RHS_Type->Str))
            {
                printf("Error: Type on LHS and RHS don't match.\n");
                return;
            }

            PushAssignInstr(LHS_Var->Str, RHS_Str);
            
            return;
        }

        printf("Error: LHS is not a variable name or variable decleration.\n");
        return;
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

internal void ScanFileToTokens(char* File, u32 NumChars, token* TokenArray, FILE* OutCFile)
{
    char* CurrChar = File;
    u32 CurrCharCount = 0;
    
    // NOTE: We use the token array to store one line of tokens which we then parse. Thus, our
    // memory requirments fall to only storing a single line of tokens

    u32 NumTokens = 0;
    token* CurrToken = TokenArray;
    while (CurrChar <= (File + NumChars))
    {
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
            CurrToken->Str = InitStr(CurrChar, NumCharsToWhiteSpace(CurrChar, CurrCharCount, NumChars));
        }

        CurrCharCount += CurrToken->Str.NumChars;
        CurrChar += CurrToken->Str.NumChars;
        
        if (FinishedLine)
        {
            ParseTokens(TokenArray, NumTokens, OutCFile);
            // TODO: If we don't finish on a end line output a error for now until
            // we support scopes
            // TODO: Do we need to clear the memory?
            NumTokens = 0;
            CurrToken = TokenArray;
        }
        else
        {
            ++NumTokens;
            ++CurrToken;
        }
    }

    Assert(NumTokens == 0);
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

    // TODO: Figure out how to manage our memory here
    IR_Mem = malloc(100000);
    u32 NumTokens = 0;
    token TokenArray[10000] = {};
    
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
    
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), TokenArray, OutCFile);
    ConvertIrToC(OutCFile);
    
    fclose(OutCFile);
    fclose(NutellaFile);

    return 1;
}
