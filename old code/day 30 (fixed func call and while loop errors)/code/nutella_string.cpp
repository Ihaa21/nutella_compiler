
inline string AllocStr(mem_arena* Arena, u32 NumChars)
{
    string Result = {};
    Result.Text = PushArray(Arena, char, NumChars);
    Result.NumChars = NumChars;

    return Result;
}

inline string InitStr(char* Text, u32 NumChars)
{
    string Result = {};
    Result.Text = Text;
    Result.NumChars = NumChars;

    return Result;
}

inline i32 ConvertStringToInt(string Str)
{
    i32 Result = 0;
    for (u32 DigitIndex = 0; DigitIndex < Str.NumChars; ++DigitIndex)
    {
        char Digit = Str.Text[DigitIndex];
        i32 DigitVal = 0;

        if (Digit == '0')
        {
            DigitVal = 0;
        }
        else if (Digit == '1')
        {
            DigitVal = 1;
        }
        else if (Digit == '2')
        {
            DigitVal = 2;
        }
        else if (Digit == '3')
        {
            DigitVal = 3;
        }
        else if (Digit == '4')
        {
            DigitVal = 4;
        }
        else if (Digit == '5')
        {
            DigitVal = 5;
        }
        else if (Digit == '6')
        {
            DigitVal = 6;
        }
        else if (Digit == '7')
        {
            DigitVal = 7;
        }
        else if (Digit == '8')
        {
            DigitVal = 8;
        }
        else if (Digit == '9')
        {
            DigitVal = 9;
        }
        
        Result *= 10;
        Result = (Result + DigitVal);
    }

    return Result;
}

inline f32 ConvertStringToFloat(string Str)
{
    f32 Result = 0.0;
    b32 OnPositiveSide = true;
    f32 Frac = 1.0;
    for (u32 DigitIndex = 0; DigitIndex < Str.NumChars; ++DigitIndex)
    {
        char Digit = Str.Text[DigitIndex];
        f32 DigitVal = 0.0;

        if (Digit == '0')
        {
            DigitVal = 0.0;
        }
        else if (Digit == '1')
        {
            DigitVal = 1.0;
        }
        else if (Digit == '2')
        {
            DigitVal = 2.0;
        }
        else if (Digit == '3')
        {
            DigitVal = 3.0;
        }
        else if (Digit == '4')
        {
            DigitVal = 4.0;
        }
        else if (Digit == '5')
        {
            DigitVal = 5.0;
        }
        else if (Digit == '6')
        {
            DigitVal = 6.0;
        }
        else if (Digit == '7')
        {
            DigitVal = 7.0;
        }
        else if (Digit == '8')
        {
            DigitVal = 8.0;
        }
        else if (Digit == '9')
        {
            DigitVal = 9.0;
        }
        else if (Digit == '.')
        {
            OnPositiveSide = false;
            continue;
        }
        else
        {
            InvalidCodePath;
        }

        if (OnPositiveSide)
        {
            Result *= 10.0f;
            Result = (Result + DigitVal);
        }
        else
        {
            Frac *= 10.0;
            Result = Result + (DigitVal / Frac);
        }
    }

    return Result;
}

inline string ConvertIntToString(mem_arena* Arena, u32 Val)
{
    string Result = {};
    
    u32 NumDigits = Val > 0 ? (u32)log10((f64)Val) + 1 : 1;
    Result = AllocStr(Arena, NumDigits);
    for (i32 DigitIndex = NumDigits - 1; DigitIndex >= 0; --DigitIndex)
    {
        u32 Digit = Val % 10;

        char DigitChar = 0;
        if (Digit == 0)
        {
            DigitChar = '0';
        }
        else if (Digit == 1)
        {
            DigitChar = '1';
        }
        else if (Digit == 2)
        {
            DigitChar = '2';
        }
        else if (Digit == 3)
        {
            DigitChar = '3';
        }
        else if (Digit == 4)
        {
            DigitChar = '4';
        }
        else if (Digit == 5)
        {
            DigitChar = '5';
        }
        else if (Digit == 6)
        {
            DigitChar = '6';
        }
        else if (Digit == 7)
        {
            DigitChar = '7';
        }
        else if (Digit == 8)
        {
            DigitChar = '8';
        }
        else if (Digit == 9)
        {
            DigitChar = '9';
        }

        Result.Text[DigitIndex] = DigitChar;
        Val = Val / 10;
    }
    
    return Result;
}

inline string ConvertFloatToString(mem_arena* Arena, f32 Val)
{
    string Result = {};

    char Text[256];
    sprintf(Text, "%f", Val);
    
    u32 NumDigits = (u32)strlen(Text);
    Result = AllocStr(Arena, NumDigits);
    sprintf(Result.Text, "%f", Val);
    
    return Result;
}

inline string StringConcat(char* Str1, string Str2)
{
    u32 Str1Length = (u32)strlen(Str1);
    u32 ResultLength = Str1Length + Str2.NumChars;
    string Result = InitStr((char*)malloc(sizeof(char)*(ResultLength)), ResultLength);
    memcpy(Result.Text, Str1, sizeof(char)*Str1Length);
    memcpy(Result.Text + Str1Length, Str2.Text, sizeof(char)*Str2.NumChars);
    
    return Result;
}

inline string StringConcat(string Str1, char* Str2)
{
    u32 Str2Length = (u32)strlen(Str2);
    u32 ResultLength = Str1.NumChars + Str2Length;
    string Result = InitStr((char*)malloc(sizeof(char)*(ResultLength)), ResultLength);
    memcpy(Result.Text, Str1.Text, sizeof(char)*Str1.NumChars);
    memcpy(Result.Text + Str2Length, Str2, sizeof(char)*Str2Length);
    
    return Result;
}

inline string StringConcat(string Str1, string Str2)
{
    u32 ResultLength = Str1.NumChars + Str2.NumChars;
    string Result = InitStr((char*)malloc(sizeof(char)*(ResultLength)), ResultLength);
    memcpy(Result.Text, Str1.Text, sizeof(char)*Str1.NumChars);
    memcpy(Result.Text + Str1.NumChars, Str2.Text, sizeof(char)*Str2.NumChars);
    
    return Result;
}

inline b32 StringCompare(string Str1, string Str2)
{
    if (Str1.NumChars != Str2.NumChars)
    {
        return false;
    }
    
    b32 Result = true;
    char* CurrCharStr1 = Str1.Text;
    char* CurrCharStr2 = Str2.Text;
    for (u32 CurrChar = 0; CurrChar < Str1.NumChars; ++CurrChar)
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

inline b32 IsCharIdentifier(char C)
{
    b32 Result = ((C >= 'a' && C <= 'z') ||
                  (C >= 'A' && C <= 'Z') ||
                  IsCharNumeric(C));

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

inline u32 NumCharsToNonIdentifier(char* CurrChar, u32 CurrCharCount, u32 NumChars)
{
    u32 Result = 0;

    while (IsCharIdentifier(*CurrChar) && Result <= (NumChars - CurrCharCount + 1))
    {
        ++Result;
        ++CurrChar;
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

inline str_build_state StartBuildingString(mem_arena* Arena)
{
    str_build_state Result = {};
    Result.Arena = Arena;
    Result.TempMem = BeginTempMem(Arena);
    Result.Str.Text = (char*)(Arena->Mem + Arena->Used);
    return Result;
}

inline void AddString(u32 Val, str_build_state* State)
{
    string Str = ConvertIntToString(State->Arena, Val);
    State->Str.NumChars += Str.NumChars;
}

inline void AddString(char* Str, u32 Length, str_build_state* State)
{
    char* AllocatedStr = PushArray(State->Arena, char, Length);
    memcpy(AllocatedStr, Str, Length*sizeof(char));
    State->Str.NumChars += Length;
}

inline void AddString(string Str, str_build_state* State)
{
    char* AllocatedStr = PushArray(State->Arena, char, Str.NumChars);
    memcpy(AllocatedStr, Str.Text, Str.NumChars*sizeof(char));
    State->Str.NumChars += Str.NumChars;
}

inline void EndBuildingString(str_build_state* State)
{
    ClearTempMem(State->TempMem);
}
