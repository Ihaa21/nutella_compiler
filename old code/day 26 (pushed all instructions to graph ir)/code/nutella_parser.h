#if !defined(NUTELLA_PARSER_H)

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

enum SyntaxNodeId
{
    NodeId_Var = TokenId_Var,
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

    NodeId_Zero = TokenId_NumVals + 1,
    NodeId_Negate = TokenId_NumVals + 2,
    
    NodeId_FuncDef = TokenId_NumVals + 3,
    NodeId_FuncCall = TokenId_NumVals + 4,
    NodeId_Return = TokenId_NumVals + 5,
    
    NodeId_If = ControlId_If,
    NodeIf_ElseIf = TokenId_NumVals + 6,
    NodeId_Else = ControlId_Else,
    NodeId_While = ControlId_While,
};

struct syntax_node
{
    u32 Id;
    
    union
    {
        token Token;
        function* Func;
        string HeaderName;
    };

    u32 NumArgs;
    syntax_node** Children;

    // NOTE: Nodes are linked with neighbours as well
    syntax_node* Next;
};

struct scope
{
    u32 Id;
    linked_list VarList;
    temp_mem TempMem;

    // NOTE: We use this value when our scope contains a if statement
    string ExitIfStatement;

    // TODO: Just allocate these dynamically like syntax nodes
    union
    {
        // NOTE: We use this type if we are in the scope of a struct
        type* StructType;
        
        // NOTE: We use this value if we are in the scope of a function
        function* Func;

        // NOTE: This is used for if statements
        syntax_node** StartCodeBlock;
        
        // NOTE: This is used for while loops
        //struct
        //{
            //string StartHeader;
            //string ExitHeader;
        //};
    };

    // NOTE: Used by if nodes and while loops to track where we exit a scope
    syntax_node** ExitCodeBlock;
    
    scope* PrevScope;
    scope* NextScope;
};

struct parser_state
{
    scope* Scope;
    token* StartToken;
    token* LastToken;
    FILE* OutFile;

    syntax_node* PrevNode;
    
    //ir_state IrState;
};

#define NUTELLA_PARSER_H
#endif
