#if !defined(NUTELLA_BACKEND_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

struct reg_list
{
    virtual_reg* Reg;

    reg_list* Next;
    reg_list* Prev;
};

struct backend_state
{
    // NOTE: This offset is for temporary variables which arent params or defined using IR_Define
    i32 TempStackOffset;
    
    u32 NumEmpty;
    reg_list LRU;
    virtual_reg* LoadedRegisters[6];
    reg_list* ListNodes[6];
    string CPU_RegNames[6];

    //mem_arena RegAllocStack;
    
    u8* CurrByte;
    FILE* OutFile;
};

#define EAX_REG_INDEX 0
#define EBX_REG_INDEX 1
#define EDI_REG_INDEX 5
#define ESI_REG_INDEX 4

#define NUTELLA_BACKEND_H
#endif
