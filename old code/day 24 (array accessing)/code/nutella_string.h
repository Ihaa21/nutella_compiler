#if !defined(NUTELLA_STRING_H)

struct string
{
    u32 NumChars;
    char* Text;
};

inline string AllocStr(mem_arena* Arena, u32 NumChars);
    
#define NUTELLA_STRING_H
#endif
