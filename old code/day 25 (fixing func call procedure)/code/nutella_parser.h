#if !defined(NUTELLA_PARSER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

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
};

struct parser_state
{
    scope* Scope;
    token* StartToken;
    token* LastToken;
    FILE* OutFile;
};

#define NUTELLA_PARSER_H
#endif
