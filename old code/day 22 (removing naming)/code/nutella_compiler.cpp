
#include "nutella_compiler.h"

// TODO: Put this into a global state or maybe its okay to keep here?
// NOTE: Stores struct types as well as default types
global mem_arena TypeArena;
// NOTE: Stores pointer types
global mem_arena PtrTypeArena;
global mem_arena FuncArena;
// NOTE: Stores all the temporary scope data (variables and ptr types)
// NOTE: This arena frees all scope specific data once the scope closes itself
global mem_arena ScopeArena;
global mem_arena SyntaxTreeArena;
// NOTE: We store our register name mappings here
global mem_arena RegNameArena;
global reg_hash_table RegHashTable;
// NOTE: Stores tokens. Once we parse a statement of tokens, this arena goes empty and is refilled.
global mem_arena TokenArena;
global mem_arena IR_Arena;
global mem_arena TempArena;

global name_state NameState;

#include "nutella_string.cpp"
#include "nutella_linked_list.cpp"

/*
  TODO List:

      - We can have a int for each type and var that says which numbered scope that var is from. Then,
        when we check if a var is defined in a given scope, we just check if the num scope of that var
        is less than or equal to our current scope. We would have to remove all variables and types
        from the list of a given number once we evict that scope to not cause scope num collisions. Take
        a look at how useful this optimization would actually be.

      - For memory, make linked lists of 4 meg or 1 meg blocks of mem where in that block, its just a
        flat array. For other things like types, we can resizeable arrays?

      - Check that all code paths (IsVar/TypeDefined calls, etc) have a error returned if the thing
        searched for isn't found.

      - We need to be able to load include files.

      - We need to store the size of types

      - Currently, we support translating our ops to x86 for int types. We need to allow floats and
        pointers (pointers should just be ints but they behave a bit diff)

      - If statements

      - Func calls

      - Make sure we support a limited size of registers

      - Clean up memory usage for our various strings and better define what goes where
 */

inline void OutputError(char* ErrorMsg)
{
    printf(ErrorMsg);
    exit(0);
}

inline type* GetBasicType(u32 Id)
{
    Assert(Id >= 0 && Id < TypeId_EndTypes);
    type* ArenaMem = (type*)TypeArena.Mem;
    type* Result = ArenaMem + (Id);
    return Result;
}

inline b32 IsTokenLValue(token Token)
{
    b32 Result = Token.Flags == TokenFlag_IsLValue;
    return Result;
}

inline b32 IsTokenConstant(token Token)
{
    b32 Result = Token.Flags == TokenFlag_IsConstant;
    return Result;
}

inline b32 IsTokenBinOp(token Token)
{
    b32 Result = Token.Id > OpId_StartBinOps && Token.Id < OpId_EndBinOps;
    return Result;
}

inline b32 IsTypePointer(type* Type)
{
    b32 Result = (Type->Flags & TypeFlag_IsTypePointer) != 0;
    return Result;
}

inline b32 IsTypeStruct(type* Type)
{
    b32 Result = (Type->Flags & TypeFlag_IsTypeStruct) != 0;
    return Result;
}

inline b32 IsScopeCodeBlock(u32 Id)
{
    b32 Result = (Id < ScopeId_EndCodeScope && Id > ScopeId_StartCodeScope);
    return Result;
}

inline b32 IsSyntaxNodeUniOp(syntax_node* Node)
{
    b32 Result = Node->Id > OpId_StartUniOps && Node->Id < OpId_EndUniOps;
    return Result;
}

inline b32 IsSyntaxNodeBinOp(syntax_node* Node)
{
    b32 Result = Node->Id > OpId_StartBinOps && Node->Id < OpId_EndBinOps;
    return Result;
}

inline type* GetTokenType(token Token)
{
    type* Result = 0;

    if (Token.Id == TokenId_Var)
    {
        // NOTE: This case handles constants as well as variables
        Result = Token.Var->Type;
    }
    else if (Token.Id == TokenId_Type)
    {
        InvalidCodePath;
        Result = Token.Type;
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
        // NOTE: This case handles constants and variables
        Result = Token.Var->Str;
    }
    else if (Token.Id == TokenId_Func)
    {
        Result = Token.Func->Name;
    }
    else if (Token.Id == TokenId_Type) 
    {
        // NOTE: We call this when our var name is a type name
        Result = Token.Type->Str;
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

internal type* IsTypeDefined(string MatchStr)
{
    // NOTE: We don't match against pointer types here. This is used to find if a
    // token is a type
    type* Result = 0;
    
    u8* CurrByte = TypeArena.Mem;
    while (CurrByte < (TypeArena.Mem + TypeArena.Used))
    {
        type* CurrType = (type*)CurrByte;
        if (StringCompare(CurrType->Str, MatchStr))
        {
            Result = CurrType;
            break;
        }
        CurrByte += sizeof(type);

        // NOTE: If we have a struct type, we have to get past all of its members in memory
        if (IsTypeStruct(CurrType))
        {
            CurrByte += (sizeof(type*) + sizeof(string))*CurrType->NumMembers;
        }
    }

    return Result;
}

inline b32 AreSameType(type* Type1, type* Type2)
{
    // NOTE: We guarentee that tokens of same type share the same type pointer if
    // they aren't pointer types. Pointer types will have the same PointerType but we must check
    // NumRef first. We don't allow pointertype to point to another pointer type.
    b32 Result = 0;
    if (IsTypePointer(Type1) && IsTypePointer(Type2))
    {
        Result = Type1->NumRef == Type2->NumRef && Type1->PointerType == Type2->PointerType;
    }
    else
    {
        Result = Type1 == Type2;
    }

    // NOTE: These asserts make sure our struct types are set up correctly
    Assert(!IsTypeStruct(Type1) || (IsTypeStruct(Type1) && Type1->NumMembers > 0));
    Assert(!IsTypeStruct(Type2) || (IsTypeStruct(Type2) && Type2->NumMembers > 0));

    // NOTE: These asserts make sure our pointer types are set up correctly
    Assert(!IsTypePointer(Type1) || (IsTypePointer(Type1) && Type1->NumRef > 0));
    Assert(!IsTypePointer(Type2) || (IsTypePointer(Type2) && Type2->NumRef > 0));
    
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
    var* NewVar = PushStruct(&ScopeArena, var);
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

inline syntax_node* CreateSyntaxNode(u32 Id, u32 NumArgs)
{
    syntax_node* Result = PushStruct(&SyntaxTreeArena, syntax_node);
    Result->Id = Id;
    Result->NumArgs = NumArgs;
    Result->Children = PushArray(&SyntaxTreeArena, syntax_node*, NumArgs);

    return Result;
}

inline scope* AddScope(parser_state* State, u32 ScopeId = ScopeId_None)
{
    temp_mem ScopeTempMem = BeginTempMem(&ScopeArena);
        
    scope* NewScope = PushStruct(&ScopeArena, scope);
    NewScope->TempMem = ScopeTempMem;
    NewScope->PrevScope = State->Scope;
    NewScope->Id = ScopeId;
    InitList(&NewScope->VarList);

    State->Scope->NextScope = NewScope;
    State->Scope = NewScope;

    return NewScope;
}

// TODO: We need to move this
#include "nutella_ir.cpp"
#include "nutella_backend.cpp"
#include "nutella_typechecking.cpp"
#include "nutella_parser.cpp"

internal void ScanFileToTokens(char* File, u32 NumChars, scope* GlobalScope, FILE* OutCFile)
{
    char* CurrChar = File;
    u32 CurrCharCount = 0;
    
    // NOTE: We store tokens until we encounter either a ;, {, }, of end of file. We then give that
    // entire statement of tokens to the parser.

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
        else if (StringCompare(CurrChar, "<=", 2))
        {
            CurrToken->Id = OpId_LessEqual;
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, "<", 1))
        {
            CurrToken->Id = OpId_Less;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, ">=", 2))
        {
            CurrToken->Id = OpId_GreaterEqual;
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, ">", 1))
        {
            CurrToken->Id = OpId_Greater;
            CurrToken->Str = InitStr(CurrChar, 1);
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
        else if (StringCompare(CurrChar, ".", 1))
        {
            CurrToken->Id = OpId_GetMember;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "[", 1))
        {
            CurrToken->Id = OpId_OpenSqBracket;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "]", 1))
        {
            CurrToken->Id = OpId_CloseSqBracket;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, "return", 6))
        {
            CurrToken->Id = ScannerId_Return;
            CurrToken->Str = InitStr(CurrChar, 6);
        }
        else if (StringCompare(CurrChar, "struct", 6))
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
        else if (StringCompare(CurrChar, "true", 4))
        {
            CurrToken->Id = TokenId_Var;
            CurrToken->Flags = TokenFlag_IsConstant;
            CurrToken->Var = PushStruct(&ScopeArena, var);
            CurrToken->Var->Str = InitStr(CurrChar, 4);
            CurrToken->Var->Type = GetBasicType(TypeId_Int);
            CurrCharCount += CurrToken->Var->Str.NumChars;
            CurrChar += CurrToken->Var->Str.NumChars;

            DontChangeCurrChar = true;
        }
        else if (StringCompare(CurrChar, "false", 5))
        {
            CurrToken->Id = TokenId_Var;
            CurrToken->Flags = TokenFlag_IsConstant;
            CurrToken->Var = PushStruct(&ScopeArena, var);
            CurrToken->Var->Str = InitStr(CurrChar, 5);
            CurrToken->Var->Type = GetBasicType(TypeId_Int);
            CurrCharCount += CurrToken->Var->Str.NumChars;
            CurrChar += CurrToken->Var->Str.NumChars;

            DontChangeCurrChar = true;
        }
        else if (IsCharNumeric(*CurrChar))
        {
            u32 NumNumbers = NumCharsToNonInt(CurrChar, CurrCharCount, NumChars);
            CurrToken->Id = TokenId_Var;
            CurrToken->Flags = TokenFlag_IsConstant;
            CurrToken->Var = PushStruct(&ScopeArena, var);
    
            if (*(CurrChar + NumNumbers) == '.')
            {
                NumNumbers += 1;
                u32 NumNumbersAfterDot = NumCharsToNonInt(CurrChar + NumNumbers,
                                                          CurrCharCount + NumNumbers, NumChars);
                NumNumbers += NumNumbersAfterDot;
                CurrToken->Var->Type = GetBasicType(TypeId_Float);
            }
            else
            {
                CurrToken->Var->Type = GetBasicType(TypeId_Int);
            }

            // NOTE: We increment our CurrChar here so don't apply the CurrChar incrementor later
            CurrToken->Var->Str = InitStr(CurrChar, NumNumbers);
            CurrCharCount += CurrToken->Var->Str.NumChars;
            CurrChar += CurrToken->Var->Str.NumChars;

            DontChangeCurrChar = true;
        }
        else if (IsCharIdentifier(*CurrChar))
        {
            CurrToken->Id = ScannerId_Identifier;
            CurrToken->Str = InitStr(CurrChar,
                                     NumCharsToNonIdentifier(CurrChar, CurrCharCount, NumChars));

            // NOTE: We increment our CurrChar here so don't apply the CurrChar incrementor later
            CurrCharCount += CurrToken->Str.NumChars;
            CurrChar += CurrToken->Str.NumChars;

            DontChangeCurrChar = true;
            type* Type = IsTypeDefined(CurrToken->Str);
            if (Type)
            {
                *CurrToken = {};
                CurrToken->Type = Type;
                CurrToken->Id = TokenId_Type;
            }
            else
            {
                var* Var = IsVarDefined(&State, CurrToken->Str);
                if (Var)
                {
                    *CurrToken = {};
                    CurrToken->Var = Var;
                    CurrToken->Id = TokenId_Var;
                }
                else
                {
                    function* Func = IsFuncDefined(CurrToken->Str);
                    if (Func)
                    {
                        *CurrToken = {};
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

internal FILE* CompileFileToIR(char* FileName, scope* GlobalScope, FILE* OutCFile)
{
    // NOTE: We expect the global state (memory arenas and scope) to be setup by this point
    FILE* NutellaFile = fopen(FileName, "rb");
    if (!NutellaFile)
    {
        OutputError("Couldn't find nutella file\n");
    }

    fseek(NutellaFile, 0, SEEK_END);
    u32 FileSizeInBytes = ftell(NutellaFile);
    fseek(NutellaFile, 0, SEEK_SET);

    // TODO: Move the file to a arena as well?
    char* Data = (char*)malloc(FileSizeInBytes + sizeof(char));
    Data[FileSizeInBytes] = 0;
    if (!fread(Data, FileSizeInBytes, 1, NutellaFile))
    {
        OutputError("Couldn't read nutella file\n");
    }
    
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), GlobalScope, OutCFile);

    return NutellaFile;
}

int main(int argc, char** argv)
{
#if 0
    // NOTE: Right now, we assume we always have one argument (the file name)
    // NOTE: We also assume the file is a .nutella file
    char* FileName = argv[1];
    u32 FileNameLen = (u32)strlen(FileName);
    char* OutputFileName = (char*)malloc(sizeof(char)*(FileNameLen - 3));
    memcpy(OutputFileName, FileName, sizeof(char)*(FileNameLen - 7));
    OutputFileName[FileNameLen - 7] = 'c';
    OutputFileName[FileNameLen - 6] = 'p';
    OutputFileName[FileNameLen - 5] = 'p';
    OutputFileName[FileNameLen - 4] = 0;
#else    
    char* FileName = "test1.nutella";
    char* OutputFileName = "test1.cpp";
#endif
    
    // NOTE: We setup our global state
    // TODO: Figure out sizes and methods of expanding
    TypeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    PtrTypeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    FuncArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    ScopeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    SyntaxTreeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    RegNameArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    TokenArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    IR_Arena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    TempArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));

    // NOTE: Setup our global scope and basic types array
    temp_mem ScopeTempMem = BeginTempMem(&ScopeArena);        
    scope* GlobalScope = PushStruct(&ScopeArena, scope);
    GlobalScope->TempMem = ScopeTempMem;
    GlobalScope->Id = ScopeId_Global;

    // NOTE: We are pushing our types onto the arena in the order they appear in our enum!!
    type* IntType = PushStruct(&TypeArena, type);
    {
        char IntText[] = "int";
        IntType->Str = InitStr(IntText, 3);
    }

    type* FloatType = PushStruct(&TypeArena, type);
    {
        char FloatText[] = "float";
        FloatType->Str = InitStr(FloatText, 5);
    }

    type* VoidType = PushStruct(&TypeArena, type);
    {
        char VoidText[] = "void";
        VoidType->Str = InitStr(VoidText, 4);
    }
    InitList(&GlobalScope->VarList);

    // NOTE: Init our register name hash table
    RegHashTable = {};
    // TODO: Figure out a good size here
    RegHashTable.ArraySize = 4000;
    RegHashTable.Array = PushArray(&RegNameArena, reg_hash_entry, RegHashTable.ArraySize);
    RegHashTable.RegArena = &RegNameArena;
    RegHashTable.TempArena = &TempArena;
    
    FILE* OutCFile = fopen(OutputFileName, "wb");
    FILE* NutellaFile = CompileFileToIR(FileName, GlobalScope, OutCFile);
    ConvertIrToAssembly(OutCFile);
                
    fclose(OutCFile);
    fclose(NutellaFile);
    
    return 1;
}
