#if !defined(NUTELLA_BACKEND_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

enum IR_Instr
{
    IR_Define,
    IR_Assign,
    IR_GetMember,
    IR_GetArrayIndex,

    // NOTE: Define uniary operations. Order matters here for uniary ops
    IR_UniaryOpStart,

    IR_AddressOff,
    IR_Dereference,
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
    IR_Add,
    IR_Sub,
    IR_Mul,
    IR_Div,

    IR_BinaryOpEnd,

    // NOTE: Define control flow operations
    IR_StartScope,
    IR_EndScope,
    IR_If,
    IR_ElseIf,
    IR_Else,
    IR_While,
    IR_Struct,
    IR_FuncDecl,
    IR_FuncCall,
    IR_Return,
};

struct ir_instr
{
    u32 Id;
};

#define NUTELLA_BACKEND_H
#endif
