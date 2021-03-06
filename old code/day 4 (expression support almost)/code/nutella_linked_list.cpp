
inline void InitList(linked_list* List)
{
    List->Last = &List->Node;
}

inline linked_list_node* GetFirstNode(linked_list* List)
{
    linked_list_node* Result = List->Node.Next;
    return Result;
}

inline void AddNodeToList(linked_list* List, void* Data)
{
    linked_list_node* Last = (linked_list_node*)malloc(sizeof(linked_list_node));
    *Last = {};
    Last->Data = Data;
    List->Last->Next = Last;
    List->Last = Last;
}

inline void FreeList(linked_list Sentinel)
{
    linked_list_node* CurrNode = Sentinel.Node.Next;

    while (CurrNode)
    {
        linked_list_node* Prev = CurrNode;
        CurrNode = CurrNode->Next;
        free(Prev);
    }
}

internal type* FindTypeNode(linked_list* List, char* MatchString, u32 NumChars)
{
    type* Result = 0;

    linked_list_node* CurrNode = GetFirstNode(List);
    type* CurrType = 0;
    while (CurrNode)
    {
        CurrType = (type*)CurrNode->Data;
        if (CurrType->NumChars == NumChars &&
            StringCompare(MatchString, CurrType->Text, NumChars))
        {
            Result = CurrType;
            break;
        }

        CurrNode = CurrNode->Next;
    }

    return Result;
}

internal var* FindVarNode(linked_list* List, char* MatchString, u32 NumChars)
{
    var* Result = 0;
    
    linked_list_node* CurrNode = GetFirstNode(List);
    var* CurrVar = 0;
    while (CurrNode)
    {
        CurrVar = (var*)CurrNode->Data;
        if (CurrVar->NumChars == NumChars &&
            StringCompare(MatchString, CurrVar->Text, NumChars))
        {
            Result = CurrVar;
            break;
        }

        CurrNode = CurrNode->Next;
    }

    return Result;
}
