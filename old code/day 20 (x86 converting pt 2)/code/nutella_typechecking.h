#if !defined(NUTELLA_TYPECHECKING_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

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
};

#define NUTELLA_TYPECHECKING_H
#endif
