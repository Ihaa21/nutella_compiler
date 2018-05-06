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

    // TODO: Remove this, we need to do a extra pass in x86 instead of this
    // NOTE: This is the type checking state
    u32 CurrNumLocals;
};

#define NUTELLA_PARSER_H
#endif
