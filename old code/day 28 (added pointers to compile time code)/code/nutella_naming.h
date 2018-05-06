#if !defined(NUTELLA_NAMING_H)

// TODO: Merge name_state and reg_hash_table
struct name_state
{
    u32 TempNumDigits;
    u32 TempIndex;

    u32 ExitIfIndex;
    
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
    RegFlag_InCPU = 1 << 3,
    RegFlag_IsTemp = 1 << 4,
    RegFlag_IsConstVal = 1 << 5,
};

struct virtual_reg
{
    u32 Flags;
    type* Type;

    // NOTE: For constants, the name is the constant itself
    // NOTE: For registers, the name is the cpu register name
    string Name;
    // NOTE: For temp registers stores the regular address
    // NOTE: For variable registers, stores its index in local var count fr the current func
    // which later becomes a memory address
    // NOTE: We also use memory indexes during run time compiled execution. We clear them to 0
    // afterwards.
    union
    {
        i32 MemoryIndex;
        void* DataPtr;
    };
    u32 CPU_RegIndex;
};

#define NUTELLA_NAMING_H
#endif
