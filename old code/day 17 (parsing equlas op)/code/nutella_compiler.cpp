
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

      - We need to be able to write just a function call or just a statement. We need to be able to
        do pointer modifications on the left side of our equals statement. We need to be able to
        access members of a struct using the dot operator. We need to be able to load include files.
        Make a ton of test cases too and check for errors. We need to give line numbers and better
        errors.

 */

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

// TODO: Should we implicitly find the type here?
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

// TODO: This shouldn't take a state since we don't allow local types
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

        CurrByte += sizeof(function) + sizeof(type*)*CurrFunc->NumArgs;
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

inline expression_node* CreateSyntaxNode(u32 NumArgs)
{
    expression_node* Result = PushStruct(&SyntaxTreeArena, expression_node);
    *Result = {};
    Result->NumArgs = NumArgs;
    Result->Children = PushArray(&SyntaxTreeArena, expression_node*, NumArgs);

    return Result;
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

// TODO: Remove this!!
// TODO: Make a table which has variable names, types, and functions as they appear in the code
// and then a mapping to dummy names so that we can always be sure that we don't have name conflicts
global u32 UsedCount = 0;
global const char* VarNames[] = {"a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9"};

internal token ExpressionTreeToIR(expression_node* Root, FILE* OutCFile)
{
    // NOTE: We just have a constant or variable
    if (Root->NumArgs == 0)
    {
        return Root->Token;
    }
    
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
        token Right = ExpressionTreeToIR(Root->Children[0], OutCFile);
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
        Assert(Root->NumArgs == 2);

        token Left = ExpressionTreeToIR(Root->Children[0], OutCFile);
        token Right = ExpressionTreeToIR(Root->Children[1], OutCFile);

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

// TODO: Organize better so that this isnt here
#include "nutella_parser.cpp"

internal void ScanFileToTokens(char* File, u32 NumChars, scope* GlobalScope, FILE* OutCFile)
{
    char* CurrChar = File;
    u32 CurrCharCount = 0;
    
    // NOTE: We use the token array to store one line of tokens which we then parse. Thus, our
    // memory requirments fall to only storing a single line of tokens

    parser_state State = {};
    State.Scope = GlobalScope;
    State.OutFile = OutCFile;

    b32 DontChangeCurrChar = false;
    u32 NumTokens = 0;
    while (CurrChar <= (File + NumChars))
    {
        token* CurrToken = PushStruct(&TokenArena, token);
        CurrCharCount += GetPastWhiteSpace(&CurrChar);

        b32 FinishedLine = false;
        if (StringCompare(CurrChar, "==", 2))
        {
            CurrToken->Id = OpId_BinaryEqual;
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, "=", 1))
        {
            CurrToken->Id = OpId_Equal;
            CurrToken->Str = InitStr(CurrChar, 1);
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
        else if (StringCompare(CurrChar, "#run", 4))
        {
            CurrToken->Id = OpId_Run;
            CurrToken->Str = InitStr(CurrChar, 4);
        }        
        else if (StringCompare(CurrChar, ";", 1))
        {
            CurrToken->Id = ScannerId_EndLine;
            CurrToken->Str = InitStr(CurrChar, 1);
            FinishedLine = true;
        }
        else if (StringCompare(CurrChar, "{", 1))
        {
            CurrToken->Id = ScannerId_OpenScope;
            CurrToken->Str = InitStr(CurrChar, 1);
            FinishedLine = true;
            ++NumTokens;
        }
        else if (StringCompare(CurrChar, "}", 1))
        {
            CurrToken->Id = ScannerId_CloseScope;
            CurrToken->Str = InitStr(CurrChar, 1);
            FinishedLine = true;
            ++NumTokens;
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

            CurrCharCount += CurrToken->Str.NumChars;
            CurrChar += CurrToken->Str.NumChars;

            DontChangeCurrChar = true;
            type* Type = IsTypeDefined(&State, CurrToken->Str);
            if (Type)
            {
                CurrToken->Type = Type;
                CurrToken->Id = TokenId_Type;
            }
            else
            {
                var* Var = IsVarDefined(&State, CurrToken->Str);
                if (Var)
                {
                    CurrToken->Var = Var;
                    CurrToken->Id = TokenId_Var;
                }
                else
                {
                    function* Func = IsFuncDefined(CurrToken->Str);
                    if (Func)
                    {
                        CurrToken->Func = Func;
                        CurrToken->Id = TokenId_Func;
                    }
                }
            }
        }
        else
        {
            OutputError("Error: Unrecognized token.\n");
        }

        if (DontChangeCurrChar)
        {
            DontChangeCurrChar = false;
        }
        else
        {
            CurrCharCount += CurrToken->Str.NumChars;
            CurrChar += CurrToken->Str.NumChars;            
        }
        
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
