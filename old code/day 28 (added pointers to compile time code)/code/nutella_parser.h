#if !defined(NUTELLA_PARSER_H)

enum SyntaxNodeId
{
    NodeId_Token = TokenId_Var ,
    NodeId_Reg = TokenId_NumVals + 1,
    NodeId_Define = OpId_Define,
    NodeId_Dereference = OpId_Dereference,
    NodeId_AddressOff = OpId_AddressOff,
    NodeId_Run = OpId_Run,
    
    NodeId_BinaryEqual = OpId_BinaryEqual,
    NodeId_BinaryNotEqual = OpId_BinaryNotEqual,
    NodeId_Less = OpId_Less,
    NodeId_LessEqual = OpId_LessEqual,
    NodeId_Greater = OpId_Greater,
    NodeId_GreaterEqual = OpId_GreaterEqual,
    
    NodeId_Add = OpId_Add,
    NodeId_Sub = OpId_Sub,
    NodeId_Mul = OpId_Mul,
    NodeId_Div = OpId_Div,
    NodeId_Equal = OpId_Equal,
    
    NodeId_GetMember = OpId_GetMember,
    NodeId_GetArrayIndex = OpId_GetArrayIndex,

    NodeId_Zero = TokenId_NumVals + 2,
    NodeId_Negate = TokenId_NumVals + 3,
    
    NodeId_FuncDef = TokenId_NumVals + 4,
    NodeId_FuncCall = TokenId_NumVals + 5,
    NodeId_Return = TokenId_NumVals + 6,
    
    NodeId_If = ControlId_If,
    NodeId_ElseIf = TokenId_NumVals + 7,
    NodeId_Else = ControlId_Else,
    NodeId_While = ControlId_While,
};

struct syntax_node
{
    u32 Id;
    
    union
    {
        // NOTE: Nodes which are created by the parser that have vars or consts use this
        token* Token;
        // NOTE: We convert the token nodes to contain registers instead
        virtual_reg* Reg;
        
    };

    // TODO: Only allocate this for nodes that need it
    // NOTE: This is used for func calls 
    function* Func;

    u32 NumArgs;
    syntax_node** Children;

    // NOTE: Nodes are linked with neighbours as well
    syntax_node* Next;
};

enum ScopeId
{
    ScopeId_None,

    // NOTE: We can only declare functions, structs and variables
    ScopeId_Global,

    // NOTE: We can only define variables in this scope
    ScopeId_Struct,

    // NOTE: We cannot declare new structs or functions in these scopes
    ScopeId_StartCodeScope,
    
    ScopeId_If,
    ScopeId_ElseIf,
    ScopeId_Else,    
    ScopeId_WhileLoop,
    ScopeId_Function,

    ScopeId_EndCodeScope,
};

struct scope
{
    u32 Id;
    linked_list VarList;
    temp_mem TempMem;

    // TODO: Just allocate these dynamically like syntax nodes
    union
    {
        // NOTE: We use this type if we are in the scope of a struct
        type* StructType;
        
        // NOTE: We use this value if we are in the scope of a function
        function* Func;

        // NOTE: This is used for if and while statements
        syntax_node** StartCodeBlock;
    };

    // NOTE: Used by if nodes and while loops to track where we exit a scope
    syntax_node** ExitCodeBlock;
    
    scope* PrevScope;
    scope* NextScope;
};

struct run_node_list
{
    syntax_node* Node;

    run_node_list* Next;
};

struct parser_state
{
    scope* Scope;
    token* StartToken;
    token* LastToken;
    FILE* OutFile;

    b32 IsPrevIf;
    syntax_node* PrevNode;

    run_node_list* StartList;
    run_node_list* EndList;
};

#define NUTELLA_PARSER_H
#endif
