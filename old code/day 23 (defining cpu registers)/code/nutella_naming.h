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

enum Register_Flags
{
    RegFlag_IsConstant = 1 << 0,
    RegFlag_IsLValue = 1 << 1,

    RegFlag_LoadToMem = 1 << 2,
    RegFlag_InCPU = 1 << 3,
};

// TODO: How do we wanna handle constants?
struct virtual_reg
{
    u32 Flags;
    type* Type;

    // TODO: Make a pointer to this data since constants don't need to store a mem index?
    // NOTE: For constants, the name is the constant itself
    // NOTE: For registers, the name is the cpu register name
    string Name;
    u32 MemoryIndex;
    
    // TODO: Well need flags on this to know if we should load it back into memory
    // TODO: Also need a address for its memory location or offset from stack
};

struct reg_hash_entry
{
    string EntryName;
    virtual_reg Register;
    
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
};

#define NUTELLA_NAMING_H
#endif
