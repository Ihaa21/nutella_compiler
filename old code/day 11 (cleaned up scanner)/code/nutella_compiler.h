#if !defined(NUTELLA_COMPILER_H)

/* TODO: Feature List

   - Add type casting for variables
   - Add pointer types and address of/dereference operator (for deref, make it work only if the type
     on the right side of it has a pointer count, otherwise, treat it as a mul op)
   - Add struct definitions that can be used as types (and allow lang to get members from them)
   - Add default values to structs through their definitions
   - Add functions
   - Make the #run keyword work (if we have a pointer to a lot of data, have the ability for the code
     called from the #run keyword to store that data as static variables in the exe that can be accessed
     later through a pointer. Figure that shit out)
   - Add a string type and let the code call C functions (like windows functions or printf in our lang)
   - Add if statements
   - Add while loop and for loop
   - Add array type

 */

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

enum TokenId
{
    TokenId_None,

    TokenId_ConstVar,
    TokenId_Var,
    TokenId_Type,

    // NOTE: Scanner definitions    
    ScannerId_Identifier,
    ScannerId_OpenScope,
    ScannerId_CloseScope,
    ScannerId_EndLine,
    
    // NOTE: Type definitions
    TypeId_StartTypes,
    
    TypeId_Int,
    TypeId_Float,
    TypeId_Void,
    TypeId_PointerType,

    TypeId_EndTypes,

    // NOTE: Operator definitions, We order these from lowest priority to highest
    OpId_Add,
    OpId_Sub,
    OpId_Mul,
    OpId_Div,
    OpId_AddressOff,
    OpId_OpenBracket,
    OpId_CloseBracket,
    OpId_Equals,    
};

struct type
{    
    // NOTE: We choose a pointer type if num ref > 0
    u32 NumRef;
    u32 SizeInBytes;
    union
    {
        type* PointerType;
        string Str;
    };
};

struct var
{
    string Str;
    type* Type;
};

struct func_operator
{
    u32 OpId;
    u32 NumArgs;
    type** ArgTypeList;
    type* ReturnType;

    // TODO: A function pointer for what to do with the taken in types?
};

struct scope
{
    linked_list TypeList;
    linked_list VarList;

    temp_mem TempMem;
    
    scope* PrevScope;
    scope* NextScope;
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

enum IR_Instr
{
    IR_Define,
    IR_Assign,

    // NOTE: Order matters here for uniary ops
    IR_UniaryOpStart,

    //IR_Increment, Make into add with a const 1
    //IR_Decrement, Make into subtract with a const 1
    //IR_Negate,
    IR_AddressOff,
    //IR_Dereference,

    IR_UniaryOpCount,

    // NOTE: Order matters here for binary ops
    IR_BinaryOpStart,
    
    IR_Add,
    IR_Sub,
    IR_Mul,
    IR_Div,

    IR_BinaryOpCount,
};

struct ir_instr
{
    u32 Id;
};

#define NUTELLA_COMPILER_H
#endif
