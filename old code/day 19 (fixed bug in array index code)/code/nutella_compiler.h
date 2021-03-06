#if !defined(NUTELLA_COMPILER_H)

#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <cfloat>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t mm;
typedef uintptr_t umm;

typedef int32_t b32;

#define internal static
#define global static
#define local_global static

#define Assert(Expression) if (!(Expression)) {*(int*)0 = 0;}
#define InvalidCodePath Assert(!"Invalid Code Path")
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define KiloBytes(Val) ((Val)*1024LL)
#define MegaBytes(Val) (KiloBytes(Val)*1024LL)
#define GigaBytes(Val) (MegaBytes(Val)*1024LL)

#include "nutella_memory.h"
#include "nutella_string.h"
#include "nutella_linked_list.h"
#include "nutella_backend.h"

enum BasicTypeIds
{
    TypeId_Int,
    TypeId_Float,
    TypeId_Void,
    
    TypeId_EndTypes,
};

// TODO: These at the moment are mutally exclusive so they shouldn't be different states
enum TypeFlags
{
    TypeFlag_IsTypePointer = 0x1,
    TypeFlag_IsTypeStruct = 0x2,
};

struct type
{
    u32 Flags;
    union
    {
        u32 NumMembers;
        u32 NumRef;
    };
    
    u32 SizeInBytes;
    union
    {
        type* PointerType;

        struct
        {
            string Str;
            u8* Members;
        };
    };
};

struct var
{
    string Str;
    type* Type;
};

struct function
{
    type* ReturnType;
    string Name;
    
    u32 NumArgs;
    type** ArgTypes;
};

enum TokenId
{
    TokenId_None,

    TokenId_Var,
    TokenId_Type,
    TokenId_Func,

    // NOTE: Scanner definitions    
    ScannerId_Identifier,
    ScannerId_OpenScope,
    ScannerId_CloseScope,
    ScannerId_Struct,
    ScannerId_Comma,
    ScannerId_Return,
    ScannerId_EndLine,

    // NOTE: Operator definitions, We order these from lowest priority to highest

    OpId_OpenBracket,
    OpId_CloseBracket,
    OpId_OpenSqBracket,
    OpId_CloseSqBracket,
    OpId_GetArrayIndex,
    OpId_GetMember,
    OpId_Run,
    OpId_FuncCall,
    
    OpId_StartBinOps,

    // TODO: Verify this should be here
    OpId_Equal,    

    OpId_BinaryEqual,
    OpId_BinaryNotEqual,
    OpId_Less,
    OpId_LessEqual,
    OpId_Greater,
    OpId_GreaterEqual,
    
    OpId_Add,
    OpId_Sub,
    OpId_Mul,
    OpId_Div,
    
    OpId_EndBinOps,

    OpId_StartUniOps,

    OpId_Negate,
    OpId_AddressOff,
    OpId_Dereference,
    
    OpId_EndUniOps,
    
    // NOTE: Control flow definitions
    ControlId_If,
    ControlId_Else,
    ControlId_While,
};

enum TokenFlags
{        
    // NOTE: Flags
    TokenFlag_IsLValue,
    TokenFlag_IsConstant,
};

// NOTE: Strings point into our file text. Vars are stored in lists but their text also
// points into the text file.
struct token
{
    u16 Id;
    u16 Flags;
    
    // NOTE: Pointer to the parsed data of what this token represents
    union
    {
        // NOTE: Var stores data if its a variable or a constant
        var* Var;
        type* Type;
        function* Func;

        // NOTE: This is stored if we have an identifier which isn't classified yet
        string Str;
    };
};

enum ScopeId
{
    ScopeId_None,

    // NOTE: We can only declare functions, structs and variables
    ScopeId_Global,

    // NOTE: We can only define variables in this scope
    ScopeId_Struct,

    // NOTE: We cannot declare new structs or functions in these scopes
    ScopeId_StartCodeScope,
    
    ScopeId_If,
    ScopeId_ElseIf,
    ScopeId_Else,    
    ScopeId_WhileLoop,
    ScopeId_Function,

    ScopeId_EndCodeScope,
};

struct scope
{
    u32 Id;
    linked_list VarList;
    temp_mem TempMem;
    
    union
    {
        // NOTE: We use this type if we are in the scope of a struct
        type* StructType;
        // NOTE: We use this value if we are in the scope of a function
        type* FuncReturnType;
    };

    scope* PrevScope;
    scope* NextScope;
};

struct syntax_node
{
    u32 Id;
    
    union
    {
        token Token;
        function* Func;
    };

    u32 NumArgs;
    syntax_node** Children;
};

struct parser_state
{
    scope* Scope;
    token* StartToken;
    token* LastToken;
    FILE* OutFile;
};

#define NUTELLA_COMPILER_H
#endif
