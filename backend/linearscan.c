#include <stdlib.h>
#include <string.h>
#include "all.h"

#if !defined(NDEBUG)

#if 0

  /* 用于代码中跟踪输出简短定义的辅助宏  */
  #define TRACE_LINEAR_SCAN(code)       \
    if (1) {       \
      code;                                    \
    }
#else
  #define TRACE_LINEAR_SCAN(code)
#endif

#else
  #define TRACE_LINEAR_SCAN(code)
#endif

#ifndef min
#define min(a,b) ((a) <= (b)? (a):(b))
#endif
#ifndef max
#define max(a,b) ((a) >= (b)? (a):(b))
#endif

struct Range
{
    int _from; /* from (inclusive) */
    int _to; /* to (exclusive) */
    struct Range* _next; /* linear list of Ranges */
    struct Interval* interval;
};

struct Interval
{
    /* Unique id of the node.  */
    int uid;

    /* sorted list of Ranges */
    struct Range* _first;
    int vreg;
    int weight;
};

struct op_reg
{
    /* 如果寄存器分配器决定溢出寄存器，这就是适当的溢出符号。  */
    int spill_slot;

    /* 该寄存器结构的编号，按分配顺序排列。  */
    int order;
    int lr_begin, lr_end;
    struct Interval* interval;

    int reg_class;
    /* 如果已分配，则为寄存器的编号  */
    int hard_num;

    LIST use_pos; /* 使用位置的有序列表  */

};

struct reg_class_desc
{
    unsigned next_avail, max_num;
    unsigned used_num, max_used;
    bitmap used, available;
};

static int
compare_insns (LIR_Op first, LIR_Op second)
{
    int ret = 0 ;

    if ( first->uid < second->uid )
        ret = -1 ;
    else if ( first->uid > second->uid )
        ret = 1 ;

    return( ret );
}

static struct Interval* create_interval(int vreg)
{
    struct Interval* interval = (struct Interval *) xmalloc (sizeof (struct Interval));
    static int counter = 0;
    memset (interval, '\0', sizeof (*interval));
    interval->vreg = vreg;
    interval->uid = counter++;
    return interval;
}

static void delete_interval(struct Interval* interval)
{
    struct Range * p,*r;

    if  (interval)
    {
        p=interval->_first;
        while(p)
        {
            r=p;
            p=p->_next;
            free(r);
        }

        free(interval);
    }
}

static void
init_reg_data (struct op_reg *reg, int order, char reg_class)
{
    reg->use_pos = List_Create();
    reg->order = order;
    reg->reg_class = reg_class;
    reg->spill_slot = -1;
    reg->interval = create_interval (reg->order);
}

static void
free_reg_data (struct op_reg *reg)
{
    List_Destroy (&reg->use_pos);
    delete_interval (reg->interval);
}

static void add_range(struct Interval* it, int from, int to)
{
    if (it->_first && it->_first->_from <= to)
    {
        /* 连接相交范围  */
        it->_first->_from = min(from, it->_first->_from);
        it->_first->_to  = max(to,   it->_first->_to);
    }
    else
    {
        /* 插入新范围  */
        struct Range* next = it->_first;
        it->_first = (struct Range *) xmalloc (sizeof (struct Range));
        it->_first->_from = from;
        it->_first->_to = to;
        it->_first->_next = next;
        it->_first->interval = it;
    }
}

static void add_def(struct Interval* interval, int def_pos)
{
    if (interval != NULL)
    {
        struct Range* r = interval->_first;
        if (r != NULL && r->_from <= def_pos)
        {
            /* 更新起始点(当一个范围第一次被创建用于某个用途时，它的起始点是
               当前块的开始，直到遇到一个def)。  */
            r->_from = def_pos;
        }
        else
        {
            /* 死值  */
            add_range(interval, def_pos, def_pos + 1);
        }

    }
}

static void add_use(struct Interval* interval, int from, int to)
{
    add_range(interval, from, to);
}

static int intersects_at(const struct Range* r1, const struct Range* r2)
{
    if (!r1)      return -1;
    if (!r2)      return -1;
    do {
        if (r1->_from < r2->_from) {
            if (r1->_to <= r2->_from) {
                r1 = r1->_next;
                if (r1 == NULL)
                    return -1;
            } else {
                return r2->_from;
            }
        } else if (r2->_from < r1->_from) {
            if (r2->_to <= r1->_from) {
                r2 = r2->_next;
                if (r2 == NULL)
                    return -1;
            } else {
                return r1->_from;
            }
        } else { // r1->from() == r2->from()
            if (r1->_from == r1->_to) {
                r1 = r1->_next;
                if (r1 == NULL)
                    return -1;
            } else if (r2->_from == r2->_to) {
                r2 = r2->_next;
                if (r2 == NULL)
                    return -1;
            } else {
                return r1->_from;
            }
        }
    } while (TRUE);
}

static BOOL intersects(const struct Range* r1, const struct Range* r2)
{
    return intersects_at(r1, r2) != -1; 
}

/* 释放REG使用的硬寄存器到分配状态CLASSES中。  */
static void
free_reg (struct reg_class_desc *classes, struct op_reg *reg)
{
    int cl = reg->reg_class;
    int ret = reg->hard_num;
    classes[cl].used_num--;
    bitmap_clear_bit (classes[cl].used, ret);
}

/* 记录REG的有效范围至少在END结束。  */
static void
note_lr_end (struct op_reg *reg, int end)
{
  if (reg->lr_end < end)
    reg->lr_end = end;
}

/* 记录REG的活动范围至少从BEGIN开始。  */
static void
note_lr_begin (struct op_reg *reg, int begin)
{
  if (reg->lr_begin > begin)
    reg->lr_begin = begin;
}

static int compare( struct op_reg*arg1, struct op_reg*arg2 )
{
   return ( arg1->order- arg2->order );
}

/* 给定两个寄存器A和B，如果A的活动范围开始于B的活动范围之前、在或在
   B的活动范围之后，则返回-1、0或1。  */
static int
cmp_begin (const void *a, const void *b)
{
    const struct op_reg * rega = (const struct op_reg *)a;
    const struct op_reg * regb = (const struct op_reg *)b;
    int ret;
    if (rega == regb)
        return 0;
    ret = (*rega).lr_begin - (*regb).lr_begin;
    if (ret)
        return ret;
    return ((*rega).order - (*regb).order);
}

/* 给定两个寄存器REGA和REGB，如果REGA的活动范围在REGB之后结
   束，则返回true。这将导致以更早的端点结尾的排序顺序。  */
static int
cmp_end (const void *a, const void *b)
{
    const struct op_reg * rega = (const struct op_reg *)a;
    const struct op_reg * regb = (const struct op_reg *)b;
    int ret;
    if (rega == regb)
        return 0;
    ret = (*regb).lr_end - (*rega).lr_end;
    if (ret)
        return ret;
    return ((*regb).order - (*rega).order);
}

/* 给定全局寄存器分配状态CLASSES和寄存器REG，尝试给它一个硬件寄存
   器。如果成功，将该硬件寄存器存储在REG中并返回，否则返回-1。还
   更改CLASSES以适应分配的寄存器。  */
static int
try_alloc_reg (struct op_reg* ind2reg, struct reg_class_desc *classes, struct op_reg *reg)
{
    int cl = reg->reg_class;
    struct Range temp;
    int ret = -1;
    unsigned int i;
    if (classes[cl].used_num < classes[cl].max_num - classes[cl].next_avail)
    {
        for (i = classes[cl].next_avail; i < classes[cl].max_num; i++)
        {
            temp._next = NULL;
            temp._from = reg->lr_begin;
            temp._to = reg->lr_end;
            if (! bitmap_bit_p (classes[cl].used, i) &&
                ! intersects (ind2reg[i].interval->_first, &temp))
                break;
        }
        if (i == classes[cl].max_num)
            return -1;
        classes[cl].used_num++;
        ret = i;
        bitmap_set_bit(classes[cl].used, i);
        reg->reg_class = cl;
        reg->hard_num = i;
    }
    return ret;
}

/* 使所有旧的间隔失效，即在间隔REG开始之前结束的那些。
   将释放的资源归还给CLASSES。  */
static void
expire_old_intervals (struct op_reg *reg, struct op_reg** *active,
                      struct reg_class_desc *classes, int class_count)
{
    int i;
    for (i = 0; i < class_count; i++)
        while (classes[i].used_num)
        {
            struct op_reg *a = active[i][classes[i].used_num-1];
            if (a->lr_end > reg->lr_begin)
            {
                break;
            }
            free_reg (classes, a);
        }
}

static void bubble_sort(struct op_reg* a[],int n)
{ /* 将a中序列重新排列成自小至大有序的序列(起泡排序) */
    int i,j;
    struct op_reg*t;
    BOOL change;
    for(i=n-1, change=TRUE; i>1 && change; --i)
    {
        change=FALSE;
        for(j=0; j<i; ++j)
            if(cmp_end (a[j],a[j+1]) > 0)
            {
                t=a[j];
                a[j]=a[j+1];
                a[j+1]=t;
                change=TRUE;
            }
    }
}

/* 间隔REG没有得到一个硬件寄存器。溢出它或ACTIVE中的一个(如果是
   后者，则REG将被分配给以前被它使用的硬件寄存器)。  */
static struct op_reg *
spill_at_interval (struct op_reg *reg, struct op_reg** *active,
                   struct reg_class_desc *classes, control_flow_graph fn,
                   struct avl_table *virtual_regs, struct Backend* backend)
{
    int cl = reg->reg_class;
    struct op_reg *cand;
    unsigned int i;
    struct descriptor_s *constVal;

    constVal = backend->get_location (backend->gen_vreg (virtual_regs, reg->order, 0));
    if  (constVal != NULL &&
         ! constVal->is_addr &&
         constVal->base->var->sdSymKind == SYM_VAR &&
         constVal->base->var->sdType->tdTypeKind == TYP_INT &&
         constVal->base->var->sdVar.sdvConst &&
         reg->interval->weight != 0x7FFFFFFF)
    {
        /* 它自身就是常量，逐出自身。  */
        cand = reg;
        goto exit;
    }

    /* 尝试逐出占用寄存器的常量、全局变量地址。  */
    for (i = 0, cand = active[cl][0];
         classes[cl].used_num > i && cand->lr_end > reg->lr_end;
         ++i, cand = active[cl][i])
    {
        constVal = backend->get_location (backend->gen_vreg (virtual_regs, cand->order, 0));
        if  (constVal != NULL &&
             ! constVal->is_addr &&
             constVal->base->var->sdSymKind == SYM_VAR &&
             constVal->base->var->sdType->tdTypeKind == TYP_INT &&
             constVal->base->var->sdVar.sdvConst &&
             cand->interval->weight != 0x7FFFFFFF)
            goto success;
        if  (constVal != NULL &&
             constVal->is_addr &&
             constVal->base->var->sdSymKind == SYM_VAR &&
             is_global_var (constVal->base->var) &&
             (!constVal->base->var->sdVar.sdvConst || constVal->base->var->sdType->tdTypeKind > TYP_lastIntrins) &&
             cand->interval->weight != 0x7FFFFFFF)
            goto success;
    }

    for (i = 0, cand = active[cl][0];
         classes[cl].used_num > i && cand->lr_end > reg->lr_end;
         ++i, cand = active[cl][i])
        if  (cand->interval->weight != 0x7FFFFFFF)
            goto success;

    for (i = 0, cand = active[cl][0];
         classes[cl].used_num > i;
         ++i, cand = active[cl][i])
        if  (cand->interval->weight != 0x7FFFFFFF)
            goto success;

    cand = reg;
    goto exit;

success:
    reg->reg_class = cand->reg_class;
    reg->hard_num = cand->hard_num;
    active[cl][i] = reg;
    bubble_sort (active[cl], classes[cl].used_num);

exit:
    cand->reg_class = 0;
    cand->spill_slot = backend->assign_spill_slot (fn, reg->reg_class);
    
    return cand;
}

static BOOL linearScan(struct op_reg* ind2reg, int count, control_flow_graph fn, struct avl_table *virtual_regs, struct reg_class_desc *classes, LIST spilledRegs, struct Backend* backend)
{
    int i;
    struct op_reg** *active;

    active = (struct op_reg** *) xmalloc (sizeof (struct op_reg**) * backend->class_count);

    for (i = 0; i < backend->class_count; i++)
    {
        bitmap_clear (classes[i].used);
        classes[i].used_num = 0;
    }

    for (i = 0; i < count; i++)
        ind2reg[i].reg_class = backend->gen_vreg (virtual_regs, ind2reg[i].order, 0)->rclass;

    /* 按起始点递增的方式对所有间隔排序。  */
    qsort( (void *)(ind2reg + backend->num_physical_regs), count - backend->num_physical_regs, sizeof( struct op_reg ), cmp_begin );
    for (i = 0; i < backend->class_count; i++)
        active[i] = (struct op_reg **) xmalloc (sizeof (struct op_reg*) * backend->num_physical_regs);

    /* 接下来是线性扫描分配。  */
    for (i = backend->num_physical_regs; i < count; i++)
    {
        int cl;
        struct op_reg *reg = ind2reg + i;
        expire_old_intervals (reg, active, classes, backend->class_count);
        cl = reg->reg_class;
        if (try_alloc_reg (ind2reg, classes, reg) >= 0)
        {
            active[cl][classes[cl].used_num - 1] = reg;
            bubble_sort (active[cl], classes[cl].used_num);
        }
        else
        {
            *(struct op_reg* *)List_NewLast(spilledRegs, sizeof (struct op_reg*)) = spill_at_interval (reg, active, classes, fn, virtual_regs, backend);
        }

        /* 一些有趣的转储。  */
        TRACE_LINEAR_SCAN
        (
            fprintf (stderr, "  reg%d: [%5d, %5d)->",
                     reg->order, reg->lr_begin, reg->lr_end);
            if (reg->reg_class)
            {
                fprintf (stderr, "$%c%i", reg->reg_class == GENERAL_REGS ? 'r' : 's', reg->hard_num);
            }
            else
            {
                fprintf (stderr, "[%%__%s_%i]",
                         "spill",
                         reg->spill_slot);
            }
            for (int cl = 0; cl < LIM_REG_CLASSES; cl++)
            {
                BOOL first = TRUE;
                struct op_reg *r = NULL;
                fprintf (stderr, " {");
                for (int j = 0; classes[cl].used_num > j; j++)
                {
                    r = active[cl][j];
                    if (first)
                    {
                        fprintf (stderr, "%d", r->order);
                        first = FALSE;
                    }
                    else
                        fprintf (stderr, ", %d", r->order);
                }
                fprintf (stderr, "}");
            }
            fprintf (stderr, "\n");
        )
    }

    TRACE_LINEAR_SCAN
    (
        for (i = 0; i < count; i++)
        {
            struct op_reg *reg = ind2reg + i;
            struct Range* cur = reg->interval->_first;
            if (!reg)
              continue;
            fprintf (stderr, "  reg%d: ", reg->order);
            while (cur != NULL) {
                fprintf(stderr, "[%5d, %5d) ", cur->_from, cur->_to);
                cur = cur->_next;
            }
            fprintf (stderr, "->");
            if (reg->reg_class)
            {
                fprintf (stderr, "$%c%i\n", reg->reg_class == GENERAL_REGS ? 'r' : 's', reg->hard_num);
            }
            else
            {
                fprintf (stderr, "[%%__%s_%i]\n",
                       "spill",
                       reg->spill_slot);
            }
        }
    )

    for (i = 0; i < backend->class_count; i++)
        free (active[i]);
    free (active);

    return !List_IsEmpty (spilledRegs);
}

static void addIntervalsForSpills(struct op_reg *spilled_, struct avl_table *virtual_regs, LIST new_regs, struct Backend* backend)
{
    LIR_Op *insn, *next_insn;
    int i;
    union LIR_Opr operand;
    struct op_reg *new_reg;

    if  (spilled_->interval->weight == 0x7FFFFFFF)
        fatal ("attempt to spill already spilled interval!");

    for(  insn=(LIR_Op *)List_First(spilled_->use_pos)
       ;  insn!=NULL
       ;  insn = next_insn
       )
    {
        next_insn = (LIR_Op *)List_Next((void *)insn);
        for (i = 0; i < backend->operand_count (*insn); ++i)
        {
            if  (backend->as_register (*insn, i) &&
                 backend->as_register (*insn, i)->vregno == spilled_->order)
            {
                int start = (*insn)->uid - !backend->op_output_p (*insn, i);
                int end = 1 + (*insn)->uid - !backend->op_output_p (*insn, i);

                operand.vreg = backend->gen_vreg (virtual_regs, -1, backend->gen_vreg (virtual_regs, spilled_->order, 0)->rclass);
                backend->regdesc (operand.vreg, backend->gen_vreg (virtual_regs, spilled_->order, 0), NULL, NULL, FALSE);

                new_reg = (struct op_reg *) List_NewLast (new_regs, sizeof (struct op_reg));
                init_reg_data (new_reg, operand.vreg->vregno, operand.vreg->rclass);
                new_reg->lr_begin = start, new_reg->lr_end = end;
                backend->set_op (*insn, i, &operand);
                new_reg->interval->weight = 0x7FFFFFFF;
                new_reg->spill_slot = spilled_->spill_slot;
                if  (backend->op_output_p (*insn, i))
                    backend->spill_out (*insn, operand.vreg, spilled_->spill_slot, virtual_regs);
                else
                    backend->spill_in (*insn, operand.vreg, spilled_->spill_slot, virtual_regs);
            }
        }
    }

    spilled_->lr_begin = spilled_->lr_end = 0;
}

static void
assign_reg_num (struct avl_table *virtual_regs, struct op_reg *ind2reg, int count, struct Backend* backend)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if  (ind2reg[i].reg_class)
        {
            backend->gen_vreg (virtual_regs, ind2reg[i].order, 0)->hard_num = ind2reg[i].hard_num;
            if  (ind2reg[i].spill_slot != -1)
                backend->gen_vreg(virtual_regs, ind2reg[i].order, 0)->spill_slot = ind2reg[i].spill_slot;
        }
    }
}

static void
compute_global_live_sets (LIST blocks, struct Backend* backend)
{
    BOOL change_occurred;
    basic_block* block;
    LIR_Op instr;
    bitmap work = BITMAP_XMALLOC ();
    bitmap temp = BITMAP_XMALLOC ();

    do
      {
        change_occurred = FALSE;
        for(  block=(basic_block *)List_Last(blocks)
           ;  block!=NULL
           ;  block = (basic_block *)List_Prev((void *)block)
           )
        {
            edge *ei;
            /* 后继的livein的并集（如果没有，则为空）。  */
            BOOL first = TRUE;
            for(  ei=(edge *)List_First((*block)->succs)
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei)
               )
            {
                basic_block succ =  ((*ei)->dest);
                if (first)
                {
                    bitmap_copy (work, backend->get_live_in (succ));
                    first = FALSE;
                }
                else
                    bitmap_ior_into (work, backend->get_live_in (succ));
            }
            if (first)
                bitmap_clear (work);
  
            bitmap_copy (backend->get_live_out (*block), work);

            /* 在反向指令遍历中删除定义，包括使用。  */
            for(  instr=(LIR_Op)List_Last(backend->get_code (*block))
               ;  instr!=NULL
               ;  instr = (LIR_Op)List_Prev((void *)instr)
               )
            {
                unsigned regno;
                bitmap_iterator bi;
                backend->output_regs (instr, temp);
                for (bmp_iter_set_init (&bi, temp, 0, &regno);
                     bmp_iter_set (&bi, &regno);
                     bmp_iter_next (&bi, &regno))
                    bitmap_clear_bit (work, regno);
                backend->input_regs (instr, temp);
                for (bmp_iter_set_init (&bi, temp, 0, &regno);
                     bmp_iter_set (&bi, &regno);
                     bmp_iter_next (&bi, &regno))
                    bitmap_set_bit (work, regno);
            }

            /* 记录这是否改变了什么。  */
            if (bitmap_ior_into (backend->get_live_in (*block), work))
                change_occurred = TRUE;
        }
      }
    while (change_occurred);

    BITMAP_XFREE (work);
    BITMAP_XFREE (temp);
}

static void
linear_scan_regalloc (control_flow_graph fn, struct avl_table *virtual_regs, struct reg_class_desc *classes, struct Backend* backend)
{
    /* 计算活跃性。  */
    LIST bbs = List_Create ();
    basic_block *bb;
    int i, j;
    int insn_order;
    struct avl_traverser trav;
    vreg_t iter;
    int index;
    struct op_reg* ind2reg = NULL;
    bitmap temp = BITMAP_XMALLOC ();
    LIR_Op last_insn;
    int count = (int) avl_count(virtual_regs);
    LIST spilledRegs;
    LIR_Op insn;

    /* 我们需要逆后序遍历来进行线性化，以及后序遍历用于活跃性分析，
       这两者是相同的，都是从后向前进行。  */
    pre_and_rev_post_order_compute (fn, NULL, bbs, TRUE, FALSE);
    ind2reg = (struct op_reg *) xmalloc (sizeof (struct op_reg) * count);

    memset (ind2reg, 0, sizeof (struct op_reg) * count);

    /* 为所有指令分配一个线性编号，同时构建一个从寄存器索引到寄存器的映射。  */
    insn_order = 1;
    for(  bb=(basic_block *)List_First(bbs)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Next((void *)bb)
       )
        for(  insn=(LIR_Op)List_First(backend->get_code (*bb))
           ;  insn!=NULL
           ;  insn = (LIR_Op)List_Next((void *)insn)
           )
            insn->uid = insn_order++;

    for(  iter = (vreg_t)avl_t_first (&trav, virtual_regs), index = 0
       ;  iter != NULL
       ;  iter = (vreg_t)avl_t_next (&trav), index++
       )
    {
        init_reg_data (ind2reg+index, iter->vregno, iter->rclass);
        if (! backend->is_virtual_register (index))
            ind2reg[index].hard_num = index;
    }

    /* 将所有活跃范围初始化为[after-end, 0)。  */
    for (i = 0; i < count; i++)
        ind2reg[i].lr_begin = insn_order, ind2reg[i].lr_end = 0;

    /* 经典的活跃分析，只要有东西发生变化：
         liveout是后继的livein的并集
         livein是liveout减去定义加上使用。  */
    compute_global_live_sets (bbs, backend);

    /* 占住SPILL_REG防止被分配。  */
    for (i = 0; i < backend->class_count; i++)
    {
        for (j = classes[i].next_avail; j < classes[i].max_num; j++)
        {
            if  (!bitmap_bit_p(classes[i].available, j))
            {
                ind2reg[j].lr_begin = 0;
                ind2reg[j].lr_end = insn_order;
                add_def (ind2reg[j].interval, 0);
                add_use (ind2reg[j].interval, 0, insn_order);
            }
        }
    }

    /* 按线性顺序遍历所有指令，记录并合并可能的活动范围起点和终点。  */
    last_insn = NULL;
    for(  bb=(basic_block *)List_Last(bbs)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Prev((void *)bb)
       )
    {
        int after_end_number;
        unsigned bit;
        bitmap_iterator bi;
        int before_start_number;
        int k;
        int start;

        struct op_reg*result;
        struct op_reg key;

        if (last_insn)
            after_end_number = last_insn->uid;
        else
            after_end_number = insn_order;

        if (List_First (backend->get_code (*bb)))
            before_start_number = ((LIR_Op)List_First (backend->get_code (*bb)))->uid;
        else
            before_start_number = after_end_number;
        before_start_number--;

        /* Everything live-out in this BB has at least an end point
           after us.  */
        for (bmp_iter_set_init (&bi, backend->get_live_out (*bb), 0, &bit);
             bmp_iter_set (&bi, &bit);
             bmp_iter_next (&bi, &bit))
        {
            key.order = bit;
            result = (struct op_reg*)bsearch( (const void *) &key, (const void *)ind2reg, count,
                                       sizeof( struct op_reg ), (int (*)(const void*, const void*))compare );
            note_lr_end (result, after_end_number);
            add_use(result->interval, before_start_number, after_end_number);
        }

        for(  insn=(LIR_Op)List_Last(backend->get_code (*bb))
           ;  insn!=NULL
           ;  insn = (LIR_Op)List_Prev((void *)insn)
           )
        {
            /* 如果指令销毁调用者保存的寄存器，则为每个寄存器添加一个临时范围。  */
            if (backend->is_call (insn))
            {
                start = backend->handle_method_arguments (insn)->uid;
                for (k = 0; k < backend->num_caller_save_registers; k++)
                {
                    add_use(ind2reg[backend->caller_save_registers[k]].interval, start, insn->uid + 1);
                    note_lr_begin (ind2reg + backend->caller_save_registers[k], start);
                    note_lr_end (ind2reg + backend->caller_save_registers[k], insn->uid + 1);
                }
            }

            backend->output_regs (insn, temp);
            for (bmp_iter_set_init (&bi, temp, 0, &bit);
                 bmp_iter_set (&bi, &bit);
                 bmp_iter_next (&bi, &bit))
            {
                key.order = bit;
                result = (struct op_reg*)bsearch( (const void *) &key, (const void *)ind2reg, count,
                                           sizeof( struct op_reg ), (int (*)(const void*, const void*))compare );
                note_lr_begin (result, insn->uid);
                note_lr_end (result, insn->uid);

                add_def (result->interval, insn->uid);
                *(LIR_Op *)List_NewLast(result->use_pos, sizeof (LIR_Op)) = insn;
            }
            backend->input_regs (insn, temp);
            for (bmp_iter_set_init (&bi, temp, 0, &bit);
                 bmp_iter_set (&bi, &bit);
                 bmp_iter_next (&bi, &bit))
            {
                key.order = bit;
                result = (struct op_reg*)bsearch( (const void *) &key, (const void *)ind2reg, count,
                                           sizeof( struct op_reg ), (int (*)(const void*, const void*))compare );
                note_lr_end (result, insn->uid);
                add_use(result->interval, before_start_number, insn->uid);
                *(LIR_Op *)List_NewLast(result->use_pos, sizeof (LIR_Op)) = insn;
            }
        }

        /* Everything live-in in this BB has a start point before
           our first insn.  */
        for (bmp_iter_set_init (&bi, backend->get_live_in (*bb), 0, &bit);
             bmp_iter_set (&bi, &bit);
             bmp_iter_next (&bi, &bit))
        {
            key.order = bit;
            result = (struct op_reg*)bsearch( (const void *) &key, (const void *)ind2reg, count,
                                       sizeof( struct op_reg ), (int (*)(const void*, const void*))compare );
            note_lr_begin (result, before_start_number);
            add_def (result->interval, before_start_number);
        }
  
        if (List_First (backend->get_code (*bb)))
            last_insn = (LIR_Op)List_First (backend->get_code (*bb));
    }

    for (i = 0; i < count; i++)
    {
        /* 所有仍然在所有代码之后开始的寄存器实际上是在例程开头定义的。  */
        if (ind2reg[i].lr_begin == insn_order)
           ind2reg[i].lr_begin = 0;
        /* 所有没有使用但有定义的寄存器都有lr_end == 0，它们实际上是从
           定义开始直到定义它们的指令之后处于活跃状态。  */
/*      if (ind2reg[i].lr_end == 0)
           ind2reg[i].lr_end = ind2reg[i].lr_begin + 1; */
    }

    BITMAP_XFREE (temp);
    List_Destroy (&bbs);

    spilledRegs = List_Create ();
    while (linearScan (ind2reg, count, fn, virtual_regs, classes, spilledRegs, backend)) {
        /* 我们溢出了一些寄存器，因此我们需要为溢出代码添加间隔并重新启动算法。  */
        struct op_reg* *spilled_;
        LIST new_regs = List_Create ();
        struct op_reg *new_reg;

        for(  spilled_=(struct op_reg* *)List_First(spilledRegs)
           ;  spilled_!=NULL
           ;  spilled_ = (struct op_reg* *)List_Next((void *)spilled_)
           )
            addIntervalsForSpills (*spilled_, virtual_regs, new_regs, backend);

        List_Clear (spilledRegs);

        ind2reg =
          (struct op_reg*) xrealloc (ind2reg, sizeof (struct op_reg) * (count + List_Card (new_regs)));
        for(  new_reg=(struct op_reg*)List_First(new_regs)
           ;  new_reg!=NULL
           ;  new_reg = (struct op_reg*)List_Next((void *)new_reg)
           )
        {
            memcpy (ind2reg + count, new_reg, sizeof (struct op_reg));
            count++;
        }
        List_Destroy (&new_regs);
    }
    List_Destroy (&spilledRegs);

    assign_reg_num (virtual_regs, ind2reg, count, backend);

    for (i = 0; i < count; i++)
        free_reg_data(ind2reg+i);
    free (ind2reg);

    TRACE_LINEAR_SCAN
    (
        for(  bb=(basic_block *)List_First(fn->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
        {
            fprintf (stderr, "BB %i:\n", (*bb)->index);
            bitmap_print (stderr, ((BblockArm32)(*bb)->param)->livein, "livein  ", "\n");
            bitmap_print (stderr, ((BblockArm32)(*bb)->param)->liveout, "liveout ", "\n");
        }
    )
}

/* 寄存器分配的入口点。  */
void
LinearScanAllocator (control_flow_graph fn, struct avl_table *virtual_regs, struct Backend* backend)
{
    int i;
    struct reg_class_desc *classes;

    classes = (struct reg_class_desc *) xmalloc (sizeof (struct reg_class_desc) * backend->class_count);
    memset (classes, 0, sizeof (struct reg_class_desc) * backend->class_count);
    for (i = 0; i < backend->class_count; i++)
    {
        classes[i].available = BITMAP_XMALLOC ();
        bitmap_copy (classes[i].available, backend->classes[i].available);
        classes[i].next_avail = backend->classes[i].next_avail;
        classes[i].max_num = backend->classes[i].max_num;
        classes[i].used = BITMAP_XMALLOC ();
    }

    linear_scan_regalloc (fn, virtual_regs, classes, backend);

    for (i = 0; i < backend->class_count; i++)
    {
        BITMAP_XFREE (classes[i].used);
        BITMAP_XFREE (classes[i].available);
    }
    free (classes);

}
