
#include "nutella_compiler.h"

// TODO: Put this into a global state or maybe its okay to keep here?
// NOTE: Stores struct types as well as default types
global mem_arena TypeArena;
global mem_arena FuncArena;
// NOTE: Stores all the temporary scope data (variables and ptr types)
global mem_arena ScopeArena;
global mem_arena SyntaxTreeArena;
global mem_arena TokenArena;
global mem_arena IR_Arena;
global mem_arena StringArena;

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

      - We need to be able to load include files. Add ir variable renaming!!
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
    // TODO: Remove this case
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
    else if (Token.Id == TokenId_Type) 
    {
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

inline type* ChangeRefCountToPtr(type* Type, i32 Change)
{
    Assert(IsTypePointer(Type));
    type* NewType = PushStruct(&TypeArena, type);
    *NewType = *Type;
    if (Type->NumRef == 0)
    {
        OutputError("Error: Cannot dereference non pointer value.\n");
    }
    else
    {
        NewType->NumRef += Change;
        Assert(NewType->NumRef >= 0);
        if (NewType->NumRef == 0)
        {
            // NOTE: We don't need to free the pointer type here since it will be cleared
            // once we exit the scope
            NewType = NewType->PointerType;
        }
    }

    return NewType;
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
    *Result = {};
    Result->Id = Id;
    Result->NumArgs = NumArgs;
    Result->Children = PushArray(&SyntaxTreeArena, syntax_node*, NumArgs);

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
    InitList(&NewScope->VarList);

    State->Scope->NextScope = NewScope;
    State->Scope = NewScope;
}

// TODO: We need to move this
#include "nutella_backend.cpp"

// TODO: Make a table which has variable names, types, and functions as they appear in the code
// and then a mapping to dummy names so that we can always be sure that we don't have name conflicts
global u32 UsedCount = 0;
global const char* VarNames[] = {"a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9"};

internal token SyntaxTreeToIR(syntax_node* Root, FILE* OutCFile)
{
    // NOTE: We just have a constant or variable
    if (Root->Id == TokenId_Var && Root->NumArgs == 0)
    {
        if (IsTypePointer(GetTokenType(Root->Token)))
        {
            Root->Token.Flags = TokenFlag_IsLValue;
        }
        
        return Root->Token;
    }

    // NOTE: This variable is the returned value from all our expressions
    token ResultVarName = {};
    ResultVarName.Id = TokenId_Var;
    ResultVarName.Var = PushStruct(&ScopeArena, var);
    *ResultVarName.Var = {};
    ResultVarName.Var->Str = InitStr((char*)VarNames[UsedCount], 2);
    ++UsedCount;

    // NOTE: The current node is a operator
    switch (Root->Id)
    {
        case OpId_Dereference:
        {
            Assert(Root->NumArgs == 1);
            
            token Var = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeVar = GetTokenType(Var);
            string VarStr = GetTokenString(Var);
            
            if (IsTokenConstant(Var))
            {
                OutputError("Error: Cannot dereference a constant.\n");
            }
            if (Var.Flags != TokenFlag_IsLValue)
            {
                OutputError("Error: We can only dereference a l-value.\n");
            }

            type* NewType = ChangeRefCountToPtr(TypeVar, -1);            
            ResultVarName.Flags = TokenFlag_IsLValue;
            ResultVarName.Var->Type = NewType;
    
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushUniOpInstr(IR_Dereference, VarStr, ResultVarName.Var->Str);
        } break;

        case OpId_AddressOff:
        {
            Assert(Root->NumArgs == 1);
            
            token Var = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeVar = GetTokenType(Var);
            string VarStr = GetTokenString(Var);

            if (IsTokenConstant(Var))
            {
                OutputError("Error: Cannot take address of a constant.\n");
            }
            if (Var.Flags != TokenFlag_IsLValue)
            {
                OutputError("Error: We can only take the address of a l-value.\n");
            }
        
            type* NewType = ChangeRefCountToPtr(TypeVar, 1);            
            ResultVarName.Flags = TokenFlag_IsLValue;
            ResultVarName.Var->Type = NewType;
            
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushUniOpInstr(IR_AddressOff, VarStr, ResultVarName.Var->Str);
        } break;

        case OpId_Negate:
        {
            Assert(Root->NumArgs == 1);
            
            token Var = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeVar = GetTokenType(Var);
            string VarStr = GetTokenString(Var);

            if (!(TypeVar == GetBasicType(TypeId_Int) ||
                  TypeVar == GetBasicType(TypeId_Float)))
            {
                OutputError("Error: Can only negate a variable of type int or float.\n");
            }

            ResultVarName.Var->Type = TypeVar;
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushBinOpInstr(IR_Sub, InitStr("0", 1), VarStr, ResultVarName.Var->Str);
        } break;
        
        case OpId_Run:
        {
            Assert(Root->NumArgs == 1);

            // TODO: Currently this doesnt work properly
            // TODO: We need to properly implement the compile time execution here
            token Var = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeVar = GetTokenType(Var);

            ResultVarName.Flags = TokenFlag_IsConstant;
            ResultVarName.Var->Type = TypeVar;
        } break;

        case OpId_BinaryEqual:
        case OpId_BinaryNotEqual:
        case OpId_Less:
        case OpId_LessEqual:
        case OpId_Greater:
        case OpId_GreaterEqual:            
        {
            u32 IRCode = 0;
            if (Root->Id == OpId_BinaryEqual) {IRCode = IR_Equal;}
            if (Root->Id == OpId_BinaryNotEqual) {IRCode = IR_NotEqual;}
            if (Root->Id == OpId_Less) {IRCode = IR_Less;}
            if (Root->Id == OpId_LessEqual) {IRCode = IR_LessEqual;}
            if (Root->Id == OpId_Greater) {IRCode = IR_Greater;}
            if (Root->Id == OpId_GreaterEqual) {IRCode = IR_GreaterEqual;}
            
            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);
            
            Assert(Root->NumArgs == 2);
            Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
            Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));

            if (!(GetTokenType(Left) == GetBasicType(TypeId_Int) &&
                  GetTokenType(Right) == GetBasicType(TypeId_Int)))
            {
                OutputError("Error: No operator exists for given types.\n");
            }

            ResultVarName.Var->Type = GetBasicType(TypeId_Int);
            if (IsTokenConstant(Left) && IsTokenConstant(Right))
            {
                ResultVarName.Flags = TokenFlag_IsConstant;
            }
        
            string LeftStr = GetTokenString(Left);
            string RightStr = GetTokenString(Right);        
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushBinOpInstr(IRCode, LeftStr, RightStr, ResultVarName.Var->Str);
        } break;

        case OpId_Add:
        case OpId_Sub:
        case OpId_Mul:
        case OpId_Div:
        {
            u32 IRCode = 0;
            if (Root->Id == OpId_Add) {IRCode = IR_Add;}
            if (Root->Id == OpId_Sub) {IRCode = IR_Sub;}
            if (Root->Id == OpId_Mul) {IRCode = IR_Mul;}
            if (Root->Id == OpId_Div) {IRCode = IR_Div;}

            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            type* TypeLeft = GetTokenType(Left);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);
            type* TypeRight = GetTokenType(Right);
            
            Assert(Root->NumArgs == 2);
            Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
            Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));

            if (TypeLeft == GetBasicType(TypeId_Int) &&
                TypeRight == GetBasicType(TypeId_Int))
            {
                ResultVarName.Var->Type = TypeLeft;
            }
            else if (TypeLeft == GetBasicType(TypeId_Int) &&
                     TypeRight == GetBasicType(TypeId_Float))
            {
                ResultVarName.Var->Type = TypeRight;
            }
            else if (TypeLeft == GetBasicType(TypeId_Float) &&
                     TypeRight == GetBasicType(TypeId_Int))
            {
                ResultVarName.Var->Type = TypeLeft;
            }
            else if (TypeLeft == GetBasicType(TypeId_Float) &&
                     TypeRight == GetBasicType(TypeId_Float))
            {
                ResultVarName.Var->Type = TypeLeft;
            }
            else if (IsTypePointer(TypeLeft) &&
                     TypeRight == GetBasicType(TypeId_Int))
            {
                ResultVarName.Flags = TokenFlag_IsLValue;
                ResultVarName.Var->Type = TypeLeft;
            }
            else if (IsTypePointer(TypeLeft) && IsTypePointer(TypeRight) &&
                     TypeLeft == TypeRight)
            {
                ResultVarName.Flags = TokenFlag_IsLValue;
                ResultVarName.Var->Type = TypeLeft;
            }
            else
            {
                OutputError("Error: No operator exists for given types.\n");
            }
            
            if (IsTokenConstant(Left) && IsTokenConstant(Right))
            {
                ResultVarName.Flags = TokenFlag_IsConstant;
            }

            string LeftStr = GetTokenString(Left);
            string RightStr = GetTokenString(Right);        
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushBinOpInstr(IRCode, LeftStr, RightStr, ResultVarName.Var->Str);
        } break;

        case OpId_GetMember:
        {
            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);

            Assert(Root->NumArgs == 2);
            Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
            Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));
        
            type* StructType = GetTokenType(Left);
            if (IsTypePointer(StructType))
            {
                StructType = StructType->PointerType;
            }
            
            if (!IsTypeStruct(StructType))
            {
                OutputError("Error: Cannot access member from a non struct type.\n");
            }

            // NOTE: We get the type of our member and make sure it exists
            type* ArgType = 0;
            u8* CurrByte = StructType->Members;
            for (u32 MemberIndex = 0; MemberIndex < StructType->NumMembers; ++MemberIndex)
            {
                type* CurrArgType = *((type**)CurrByte);
                CurrByte += sizeof(type*);

                string* ArgName = (string*)CurrByte;
                CurrByte += sizeof(string);
                if (StringCompare(GetTokenString(Right), *ArgName))
                {
                    ArgType = CurrArgType;
                    break;
                }
            }

            if (!ArgType)
            {
                OutputError("Error: Member doesn't exist in the struct.\n");
            }
            
            ResultVarName.Var->Type = ArgType;
            ResultVarName.Flags = TokenFlag_IsLValue;
            
            string LeftStr = GetTokenString(Left);
            string RightStr = GetTokenString(Right);        
            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushBinOpInstr(IR_GetMember, LeftStr, RightStr, ResultVarName.Var->Str);
        } break;
        
        case OpId_Equal:
        {            
            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);

            Assert(Root->NumArgs == 2);
            Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
            Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));

            if (!IsTokenLValue(Left))
            {
                OutputError("Error: Left of '=' must be a l-value.\n");
            }
            if (!AreSameType(GetTokenType(Left), GetTokenType(Right)))
            {
                OutputError("Error: Type on the left of '=' must be the same type as on the right.\n");
            }

            PushAssignInstr(GetTokenString(Left), GetTokenString(Right));
        } break;

        case OpId_GetArrayIndex:
        {
            token Left = SyntaxTreeToIR(Root->Children[0], OutCFile);
            token Right = SyntaxTreeToIR(Root->Children[1], OutCFile);

            Assert(Root->NumArgs == 2);
            Assert(Left.Id == TokenId_Var || IsTokenConstant(Left));
            Assert(Right.Id == TokenId_Var || IsTokenConstant(Right));

            if (!IsTypePointer(GetTokenType(Left)))
            {
                OutputError("Error: We can only index array types.\n");
            }
            if (GetTokenType(Right) != GetBasicType(TypeId_Int))
            {
                OutputError("Error: Index of an array must be of type intger.\n");
            }

            ResultVarName.Var->Type = ChangeRefCountToPtr(GetTokenType(Left), -1);
            ResultVarName.Flags = TokenFlag_IsLValue;

            PushDefineInstr(ResultVarName.Var->Type, ResultVarName.Var->Str);
            PushGetArrayIndexInstr(GetTokenString(Left), GetTokenString(Right), ResultVarName.Var->Str);
        } break;
        
        case OpId_FuncCall:
        {
            // TODO: Get rid of this allocation (make it happen on a temp arena)
            token* ArgTokens = (token*)malloc(sizeof(token)*Root->NumArgs);
            for (u32 ArgIndex = 0; ArgIndex < Root->NumArgs; ++ArgIndex)
            {
                ArgTokens[ArgIndex] = SyntaxTreeToIR(Root->Children[ArgIndex], OutCFile);
                if (Root->Func->ArgTypes[ArgIndex] != GetTokenType(ArgTokens[ArgIndex]))
                {
                    OutputError("Error: Type doesn't match the required type for the function arg.\n");
                }
            }

            ResultVarName.Var->Type = Root->Func->ReturnType;
            PushFuncCallInstr(ResultVarName.Var->Str, Root->Func->Name, Root->NumArgs, ArgTokens);

            free(ArgTokens);
        } break;

        case ScannerId_Return:
        {
            if (Root->NumArgs == 0)
            {                
                // TODO: Type check the return value

                PushReturnInstr();
            }
            else if (Root->NumArgs == 1)
            {
                token Token = SyntaxTreeToIR(Root->Children[0], OutCFile);

                // TODO: Type check the return value

                PushReturnInstr(GetTokenString(Token));
            }
            else
            {
                InvalidCodePath;
            }
            
            break;
        }
        
        default:
        {
            InvalidCodePath;
        } break;
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
            
            CurrToken->Var->Str = InitStr(CurrChar, NumCharsToNonInt(CurrChar, CurrCharCount, NumChars));
            CurrCharCount += CurrToken->Var->Str.NumChars;
            CurrChar += CurrToken->Var->Str.NumChars;

            DontChangeCurrChar = true;
        }
        else if (IsCharIdentifier(*CurrChar))
        {
            CurrToken->Id = ScannerId_Identifier;
            CurrToken->Str = InitStr(CurrChar,
                                     NumCharsToNonIdentifier(CurrChar, CurrCharCount, NumChars));

            CurrCharCount += CurrToken->Str.NumChars;
            CurrChar += CurrToken->Str.NumChars;

            DontChangeCurrChar = true;
            type* Type = IsTypeDefined(CurrToken->Str);
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
    TypeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    FuncArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    ScopeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    SyntaxTreeArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    TokenArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    IR_Arena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    StringArena = InitArena(malloc(MegaBytes(4)), MegaBytes(4));
    
    FILE* OutCFile = fopen("test.cpp", "wb");

    // NOTE: Setup our global scope and basic types array
    temp_mem ScopeTempMem = BeginTempMem(&ScopeArena);        
    scope* GlobalScope = PushStruct(&ScopeArena, scope);
    *GlobalScope = {};
    GlobalScope->TempMem = ScopeTempMem;
    GlobalScope->Id = ScopeId_Global;

    // NOTE: We are pushing our types onto the arena in the order they appear in our enum!!
    type* IntType = PushStruct(&TypeArena, type);
    *IntType = {};
    {
        char IntText[] = "int";
        IntType->Str = InitStr(IntText, 3);
    }

    type* FloatType = PushStruct(&TypeArena, type);
    *FloatType = {};
    {
        char FloatText[] = "float";
        FloatType->Str = InitStr(FloatText, 5);
    }

    type* VoidType = PushStruct(&TypeArena, type);
    *VoidType = {};
    {
        char VoidText[] = "void";
        VoidType->Str = InitStr(VoidText, 4);
    }
    InitList(&GlobalScope->VarList);
    
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), GlobalScope, OutCFile);
    ConvertIrToC(OutCFile);
    
    fclose(OutCFile);
    fclose(NutellaFile);

    return 1;
}
