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

#include "nutella_string.h"
#include "nutella_linked_list.h"

// TODO: The int constant should just be a constant and we type check to make sure
// the constants match the type of the variable
enum TokenId
{
    TokenId_None,
    
    // NOTE: These ids are emmitted by the scanner and are later relabeled
    TokenId_ScannerName,
    TokenId_Numeric,

    // NOTE: We order these from lowest priority to highest
    TokenId_Add,
    TokenId_Sub,
    TokenId_Mul,
    TokenId_Div,
    TokenId_OpenBracket,
    TokenId_CloseBracket,

    TokenId_Equals,

    TokenId_EndLine,

    // TODO: Support string constants
    // NOTE: The order of const int followed by other const ints must be preserved for basic types
    TokenId_ConstInt,
    TokenId_ConstStr,
    
    TokenId_Var,
    TokenId_Type,
};

struct type
{
    string Str;
};

struct var
{
    string Str;
    type* Type;
};

// NOTE: Strings point into our file text. Types and Vars are stored in lists but their text also
// points into the text file.
struct token
{
    u32 Id;
    
    // NOTE: Pointer to the parsed data of what this token represents
    union
    {
        string Str;
        type* Type;
        var* Var;
    };
};

struct expression_node
{
    union
    {
        u32 OperatorId;
        token Token;
    };
    
    // TODO: Currently, we only support operators with 2 arguments
    expression_node* Left;
    expression_node* Right;
};

struct scope
{
    // TODO: Add functions into scope
    linked_list TypeList;
    linked_list VarList;

    scope* PrevScope;
    scope* NextScope;
};

enum IR_Instr
{
    IR_Define,
    IR_Assign,
    
    IR_Add,
    IR_Sub,
    IR_Mul,
    IR_Div,
};

struct ir_instr
{
    u32 Id;
};

#define NUTELLA_COMPILER_H
#endif
