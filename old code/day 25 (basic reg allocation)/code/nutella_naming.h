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
    // NOTE: These flags are used during type checking exclusively
    RegFlag_StoresConstant = 1 << 0,
    RegFlag_IsLValue = 1 << 1,

    // NOTE: These flags are used by the backend exclusively
    RegFlag_LoadToMem = 1 << 2,
    RegFlag_InCPU = 1 << 3,
    RegFlag_IsTemp = 1 << 4,
    RegFlag_IsConstVal = 1 << 5,
};

// TODO: How do we wanna handle constants?
struct virtual_reg
{
    u32 Flags;

    // NOTE: For constants, the name is the constant itself
    // NOTE: For registers, the name is the cpu register name
    string Name;
    // NOTE: For temp registers stores the regular address
    // NOTE: For variable registers, stores its index in local var count fr the current func
    // which later becomes a memory address
    u32 MemoryIndex;
    u32 CPU_RegIndex;
};

// NOTE: Used for type checking
struct typed_reg
{
    virtual_reg* Reg;
    type* Type;
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
