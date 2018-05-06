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
    // NOTE: These ids are emmitted by the scanner and are later relabeled
    TokenId_ScannerName,
    TokenId_Numeric,

    TokenId_Add,
    TokenId_Equals,

    TokenId_EndLine,

    TokenId_Var,
    TokenId_Type,
};

struct type
{
    char* Text;
    u32 NumChars;
};

struct var
{
    char* Text;
    u32 NumChars;
    type* Type;
};

struct token
{
    u32 Id;
    u32 NumChars;
    char* Text;

    // NOTE: Pointer to the parsed data of what this token represents
    union
    {
        type* Type;
        var* Var;
    };
};

struct expression_node
{
    token Token;

    // TODO: Currently, we only support operators with 2 arguments
    expression_node* Left;
    expression_node* Right;
};

enum IR_Instr
{
    IR_Define,
    IR_Add,
};

#define NUTELLA_COMPILER_H
#endif
