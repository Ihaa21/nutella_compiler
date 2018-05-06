
inline char* StringConcat(char* Str1, char* Str2, u32 Str2Length)
{
    u32 Str1Length = (u32)strlen(Str1);
    char* Result = (char*)malloc(sizeof(char)*(Str1Length + Str2Length + 1));
    memcpy(Result, Str1, sizeof(char)*Str1Length);
    memcpy(Result + Str1Length, Str2, sizeof(char)*Str2Length);
    Result[Str1Length + Str2Length] = 0;

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
