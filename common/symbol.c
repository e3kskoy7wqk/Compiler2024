/****************************************************/
/* File: symbol.c                                   */
/*  符号表管理                                      */
/****************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "all.h"

int
CompareSymbolID (SymDef a, SymDef b, void *unused)
{
    return a->uid - b->uid;
}

void stInit(SymTab stab)
{
    stab->cmpCurScp = NULL;
    stab->sym    = NULL;
    stab->id_map = avl_create((avl_comparison_func *)CompareSymbolID, NULL, NULL);
}

void stDeinit(SymTab stab)
{
    if  (stab)
    {
        stab->cmpCurScp = NULL;
        stRemoveSym(stab->sym);
        avl_destroy(stab->id_map, (avl_item_func *)NULL);
    }
}

static int CompareSymbolString (SymDef pSymbol1, SymDef pSymbol2, void *lpParam)
{
    int     cmp;

    //  compare case-insensitive value of the nodes

    if (pSymbol1->sdName == NULL && pSymbol2->sdName == NULL)
      cmp = pSymbol1->uid - pSymbol2->uid;
    else if (pSymbol1->sdName == NULL)
      cmp = - 1;
    else if (pSymbol2->sdName == NULL)
      cmp = 1;
    else if (pSymbol1->sdSymKind != pSymbol2->sdSymKind)
      cmp = pSymbol1->sdSymKind - pSymbol2->sdSymKind;
    else
      cmp = strcmp(pSymbol1->sdName, pSymbol2->sdName);

    if (cmp < 0) {
        return - 1;

    } else if (cmp > 0) {
        return 1;

    } else {
        return 0;
    }
}

static int
cmp_index (const void *t1, const void *t2, void *unused)
{
    return ((ConstVal)t1)->index - ((ConstVal)t2)->index;
}

int compare_constant (ConstVal t1, ConstVal t2, void *unused)
{
    if  (t1->cvVtyp != t2->cvVtyp)
    {
        return  t1->cvVtyp - t2->cvVtyp;
    }
    switch (t1->cvVtyp)
    {
    case TYP_INT:
        return (t1->cvValue.cvIval < t2->cvValue.cvIval) ? (-1) : (t1->cvValue.cvIval > t2->cvValue.cvIval);

    case TYP_FLOAT:
        if ( t1->cvValue.cvFval < t2->cvValue.cvFval )
            return -1 ;
        if ( t1->cvValue.cvFval > t2->cvValue.cvFval )
            return 1 ;
        if ( t1->cvValue.cvFval == t2->cvValue.cvFval )
            return 0 ;
        if ( isnan (t1->cvValue.cvFval) )
            return isnan (t2->cvValue.cvFval) ? 0 : -1 ;
        return 1 ;

    case TYP_DOUBLE:
        if ( t1->cvValue.cvDval < t2->cvValue.cvDval )
            return -1 ;
        if ( t1->cvValue.cvDval > t2->cvValue.cvDval )
            return 1 ;
        if ( t1->cvValue.cvDval == t2->cvValue.cvDval )
            return 0 ;
        if ( isnan (t1->cvValue.cvDval) )
            return isnan (t2->cvValue.cvDval) ? 0 : -1 ;
        return 1 ;

    default:
        break;
    }
    return 0;
}

ConstVal
GetConstVal(SymDef sym, unsigned index)
{
    ConstVal v;
    struct constVal buf;

    if  (sym->sdSymKind != SYM_VAR)
    {
        return NULL;
    }

    buf.index = index;
    v = (ConstVal)avl_find (sym->sdVar.sdvCnsVal, &buf);

    return v;
}

BOOL SetIconVal(SymDef sym, int ival, unsigned index)
{
    ConstVal v;

    v = GetConstVal (sym, index);
    if (v)
    {
        v->cvValue.cvIval = ival;
        v->cvVtyp = TYP_INT;
        return TRUE;
    }

    if  (sym->sdSymKind != SYM_VAR)
    {
        return FALSE;
    }

    v = (ConstVal)xmalloc (sizeof (*v));
    memset(v, 0, sizeof (*v));
    v->cvValue.cvIval = ival;
    v->index = index;
    v->cvVtyp = TYP_INT;
    avl_insert(sym->sdVar.sdvCnsVal, v);
    return TRUE;
}

BOOL SetFconVal(SymDef sym, float fval, unsigned index)
{
    ConstVal v;

    v = GetConstVal (sym, index);
    if (v)
    {
        v->cvValue.cvFval = fval;
        v->cvVtyp = TYP_FLOAT;
        return TRUE;
    }

    if  (sym->sdSymKind != SYM_VAR)
    {
        return FALSE;
    }

    v = (ConstVal)xmalloc (sizeof (*v));
    memset(v, 0, sizeof (*v));
    v->cvValue.cvFval = fval;
    v->index = index;
    v->cvVtyp = TYP_FLOAT;
    avl_insert(sym->sdVar.sdvCnsVal, v);
    return TRUE;
}

BOOL SetDconVal(SymDef sym, double dval, unsigned index)
{
    ConstVal v;

    v = GetConstVal (sym, index);
    if (v)
    {
        v->cvValue.cvDval = dval;
        v->cvVtyp = TYP_FLOAT;
        return TRUE;
    }

    if  (sym->sdSymKind != SYM_VAR)
    {
        return FALSE;
    }

    v = (ConstVal)xmalloc (sizeof (*v));
    memset(v, 0, sizeof (*v));
    v->cvValue.cvDval = dval;
    v->index = index;
    v->cvVtyp = TYP_FLOAT;
    avl_insert(sym->sdVar.sdvCnsVal, v);
    return TRUE;
}

SymDef stDeclareSym(SymTab      stab,
                    char       *name,
                    enum symbolKinds kind)
{
    SymDef          sym;
    static int counter = 0;

    sym = (SymDef)xmalloc(sizeof (*sym));
    memset(sym, 0, sizeof (*sym));        // ISSUE: is this a good idea?

    sym->uid = ++counter;

    if  (sdHasScope(kind))
        sym->sdScope.sdScope = avl_create((avl_comparison_func *)CompareSymbolString, NULL, NULL);
    else if (kind == SYM_VAR)
    {
        sym->sdVar.sdvCnsVal = avl_create(cmp_index, NULL, NULL);
    }

    /* Allocate the symbol and fill in some basic information */

    if  (name)
    {
        sym->sdName = (char *) xmalloc (strlen (name)+1);
        strcpy(sym->sdName, name);
    }
    sym->sdSymKind      = kind;
    sym->sdParent       = stab->cmpCurScp;
    sym->stab       = stab;

    /* Add the symbol to the parent's list of children (if there is a parent) */

    if  (stab->cmpCurScp)
    {
        avl_replace(stab->cmpCurScp->sdScope.sdScope, sym);
    }
    else
    {
        stab->sym = sym;
    }

    avl_insert(stab->id_map, sym);

//  if  (name && !strcmp(name->idSpelling(), "<whatever>")) forceDebugBreak();
//  if  ((int)sym == 0xADDRESS                            ) forceDebugBreak();

    return sym;
}

/*****************************************************************************
 *
 *  Remove the given symbol from the symbol table.
 */

void stRemoveSym(SymDef sym)
{
    if (!sym)
        return;

    if  (sym->sdParent)
    {
        avl_delete(sym->sdParent->sdScope.sdScope, sym);
    }

    if  (sdHasScope(sym->sdSymKind))
    {
        struct avl_traverser trav;
        SymDef iter;
        SymDef *Cursor;
        LIST temp = List_Create();

        /* 因为排序平衡二叉树不允许一边遍历一边删除，所以我们把要删除的结点放在一个
           链表中，然后遍历链表删除它们。  */
        for(  iter = (SymDef)avl_t_first (&trav, sym->sdScope.sdScope)
           ;  iter != NULL
           ;  iter = avl_t_next (&trav)
           )
           *(SymDef *)List_NewLast(temp, sizeof (SymDef)) = iter;
        for(  Cursor=(SymDef *)List_Last(temp)
            ;  Cursor!=NULL
            ;  Cursor = (SymDef *)List_Prev((void *)Cursor)
            )
           stRemoveSym(*Cursor);
        List_Destroy(&temp);
        avl_destroy (sym->sdScope.sdScope, NULL);
    }

    if (sym->sdSymKind == SYM_VAR)
    {
        avl_destroy (sym->sdVar.sdvCnsVal, (avl_item_func *)free);
    }
    avl_delete(sym->stab->id_map, sym);
    stDeleteTypDef (sym->sdType);
    free (sym->sdName);
    free (sym);
}

SymDef stLookupSym(SymTab stab, char *name, enum symbolKinds kind)
{
    SymDef lclScp;
    SymDef sym;
    struct SymDefRec buffer;

    buffer.sdName = name;
    buffer.sdSymKind = kind;

    for (lclScp = stab->cmpCurScp;
         lclScp;
         lclScp = lclScp->sdParent)
    {
        sym = avl_find(lclScp->sdScope.sdScope, &buffer);
        if  (sym)
            return  sym;
    }
    return  NULL;
}

SymDef stGetSymByID (SymTab stab, int id)
{
    struct SymDefRec buf;
    buf.uid = id;
    return (SymDef)avl_find(stab->id_map, &buf);
}

/*****************************************************************************/


BOOL stMatchTypes(TypDef typ1, TypDef typ2)
{
    if  (typ1 == typ2)
        return  TRUE;

    if  (typ1 && typ2)
        return  stMatchType2(typ1, typ2);

    return  FALSE;
}

void stSetName(SymDef sym, const char *name)
{
    if  (sym->sdParent)
    {
        avl_delete(sym->sdParent->sdScope.sdScope, sym);
    }

    if  (sym->sdName)
    {
        free (sym->sdName);
        sym->sdName = NULL;
    }

    if  (name)
    {
        sym->sdName = (char *) xmalloc (sizeof(char) * (strlen (name) + 1));
        strcpy(sym->sdName, name);
    }

    if  (sym->sdParent)
    {
        avl_replace(sym->sdParent->sdScope.sdScope, sym);
    }
}

/*****************************************************************************/

void stDumpSymbol(SymDef sym, int indent, BOOL recurse, FILE *dump_file)
{
    struct avl_traverser trav;
    SymDef iter;

    fprintf (dump_file, "%*c [%05d] ", 1+4*indent, ' ', sym->uid);

    switch  (sym->sdSymKind)
    {
    case SYM_VAR:
        if  (sym->sdName)
            fprintf (dump_file, "Variable  '%s' ", sym->sdName);
        else
            fprintf (dump_file, "Variable  't%d' ", sym->uid);
        stTypeName(sym->sdType, NULL, dump_file);
        break;

    case SYM_FNC:
        fprintf (dump_file, "Function '%s' ", sym->sdName);
        stTypeName(sym->sdType, NULL, dump_file);
        break;

    case SYM_SCOPE:
        fprintf (dump_file, "Scope");
        break;

    case SYM_COMPUNIT:
        fprintf (dump_file, "Comp-unit '%s'", sym->sdName ? sym->sdName
                                             : "<NONAME>");
        break;

    default:
        break;
    }
    fprintf (dump_file, "\n");

    if  (!recurse)
        return;

    if (sdHasScope(sym->sdSymKind))
    {
        for(  iter = (SymDef)avl_t_first (&trav, sym->sdScope.sdScope)
           ;  iter != NULL
           ;  iter = avl_t_next (&trav)
           )
        {
            if  (sdHasScope(iter->sdSymKind) != FALSE)
                fprintf (dump_file, "\n");

            stDumpSymbol(iter, indent + 1, recurse, dump_file);
        }
    }
}

char *stGetSymName(SymDef sym)
{
    static char name[256];
    char *p;
    ConstVal v;

    if (
        sym->sdSymKind == SYM_VAR &&
        sym->sdVar.sdvConst &&
        sym->sdType->tdTypeKind <= TYP_lastIntrins
    ) {
        v = GetConstVal(sym, 0);
        switch (v->cvVtyp)
        {
        case TYP_INT:
            sprintf(name, "%d", v->cvValue.cvIval);
            break;
        case TYP_FLOAT:
            sprintf(name, "%g", v->cvValue.cvFval);
            break;
        case TYP_DOUBLE:
            sprintf(name, "%g", v->cvValue.cvDval);
            break;
        default:
            break;
        }
        p = name;
    }
    else if (sym->sdName)
    {
        p = sym->sdName;
    }
    else
    {
        sprintf(name, "t%d", sym->uid);
        p = name;
    }

    return p;
}

void DumpSymName_zenglj(SymDef sym, FILE *dump_file)
{
    ConstVal v;

    if (
        sym->sdSymKind == SYM_VAR &&
        sym->sdVar.sdvConst &&
        sym->sdType->tdTypeKind <= TYP_lastIntrins
    ) {
        v = GetConstVal(sym, 0);
        switch (v->cvVtyp)
        {
        case TYP_INT:
            fprintf(dump_file, "%d", v->cvValue.cvIval);
            break;
        case TYP_FLOAT:
            fprintf(dump_file, "%g", v->cvValue.cvFval);
            break;
        case TYP_DOUBLE:
            fprintf(dump_file, "%g", v->cvValue.cvDval);
            break;
        default:
            break;
        }
    }
    else if (sym->sdName)
    {
        if  (is_global_var (sym))
            fprintf(dump_file, "@%s", sym->sdName);
        else if (sym->sdVar.sdvLocal)
            fprintf(dump_file, "%%l%d", sym->uid);
        else
            fprintf(dump_file, "%%t%d", sym->uid);
    }
    else
    {
        if (sym->sdVar.sdvLocal)
            fprintf(dump_file, "%%l%d", sym->uid);
        else
            fprintf(dump_file, "%%t%d", sym->uid);
    }
}

void DumpSymbolTable(SymTab stab, FILE *dump_file)
{
    stDumpSymbol(stab->sym, 0, TRUE, dump_file);
}

SymDef stCreateIconNode(SymTab stab, int ival)
{
    SymDef sym;
    sym = stDeclareSym(stab, NULL, SYM_VAR);
    sym->sdIsImplicit = TRUE;
    sym->sdType = stAllocTypDef(TYP_INT);
    sym->sdVar.sdvConst = TRUE;
    SetIconVal(sym, ival, 0);
    return sym;
}

SymDef stCreateFconNode(SymTab stab, float fval)
{
    SymDef sym;
    sym = stDeclareSym(stab, NULL, SYM_VAR);
    sym->sdIsImplicit = TRUE;
    sym->sdType = stAllocTypDef(TYP_FLOAT);
    sym->sdVar.sdvConst = TRUE;
    SetFconVal(sym, fval, 0);
    return sym;
}

SymDef stCreateDconNode(SymTab stab, double dval)
{
    SymDef sym;
    sym = stDeclareSym(stab, NULL, SYM_VAR);
    sym->sdIsImplicit = TRUE;
    sym->sdType = stAllocTypDef(TYP_FLOAT);
    sym->sdVar.sdvConst = TRUE;
    SetDconVal(sym, dval, 0);
    return sym;
}

BOOL
is_global_var (SymDef sym)
{
  return (sym->sdParent->sdSymKind == SYM_COMPUNIT);
}

BOOL
in_global_scope(SymDef scp)
{
  return (scp->sdSymKind == SYM_COMPUNIT);
}

void
traverse_symtree (SymDef st, void (*sym_func) (SymDef, void *), void *param)
{
    SymDef iter;
    struct avl_traverser trav;

    if  (sdHasScope(st->sdSymKind))
    {
        for(  iter = (SymDef)avl_t_first (&trav, st->sdScope.sdScope)
           ;  iter != NULL
           ;  iter = (SymDef) avl_t_next (&trav)
           )
           traverse_symtree (iter, sym_func, param);
    }
    else
    {
        sym_func (st, param);
    }
}

void
traverse_global_variables (SymTab stab, void (*sym_func) (SymDef, void *), void *param)
{
    SymDef iter;
    struct avl_traverser trav;

    for(  iter = (SymDef)avl_t_first (&trav, stab->sym->sdScope.sdScope)
       ;  iter != NULL
       ;  iter = (SymDef) avl_t_next (&trav)
       )
       sym_func (iter, param);
}

LIST
get_globals (SymTab stab)
{
    SymDef iter;
    struct avl_traverser trav;
    LIST retval = List_Create ();

    for(  iter = (SymDef)avl_t_first (&trav, stab->sym->sdScope.sdScope)
       ;  iter != NULL
       ;  iter = (SymDef) avl_t_next (&trav)
       )
       *(SymDef *)List_NewLast(retval, sizeof(SymDef)) = iter;
    return retval;
}

SymDef
duplicate_symbol (SymDef sym)
{
    SymDef new_sym;
    ConstVal iter;
    ConstVal cptr;
    struct avl_traverser trav;

    new_sym = stDeclareSym (sym->stab, NULL, sym->sdSymKind);
    new_sym->sdType = CopyType (sym->sdType);
    new_sym->line = sym->line;
    new_sym->column = sym->column;
    new_sym->sdIsDefined = sym->sdIsDefined;
    new_sym->sdIsImport = sym->sdIsImport;
    new_sym->sdIsImplicit = sym->sdIsImplicit;
    new_sym->sdIsStatic = sym->sdIsStatic;
    if (sym->sdSymKind == SYM_VAR)
    {
        new_sym->sdVar.sdvLocal = sym->sdVar.sdvLocal;
        new_sym->sdVar.sdvArgument = sym->sdVar.sdvArgument;
        new_sym->sdVar.sdvHadInit = sym->sdVar.sdvHadInit;
        new_sym->sdVar.sdvConst = sym->sdVar.sdvConst;
        for(  iter = (ConstVal)avl_t_first (&trav, sym->sdVar.sdvCnsVal)
           ;  iter != NULL
           ;  iter = (ConstVal) avl_t_next (&trav)
           )
        {
            cptr = (ConstVal) xmalloc (sizeof (*cptr));
            memcpy (cptr, iter, sizeof (*iter));
            avl_replace (new_sym->sdVar.sdvCnsVal, cptr);
        }
    }
    return new_sym;
}

