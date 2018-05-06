#if !defined(NUTELLA_NAMING_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

struct name_state
{
    u32 TempNumDigits;
    u32 TempIndex;
    
    u32 WhileStartNumDigits;
    u32 WhileStartIndex;
    u32 WhileExitNumDigits;
    u32 WhileExitIndex;
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

#define NUTELLA_NAMING_H
#endif
