#if !defined(NUTELLA_BACKEND_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

struct backend_state
{
    u32 StackOffset;
    u32 CurrRegCount;
    virtual_reg* LoadedRegisters[20];
    string CPU_RegNames[20];

    u8* CurrByte;
    FILE* OutFile;
};

#define NUTELLA_BACKEND_H
#endif
