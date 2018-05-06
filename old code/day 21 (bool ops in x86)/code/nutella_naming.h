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
    
    u32 WhileStartNumDights;
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

inline string GetWhileStartHeader(mem_arena* Arena)
{
    // TODO: Fix this to generate new names
    string Result = AllocStr(Arena, 5);
    Result.Text[0] = 'w';
    Result.Text[1] = 'h';
    Result.Text[2] = 'i';
    Result.Text[3] = 'l';
    Result.Text[4] = 'e';

    return Result;
}

inline string GetWhileExitHeader(mem_arena* Arena)
{
    // TODO: Fix this to generate new names
    string Result = AllocStr(Arena, 4);
    Result.Text[0] = 'e';
    Result.Text[1] = 'x';
    Result.Text[2] = 'i';
    Result.Text[3] = 't';
    
    return Result;
}

#define NUTELLA_NAMING_H
#endif
