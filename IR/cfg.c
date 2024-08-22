#include <stdlib.h>
#include <string.h>
#if ! defined (_WIN32)
#include <unistd.h>
#endif
#include "all.h"

#define TESTFLAG(v,f)(((v)&(f))!=0)

struct addr_pair
{
    basic_block object;
    basic_block *curs;
};

static int
cmp_addr_pair (struct addr_pair *a, struct addr_pair *b, void *p)
{
    if (a->object < b->object) {
        return - 1;

    } else if (a->object > b->object) {
        return 1;

    } else {
        return 0;
    }
}

static int
compare_ids (basic_block a, basic_block b, void *p)
{
    if (a->index < b->index)
        return -1;
    else if (a->index > b->index)
        return 1;
    return 0;
}

/* Allocate memory for basic_block.  */
basic_block
alloc_block (void)
{
    static int counter = 0;
    basic_block bb;
    bb = (basic_block) xmalloc (sizeof (*bb));
    memset (bb, '\0', sizeof (*bb));
    bb->insns = List_Create ();
    bb->preds = List_Create ();
    bb->succs = List_Create ();
    bb->dom[0] = BITMAP_XMALLOC();
    bb->dom[1] = BITMAP_XMALLOC();
    bb->live_in = BITMAP_XMALLOC();
    bb->live_out = BITMAP_XMALLOC();
    bb->index = ++counter;
    return bb;
}

/* 拆分边E并返回新创建的基本块。  */
basic_block
split_edge (edge e)
{
    basic_block ret;
    basic_block src = e->src, dest = e->dest;

    ret = alloc_block ();
    link_block (src->cfg, ret);
    remove_edge (e);
    make_edge(src, ret);
    make_edge(ret, dest);
    return ret;
}

void
copy_block (basic_block dest, basic_block source)
{
    IRInst *curs;
    IRInst copy;

    List_Clear (dest->insns);
    for(  curs=(IRInst *) List_First(source->insns)
       ;  curs!=NULL
       ;  curs = (IRInst *) List_Next((void *)curs)
       )
    {
        copy = IRInstCopy (*curs);
        InterCodeAddInst (dest, copy, TRUE);
    }
}

void
free_edge (edge e)
{
    free (e);
}

edge
alloc_edge (void)
{
    edge e;
    static int counter = 0;
    e = (edge) xmalloc (sizeof (*e));
    memset (e, '\0', sizeof (*e));
    e->uid = ++counter;
    return e;
}

control_flow_graph
alloc_flow (InterCode code)
{
    control_flow_graph cfg;
    static int counter = 0;
    cfg = (control_flow_graph) xmalloc (sizeof (*cfg));
    *(control_flow_graph*) List_NewLast (code->funcs, sizeof (control_flow_graph)) = cfg;
    memset (cfg, '\0', sizeof (*cfg));
    cfg->basic_block_info = List_Create();
    cfg->addr_map = avl_create((avl_comparison_func *)cmp_addr_pair, NULL, NULL);
    cfg->id_map = avl_create((avl_comparison_func *)compare_ids, NULL, NULL);
    cfg->code = code;
    cfg->funcdef_number = ++counter;
    return cfg;
}

void
free_cfg (InterCode code, control_flow_graph cfg)
{
    basic_block* block;
    void *Cursor;
    IRInst *instr;
    IRInst tmp_insn;

    for(  Cursor=List_First(code->funcs)
       ;  Cursor!=NULL
       ;  Cursor = List_Next((void *)Cursor)
       )
        if  (*(control_flow_graph *)Cursor == cfg)
        {
            for(  block=(basic_block *) List_First(cfg->basic_block_info)
               ;  block!=NULL
               ;  block= (basic_block *) List_First(cfg->basic_block_info)
               )
            {
                for(  instr=(IRInst *) List_First((*block)->insns)
                   ;  instr!=NULL
                   ;  instr = (IRInst *) List_Next((void *)instr)
                   )
                {
                    tmp_insn = *instr;
                    InterCodeRemoveInst_nobb (code, tmp_insn);
                    IRInstDelInst (tmp_insn);
                }
                free_block (*block);
            }

            List_Destroy (&cfg->basic_block_info);
            avl_destroy(cfg->addr_map, (avl_item_func *)free);
            avl_destroy(cfg->id_map, (avl_item_func *)NULL);
            free (cfg);
            List_Delete (Cursor);
            break;
        }
}

/* Connect E to E->src.  */
static void
connect_src (edge e)
{
    *(edge *)List_NewLast(e->src->succs, sizeof (edge)) = e;
}

/* Connect E to E->dest.  */
static void
connect_dest (edge e)
{
    basic_block dest = e->dest;
    *(edge *)List_NewLast(dest->preds, sizeof (edge)) = e;
}

/* Disconnect edge E from E->src.  */
static void
disconnect_src (edge e)
{
    basic_block src = e->src;
    edge * ei;

    for(  ei=(edge *)List_First(src->succs)
       ;  ei!=NULL
       ;  ei = (edge *)List_Next((void *)ei)
       )
        if  (*ei == e)
        {
            List_Delete(ei);
            return;
        }
}

/* Disconnect edge E from E->dest.  */
static void
disconnect_dest (edge e)
{
    basic_block dest = e->dest;
    edge * ei;

    for(  ei=(edge *)List_First(dest->preds)
       ;  ei!=NULL
       ;  ei = (edge *)List_Next((void *)ei)
       )
        if  (*ei == e)
        {
            List_Delete(ei);
            return;
        }
}

void
free_block (basic_block bb)
{
    edge * curs;

    if  (bb)
    {
        unlink_block (bb);

        for(  curs=(edge *)List_First(bb->succs)
           ;  curs!=NULL
           ;  curs = (edge *)List_First(bb->succs)
           )
           remove_edge (*curs);

        for(  curs=(edge *)List_First(bb->preds)
           ;  curs!=NULL
           ;  curs = (edge *)List_First(bb->preds)
           )
           remove_edge (*curs);

        List_Destroy (&bb->insns);
        List_Destroy (&bb->succs);
        List_Destroy (&bb->preds);
        BITMAP_XFREE (bb->dom[0]);
        BITMAP_XFREE (bb->dom[1]);
        BITMAP_XFREE (bb->live_in);
        BITMAP_XFREE (bb->live_out);
        free (bb);
    }
}

/* This function will remove an edge from the flow graph.  */
void
remove_edge (edge e)
{
    disconnect_src (e);
    disconnect_dest (e);

    free_edge (e);
}

edge
make_edge (basic_block src, basic_block dst)
{
    edge e;

    e = alloc_edge ();

    e->src = src;
    e->dest = dst;

    connect_src (e);
    connect_dest (e);

    return e;
}

void
link_block (control_flow_graph cfg, basic_block b)
{
    struct addr_pair *pair;
    basic_block *Cursor;

    Cursor = (basic_block *)List_NewLast(cfg->basic_block_info, sizeof (basic_block));
    *Cursor = b;
    b->cfg = cfg;
    pair = (struct addr_pair *) xmalloc (sizeof (*pair));
    memset (pair, '\0', sizeof (*pair));
    pair->object = b;
    pair->curs = Cursor;
    avl_insert(cfg->addr_map, pair);
    avl_insert(cfg->id_map, b);
}

basic_block
lookup_block_by_id (control_flow_graph cfg, int id)
{
    struct basic_block_def buf;
    buf.index = id;
    return (basic_block)avl_find(cfg->id_map, &buf);
}

void
unlink_block (basic_block b)
{
    struct addr_pair *pair;
    if  (b && b->cfg)
    {
        pair = (struct addr_pair *)avl_delete(b->cfg->addr_map, &b);
        if  (pair && pair->curs)
            List_Delete(pair->curs);
        free (pair);
        avl_delete(b->cfg->id_map, b);
    }
}

static int
compare_integers (int *sn1, int *sn2, void *p)
{
    return (*sn1 < *sn2) ? (-1) : (*sn1 > *sn2);
}

void
cfgbuild (InterCode code, SymTab stab)
{
    IRInst *Cursor;
    IRInst    *   temp;
    struct avl_table *first;
    control_flow_graph cfg = NULL;
    basic_block bb = NULL;
    control_flow_graph *ci;
    basic_block *bbi;
    IRInst insn = NULL;

    /* 创建平衡二叉树，保存首指令。  */
    first = avl_create ((avl_comparison_func *) compare_integers, NULL, NULL);

    /* 找到所有首指令。  */
    for(  Cursor=(IRInst *) List_First(code->code)
       ;  Cursor!=NULL 
       ;  Cursor = (IRInst *) List_Next((void *)Cursor) 
       )
    {
        switch ((*Cursor)->opcode)
        {
            case IRINST_OP_entry:
                avl_replace (first, &(*Cursor)->uid);

                for(  temp = (IRInst *)List_Next((void *)Cursor)
                   ;  (*temp)->opcode == IRINST_OP_fparam
                   ;  temp = List_Next((void *)temp)
                   )
                    Cursor = temp;

                Cursor = (IRInst *)List_Next ((void *)Cursor);
                avl_replace (first, &(*Cursor)->uid);
                break;

            case IRINST_OP_exit:
                avl_replace (first, &(*Cursor)->uid);
                break;

            case IRINST_OP_ifeq:
            case IRINST_OP_ifne:
            case IRINST_OP_iflt:
            case IRINST_OP_ifge:
            case IRINST_OP_ifgt:
            case IRINST_OP_ifle:
                avl_replace (first, &GetConstVal (IRInstGetOperand (*Cursor, 2)->var, 0)->cvValue.cvIval);
                avl_replace (first, &GetConstVal (IRInstGetOperand (*Cursor, 3)->var, 0)->cvValue.cvIval);

                avl_replace(first, &(*(IRInst *)List_Next((void *)Cursor))->uid);
                break;

            case IRINST_OP_goto:
                avl_replace (first, &GetConstVal (IRInstGetOperand (*Cursor, 0)->var, 0)->cvValue.cvIval);

                avl_replace (first, &(*(IRInst *)List_Next((void *)Cursor))->uid);
                break;

            default:
                break;
        }
    }

    /* 创建控制流图和基本块。  */
    for(  Cursor=(IRInst *) List_First(code->code) 
       ;  Cursor!=NULL
       ;  Cursor = (IRInst *) List_Next((void *)Cursor) 
       )
    {
        switch ((*Cursor)->opcode)
        {
            case IRINST_OP_entry:
                cfg = alloc_flow (code);
                bb = alloc_block ();
                link_block (cfg, bb);
                cfg->entry_block_ptr = bb;
                InterCodeAddExistingInst (bb, *Cursor);
                break;

            case IRINST_OP_exit:
                bb = alloc_block ();
                link_block (cfg, bb);
                cfg->exit_block_ptr = bb;
                InterCodeAddExistingInst (bb, *Cursor);
                cfg = NULL;
                break;

            default:
                if  (avl_find (first, &(*Cursor)->uid))
                {
                    bb = alloc_block ();
                    link_block (cfg, bb);
                }
                InterCodeAddExistingInst (bb, *Cursor);
                break;
        }
    }

    /* 若基本块不以分支指令结尾，则添加分支指令。  */
    for(  ci=(control_flow_graph *) List_First(code->funcs)
       ;  ci!=NULL
       ;  ci = (control_flow_graph *) List_Next((void *)ci)
       )
    {
        for(  bbi=(basic_block *) List_First((*ci)->basic_block_info)
           ;  bbi!=NULL
           ;  bbi = (basic_block *) List_Next((void *)bbi)
           )
        {
            if  (insn)
            {
                Cursor = (IRInst *)List_First((*bbi)->insns);
                IRInstSetOperand (insn, 0, stCreateIconNode (stab, (*Cursor)->uid));
                insn = NULL;
            }
            Cursor = (IRInst *)List_Last((*bbi)->insns);
            if  (!TESTFLAG (IRInstGetOpKind ((*Cursor)->opcode), BRANCH) &&
                 (*Cursor)->opcode != IRINST_OP_exit)
            {
                insn = IRInstEmitInst (IRINST_OP_goto, (*Cursor)->line, (*Cursor)->column);
                InterCodeAddInst (*bbi, insn, TRUE);
            }
        }
    }

    /* 写入基本块的前驱和后继信息。  */
    for(  ci=(control_flow_graph *) List_First(code->funcs)
       ;  ci!=NULL
       ;  ci = (control_flow_graph *) List_Next((void *)ci)
       )
    {
        for(  bbi=(basic_block *) List_First((*ci)->basic_block_info)
           ;  bbi!=NULL
           ;  bbi = (basic_block *) List_Next((void *)bbi)
           )
        {
            Cursor = (IRInst *)List_Last((*bbi)->insns);
            if  ((*Cursor)->opcode == IRINST_OP_goto)
            {
                make_edge (*bbi, InterCodeGetInstByID (code, GetConstVal (IRInstGetOperand (*Cursor, 0)->var, 0)->cvValue.cvIval)->bb);
            }
            else if ((*Cursor)->opcode != IRINST_OP_exit)
            {
                make_edge (*bbi, InterCodeGetInstByID (code, GetConstVal (IRInstGetOperand (*Cursor, 2)->var, 0)->cvValue.cvIval)->bb);
                make_edge (*bbi, InterCodeGetInstByID (code, GetConstVal (IRInstGetOperand (*Cursor, 3)->var, 0)->cvValue.cvIval)->bb);
            }
        }
    }

    /* 在将控制流图转换成静态单赋值形式之前，先对流图执行几种转换。  */
    for(  ci=(control_flow_graph *) List_First(code->funcs)
       ;  ci!=NULL
       ;  ci = (control_flow_graph *) List_Next((void *)ci)
       )
    {
        /* 死基本块删除。  */
        cleanup_cfg (*ci, stab);

        /* 将while循环转换为repeat循环。  */
        if  (comp->cmpConfig.optimize)
        {
            flow_loops_find (*ci);
/*          flow_loops_dump ((*ci)->loops, stdout);*/
            copy_loop_headers (*ci);
            flow_loops_free (&(*ci)->loops);
        }
    }

    /* 销毁平衡二叉树。  */
    avl_destroy (first, NULL);
}

void
dump_cfg (InterCode code, const char *file)
{
    FILE *fp;
    char *name;
    control_flow_graph *cfg;
    basic_block *bb;
    edge * ei;
    IRInst *Cursor;
    int count;

    fp=fopen("psg.gv","w");
    if (NULL == fp)
    {
        fprintf (stderr, "error opening psg.gv\n");
        return;
    }

    fputs("digraph g {\n", fp);
    fputs("  fontname=\"Helvetica,Arial,sans-serif\"\n", fp);
    fputs("  node [fontname=\"Helvetica,Arial,sans-serif\"]\n", fp);
    fputs("  edge [fontname=\"Helvetica,Arial,sans-serif\"]\n", fp);
    fputs("  graph [fontsize=30 labelloc=\"t\" label=\"\" splines=true overlap=false rankdir = \"TB\"];\n", fp);
    fputs("  ratio = auto;\n", fp);

    for(  cfg=(control_flow_graph *)List_First(code->funcs)
        ;  cfg!=NULL
        ;  cfg = (control_flow_graph *)List_Next((void *)cfg)
        )
    {
        for(  bb=(basic_block *)List_First((*cfg)->basic_block_info)
            ;  bb!=NULL
            ;  bb = (basic_block *)List_Next((void *)bb)
            )
        {
            if      (*bb == (*cfg)->entry_block_ptr)
                fprintf(fp, "  \"state%d\" [ style = \"filled, bold\" penwidth = 5 fillcolor = \"white\" fontname = \"Courier New\" shape = \"Mrecord\" label =<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\"><tr><td bgcolor=\"black\" align=\"center\" colspan=\"2\"><font color=\"white\">basic block %d</font></td></tr>", (*bb)->index, (*bb)->index);
            else if (*bb == (*cfg)->exit_block_ptr)
                fprintf(fp, "  \"state%d\" [ style = \"filled\" penwidth = 1 fillcolor = \"black\" fontname = \"Courier New\" shape = \"Mrecord\" label =<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"black\"><tr><td bgcolor=\"black\" align=\"center\" colspan=\"2\"><font color=\"white\">basic block %d</font></td></tr>", (*bb)->index, (*bb)->index);
            else
                fprintf(fp, "  \"state%d\" [ style = \"filled\" penwidth = 1 fillcolor = \"white\" fontname = \"Courier New\" shape = \"Mrecord\" label =<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\"><tr><td bgcolor=\"black\" align=\"center\" colspan=\"2\"><font color=\"white\">basic block %d</font></td></tr>", (*bb)->index, (*bb)->index);

            count = 0;
            for(  Cursor=(IRInst *)List_First((*bb)->insns)
                ;  Cursor!=NULL
                ;  Cursor = (IRInst *)List_Next((void *)Cursor)
                )
            {
                fprintf(fp, "<tr><td align=\"left\" port=\"r%d\">%s", count++, *bb == (*cfg)->exit_block_ptr ? "<font color=\"white\">" : "");
                IRInstDump (*Cursor, TRUE, fp);
                fprintf(fp, " %s</td></tr>", *bb == (*cfg)->exit_block_ptr ? "</font>" : "");
            }
            fprintf(fp, "</table>> ];\n");

            for(  ei=(edge *)List_First((*bb)->succs)
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei)
               )
            {
                fprintf(fp, "  state%d -> state%d [ penwidth = 1 fontsize = 14 fontcolor = \"grey28\" xlabel=\"%d\" ];\n", (*ei)->src->index, (*ei)->dest->index, (*ei)->uid);
            }
        }
    }

    fputs("}\n", fp);

    fclose (fp);

    name = (char *) xmalloc (strlen(file) + 64);
    sprintf( name, "dot -Tpdf psg.gv > \"%s\"", file );
    if (system(name) != 0) {
        fprintf( stderr, "Error executing %s\n", name );
        free (name);
        return;
    }

    free (name);
#if defined (_WIN32)
    _unlink("psg.gv");
#else
    unlink("psg.gv");
#endif
}

static void
dump_global_variables (SymDef var, FILE *file)
{
    if  (var->sdSymKind == SYM_VAR &&
         var->sdType->tdTypeKind != TYP_VOID &&
         (var->sdType->tdTypeKind > TYP_lastIntrins || !var->sdVar.sdvConst))
    {
        fprintf (file, "declare ");
        stTypeName_zenglj (var->sdType, var, file);
        if  (var->sdVar.sdvHadInit &&
             var->sdType->tdTypeKind <= TYP_lastIntrins)
        {
            fprintf (file, " = ");
            if  (var->sdType->tdTypeKind == TYP_FLOAT)
                fprintf (file, "%g", GetConstVal(var, 0)->cvValue.cvFval);
            else
                fprintf (file, "%i", GetConstVal(var, 0)->cvValue.cvIval);
        }
        fprintf (file, "\n");
    }
}

void
dump_cfg_zenglj (InterCode code, SymTab stab, FILE *dump_file)
{
    control_flow_graph *cfg;
    basic_block *bb;
    IRInst *Cursor;

    traverse_global_variables (stab, (void (*) (SymDef, void *))dump_global_variables, dump_file);

    for(  cfg=(control_flow_graph *)List_First(code->funcs)
        ;  cfg!=NULL
        ;  cfg = (control_flow_graph *)List_Next((void *)cfg)
        )
    {
        for(  bb=(basic_block *)List_First((*cfg)->basic_block_info)
            ;  bb!=NULL
            ;  bb = (basic_block *)List_Next((void *)bb)
            )
        {
            Cursor=(IRInst *)List_First((*bb)->insns);
            if  (*bb != (*cfg)->entry_block_ptr)
                fprintf( dump_file, ".L%d:\n", (*Cursor)->uid );
            
            for(  Cursor=(IRInst *)List_First((*bb)->insns)
                ;  Cursor!=NULL
                ;  Cursor = (IRInst *)List_Next((void *)Cursor)
                )
            {
                IRInstDump_zenglj (*Cursor, stab, dump_file);
            }
        }
    }
}

void
pre_and_rev_post_order_compute (control_flow_graph fn,
                               LIST pre_order, LIST rev_post_order,
                               BOOL include_entry_exit, BOOL reverse)
{
    bitmap visited;
    LIST stack;

    /* Allocate stack for back-tracking up CFG.  */
    stack = List_Create();

    if (include_entry_exit)
    {
        if (pre_order)
            *(basic_block *)List_NewLast (pre_order, sizeof (basic_block)) = (reverse) ? fn->exit_block_ptr : fn->entry_block_ptr;
        if (rev_post_order)
            *(basic_block *)List_NewFirst (rev_post_order, sizeof (basic_block)) = (reverse) ? fn->entry_block_ptr : fn->exit_block_ptr;
    }

    /* Allocate bitmap to track nodes that have been visited.  */
    visited = BITMAP_XMALLOC ();

    /* None of the nodes in the CFG have been visited yet.  */
    bitmap_clear (visited);

    /* Push the first edge on to the stack.  */
    *(edge **)List_NewLast(stack, sizeof (edge *)) = (edge *)List_First ((reverse) ? fn->exit_block_ptr->preds : fn->entry_block_ptr->succs);

    while (!List_IsEmpty(stack))
    {
        edge *ei;
        basic_block src;
        basic_block dest;

        /* Look at the edge on the top of the stack.  */
        ei = *(edge **)List_Last (stack);
        src = (reverse) ? (*ei)->dest : (*ei)->src;
        dest = (reverse) ? (*ei)->src : (*ei)->dest;

        /* Check if the edge destination has been visited yet.  */
        if (dest != ((reverse) ? fn->entry_block_ptr : fn->exit_block_ptr)
            && ! bitmap_bit_p (visited, dest->index))
        {
            /* Mark that we have visited the destination.  */
            bitmap_set_bit (visited, dest->index);

            if (pre_order)
                *(basic_block *)List_NewLast (pre_order, sizeof (basic_block)) = dest;

            if (!List_IsEmpty ((reverse) ? dest->preds : dest->succs))
                /* Since the DEST node has been visited for the first
                   time, check its successors.  */
                *(edge **)List_NewLast(stack, sizeof (edge *)) = (edge *)List_First ((reverse) ? dest->preds : dest->succs);
            else if (rev_post_order)
                /* There are no successors for the DEST node so assign
                   its reverse completion number.  */
                *(basic_block *)List_NewFirst (rev_post_order, sizeof (basic_block)) = dest;

        }
        else
        {
            if (!List_Next (ei)
                && src != ((reverse) ? fn->exit_block_ptr : fn->entry_block_ptr)
                && rev_post_order)
                /* There are no more successors for the SRC node
                   so assign its reverse completion number.  */
                *(basic_block *)List_NewFirst (rev_post_order, sizeof (basic_block)) = src;

            if (List_Next (ei))
                *(edge **)List_Last (stack) = (edge *)List_Next (ei);
            else
                List_DeleteLast(stack);
        }
    }

    List_Destroy (&stack);
    BITMAP_XFREE (visited);

    if (include_entry_exit)
    {
        if (pre_order)
            *(basic_block *)List_NewLast (pre_order, sizeof (basic_block)) = (reverse) ? fn->entry_block_ptr : fn->exit_block_ptr;
        if (rev_post_order)
            *(basic_block *)List_NewFirst (rev_post_order, sizeof (basic_block)) = (reverse) ? fn->exit_block_ptr : fn->entry_block_ptr;
    }
}

void
compute_liveness (control_flow_graph cfun, varpool_node_set set)
{
    LIST bbs = List_Create ();
    LIST globals = List_Create ();
    BOOL changed;
    basic_block *bb;
    bitmap work = BITMAP_XMALLOC ();
    struct avl_traverser trav;
    varpool_node vnode;
    varpool_node *curs;
    IRInst *insn;
    edge *ei;
    int i;

    pre_and_rev_post_order_compute (cfun, NULL, bbs, TRUE, FALSE);

    for(  bb=(basic_block *)List_Last(bbs)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Prev((void *)bb)
       )
    {
        bitmap_clear ((*bb)->live_in);
        bitmap_clear ((*bb)->live_out);
    }

    for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
        if  (is_global_var (vnode->var) &&
             vnode->var->sdSymKind == SYM_VAR &&
             !vnode->var->sdIsImplicit &&
             !vnode->var->sdVar.sdvConst)
            *(varpool_node *)List_NewLast (globals, sizeof (varpool_node)) = vnode;

    do
      {
        changed = FALSE;
        for(  bb=(basic_block *)List_Last(bbs)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Prev((void *)bb)
           )
        {

            /* 后继的livein的并集（如果没有，则为空）。  */
            BOOL first = TRUE;
            for(  ei=(edge *)List_First((*bb)->succs)
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei)
               )
            {
                basic_block succ =  ((*ei)->dest);
                if (first)
                {
                    bitmap_copy (work, succ->live_in);
                    first = FALSE;
                }
                else
                    bitmap_ior_into (work, succ->live_in);
            }
            if (first)
                bitmap_clear (work);
  
            bitmap_copy ((*bb)->live_out, work);

            /* 在反向指令遍历中删除定值，包括使用。  */
            for(  insn=(IRInst *)List_Last((*bb)->insns)
               ;  insn!=NULL
               ;  insn = (IRInst *)List_Prev((void *)insn)
               )
            {
                for (i = IRInstGetNumOperands (*insn) - 1; i >= 0; i--)
                {
                    if  (IRInstIsOutput (*insn, i))
                    {
                        vnode = varpool_get_node (set, IRInstGetOperand (*insn, i));
                        bitmap_clear_bit (work, vnode->uid);
                    }
                }
                for (i = IRInstGetNumOperands (*insn) - 1; i >= 0; i--)
                {
                    if  (! IRInstIsOutput (*insn, i))
                    {
                        vnode = varpool_get_node(set, IRInstGetOperand (*insn, i));
                        bitmap_set_bit (work, vnode->uid);
                    }
                }
                if  ((*insn)->opcode == IRINST_OP_call)
                {
                    for(  curs=(varpool_node*)List_First(globals)
                       ;  curs!=NULL
                       ;  curs = (varpool_node *)List_Next((void *)curs)
                       )
                        bitmap_set_bit (work, (*curs)->uid);
                }
            }

            /* Note if that changed something.  */
            if (bitmap_ior_into ((*bb)->live_in, work))
                changed = TRUE;
        }
      }
    while (changed);

    List_Destroy (&bbs);
    BITMAP_XFREE (work);
    List_Destroy (&globals);
}

/* 通过删除不可访问的代码和其他内容来清理CFG。  */
void
cleanup_cfg (control_flow_graph cfun, SymTab stab)
{
    LIST blocks = List_Create ();
    BOOL changed;
    basic_block *bb;
    basic_block *next_bb;
    bitmap visited = BITMAP_XMALLOC ();
    edge *ei;
    int i, k;
    IRInst *insn;
    IRInst *next_insn;
    IRInst temp;
    ssa_name name;
    basic_block jump_dest_block;
    struct ssa_name old, new;
    basic_block* block;
    IRInst *instr;
    BOOL fssa = FALSE;

    pre_and_rev_post_order_compute (cfun, blocks, NULL, TRUE, FALSE);
    for(  bb=(basic_block *)List_Last(blocks)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Prev((void *)bb)
       )
        bitmap_set_bit(visited, (*bb)->index);

    for(  bb=(basic_block *)List_First(cfun->basic_block_info)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Next((void *)bb)
       )
    {
        for(  ei=(edge *)List_First((*bb)->preds), i = 1
           ;  ei!=NULL
           ;  ei = (edge *)List_Next((void *)ei), ++i
           )
        {
            if  (! bitmap_bit_p (visited, (*ei)->src->index))
            {
                for(  insn=(IRInst *)List_First((*bb)->insns)
                   ;  insn!=NULL
                   ;  insn = next_insn
                   )
                {
                    next_insn = (IRInst *)List_Next((void *)insn);
                    if  ((*insn)->opcode == IRINST_OP_phi)
                    {
                        temp = IRInstEmitInst(IRINST_OP_phi, (*insn)->line, (*insn)->column);
                        for (k = 0; k < IRInstGetNumOperands(*insn); ++k)
                        {
                            if  (k != i)
                            {
                                name = IRInstGetOperand(*insn, k);
                                IRInstSetOperand(temp, ((k > i) ? k - 1 : k), name->var);
                                IRInstGetOperand(temp, ((k > i) ? k - 1 : k))->version = name->version;
                            }
                        }
                        InterCodeInsertAfter (*bb, insn, temp, TRUE, NULL);
                        temp = *insn;
                        InterCodeRemoveInst(cfun->code, *insn, NULL);
                        IRInstDelInst(temp);
                    }
                }
                --i;
            }
        }
    }

    for(  bb=(basic_block *)List_Last(cfun->basic_block_info)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Prev((void *)bb)
       )
    {
        for(  insn=(IRInst *)List_First ((*bb)->insns)
           ;  insn!=NULL
           ;  insn = next_insn
           )
        {
            next_insn = (IRInst *)List_Next((void *)insn);
            if  ((*insn)->opcode == IRINST_OP_phi &&
                 IRInstGetNumOperands (*insn) == 2)
            {
                memcpy (&new, IRInstGetOperand (*insn, 1), sizeof (old));
                memcpy (&old, IRInstGetOperand (*insn, 0), sizeof (new));
                for(  block=(basic_block *) List_First(cfun->basic_block_info)
                   ;  block!=NULL
                   ;  block = (basic_block *) List_Next((void *)block)
                   )
                {
                    for(  instr=(IRInst *) List_First((*block)->insns) 
                       ;  instr!=NULL 
                       ;  instr = (IRInst *) List_Next((void *)instr)
                       )
                    {
                        for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                        {
                            name = IRInstGetOperand (*instr, i);
                            if  (name->var == old.var &&
                                 name->version == old.version)
                            {
                                IRInstSetOperand (*instr, i, new.var);
                                IRInstGetOperand (*instr, i)->version = new.version;
                            }
                        }
                    }
                }
                temp = *insn;
                InterCodeRemoveInst(cfun->code, *insn, NULL);
                IRInstDelInst(temp);
            }
        }
    }

    for(  bb=(basic_block *)List_Last(cfun->basic_block_info)
       ;  bb!=NULL
       ;  bb = next_bb
       )
    {
        next_bb = (basic_block *)List_Prev((void *)bb);
        if  (! bitmap_bit_p (visited, (*bb)->index))
        {
            free_block (*bb);
        }
    }

    bitmap_clear (visited);

    for(  bb=(basic_block *)List_Last(cfun->basic_block_info)
       ;  bb!=NULL && !fssa
       ;  bb = (basic_block *)List_Prev((void *)bb)
       )
    {
        for(  insn=(IRInst *)List_First ((*bb)->insns)
           ;  insn!=NULL && !fssa
           ;  insn = (IRInst *)List_Next((void *)insn)
           )
        {
            if  ((*insn)->opcode == IRINST_OP_phi)
                fssa = TRUE;
        }
    }

    do
    {
        changed = FALSE;
        List_Clear(blocks);
        pre_and_rev_post_order_compute (cfun, NULL, blocks, TRUE, FALSE);
        for(  bb=(basic_block *)List_Last(blocks)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Prev((void *)bb)
           )
        {
            if  (!fssa)
            {
                /* 合并冗余分支指令。  */
                temp = *(IRInst *)List_Last((*bb)->insns);
                if  (temp->opcode != IRINST_OP_goto &&
                     temp->opcode != IRINST_OP_exit &&
                     List_Last ((*bb)->succs) &&
                     GetConstVal(IRInstGetOperand(temp, 2)->var, 0)->cvValue.cvIval == GetConstVal(IRInstGetOperand(temp, 3)->var, 0)->cvValue.cvIval)
                {
                    changed = TRUE;
                    remove_edge (*(edge *)List_Last ((*bb)->succs));
                    temp = IRInstEmitInst (IRINST_OP_goto, temp->line, temp->column);
                    InterCodeInsertBefore (*bb, (IRInst *)List_Last((*bb)->insns), temp, TRUE, NULL);
                    IRInstSetOperand(temp, 0, IRInstGetOperand (*(IRInst *)List_Last((*bb)->insns), 2)->var);
                    temp = *(IRInst *)List_Last ((*bb)->insns);
                    InterCodeRemoveInst (cfun->code, temp, NULL);
                    IRInstDelInst (temp);
                }
            }

            ei = (edge *)List_First ((*bb)->succs);
            if  (ei != NULL &&
                 ei == (edge *)List_Last ((*bb)->succs) &&
                 *bb != cfun->entry_block_ptr)
            {
                jump_dest_block = (*ei)->dest;

                /* 删除空程序块。  */
                if  (!fssa &&
                     (IRInst *)List_Last((*bb)->insns) == (IRInst *)List_First((*bb)->insns) &&
                     *bb != cfun->exit_block_ptr)
                {
                    changed = TRUE;

                    remove_edge (*ei);
                    for(  ei=(edge *)List_First((*bb)->preds)
                       ;  ei!=NULL
                       ;  ei = (edge *)List_First((*bb)->preds)
                       )
                    {
                        make_edge ((*ei)->src, jump_dest_block);
                        remove_edge (*ei);
                    }

                    update_destinations (jump_dest_block, (*(IRInst *)List_First((*bb)->insns))->uid, (*(IRInst *)List_First(jump_dest_block->insns))->uid, NULL);
                    continue;
                }

                /* 合并程序块。  */
                if  (NULL != (edge *)List_Last (jump_dest_block->preds) &&
                     (edge *)List_First (jump_dest_block->preds) == (edge *)List_Last (jump_dest_block->preds) &&
                     jump_dest_block != cfun->exit_block_ptr)
                {
                    changed = TRUE;

                    for(  insn=(IRInst *)List_Last((*bb)->insns)
                       ;  insn!=NULL
                       ;  insn = (IRInst *)List_Prev((void *)insn)
                       )
                    {
                        if  (insn != (IRInst *)List_Last((*bb)->insns))
                        {
                            temp = IRInstCopy (*insn);
                            InterCodeInsertBefore (jump_dest_block, (IRInst *)List_First(jump_dest_block->insns), temp, TRUE, NULL);
                        }
                    }

                    remove_edge (*ei);
                    for(  ei=(edge *)List_First((*bb)->preds)
                       ;  ei!=NULL
                       ;  ei = (edge *)List_First((*bb)->preds)
                       )
                    {
                        make_edge ((*ei)->src, jump_dest_block);
                        remove_edge (*ei);
                    }

                    update_destinations (jump_dest_block, (*(IRInst *)List_First((*bb)->insns))->uid, (*(IRInst *)List_First(jump_dest_block->insns))->uid, NULL);
                    continue;
                }

                /* 提升分支指令。  */
                if  (!fssa &&
                     (IRInst *)List_Last(jump_dest_block->insns) == (IRInst *)List_First(jump_dest_block->insns) &&
                     (*(IRInst *)List_Last(jump_dest_block->insns))->opcode != IRINST_OP_goto &&
                     jump_dest_block != cfun->exit_block_ptr)
                {
                    changed = TRUE;

                    remove_edge (*ei);
                    for(  ei=(edge *)List_First(jump_dest_block->succs)
                       ;  ei!=NULL
                       ;  ei = (edge *)List_Next((void *)ei)
                       )
                        make_edge (*bb, (*ei)->dest);
                    
                    temp = IRInstCopy (*(IRInst *)List_Last(jump_dest_block->insns));
                    InterCodeInsertBefore (*bb, (IRInst *)List_Last((*bb)->insns), temp, TRUE, NULL);
                    temp = *(IRInst *)List_Last ((*bb)->insns);
                    InterCodeRemoveInst (cfun->code, temp, NULL);
                    IRInstDelInst (temp);
                }

            }
        }
    }
    while (changed);

    List_Clear(blocks);
    pre_and_rev_post_order_compute (cfun, NULL, blocks, TRUE, FALSE);
    for(  bb=(basic_block *)List_Last(blocks)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Prev((void *)bb)
       )
        bitmap_set_bit(visited, (*bb)->index);

    for(  bb=(basic_block *)List_Last(cfun->basic_block_info)
       ;  bb!=NULL
       ;  bb = next_bb
       )
    {
        next_bb = (basic_block *)List_Prev((void *)bb);
        if  (! bitmap_bit_p (visited, (*bb)->index))
        {
            free_block (*bb);
        }
    }

    List_Destroy (&blocks);
    BITMAP_XFREE (visited);
}
