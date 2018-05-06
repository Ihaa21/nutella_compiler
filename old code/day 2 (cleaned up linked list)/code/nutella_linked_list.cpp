
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

inline type* FindTypeNode(linked_list* List, char* MatchString, u32 NumChars)
{
    type* Result = 0;
    
    linked_list_node* CurrNode = GetFirstNode(List);
    type* CurrType = (type*)CurrNode->Data;
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

    return CurrType;
}

inline var_name* FindVarNode(linked_list* List, char* MatchString, u32 NumChars)
{
    var_name* Result = 0;
    
    linked_list_node* CurrNode = GetFirstNode(List);
    var_name* CurrVar = 0;
    while (CurrNode)
    {
        CurrVar = (var_name*)CurrNode->Data;
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
