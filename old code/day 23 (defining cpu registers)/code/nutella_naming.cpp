/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

inline virtual_reg* GetTempRegister(reg_hash_table* Table)
{
    virtual_reg* Result = PushStruct(Table->RegArena, virtual_reg);
    *Result = {};
    ++Table->CurrRegCount;

    return Result;
}

inline virtual_reg* GetOrCreateRegisterForVar(reg_hash_table* Table, var* Var)
{
    virtual_reg* Result = 0;

    // TODO: Make a better hash function ;)
    u32 HashId = 199*Var->Str.NumChars + 103*(u32)Var->Str.Text;
    HashId %= Table->ArraySize;

    reg_hash_entry* Entry = Table->Array + HashId;
    if (Entry->EntryName.NumChars == 0)
    {
        // NOTE: We have a empty cell so we fill it
        Entry->EntryName = Var->Str;
        Result = &Entry->Register;
        Result->Type = Var->Type;
        ++Table->CurrRegCount;
    }
    else
    {
        // NOTE: We have a filled cell so we check for a match
        b32 FoundMatch = false;

        while (true)
        {            
            if (StringCompare(Entry->EntryName, Var->Str))
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
            Entry->EntryName = Var->Str;
            Result = &Entry->Register;
            Result->Type = Var->Type;
            ++Table->CurrRegCount;
        }
    }
    
    return Result;
}

#if 1
inline string GetTempHeader(mem_arena* Arena, name_state* State)
{
    if (State->TempIndex % 10 == 0)
    {
        ++State->TempNumDigits;
    }

    string Result = AllocStr(Arena, State->TempNumDigits + 4);
    Result.Text[0] = 'T';
    Result.Text[1] = 'E';
    Result.Text[2] = 'M';
    Result.Text[3] = 'P';
    ConvertIntToStr(Result.Text + 4, State->TempNumDigits, State->TempIndex);
    ++State->TempIndex;
    
    return Result;
}

inline string GetWhileStartHeader(mem_arena* Arena, name_state* State)
{
    if (State->WhileStartIndex % 10 == 0)
    {
        ++State->WhileStartNumDigits;
    }
    
    string Result = AllocStr(Arena, State->WhileStartNumDigits + 5);
    Result.Text[0] = 'W';
    Result.Text[1] = 'H';
    Result.Text[2] = 'I';
    Result.Text[3] = 'L';
    Result.Text[4] = 'E';
    ConvertIntToStr(Result.Text + 5, State->WhileStartNumDigits, State->WhileStartIndex);
    ++State->WhileStartIndex;
    
    return Result;
}

inline string GetWhileExitHeader(mem_arena* Arena, name_state* State)
{
    if (State->WhileExitIndex % 10 == 0)
    {
        ++State->WhileExitNumDigits;
    }
    
    string Result = AllocStr(Arena, State->WhileExitNumDigits + 10);
    Result.Text[0] = 'W';
    Result.Text[1] = 'H';
    Result.Text[2] = 'I';
    Result.Text[3] = 'L';
    Result.Text[4] = 'E';
    Result.Text[5] = '_';
    Result.Text[6] = 'E';
    Result.Text[7] = 'X';
    Result.Text[8] = 'I';
    Result.Text[9] = 'T';
    ConvertIntToStr(Result.Text + 10, State->WhileExitNumDigits, State->WhileExitIndex);
    ++State->WhileExitIndex;
    
    return Result;
}

inline string GetTempRegisterName(reg_hash_table* Table)
{
    string Result = GetRegisterName(Table->RegArena, Table->CurrRegCount++);
    return Result;
}
#endif
