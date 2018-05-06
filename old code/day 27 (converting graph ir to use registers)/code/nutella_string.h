#if !defined(NUTELLA_STRING_H)

struct string
{
    u32 NumChars;
    char* Text;
};

struct str_build_state
{
    mem_arena* Arena;
    temp_mem TempMem;
    string Str;
};

inline string AllocStr(mem_arena* Arena, u32 NumChars);
    
#define NUTELLA_STRING_H
#endif
