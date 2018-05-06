#if !defined(NUTELLA_NAMING_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

// TODO: Merge name_state and reg_hash_table
struct name_state
{
    u32 TempNumDigits;
    u32 TempIndex;
    
    u32 WhileStartNumDigits;
    u32 WhileStartIndex;
    u32 WhileExitNumDigits;
    u32 WhileExitIndex;
};

struct reg_hash_entry
{
    string EntryName;
    string RegName;

    reg_hash_entry* Next;
};

struct reg_hash_table
{
    // TODO: We generate names for locations to jump to for our funcs and conditionals
    
    // NOTE: This is used to generate new register names
    u32 CurrRegCount;
    
    u32 ArraySize;
    reg_hash_entry* Array;
    //reg_hash_entry* FreeList;

    mem_arena* RegArena;
    mem_arena* TempArena;
};

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

inline string GetRegisterNameForVar(reg_hash_table* Table, string Name)
{
    string Result = {};

    // TODO: Make a better hash function ;)
    u32 HashId = 199*Name.NumChars + 103*(u32)Name.Text;
    HashId %= Table->ArraySize;

    reg_hash_entry* Entry = Table->Array + HashId;
    if (Entry->EntryName.NumChars == 0)
    {
        // NOTE: We have a empty cell so we fill it
        Entry->EntryName = Name;
        Entry->RegName = GetRegisterName(Table->RegArena, Table->CurrRegCount++);

        Result = Entry->RegName;
    }
    else
    {
        // NOTE: We have a filled cell so we attach our entry as a linked list
        while (Entry->Next)
        {
            Entry = Entry->Next;
        }

        Entry->Next = PushStruct(Table->RegArena, reg_hash_entry);
        Entry = Entry->Next;
        Entry->EntryName = Name;
        Entry->RegName = GetRegisterName(Table->RegArena, Table->CurrRegCount++);
        
        Result = Entry->RegName;
    }

    return Result;
}

inline string GetTempRegisterName(reg_hash_table* Table)
{
    string Result = GetRegisterName(Table->TempArena, Table->CurrRegCount++);
    return Result;
}

#define NUTELLA_NAMING_H
#endif
