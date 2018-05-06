
#include <cstdio>
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

inline b32 StringCompare(char* Str1, char* Str2, u32 NumChars)
{
    b32 Result = true;

    char* CurrCharStr1 = Str1;
    char* CurrCharStr2 = Str2;
    for (u32 CurrChar = 0; CurrChar < NumChars; ++CurrChar)
    {
        if (*CurrCharStr1 == *CurrCharStr2)
        {
            ++CurrCharStr1;
            ++CurrCharStr2;
        }
        else
        {
            Result = false;
            break;
        }
    }

    return Result;
}

inline b32 IsCharInt(char C)
{
    b32 Result = (C == '0' ||
                  C == '1' ||
                  C == '2' ||
                  C == '3' ||
                  C == '4' ||
                  C == '5' ||
                  C == '6' ||
                  C == '7' ||
                  C == '8' ||
                  C == '9');

    return Result;
}

inline u32 NumCharsToNonInt(char* CurrChar)
{
    u32 Result = 0;

    while (IsCharInt(*CurrChar))
    {
        ++Result;
        ++CurrChar;
    }

    return Result;
}

inline b32 IsCharWhiteSpace(char C)
{
    b32 Result = (C == ' ' ||
                  C == '\n' ||
                  C == '\r' ||
                  C == '\t' ||
                  C == '\v' ||
                  C == '\f');

    return Result;
}

inline u32 NumCharsToWhiteSpace(char* CurrChar)
{
    u32 Result = 0;

    while (!IsCharWhiteSpace(*CurrChar))
    {
        ++Result;
        ++CurrChar;
    }

    return Result;
}

inline void GetPastWhiteSpace(char** CurrChar)
{
    while (IsCharWhiteSpace(**CurrChar))
    {
        ++(*CurrChar);
    }
}

enum TokenId
{
    TokenId_Variable,
    TokenId_Int,
    TokenId_Equals,
    TokenId_EndLine,
    TokenId_TypeInt,
};

struct token
{
    TokenId Id;
    u32 CharCount;
    char* Text;
};

// TODO: Make the tokenid_int not exist, just keep it as a name and parse it as a int later
// and parse names to variable names as well

internal void ScanFileToTokens(char* File, u32 NumChars, token* TokenArray, u32* NumTokens)
{
    token* CurrToken = TokenArray;
    char* CurrChar = File;
    while (CurrChar <= (File + NumChars))
    {
        GetPastWhiteSpace(&CurrChar);
        
        if (StringCompare(CurrChar, "int", 3))
        {
            CurrToken->Id = TokenId_TypeInt;
            CurrToken->CharCount = 3;
            CurrToken->Text = CurrChar;
            
            CurrChar += 3;
        }
        else if (StringCompare(CurrChar, ":=", 2))
        {
            CurrToken->Id = TokenId_Equals;
            CurrToken->CharCount = 2;
            CurrToken->Text = CurrChar;
            
            CurrChar += 2;
        }
        else if (StringCompare(CurrChar, ";", 1))
        {
            CurrToken->Id = TokenId_EndLine;
            CurrToken->CharCount = 1;
            CurrToken->Text = CurrChar;
            
            CurrChar += 1;
        }
        else if (IsCharInt(*CurrChar))
        {
            // NOTE: If the first char is a number, we assume its a integer (no floats yet)
            CurrToken->Id = TokenId_Int;
            CurrToken->CharCount = NumCharsToNonInt(CurrChar);
            CurrToken->Text = CurrChar;

            CurrChar += CurrToken->CharCount;
        }
        else
        {
            // NOTE: Right now, we assume that the only other possibilities are other variable names
            CurrToken->Id = TokenId_Variable;
            CurrToken->CharCount = NumCharsToWhiteSpace(CurrChar);
            CurrToken->Text = CurrChar;

            CurrChar += CurrToken->CharCount;
        }

        ++CurrToken;
        ++(*NumTokens);
    }
}

internal void ParseTokens(token* TokenArray, u32 NumTokens)
{
    /* NOTE:
       We define rulesets(grammar) here and make sure that our token array follows them properly

           Statement - (type) (variable name) (end) |
                       (type) (variable name) (equals) (integer) (end)

    */

    token* CurrToken = TokenArray;
    u32 CurrTokenCount = 0;
    while (CurrTokenCount < NumTokens)
    {
        if (CurrToken->Id == TokenId_TypeInt &&
            (CurrToken+1)->Id == TokenId_Variable &&
            (CurrToken+2)->Id == TokenId_Equals &&
            (CurrToken+3)->Id == TokenId_Int &&
            (CurrToken+4)->Id == TokenId_EndLine)
        {
            CurrToken += 5;
            CurrTokenCount += 5;
        }
        else if (CurrToken->Id == TokenId_TypeInt &&
                 (CurrToken+1)->Id == TokenId_Variable &&
                 (CurrToken+2)->Id == TokenId_EndLine)
        {
            CurrToken += 3;
            CurrTokenCount += 3;
        }
        else
        {
            InvalidCodePath;
        }
    }
}

internal void OutputCFile(token* TokenArray, u32 NumTokens, FILE* OutCFile)
{
    for (u32 CurrTokenNum = 0; CurrTokenNum <= NumTokens; ++CurrTokenNum)
    {
        token* CurrToken = TokenArray + CurrTokenNum;

        switch (CurrToken->Id)
        {
            case TokenId_Variable:
            {
                fwrite(CurrToken->Text, sizeof(char), CurrToken->CharCount, OutCFile);
                fwrite(" ", sizeof(char), 1, OutCFile);
            } break;

            case TokenId_Int:
            {
                fwrite(CurrToken->Text, sizeof(char), CurrToken->CharCount, OutCFile);
                fwrite(" ", sizeof(char), 1, OutCFile);
            } break;

            case TokenId_Equals:
            {
                fwrite("= ", sizeof(char), 2, OutCFile);
            } break;

            case TokenId_EndLine:
            {
                fwrite(";\n", sizeof(char), 2, OutCFile);
            } break;

            case TokenId_TypeInt:
            {
                fwrite("int ", sizeof(char), 4, OutCFile);
            } break;
        }
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

    u32 NumTokens = 0;
    token TokenArray[100] = {};
    
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), TokenArray, &NumTokens);
    ParseTokens(TokenArray, NumTokens);
    
    FILE* OutCFile = fopen("test.cpp", "wb");
    OutputCFile(TokenArray, NumTokens, OutCFile);
    
    fclose(OutCFile);
    fclose(NutellaFile);

    return 1;
}
