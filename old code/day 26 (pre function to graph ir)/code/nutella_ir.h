#if !defined(NUTELLA_IR_H)

/* NOTE: This is partially graphical and linear IR. Code blocks are linear while control flow
         statements use pointers to the code blocks they affect. This makes it easier for us
         to run #run statements in our code.

         We have 2 IR's. The first one is the graphical mix with linear. We then run and convert
         this IR to just linear IR but where our control flow is replaced with jump statements and
         we add register allocation statements.
 */

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
    IR_LoadToMem,
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
    IR_DivIntFlt,
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

struct code_block
{
    
};

struct if_statement
{
    
};

struct while_statement
{
    
};

enum IR_State
{
    IrState_CodeBlock,
};

struct ir_state
{
    u8 CurrState;
};

#define NUTELLA_IR_H
#endif
