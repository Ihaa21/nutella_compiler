#if !defined(NUTELLA_BACKEND_H)

struct reg_list
{
    var* Reg;

    reg_list* Next;
    reg_list* Prev;
};

struct backend_state
{
    // NOTE: This offset is for temporary variables which arent params or defined using IR_Define
    i32 TempStackOffset;
    
    u32 NumEmpty;
    reg_list LRU;
    var* LoadedRegisters[6];
    // TODO: Use linked list class ya bish
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
