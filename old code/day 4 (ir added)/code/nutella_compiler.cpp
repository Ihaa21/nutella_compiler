
#include "nutella_compiler.h"
#include "nutella_string.cpp"
#include "nutella_linked_list.cpp"

/*
  TODO List:
      - Define more strictly what it means to be a statement and expression (using grammars)

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

global u32 ByteCount = 0;
global void* IR_Mem = 0;

// NOTE: We decompose int x = 5 to int x; x = 5; so that we don't need to vary the number
// of arguments/tokens passed
inline void PushDefineInstr(type Type, token NewVar)
{
    ir_instr Instr = {};
    Instr.Id = IR_Define;
    
    void* CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    ByteCount += sizeof(ir_instr);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Type, sizeof(type));
    ByteCount += sizeof(type);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &NewVar, sizeof(token));
    ByteCount += sizeof(token);
}

inline void PushAssignInstr(token Left, token Right)
{
    ir_instr Instr = {};
    Instr.Id = IR_Assign;
    
    void* CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    ByteCount += sizeof(Instr);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Left, sizeof(token));
    ByteCount += sizeof(token);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Right, sizeof(token));
    ByteCount += sizeof(token);    
}

inline void PushAddInstr(token Arg1, token Arg2, token Ret)
{
    ir_instr Instr = {};
    Instr.Id = IR_Add;

    void* CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Instr, sizeof(ir_instr));
    ByteCount += sizeof(Instr);
        
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Arg1, sizeof(token));
    ByteCount += sizeof(token);
    
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Arg2, sizeof(token));
    ByteCount += sizeof(token);
        
    CurrByte = (void*)((u8*)IR_Mem + ByteCount);
    memcpy(CurrByte, &Ret, sizeof(token));
    ByteCount += sizeof(token);
}

internal void ConvertIrToC(FILE* OutFile)
{
    u8* CurrByte = (u8*)IR_Mem;
    
    while (CurrByte < ((u8*)IR_Mem + ByteCount))
    {
        ir_instr* Instr = (ir_instr*)CurrByte;
        CurrByte += sizeof(ir_instr);

        switch(Instr->Id)
        {
            case IR_Define:
            {
                type* Type = (type*)CurrByte;
                CurrByte += sizeof(type);
                token* VarName = (token*)CurrByte;
                CurrByte += sizeof(token);
                
                char Temp1[] = "int ";
                char Temp2[] = ";\n";

                string Temp3 = StringConcat(Temp1, VarName->Str);
                string OutputText = StringConcat(Temp3, InitStr(Temp2, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp3.Text);
                free(OutputText.Text);
            } break;

            case IR_Assign:
            {
                token* LHS = (token*)CurrByte;
                CurrByte += sizeof(token);
                token* RHS = (token*)CurrByte;
                CurrByte += sizeof(token);

                char Temp1[] = " = ";
                char Temp2[] = ";\n";

                string Temp3 = StringConcat(LHS->Str, InitStr(Temp1, 3));
                string Temp4 = StringConcat(Temp3, RHS->Str);
                string OutputText = StringConcat(Temp4, InitStr(Temp2, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp3.Text);
                free(OutputText.Text);
            } break;
            
            case IR_Add:
            {
                token* Arg1 = (token*)CurrByte;
                CurrByte += sizeof(token);
                token* Arg2 = (token*)CurrByte;
                CurrByte += sizeof(token);
                token* Return = (token*)CurrByte;
                CurrByte += sizeof(token);
                
                char Temp1[] = " = ";
                char Temp2[] = " + ";
                char Temp3[] = ";\n";

                string Temp4 = StringConcat(Return->Str, InitStr(Temp1, 3));
                string Temp5 = StringConcat(Temp4, Arg1->Str);
                string Temp6 = StringConcat(Temp5, InitStr(Temp2, 3));
                string Temp7 = StringConcat(Temp6, Arg2->Str);
                string OutputText = StringConcat(Temp7, InitStr(Temp3, 2));
                fwrite(OutputText.Text, OutputText.NumChars, sizeof(char), OutFile);

                free(Temp4.Text);
                free(Temp5.Text);
                free(Temp6.Text);
                free(Temp7.Text);
                free(OutputText.Text);
            } break;
        }
    }
}

internal void RelabelExpressionTokens(token* Tokens, u32 NumTokens, linked_list* VarList,
                                      linked_list* TypeList)
{
    token* Token = Tokens;
    for (u32 TokenId = 0; TokenId < NumTokens; ++TokenId, ++Token)
    {
        switch (Token->Id)
        {
            case TokenId_ScannerName:
            {
                type* InferType = FindTypeNode(TypeList, Token->Str);
                if (InferType)
                {
                    Token->Type = InferType;
                    Token->Id = TokenId_Type;
                }
                else
                {
                    var* InferVar = FindVarNode(VarList, Token->Str);
                    if (InferVar)
                    {
                        Token->Var = InferVar;
                        Token->Id = TokenId_Var;
                    }
                }
            } break;

            case TokenId_Numeric:
            {
                // TODO: When we support floats, infer the type here
                //Token->Id = TokenId_Const;
                // TODO: Store the type of int inside the type pointer here
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
        if (!(CurrToken->Id == TokenId_Var ||
              CurrToken->Id == TokenId_Numeric))
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
        if (!(CurrToken->Id == TokenId_Add))
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
        ResultVarName.Id = TokenId_Var;
        ResultVarName.Str = InitStr((char*)VarNames[UsedCount], 2);
        ++UsedCount;

        // TODO: Deduce the type and pass it here
        // TODO: Pass type struct and varname struct instead of tokens
        // That will make our prog translate tokens to type and var name structs before it becomes
        // ir instructions and C
        type TEMP_IntType = {};
        TEMP_IntType.Str = InitStr("int", 3);
        PushDefineInstr(TEMP_IntType, ResultVarName);
        PushAddInstr(Left, Right, ResultVarName);
                
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

    if (CurrToken->Id == TokenId_ScannerName)
    {
        // NOTE: Loop through types in this scope
        type* DeclareType = FindTypeNode(TypeList, CurrToken->Str);
        if (DeclareType)
        {            
            ++CurrToken;
            ++CurrTokenCount;

            if (!(CurrToken->Id == TokenId_ScannerName))
            {
                printf("Error: type not followed by a variable name\n");
                return;
            }

            if (FindVarNode(VarList, CurrToken->Str))
            {
                printf("Error: Variable redefined.\n");
                return;
            }

            // TODO: Currently this leaks. Figure out how to manage memory
            token* VarToken = CurrToken;
            var* VarName = (var*)malloc(sizeof(var));
            VarName->Str = CurrToken->Str;
            VarName->Type = DeclareType;
            AddNodeToList(VarList, VarName);

            ++CurrToken;
            ++CurrTokenCount;

            if (CurrToken->Id == TokenId_EndLine)
            {
                ++CurrToken;
                ++CurrTokenCount;

                PushDefineInstr(*VarName->Type, *VarToken);
                
                return;
            }
            else if (CurrToken->Id != TokenId_Equals)
            {
                printf("Error: Variable definition followed by unsupported operations\n");
                return;
            }
            // TODO: Check against the rest of the possible outcomes

            ++CurrToken;
            ++CurrTokenCount;

            // NOTE: Due to how parsing works, it is guarenteed that the last token is a end line
            u32 NewNumTokens = NumTokens - CurrTokenCount;
            RelabelExpressionTokens(CurrToken, NewNumTokens, VarList, TypeList);
            expression_node* ExprTree = ParseExpressionToTree(CurrToken, NewNumTokens);
            token RHS = ExpressionTreeToC(ExprTree, OutFile);

            PushDefineInstr(*VarName->Type, *VarToken);
            PushAssignInstr(*VarToken, RHS);
            
            return;
        }

        var* LHS_Var = FindVarNode(VarList, CurrToken->Str);
        if (LHS_Var)
        {
            // TODO: Simplify this, standarize what we actually pass
            token* VarToken = CurrToken;
            
            ++CurrToken;
            ++CurrTokenCount;

            if (!(CurrToken->Id == TokenId_Equals))
            {
                printf("Error: Variable statement without equals operator.\n");
                return;
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

            PushAssignInstr(*VarToken, RHS);
            
            return;
        }

        printf("Error: LHS is not a variable name or variable decleration.\n");
        return;
    }
    else if (CurrToken->Id == TokenId_Numeric)
    {
        printf("Error: Can't start statement with a number.\n");
        return;
    }
    else if (CurrToken->Id == TokenId_Equals)
    {
        printf("Error: Can't start statement with eqauls.\n");
        return;
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
    IntType.Str = InitStr(IntText, 3);
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
            CurrToken->Str = InitStr(CurrChar, 2);
        }
        else if (StringCompare(CurrChar, "+", 1))
        {
            CurrToken->Id = TokenId_Add;
            CurrToken->Str = InitStr(CurrChar, 1);
        }
        else if (StringCompare(CurrChar, ";", 1))
        {
            CurrToken->Id = TokenId_EndLine;
            CurrToken->Str = InitStr(CurrChar, 1);
            FinishedLine = true;
        }
        else if (IsCharNumeric(*CurrChar))
        {
            // NOTE: If the first char is a number, we assume its a integer (no floats yet)
            CurrToken->Id = TokenId_Numeric;
            CurrToken->Str = InitStr(CurrChar, NumCharsToNonInt(CurrChar, CurrCharCount, NumChars));
        }
        else
        {
            CurrToken->Id = TokenId_ScannerName;
            CurrToken->Str = InitStr(CurrChar, NumCharsToWhiteSpace(CurrChar, CurrCharCount, NumChars));
        }

        CurrCharCount += CurrToken->Str.NumChars;
        CurrChar += CurrToken->Str.NumChars;
        
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

    // TODO: Figure out how to manage our memory here
    IR_Mem = malloc(100000);
    u32 NumTokens = 0;
    token TokenArray[10000] = {};
    
    FILE* OutCFile = fopen("test.cpp", "wb");
    
    ScanFileToTokens(Data, (FileSizeInBytes - 1)/sizeof(char), TokenArray, OutCFile);
    ConvertIrToC(OutCFile);
    
    fclose(OutCFile);
    fclose(NutellaFile);

    return 1;
}
