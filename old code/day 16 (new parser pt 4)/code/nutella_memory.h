#if !defined(NUTELLA_MEMORY_H)

// TODO: Make arenas able to resize or expand their memory
struct mem_arena
{
    mm Size;
    mm Used;
    u8* Mem;
};

struct temp_mem
{
    mem_arena* Arena;
    mm Used;
};

inline mem_arena InitArena(void* Mem, mm Size)
{
    mem_arena Result = {};
    Result.Size = Size;
    Result.Used = 0;
    Result.Mem = (u8*)Mem;

    return Result;
}

inline void ClearArena(mem_arena* Arena)
{
    Arena->Used = 0;
}

inline temp_mem BeginTempMem(mem_arena* Arena)
{
    temp_mem TempMem = {};
    TempMem.Arena = Arena;
    TempMem.Used = Arena->Used;

    return TempMem;
}

inline void ClearTempMem(temp_mem TempMem)
{
    TempMem.Arena->Used = TempMem.Used;
}

#define PushStruct(Arena, type) (type*)PushSize(Arena, sizeof(type))
#define PushArray(Arena, type, count) (type*)PushSize(Arena, sizeof(type)*count)
inline void* PushSize(mem_arena* Arena, mm Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void* Result = Arena->Mem + Arena->Used;
    Arena->Used += Size;

    return Result;
}

#define NUTELLA_MEMORY_H
#endif
