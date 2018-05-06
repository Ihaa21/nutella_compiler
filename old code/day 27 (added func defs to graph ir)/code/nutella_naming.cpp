
inline virtual_reg* GetTempRegister(reg_hash_table* Table)
{
    virtual_reg* Result = PushStruct(Table->RegArena, virtual_reg);
    *Result = {};
    Result->Flags = RegFlag_IsTemp;
    ++Table->CurrRegCount;

    return Result;
}

inline virtual_reg* GetOrCreateRegister(reg_hash_table* Table, string Str)
{
    virtual_reg* Result = 0;

    // TODO: Make a better hash function ;)
    u32 HashId = 199*Str.NumChars + 103*(u32)Str.Text;
    HashId %= Table->ArraySize;

    reg_hash_entry* Entry = Table->Array + HashId;
    if (Entry->EntryName.NumChars == 0)
    {
        // NOTE: We have a empty cell so we fill it
        Entry->EntryName = Str;
        Result = &Entry->Register;
        ++Table->CurrRegCount;
    }
    else
    {
        // NOTE: We have a filled cell so we check for a match
        b32 FoundMatch = false;

        while (true)
        {            
            if (StringCompare(Entry->EntryName, Str))
            {
                FoundMatch = true;
                break;
            }

            if (Entry->Next)
            {
                Entry = Entry->Next;
            }
            else
            {
                break;
            } 
        }

        if (FoundMatch)
        {
            Result = &Entry->Register;
        }
        else
        {        
            // NOTE: The element does not exist so we attach our entry as a linked list
            Entry->Next = PushStruct(Table->RegArena, reg_hash_entry);
            Entry = Entry->Next;
            Entry->EntryName = Str;
            Result = &Entry->Register;
            ++Table->CurrRegCount;
        }
    }
    
    return Result;
}

inline virtual_reg* GetOrCreateConstIntRegister(reg_hash_table* Table, u32 Val)
{
    temp_mem TempMem = BeginTempMem(&RegArena);
    string Str = ConvertIntToString(&RegArena, Val);
    virtual_reg* Result = GetOrCreateRegister(Table, Str);
    if (Result->Flags == 0)
    {
        // NOTE: Since the register is unset, this means we found a new one
        // and we need to initialize it
        Result->Flags = RegFlag_IsConstVal | RegFlag_StoresConstant;
        Result->Name = Str;
    }
    else
    {
        // NOTE: We already created this const so we clear our allocated string
        ClearTempMem(TempMem);
    }

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

inline string GetExitIfHeader(mem_arena* Arena, name_state* State)
{
    if (State->ExitIfIndex % 10 == 0)
    {
        ++State->ExitIfIndex;
    }

    str_build_state StrState = StartBuildingString(Arena);
    AddString("EXIT_IF", 7, &StrState);
    AddString(State->ExitIfIndex, &StrState);
    ++State->ExitIfIndex;
    
    return StrState.Str;
}

inline string GetWhileStartHeader(mem_arena* Arena, name_state* State)
{
    if (State->WhileStartIndex % 10 == 0)
    {
        ++State->WhileStartNumDigits;
    }

    str_build_state StrState = StartBuildingString(Arena);
    AddString("WHILE", 5, &StrState);
    AddString(State->WhileStartIndex, &StrState);
    ++State->WhileStartIndex;
    
    return StrState.Str;
}

inline string GetWhileExitHeader(mem_arena* Arena, name_state* State)
{
    if (State->WhileExitIndex % 10 == 0)
    {
        ++State->WhileExitNumDigits;
    }

    str_build_state StrState = StartBuildingString(Arena);
    AddString("WHILE_EXIT", 10, &StrState);
    AddString(State->WhileExitIndex, &StrState);
    ++State->WhileExitIndex;
    
    return StrState.Str;
}
