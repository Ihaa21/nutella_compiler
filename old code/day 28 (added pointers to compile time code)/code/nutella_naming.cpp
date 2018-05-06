
inline virtual_reg* GetTempRegister()
{
    virtual_reg* Result = PushStruct(&RegArena, virtual_reg);
    Result->Flags = RegFlag_IsTemp;
    
    return Result;
}

inline virtual_reg* GetOrCreateRegForToken(token* Token)
{
    if (Token->Var->Reg)
    {
        // NOTE: We already assigned a register to this token
        return Token->Var->Reg;
    }
    else
    {
        virtual_reg* Result = PushStruct(&RegArena, virtual_reg);
        if (IsTokenConstant(*Token))
        {
            Result->Flags |= RegFlag_StoresConstant | RegFlag_IsConstVal;
            Result->Name = Token->Var->Str;
        }
        Result->Type = GetTokenType(*Token);
        
        Token->Var->Reg = Result;
        return Result;
    }
}

inline virtual_reg* CreateConstIntRegister(i32 Val)
{
    virtual_reg* Result = PushStruct(&RegArena, virtual_reg);
    Result->Flags = RegFlag_IsConstVal | RegFlag_StoresConstant;
    Result->Name = ConvertIntToString(&RegArena, Val);
    
    return Result;
}

inline virtual_reg* CreateConstFloatRegister(f32 Val)
{
    virtual_reg* Result = PushStruct(&RegArena, virtual_reg);
    Result->Flags = RegFlag_IsConstVal | RegFlag_StoresConstant;
    Result->Name = ConvertFloatToString(&RegArena, Val);
    
    return Result;
}

inline string GetTempHeader(mem_arena* Arena, name_state* State)
{
    if (State->TempIndex % 10 == 0)
    {
        ++State->TempNumDigits;
    }

    str_build_state StrState = StartBuildingString(Arena);
    AddString("TEMP", 4, &StrState);
    AddString(State->TempIndex, &StrState);
    ++State->TempIndex;
    
    return StrState.Str;
}

inline string GetExitIfHeader(name_state* State)
{
    if (State->ExitIfIndex % 10 == 0)
    {
        ++State->ExitIfIndex;
    }

    str_build_state StrState = StartBuildingString(&RegArena);
    AddString("EXIT_IF", 7, &StrState);
    AddString(State->ExitIfIndex, &StrState);
    ++State->ExitIfIndex;
    
    return StrState.Str;
}

inline string GetWhileStartHeader(name_state* State)
{
    if (State->WhileStartIndex % 10 == 0)
    {
        ++State->WhileStartNumDigits;
    }

    str_build_state StrState = StartBuildingString(&RegArena);
    AddString("WHILE", 5, &StrState);
    AddString(State->WhileStartIndex, &StrState);
    ++State->WhileStartIndex;
    
    return StrState.Str;
}

inline string GetWhileExitHeader(name_state* State)
{
    if (State->WhileExitIndex % 10 == 0)
    {
        ++State->WhileExitNumDigits;
    }

    str_build_state StrState = StartBuildingString(&RegArena);
    AddString("WHILE_EXIT", 10, &StrState);
    AddString(State->WhileExitIndex, &StrState);
    ++State->WhileExitIndex;
    
    return StrState.Str;
}
