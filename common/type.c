#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "all.h"

/*****************************************************************************/

TypDef              stAllocTypDef(enum var_types kind)
{
    TypDef          typ;

    /* Now allocate and clear the type descriptor */

    typ = (TypDef)xmalloc(sizeof( *typ )); memset(typ, 0, sizeof( *typ ));

    typ->tdTypeKind = kind;

    return  typ;
}

void              stDeleteTypDef(TypDef typ)
{
    if (typ)
    {
        switch (typ->tdTypeKind)
        {
            case TYP_PTR:
                stDeleteTypDef(typ->tdRef.tdrBase);
                break;

            case TYP_ARRAY:
                free (typ->tdArr.tdaDims);
                stDeleteTypDef(typ->tdArr.tdaElem);
                break;

            case TYP_FNC:
                stDeleteTypDef(typ->tdFnc.tdfRett);
                List_Destroy(&typ->tdFnc.tdfArgs);
                break;

            default:
                break;
        }
        free (typ);
    }
}

/*****************************************************************************
 *
 *  Allocate an array dimension descriptor.
 */

DimDef              stNewDimDesc(unsigned size, SymDef sym)
{
    DimDef          dim;

    dim =    (DimDef)xmalloc(sizeof(*dim)); memset(dim, 0, sizeof( *dim ));

    dim->ddHiTree = sym;
    if  (size)
    {
        dim->ddIsConst = TRUE;
        dim->ddNoDim   = FALSE;
        dim->ddSize    = size;
    }
    else
    {
        dim->ddIsConst = FALSE;
        dim->ddNoDim   = TRUE;
    }

    return  dim;
}

/*****************************************************************************
 *
 *  Create a new ref/ptr type.
 */

TypDef              stNewPtrType(TypDef elem, BOOL impl)
{
    TypDef type;

    type = stAllocTypDef(TYP_PTR);
    type->tdRef.tdrBase = elem;
    type->tdIsImplicit  = impl;

    return  type;
}

TypDef              stNewArrType(DimDef dims, TypDef elem)
{
    TypDef          type;

    /* Is this a generic (non-zero lower bound) array? */

    type = stAllocTypDef(TYP_ARRAY);

//  arrTypeCnt++;

    type->tdArr.tdaElem  = elem;
    type->tdArr.tdaDims  = dims;

    if  (elem)
        type->tdIsGenArg = elem->tdIsGenArg;

    return  type;
}

/*****************************************************************************
 *
 *  Returns non-zero if the two function argument lists are equivalent.
 */

BOOL stArgsMatch(TypDef typ1, TypDef typ2)
{
    SymDef          *arg1;
    SymDef          *arg2;

    /* Compare the two argument lists */

    for (arg1 = (SymDef *)List_First(typ1->tdFnc.tdfArgs),
         arg2 = (SymDef *)List_First(typ2->tdFnc.tdfArgs);
         arg1 &&
         arg2;
         arg1 = (SymDef *)List_Next((void *)arg1),
         arg2 = (SymDef *)List_Next((void *)arg2))
    {
        if  (!stMatchTypes((*arg1)->sdType, (*arg2)->sdType))
            return FALSE;
    }

    if  (arg1 || arg2)
        return FALSE;

    return  TRUE;
}

/*****************************************************************************
 *
 *  Compare two array types and return true if they are identical/compatible,
 *  depending on the value of 'subtype'.
 */

BOOL stMatchArrays(TypDef typ1, TypDef typ2)
{
AGAIN:

    if  (typ1->tdArr.tdaDims->ddNoDim != typ2->tdArr.tdaDims->ddNoDim)
    {
        if  (typ1->tdArr.tdaDims->ddNoDim)
            return  FALSE;
    }
    else if (!typ1->tdArr.tdaDims->ddNoDim)
    {
        /* ISSUE: Is the following correct? */

        if  (!typ1->tdArr.tdaDims->ddIsConst)
            return FALSE;
        if  (!typ2->tdArr.tdaDims->ddIsConst)
            return FALSE;

        if  (typ1->tdArr.tdaDims->ddSize != typ2->tdArr.tdaDims->ddSize &&
             typ1->tdArr.tdaDims->ddSize != 0 &&
             typ2->tdArr.tdaDims->ddSize != 0)
            return  FALSE;
    }

    typ1 = typ1->tdArr.tdaElem;
    typ2 = typ2->tdArr.tdaElem;

    if  (typ1->tdTypeKind == TYP_ARRAY &&
         typ2->tdTypeKind == TYP_ARRAY)
    {
        goto AGAIN;
    }

    return  stMatchTypes(typ1, typ2);
}

/*****************************************************************************
 *
 *  Given two types (which may potentially come from two different
 *  symbol tables), return true if they represent the same type.
 */

BOOL stMatchType2(TypDef typ1, TypDef typ2)
{
    for (;;)
    {
        enum var_types       kind;

        if  (typ1 == typ2)
            return  TRUE;

        kind = typ1->tdTypeKind;

        if  (kind != typ2->tdTypeKind)
            return  FALSE;

        if  (kind <= TYP_lastIntrins)
            return  TRUE;

        switch  (kind)
        {
        case TYP_FNC:

            /* First match the argument lists */

            if  (!stArgsMatch(typ1, typ2))
                return  FALSE;

            // UNDONE: Match calling conventions and all that ....

            /* Now match the return types */

            typ1 = typ1->tdFnc.tdfRett;
            typ2 = typ2->tdFnc.tdfRett;
            break;

        case TYP_ARRAY:
            return  stMatchArrays(typ1, typ2);

        case TYP_PTR:
            typ1 = typ1->tdRef.tdrBase;
            typ2 = typ2->tdRef.tdrBase;
            break;

        default:
            assert(!"unexpected type kind in typ_mgr::tmMatchTypes()");
        }
    }
}

/*****************************************************************************
 *
 *  Create a new function type.
 */

TypDef stNewFncType(TypDef rett)
{
    TypDef          type;

    type = stAllocTypDef(TYP_FNC);

    type->tdFnc.tdfRett   = rett;
    type->tdFnc.tdfArgs   = List_Create();

    return  type;
}

void stTypeName(TypDef typ, SymDef sym, FILE *dump_file)
{
    void *Cursor;
    const char *comma = "";
    TypDef tmpt;

    switch  (typ->tdTypeKind)
    {
    case TYP_PTR:
        stTypeName(typ->tdArr.tdaElem, NULL, dump_file);
        fputs (" ", dump_file);
        fputs ("*", dump_file);
        if  (sym)
        {
            fputs (stGetSymName(sym), dump_file);
        }
        return;

    case TYP_ARRAY:
        stTypeName(stGetBaseType (typ), NULL, dump_file);
        fputs (" ", dump_file);
        if  (sym)
        {
            fputs (stGetSymName(sym), dump_file);
        }

        tmpt = typ;
        while   (tmpt->tdTypeKind > TYP_lastIntrins)
        {
            if (tmpt->tdArr.tdaDims->ddSize)
            {
                fprintf (dump_file, "[%d]", tmpt->tdArr.tdaDims->ddSize);
            }
            else if (tmpt->tdArr.tdaElem->tdTypeKind > TYP_lastIntrins)
            {
                fputs ("(*)", dump_file);
            }
            else
            {
                fputs ("*", dump_file);
            }
            tmpt = tmpt->tdArr.tdaElem;
        }

        return;

    case TYP_FNC:
        stTypeName(typ->tdFnc.tdfRett, NULL, dump_file);
        fputs (" ", dump_file);
        if  (sym)
        {
            fputs (stGetSymName(sym), dump_file);
        }
        fputs ("(", dump_file);
        for(  Cursor=List_First(typ->tdFnc.tdfArgs)
            ;  Cursor!=NULL
            ;  Cursor = List_Next((void *)Cursor)
            )
        {
            fprintf (dump_file, "%s", comma);
            stTypeName((*(SymDef *)Cursor)->sdType, NULL, dump_file);
            comma = ", ";
        }
        fputs (")", dump_file);
        return;

    case TYP_VOID:
        fputs ("void", dump_file);
        if  (sym)
        {
            fputs (" ", dump_file);
            fputs (stGetSymName(sym), dump_file);
        }
        return;

    case TYP_UNDEF:
        fputs ("<undefined>", dump_file);
        if  (sym)
        {
            fputs (" ", dump_file);
            fputs (stGetSymName(sym), dump_file);
        }
        return;

    default:
        fputs (stIntrinsicTypeName(typ->tdTypeKind), dump_file);
        if  (sym)
        {
            fputs (" ", dump_file);
            fputs (stGetSymName(sym), dump_file);
        }
        return;
    }
}

void stTypeName_zenglj(TypDef typ, SymDef sym, FILE *dump_file)
{
    void *Cursor;
    const char *comma = "";
    TypDef tmpt;

    switch  (typ->tdTypeKind)
    {
    case TYP_PTR:
        stTypeName_zenglj(typ->tdArr.tdaElem, NULL, dump_file);
        fputs (" ", dump_file);
        fputs ("*", dump_file);
        if  (sym)
        {
            DumpSymName_zenglj (sym, dump_file);
        }
        return;

    case TYP_ARRAY:
        stTypeName_zenglj(stGetBaseType (typ), NULL, dump_file);
        fputs (" ", dump_file);
        if  (sym)
        {
            DumpSymName_zenglj (sym, dump_file);
        }

        tmpt = typ;
        while   (tmpt->tdTypeKind > TYP_lastIntrins)
        {
            fprintf (dump_file, "[%d]", tmpt->tdArr.tdaDims->ddSize);
            tmpt = tmpt->tdArr.tdaElem;
        }

        return;

    case TYP_FNC:
        stTypeName_zenglj(typ->tdFnc.tdfRett, NULL, dump_file);
        fputs (" ", dump_file);
        if  (sym)
        {
            DumpSymName_zenglj (sym, dump_file);
        }
        fputs ("(", dump_file);
        for(  Cursor=List_First(typ->tdFnc.tdfArgs)
            ;  Cursor!=NULL
            ;  Cursor = List_Next((void *)Cursor)
            )
        {
            fprintf (dump_file, "%s", comma);
            (*(SymDef *)Cursor)->sdVar.sdvLocal = FALSE;
            stTypeName_zenglj((*(SymDef *)Cursor)->sdType, *(SymDef *)Cursor, dump_file);
            (*(SymDef *)Cursor)->sdVar.sdvLocal = TRUE;
            comma = ", ";
        }
        fputs (")", dump_file);
        return;

    case TYP_VOID:
        fputs ("void", dump_file);
        if  (sym)
        {
            fputs (" ", dump_file);
            DumpSymName_zenglj (sym, dump_file);
        }
        return;

    case TYP_UNDEF:
        fputs ("<undefined>", dump_file);
        if  (sym)
        {
            fputs (" ", dump_file);
            DumpSymName_zenglj (sym, dump_file);
        }
        return;

    default:
        fputs (stIntrinsicTypeName_zenglj(typ->tdTypeKind), dump_file);
        if  (sym)
        {
            fputs (" ", dump_file);
            DumpSymName_zenglj (sym, dump_file);
        }
        return;
    }
}

TypDef CopyType(TypDef srcType)
{
    TypDef dstType;
    void *Cursor;

    dstType = stAllocTypDef(srcType->tdTypeKind);
    memcpy ( dstType, srcType, sizeof(*dstType ) );

    switch  (srcType->tdTypeKind)
    {
    case TYP_PTR:
        dstType->tdRef.tdrBase = CopyType(srcType->tdRef.tdrBase);
        break;

    case TYP_ARRAY:
        dstType->tdArr.tdaElem = CopyType(srcType->tdArr.tdaElem);
        dstType->tdArr.tdaDims = stNewDimDesc (srcType->tdArr.tdaDims->ddSize, srcType->tdArr.tdaDims->ddHiTree);
        break;

    case TYP_FNC:
        dstType->tdFnc.tdfRett = CopyType(srcType->tdFnc.tdfRett);
        dstType->tdFnc.tdfArgs = List_Create();
        for(  Cursor=List_First(srcType->tdFnc.tdfArgs)
            ;  Cursor!=NULL
            ;  Cursor = List_Next((void *)Cursor)
            )
        {
            *(SymDef *)List_NewLast(dstType->tdFnc.tdfArgs, sizeof(SymDef )) = *(SymDef *)Cursor;
        }
        break;

    default:
        break;
    }

    return dstType;
}

int type_size(TypDef type)
{
    if (type->tdTypeKind == TYP_ARRAY) {
        int ts;

        ts = type_size(type->tdArr.tdaElem);

        return max (ts * type->tdArr.tdaDims->ddSize, 4);
        
    } else if (type->tdTypeKind == TYP_DOUBLE) {
        return 8;
    } else if (type->tdTypeKind == TYP_INT || type->tdTypeKind == TYP_FLOAT) {
        return 4;
    } else if (type->tdTypeKind == TYP_PTR) {
        return 4;
    } else {
        /* char, void, function, _Bool */
        return 1;
    }
}

const char *stIntrinsicTypeName(enum var_types vt)
{
    static
    const   char *      typeNames[] =
    {
        #define DEF_TP(tn,sz,al,nm,nm_zenglj,tf) nm,
        #include "typelist.h"
        #undef  DEF_TP
    };

    return  typeNames[vt];
}

const char *stIntrinsicTypeName_zenglj(enum var_types vt)
{
    static
    const   char *      typeNames[] =
    {
        #define DEF_TP(tn,sz,al,nm,nm_zenglj,tf) nm_zenglj,
        #include "typelist.h"
        #undef  DEF_TP
    };

    return  typeNames[vt];
}

TypDef
stGetBaseType(TypDef type)
{
    switch  (type->tdTypeKind)
    {
    case TYP_PTR:
        type = type->tdRef.tdrBase;
        break;

    case TYP_ARRAY:
        type = stGetBaseType(type->tdArr.tdaElem);
        break;

    default:
        break;
    }

    return type;
}
