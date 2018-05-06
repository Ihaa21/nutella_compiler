#if !defined(NUTELLA_IR_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

enum IR_Instr
{
    IR_ClearRegs,
    
    IR_Define,
    IR_Assign,
    IR_AssignToMem,
    IR_GetMember,

    // NOTE: Define uniary operations. Order matters here for uniary ops
    IR_UniaryOpStart,

    IR_AddressOff,
    IR_LoadFromMem,
    IR_Run,
    
    IR_UniaryOpEnd,

    // NOTE: Define binary operations. Order matters here for binary ops
    IR_BinaryOpStart,

    IR_Equal,
    IR_NotEqual,
    IR_Less,
    IR_LessEqual,
    IR_Greater,
    IR_GreaterEqual,
    
    IR_AddIntInt,
    IR_AddFltInt,
    IR_AddFltFlt,

    IR_SubIntInt,
    IR_SubFltInt,
    IR_SubFltFlt,

    IR_MulIntInt,
    IR_MulFltInt,
    IR_MulFltFlt,

    IR_DivIntInt,
    IR_DivFltInt,
    IR_DivFltFlt,

    IR_BinaryOpEnd,

    IR_JumpHeader,
    IR_Jump,
    IR_JumpCond,
    
    IR_FuncDecl,
    IR_Return,
    IR_FuncEnd,
    IR_FuncCall,
};

struct ir_instr
{
    u32 Id;
};

#define NUTELLA_IR_H
#endif
