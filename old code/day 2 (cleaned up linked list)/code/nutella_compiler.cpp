
#include "nutella_compiler.h"
#include "nutella_string.cpp"
#include "nutella_linked_list.cpp"

inline b32 IsCharNumeric(char C)
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

    while (IsCharNumeric(*CurrChar))
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

// TODO: Make the tokenid_int not exist, just keep it as a name and parse it as a int later
// and parse names to variable names as well

internal void ScanFileToTokens(char* File, u32 NumChars, token* TokenArray, u32* NumTokens)
{
    token* CurrToken = TokenArray;
    char* CurrChar = File;
    
    while (CurrChar <= (File + NumChars))
    {
        GetPastWhiteSpace(&CurrChar);
        
        if (StringCompare(CurrChar, ":=", 2))
        {
            CurrToken->Id = TokenId_Equals;
            CurrToken->NumChars = 2;
            CurrToken->Text = CurrChar;
            
            CurrChar += 2;
        }
        else if (StringCompare(CurrChar, ";", 1))
        {
            CurrToken->Id = TokenId_EndLine;
            CurrToken->NumChars = 1;
            CurrToken->Text = CurrChar;
            
            CurrChar += 1;
        }
        else if (IsCharNumeric(*CurrChar))
        {
            // NOTE: If the first char is a number, we assume its a integer (no floats yet)
            CurrToken->Id = TokenId_Numeric;
            CurrToken->NumChars = NumCharsToNonInt(CurrChar);
            CurrToken->Text = CurrChar;

            CurrChar += CurrToken->NumChars;
        }
        else
        {
            CurrToken->Id = TokenId_Name;
            CurrToken->NumChars = NumCharsToWhiteSpace(CurrChar);
            CurrToken->Text = CurrChar;

            CurrChar += CurrToken->NumChars;
        }

        ++CurrToken;
        ++(*NumTokens);
    }
}

internal void ParseTokens(token* TokenArray, u32 NumTokens, FILE* OutFile)
{
    /* NOTE:
       We define rulesets(grammar) here and make sure that our token array follows them properly

           Statement - (type) (variable name) (end) |
                       (type) (variable name) (equals) (integer) (end)

    */

    // NOTE: Currently, the only supported type is int
    linked_list TypeList = {};
    InitList(&TypeList);
    type IntType = {};
    char IntText[] = "int";
    IntType.Text = IntText;
    IntType.NumChars = 3;
    AddNodeToList(&TypeList, &IntType);
    
    // NOTE: Stores the variable names in the current scope
    linked_list VarList = {};
    InitList(&VarList);
    
    token* CurrToken = TokenArray;
    u32 CurrTokenCount = 0;
    
    while (CurrTokenCount < NumTokens)
    {
        if (CurrToken->Id == TokenId_Name)
        {
            // NOTE: Loop through types in this scope
            type* DeclareType = FindTypeNode(&TypeList, CurrToken->Text, CurrToken->NumChars);
            if (DeclareType)
            {
                ++CurrToken;
                ++CurrTokenCount;

                if (!(CurrToken->Id == TokenId_Name))
                {
                    printf("Error: type not followed by a variable name\n");
                    break;
                }

                if (FindVarNode(&VarList, CurrToken->Text, CurrToken->NumChars))
                {
                    printf("Error: Variable redefined.\n");
                    break;
                }

                var_name VarName = {};
                VarName.Text = CurrToken->Text;
                VarName.NumChars = CurrToken->NumChars;
                VarName.Type = DeclareType;
                AddNodeToList(&VarList, &VarName);

                ++CurrToken;
                ++CurrTokenCount;

                if (CurrToken->Id == TokenId_EndLine)
                {
                    ++CurrToken;
                    ++CurrTokenCount;

                    // TODO: Do this somewhere else or do a IR lang
                    
                    char Temp1[] = "int ";
                    char Temp2[] = ";\n";

                    char* Temp3 = StringConcat(Temp1, VarName.Text, VarName.NumChars);
                    char* OutputText = StringConcat(Temp3, Temp2, 2);
                    fwrite(OutputText, strlen(OutputText), sizeof(char), OutFile);

                    free(Temp3);
                    free(OutputText);

                    fwrite(OutputText, strlen(OutputText), sizeof(char), OutFile);
                    
                    continue;
                }
                else if (CurrToken->Id != TokenId_Equals)
                {
                    printf("Error: Variable definition followed by unsupported operations\n");
                    break;
                }
                // TODO: Check against the rest of the possible outcomes

                ++CurrToken;
                ++CurrTokenCount;

                // TODO: Reorganize this code and make it cleaner please
                var_name RHS = {};
                
                if (CurrToken->Id == TokenId_Numeric)
                {
                    // TODO: Deduce numeric type based on variable type?
                    RHS.Text = CurrToken->Text;
                    RHS.NumChars = CurrToken->NumChars;
                    //RHS.Type = TypeInt;
                }
                else if (CurrToken->Id == TokenId_Name)
                {
                    var_name* SearchResult = FindVarNode(&VarList, CurrToken->Text, CurrToken->NumChars);
                    if (!SearchResult)
                    {
                        printf("Error: Variable assignment followed by undefined variable.\n");
                        break;
                    }

                    RHS = *SearchResult;
                }
                else
                {
                    printf("Error: Variable assignment followed by invalid expression\n");
                }
                
                ++CurrToken;
                ++CurrTokenCount;

                if (CurrToken->Id == TokenId_EndLine)
                {
                    ++CurrToken;
                    ++CurrTokenCount;

                    char Temp1[] = "int ";
                    char Temp2[] = " = ";
                    char Temp3[] = ";\n";

                    char* Temp4 = StringConcat(Temp1, VarName.Text, VarName.NumChars);
                    char* Temp5 = StringConcat(Temp4, Temp2, 3);
                    char* Temp6 = StringConcat(Temp5, RHS.Text, RHS.NumChars);
                    char* OutputText = StringConcat(Temp6, Temp3, 2);
                    fwrite(OutputText, strlen(OutputText), sizeof(char), OutFile);

                    free(Temp4);
                    free(Temp5);
                    free(Temp6);free(OutputText);
                    
                    continue;
                }
                else if (CurrToken->Id != TokenId_Equals)
                {
                    printf("Error: Variable definition followed by unsupported operations\n");
                    break;
                }
            }

            // NOTE: Loop through variables defined in this scope
        }
        else if (CurrToken->Id == TokenId_Numeric)
        {
            printf("Error: Can't start statement with a number.\n");
            break;
        }
        else if (CurrToken->Id == TokenId_Equals)
        {
            printf("Error: Can't start statement with eqauls.\n");
            break;
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
}

#if 0
internal void OutputCFile(token* TokenArray, u32 NumTokens, FILE* OutCFile)
{
    for (u32 CurrTokenNum = 0; CurrTokenNum <= NumTokens; ++CurrTokenNum)
    {
        token* CurrToken = TokenArray + CurrTokenNum;

        switch (CurrToken->Id)
        {
            case TokenId_Variable:
            {
                fwrite(CurrToken->Text, sizeof(char), CurrToken->NumChars, OutCFile);
                fwrite(" ", sizeof(char), 1, OutCFile);
            } break;

            case TokenId_Int:
            {
                fwrite(CurrToken->Text, sizeof(char), CurrToken->NumChars, OutCFile);
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
#endif

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

    FILE* OutCFile = fopen("test.cpp", "wb");
        
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), TokenArray, &NumTokens);
    ParseTokens(TokenArray, NumTokens, OutCFile);
    
    //OutputCFile(TokenArray, NumTokens, OutCFile);
    
    fclose(OutCFile);
    fclose(NutellaFile);

    return 1;
}
