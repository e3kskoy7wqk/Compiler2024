/*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "all.h"

unsigned char varTypeClassification[] =
{
    #define DEF_TP(tn,sz,al,nm,nm_zenglj,tf) tf,
    #include "typelist.h"
    #undef  DEF_TP
};

/*****************************************************************************
 *
 *  The low-level tree node allocation routines.
 */

Tree parseAllocNode()
{
    Tree node;
    static int count=0;

    node = (Tree)xmalloc(sizeof(*node));

    memset (node, 0, sizeof(*node));
    node->children = List_Create();
    node->true_list = List_Create();
    node->false_list = List_Create();
    node->next_list = List_Create();
    node->uid = ++count;

#if !defined(NDEBUG)
    node->tnLineNo = -1;
//  node->tnColumn = -1;
#endif

    return  node;
}

void parseDeleteNode( Tree node )
{
    if ( node)
    {
        switch (node->tnOper)
        {
            case TN_NAME               :
            case TN_CALL               : free (node->tnName.tnNameId); break;

            default                    : break;
        }
        List_Destroy(&node->true_list);
        List_Destroy(&node->false_list);
        List_Destroy(&node->next_list);
        List_Destroy(&node->children);
        stDeleteTypDef(node->tnType);
        free (node);
    }
}

/*****************************************************************************
 *
 *  Allocate a parse tree node with the given operator.
 */

Tree parseCreateNode(enum treeOps op, int tnLineNo, int tnColumn)
{
    Tree tree = parseAllocNode();

    tree->tnOper   = op;
    tree->tnFlags  = 0;

    tree->tnLineNo = tnLineNo;
    tree->tnColumn = tnColumn;

    return  tree;
}

Tree parseCreateNameNode(const char *name, size_t length, int tnLineNo, int tnColumn)
{
    Tree node = parseCreateNode(TN_NAME, tnLineNo, tnColumn);

    node->tnName.tnNameId = xcalloc (sizeof(char), length+1);
    memcpy (node->tnName.tnNameId, name, length);
    node->tnName.tnNameId[length] = '\0';

    return  node;
}

void InsertChildNode (Tree lpParent, Tree lpChildNode)
{
    *((Tree *)List_NewLast(lpParent->children, sizeof(Tree))) = lpChildNode;
    lpChildNode->lpParent = lpParent;
}

Tree                parseCreateIconNode(unsigned int ival, enum var_types typ, int tnLineNo, int tnColumn)
{
    Tree            node = parseCreateNode(TN_CNS_INT, tnLineNo, tnColumn);

    node->tnVtyp             = typ;
    node->tnIntCon.tnIconVal = ival;

    node->tnOper = TN_CNS_INT;

    return  node;
}

Tree                parseCreateFconNode(float fval, int tnLineNo, int tnColumn)
{
    Tree            node = parseCreateNode(TN_CNS_FLT, tnLineNo, tnColumn);

    node->tnVtyp             = TYP_FLOAT;
    node->tnFltCon.tnFconVal = fval;

    return  node;
}

/* 这个例程释放了我们为树分配的所有内存。  */
void DestroyTree(Tree node)
{
    Tree *Cursor;
    if (!node)
      return;
    for(  Cursor=(Tree *)List_First(node->children)
        ;  Cursor!=NULL
        ;  Cursor = (Tree *)List_Next((void *)Cursor)
        )
        DestroyTree(*Cursor);
    parseDeleteNode(node);
}
