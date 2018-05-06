#if !defined(NUTELLA_LINKED_LIST_H)

struct linked_list_node
{
    void* Data;
    linked_list_node* Next;
};

struct linked_list
{
    linked_list_node Node;
    linked_list_node* Last;
};

#define NUTELLA_LINKED_LIST_H
#endif
