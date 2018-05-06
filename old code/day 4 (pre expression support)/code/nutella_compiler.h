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

#include "nutella_linked_list.h"

// TODO: Merge token id and parse id enums together
enum TokenId
{
    TokenId_Name,
    TokenId_Numeric,

    TokenId_Add,
    TokenId_Equals,

    TokenId_EndLine,
};

// TODO: The int constant should just be a constant and we type check to make sure
// the constants match the type of the variable
enum ParseId
{
    ParseId_Var,
    ParseId_Type,

    ParseId_Add,
    ParseId_Equals,
    ParseId_EndLine,

    ParseId_Const,
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

#define NUTELLA_COMPILER_H
#endif
