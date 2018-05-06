
#include "nutella_compiler.h"
#include "nutella_string.cpp"
#include "nutella_linked_list.cpp"

/*
  TODO List:
      - Define more strictly what it means to be a statement and expression (using grammars)

      - Combine scanning and parsing so that we have much easier memory requirments.

      - Try to make the rules for our grammars more generalized? So that we can specify rules
        and have them checked in a order and not have to have too much rule specific code

      - Figure out memory management since currently we are leaking and using malloc all over the
        place. Probably better to define arenas which we grow as we need memory for different parts
        of our program.

      - Standarize what it means for something to be a string in the compiler code (rn we use both
        definitions and its confusing).

      Problems with grammar based approach:
        - we get bad compiler errors
        - its harder to infer what I want the IR language to be for a given line of code

      Problems with hard coded approach:
        - we need more code to hard code the different parsing paths (might be okay)

      I think I dont want to do the grammar rule checking approach given the cons and how hard it
      is to infer errors and type matching, etc. I still want to parse as i create tokens however,
      just break up my parsing to be incremental or be given a list of tokens for a single line of
      code which i then parse.
 */

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

inline u32 NumCharsToNonInt(char* CurrChar, u32 CurrCharCount, u32 NumChars)
{
    u32 Result = 0;

    while (IsCharNumeric(*CurrChar))
    {
        ++Result;
        ++CurrChar;
    }

    if (Result > (NumChars - CurrCharCount))
    {
        Result = NumChars - CurrCharCount;
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

inline u32 NumCharsToWhiteSpace(char* CurrChar, u32 CurrCharCount, u32 NumChars)
{
    u32 Result = 0;

    while (!IsCharWhiteSpace(*CurrChar))
    {
        ++Result;
        ++CurrChar;
    }

    if (Result > (NumChars - CurrCharCount))
    {
        Result = NumChars - CurrCharCount;
    }

    return Result;
}

inline u32 GetPastWhiteSpace(char** CurrChar)
{
    u32 Result = 0;
    
    while (IsCharWhiteSpace(**CurrChar))
    {
        ++Result;
        ++(*CurrChar);
    }

    return Result;
}

internal void RelabelExpressionTokens(token* Tokens, u32 NumTokens, linked_list* VarList,
                                      linked_list* TypeList)
{
    token* Token = Tokens;
    for (u32 TokenId = 0; TokenId < NumTokens; ++TokenId, ++Token)
    {
        switch (Token->Id)
        {
            case TokenId_Name:
            {
                type* InferType = FindTypeNode(TypeList, Token->Text, Token->NumChars);
                if (InferType)
                {
                    Token->Type = InferType;
                    Token->Id = ParseId_Type;
                }
                else
                {
                    var* InferVar = FindVarNode(VarList, Token->Text, Token->NumChars);
                    if (InferVar)
                    {
                        Token->Var = InferVar;
                        Token->Id = ParseId_Var;
                    }
                }
            } break;

            case TokenId_Numeric:
            {
                // TODO: When we support floats, infer the type here
                Token->Id = ParseId_Const;
                // TODO: Store the type of int inside the type pointer here
            } break;

            case TokenId_Add:
            {
                Token->Id = ParseId_Add;
            } break;

            case TokenId_Equals:
            {
                Token->Id = ParseId_Equals;
            } break;

            case TokenId_EndLine:
            {
                Token->Id = ParseId_EndLine;
            } break;
        }
    }
}

internal expression_node* ParseExpressionToTree(token* Tokens, u32 NumTokens)
{
    expression_node* Result = (expression_node*)malloc(sizeof(expression_node));
    memset(Result, 0, sizeof(expression_node));

    // NOTE: Scan left to right, make sure we have (var) (op) (var) ..
    // NOTE: We assume we don't have a end line at the end

    // TODO: Formalize what data types the parses uses and what data types the scanner uses, etc. 
    
    token* CurrToken = Tokens;
    while (CurrToken <= (Tokens + NumTokens))
    {
        if (!(CurrToken->Id == ParseId_Var ||
              CurrToken->Id == ParseId_Const))
        {
            printf("Error: Expression does not contain a variable name or constant.\n");
            break;
        }

        token* TempToken = CurrToken + 1;
        if (TempToken >= (Tokens + NumTokens))
        {
            Result->Token = *CurrToken;
            break;
        }

        CurrToken = TempToken;
        if (!(CurrToken->Id == ParseId_Add))
        {
            printf("Error: Expression must contain operation after variables.\n");
            break;
        }

        Result->Token = *CurrToken;
        Result->Left = ParseExpressionToTree(CurrToken-1, 1);
        Result->Right = ParseExpressionToTree(CurrToken+1, NumTokens - 2);

        break;
    }

    return Result;
}

// TODO: Remove this!!
global u32 UsedCount = 0;
global const char* VarNames[] = {"a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9"};

internal token ExpressionTreeToC(expression_node* Root, FILE* OutCFile)
{
    if (Root->Left)
    {
        // NOTE: If one branch exists, both branches exist
        Assert(Root->Left && Root->Right);
        token Left = ExpressionTreeToC(Root->Left, OutCFile);
        token Right = ExpressionTreeToC(Root->Right, OutCFile);

        token ResultVarName = {};
        ResultVarName.Id = ParseId_Var;
        ResultVarName.NumChars = 2;
        ResultVarName.Text = (char*)VarNames[UsedCount];
        ++UsedCount;
        
        char Temp1[] = "int ";
        char Temp2[] = " = ";
        char Temp3[] = " + ";
        char Temp4[] = ";\n";

        char* Temp5 = StringConcat(Temp1, ResultVarName.Text, ResultVarName.NumChars);
        char* Temp6 = StringConcat(Temp5, Temp2, 3);
        char* Temp7 = StringConcat(Temp6, Left.Text, Left.NumChars);
        char* Temp8 = StringConcat(Temp7, Temp3, 3);
        char* Temp9 = StringConcat(Temp8, Right.Text, Right.NumChars);
        char* OutputText = StringConcat(Temp9, Temp4, 2);
        fwrite(OutputText, strlen(OutputText), sizeof(char), OutCFile);

        free(Temp5);
        free(Temp6);
        free(Temp7);
        free(Temp8);
        free(Temp9);
        free(OutputText);
        
        return ResultVarName;
    }
    else
    {
        return Root->Token;
    }
}

internal void ParseTokens(linked_list* VarList, linked_list* TypeList, token* TokenArray,
                          u32 NumTokens, FILE* OutFile)
{
/*
  NOTE:
       We define rulesets(grammar) here and make sure that our token array follows them properly

           Statement - (type) (variable name) (end) |
                       (type) (variable name) (equals) (integer) (end)
                       (var_name) (equals) (numeric) (end) |
                       (var_name) (equals) (var_name) (end)
                       (var_name) = (expression)

           Expression - (var + const) |
                        (var + var) |
                        (const + var) |
                        (const + const)

*/

    token* CurrToken = TokenArray;
    u32 CurrTokenCount = 0;
    
    while (CurrTokenCount < NumTokens)
    {
        if (CurrToken->Id == TokenId_Name)
        {
            // NOTE: Loop through types in this scope
            type* DeclareType = FindTypeNode(TypeList, CurrToken->Text, CurrToken->NumChars);
            if (DeclareType)
            {
                ++CurrToken;
                ++CurrTokenCount;

                if (!(CurrToken->Id == TokenId_Name))
                {
                    printf("Error: type not followed by a variable name\n");
                    break;
                }

                if (FindVarNode(VarList, CurrToken->Text, CurrToken->NumChars))
                {
                    printf("Error: Variable redefined.\n");
                    break;
                }

                // TODO: Currently this leaks. Figure out how to manage memory
                var* VarName = (var*)malloc(sizeof(var));
                VarName->Text = CurrToken->Text;
                VarName->NumChars = CurrToken->NumChars;
                VarName->Type = DeclareType;
                AddNodeToList(VarList, VarName);

                ++CurrToken;
                ++CurrTokenCount;

                if (CurrToken->Id == TokenId_EndLine)
                {
                    ++CurrToken;
                    ++CurrTokenCount;

                    // TODO: Do this somewhere else or do a IR lang
                    
                    char Temp1[] = "int ";
                    char Temp2[] = ";\n";

                    char* Temp3 = StringConcat(Temp1, VarName->Text, VarName->NumChars);
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

                // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
                u32 NewNumTokens = NumTokens - CurrTokenCount;
                RelabelExpressionTokens(CurrToken, NewNumTokens, VarList, TypeList);
                expression_node* ExprTree = ParseExpressionToTree(CurrToken, NewNumTokens);
                token RHS = ExpressionTreeToC(ExprTree, OutFile);

                char Temp1[] = "int ";
                char Temp2[] = " = ";
                char Temp3[] = ";\n";

                char* Temp4 = StringConcat(Temp1, VarName->Text, VarName->NumChars);
                char* Temp5 = StringConcat(Temp4, Temp2, 3);
                char* Temp6 = StringConcat(Temp5, RHS.Text, RHS.NumChars);
                char* OutputText = StringConcat(Temp6, Temp3, 2);
                fwrite(OutputText, strlen(OutputText), sizeof(char), OutFile);

                free(Temp4);
                free(Temp5);
                free(Temp6);
                free(OutputText);

                ++CurrToken;
                ++CurrTokenCount;
                
                continue;
            }

            // NOTE: Loop through variables defined in this scope
            var* LHS_Var = FindVarNode(VarList, CurrToken->Text, CurrToken->NumChars);
            if (LHS_Var)
            {
                ++CurrToken;
                ++CurrTokenCount;

                if (!(CurrToken->Id == TokenId_Equals))
                {
                    printf("Error: Variable statement without equals operator.\n");
                    break;
                }

                ++CurrToken;
                ++CurrTokenCount;

                // TODO: Let Parse Expression tree return # nodes touched
                // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
                u32 NewNumTokens = NumTokens - CurrTokenCount;
                RelabelExpressionTokens(CurrToken, NewNumTokens, VarList, TypeList);
                expression_node* ExprTree = ParseExpressionToTree(CurrToken, NewNumTokens);
                token RHS = ExpressionTreeToC(ExprTree, OutFile);

                ++CurrToken;
                ++CurrTokenCount;
                
                char Temp1[] = " = ";
                char Temp2[] = ";\n";

                char* Temp3 = StringConcat(LHS_Var->Text, LHS_Var->NumChars, Temp1, 3);
                char* Temp4 = StringConcat(Temp3, RHS.Text, RHS.NumChars);
                char* OutputText = StringConcat(Temp4, Temp2, 2);
                fwrite(OutputText, strlen(OutputText), sizeof(char), OutFile);

                free(Temp3);
                free(Temp4);
                free(OutputText);

                ++CurrToken;
                ++CurrTokenCount;

                continue;
            }

            printf("Error: LHS is not a variable name or variable decleration.\n");
            break;
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

internal void ScanFileToTokens(char* File, u32 NumChars, token* TokenArray, FILE* OutCFile)
{
    char* CurrChar = File;
    u32 CurrCharCount = 0;

    // TODO: Add functions into scope
    // NOTE: Setup our list of variables and types in scope
    linked_list TypeList = {};
    InitList(&TypeList);
    type IntType = {};
    char IntText[] = "int";
    IntType.Text = IntText;
    IntType.NumChars = 3;
    AddNodeToList(&TypeList, &IntType);
    
    linked_list VarList = {};
    InitList(&VarList);

    // NOTE: We use the token array to store one line of tokens which we then parse. Thus, our
    // memory requirments fall to only storing a single line of tokens

    u32 NumTokens = 0;
    token* CurrToken = TokenArray;
    while (CurrChar <= (File + NumChars))
    {
        CurrCharCount += GetPastWhiteSpace(&CurrChar);

        b32 FinishedLine = false;
        if (StringCompare(CurrChar, ":=", 2))
        {
            CurrToken->Id = TokenId_Equals;
            CurrToken->NumChars = 2;
            CurrToken->Text = CurrChar;
        }
        else if (StringCompare(CurrChar, "+", 1))
        {
            CurrToken->Id = TokenId_Add;
            CurrToken->NumChars = 1;
            CurrToken->Text = CurrChar;
        }
        else if (StringCompare(CurrChar, ";", 1))
        {
            CurrToken->Id = TokenId_EndLine;
            CurrToken->NumChars = 1;
            CurrToken->Text = CurrChar;
            FinishedLine = true;
        }
        else if (IsCharNumeric(*CurrChar))
        {
            // NOTE: If the first char is a number, we assume its a integer (no floats yet)
            CurrToken->Id = TokenId_Numeric;
            CurrToken->NumChars = NumCharsToNonInt(CurrChar, CurrCharCount, NumChars);
            CurrToken->Text = CurrChar;
        }
        else
        {
            CurrToken->Id = TokenId_Name;
            CurrToken->NumChars = NumCharsToWhiteSpace(CurrChar, CurrCharCount, NumChars);
            CurrToken->Text = CurrChar;
        }

        CurrCharCount += CurrToken->NumChars;
        CurrChar += CurrToken->NumChars;
        
        if (FinishedLine)
        {
            ParseTokens(&VarList, &TypeList, TokenArray, NumTokens, OutCFile);
            // TODO: If we don't finish on a end line output a error for now until
            // we support scopes
            // TODO: Do we need to clear the memory?
            NumTokens = 0;
            CurrToken = TokenArray;
        }
        else
        {
            ++NumTokens;
            ++CurrToken;
        }
    }

    Assert(NumTokens == 0);
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
    token TokenArray[10000] = {};
    
    FILE* OutCFile = fopen("test.cpp", "wb");
    
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), TokenArray, OutCFile);
        
    fclose(OutCFile);
    fclose(NutellaFile);

    return 1;
}
