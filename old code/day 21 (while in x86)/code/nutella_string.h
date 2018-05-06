#if !defined(NUTELLA_STRING_H)

struct string
{
    u32 NumChars;
    char* Text;
};

inline string AllocStr(mem_arena* Arena, u32 NumChars);
inline string InitStr(char* Text, u32 NumChars);
inline void ConvertIntToStr(char* CurrChar, u32 NumDigits, u32 Index);

#define NUTELLA_STRING_H
#endif
