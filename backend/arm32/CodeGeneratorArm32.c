#include <stdlib.h>
#include <string.h>
#include "all.h"

static varpool_node zero;

static void
init_caller_save (void)
{
    int i;
    for (i = 0; i < 16; i++) {
      caller_save_registers[num_caller_save_registers++] = VREG(i);
    }
    for (i = 0; i < 4; i++) {
      caller_save_registers[num_caller_save_registers++] = i;
    }
}

static void
init_callee_save (void)
{
    int i;

    /* 子程序必须保存寄存器r4 ~ r8、r10、r11和SP的内容。
       寄存器r9的角色是平台相关的。  */
    for (i = 4; i < 12; i++) {
      callee_save_registers[num_callee_save_registers++] = i;
    }

    callee_save_registers[num_callee_save_registers++] = LR_REGNUM;

    for (i = 16; i < 32; i++) {
      callee_save_registers[num_callee_save_registers++] = VREG(i);
    }
}

static int
cmp_vreg (vreg_tArm32 a, vreg_tArm32 b, void *p)
{
    return a->vregno - b->vregno;
}

static void
init_virtual_regs (struct avl_table **virtual_regs)
{
    int i;

    *virtual_regs = avl_create((avl_comparison_func *)cmp_vreg, NULL, NULL);

    /* 确保物理寄存器都存在。  */
    for (i = 0; i < FIRST_VIRTUAL_REGISTER; i++)
    {
        gen_vregArm32 (*virtual_regs, i, NO_REGS);
    }
}

static void
delete_vregArm32 (vreg_tArm32 vreg)
{
    List_Destroy (&vreg->base_offset);
    free (vreg);
}

static void
finalize_virtual_regs (struct avl_table **virtual_regs)
{
    if  (*virtual_regs)
    {
        avl_destroy(*virtual_regs, (avl_item_func *)delete_vregArm32);
        *virtual_regs = NULL;
    }
}

vreg_tArm32
gen_vregArm32 (struct avl_table *virtual_regs, int vregno, enum reg_classArm32 rclass)
{
    vreg_tArm32 vreg;
    static int counter = FIRST_VIRTUAL_REGISTER;

    if  (vregno >= 0)
    {
        vreg = (vreg_tArm32)avl_find (virtual_regs, &vregno);
        if  (vreg)
            return  vreg;
    }

    vreg = (vreg_tArm32) xmalloc (sizeof (*vreg));
    memset (vreg, 0, sizeof(*vreg));
    vreg->base_offset = List_Create();
    vreg->vregno = vregno >= 0 ? vregno : counter++;
    vreg->spill_slot = -1;
    if      (is_virtual_registerArm32 (vreg->vregno))
    {
        vreg->rclass = rclass;
        vreg->hard_num = -1;
    }
    else if (IS_VFP_REGNUM (vreg->vregno))
    {
        vreg->rclass = VFP_REGS;
        vreg->hard_num = vregno;
    }
    else
    {
        vreg->rclass = GENERAL_REGS;
        vreg->hard_num = vregno;
    }
    avl_insert (virtual_regs, vreg);
    return vreg;
}

void
output_reg (FILE *file, vreg_tArm32 reg)
{
    int regno = reg->hard_num == -1 ? reg->vregno : reg->hard_num;
    if      (is_virtual_registerArm32 (regno))
        fprintf (file, "reg%d", regno);
    else if (IS_VFP_REGNUM (regno))
        fprintf (file, "s%d", regno - FIRST_VFP_REGNUM);
    else if (regno == PC_REGNUM)
        fprintf (file, "pc");
    else if (regno == LR_REGNUM)
        fprintf (file, "lr");
    else if (regno == SP_REGNUM)
        fprintf (file, "sp");
    else if (regno == IP_REGNUM)
        fprintf (file, "ip");
    else
        fprintf (file, "r%d", regno);
}

BOOL
is_virtual_registerArm32 (int num)
{
    return (num >= FIRST_VIRTUAL_REGISTER);
}

/* 修改寄存器描述符。  */
void
register_descriptorArm32 (vreg_tArm32 vreg, vreg_tArm32 from, varpool_node base, varpool_node offset, BOOL is_addr)
{
    struct base_offset *curs;
    struct base_offset *new_ptr;

    if      (from)
    {
        if  (!offset)
            offset = zero;

        for(  curs=(struct base_offset *)List_First(from->base_offset)
           ;  curs!=NULL
           ;  curs = (struct base_offset *)List_Next((void *)curs)
           )
        {
            new_ptr = (struct base_offset *) List_NewLast (vreg->base_offset, sizeof (*new_ptr));
            memcpy (new_ptr, curs, sizeof (*curs));
        }
    }
    else if (base)
    {
        if  (!offset)
            offset = zero;
        for(  curs=(struct base_offset *)List_First(vreg->base_offset)
           ;  curs!=NULL
           ;  curs = (struct base_offset *)List_Next((void *)curs)
           )
            if  (curs->base == base &&
                 curs->offset == offset &&
                 curs->is_addr == is_addr)
                return;
        curs = (struct base_offset *) List_NewLast (vreg->base_offset, sizeof (*curs));
        memset (curs, 0, sizeof (*curs));
        curs->base = base;
        curs->offset = offset;
        curs->is_addr = is_addr;
    }
    else
        List_Clear (vreg->base_offset);
}

struct base_offset *
get_locationArm32(vreg_tArm32 vreg)
{
    return (struct base_offset *)List_First (vreg->base_offset);
}

/* 找到操作数加载到的虚拟寄存器。  */
vreg_tArm32 get_registerArm32 (varpool_node base, varpool_node offset, enum reg_classArm32 rclass, BOOL is_addr)
{
    struct locationArm32 *iter;

    if  (!offset)
        offset = zero;

    /* 是数组，只能悲观地认为值不在寄存器，需要产生load指令。  */
    if  (base->var->sdType->tdTypeKind > TYP_lastIntrins && !is_addr)
        return NULL;

    /* 地址描述符为空。  */
    if  (List_IsEmpty (is_addr ? base->addr : base->value))
        return NULL;

    if  (rclass == NO_REGS)
        return NULL;

    /* 遍历地址描述符，找到它所在的寄存器。  */
    for(  iter = (struct locationArm32 *)List_First (is_addr ? base->addr : base->value)
       ;  iter != NULL
       ;  iter = (struct locationArm32 *)List_Next ((void *)iter)
       )
        if  (iter->is_reg &&
             !compare_varpool_node(offset, iter->offset, (void *) FALSE)   &&
             (rclass == ALL_REGS || rclass == iter->vreg->rclass))
            return iter->vreg;

    /* 没有找到虚拟寄存器。  */
    return NULL;
}

void location_descriptorArm32 (varpool_node base, varpool_node offset, vreg_tArm32 vreg, BOOL reset, BOOL is_addr)
{
    struct locationArm32 *curs;

    if  (reset)
    {
        List_Clear (is_addr ? base->addr : base->value);
    }
    else
    {
        if  (!offset)
            offset = zero;

        for(  curs=(struct locationArm32 *)List_First(is_addr ? base->addr : base->value)
           ;  curs!=NULL
           ;  curs = (struct locationArm32 *)List_Next((void *)curs)
           )
            if  (curs->is_reg == (vreg != NULL) &&
                 curs->offset == offset &&
                 (curs->is_reg ? curs->vreg == vreg : curs->base == base))
                return;
        curs = (struct locationArm32 *)List_NewLast (is_addr ? base->addr : base->value, sizeof (*curs));
        memset (curs, '\0', sizeof (*curs));
        curs->is_reg = vreg != NULL;
        curs->offset = offset;
        if  (curs->is_reg)
            curs->vreg = vreg;
        else
            curs->base = base;
    }
}

void
remove_locationArm32 (vreg_tArm32 vreg)
{
    struct base_offset *curs;
    struct locationArm32 *iter;

    for(  curs=(struct base_offset *)List_First(vreg->base_offset)
       ;  curs!=NULL
       ;  curs = (struct base_offset *)List_Next((void *)curs)
       )
    {
        for(  iter=(struct locationArm32 *)List_First(curs->is_addr ? curs->base->addr : curs->base->value)
           ;  iter!=NULL
           ;  iter = (struct locationArm32 *)List_Next((void *)iter)
           )
        {
            if  (iter->is_reg &&
                 !compare_varpool_node(curs->offset, iter->offset, (void *) FALSE)   &&
                 (vreg->rclass == ALL_REGS || vreg->rclass == iter->vreg->rclass))
            {
                List_Delete (iter);
                break;
            }
        }
    }
}

static void
backend_init (InterCode code)
{
    control_flow_graph *func;
    basic_block *bb;

    for(  func=(control_flow_graph *)List_First(code->funcs)
        ;  func!=NULL
        ;  func = (control_flow_graph *)List_Next((void *)func)
        )
    {
        (*func)->param = (void *) xmalloc (sizeof (struct BfunctionArm32));
        memset ((*func)->param, 0, sizeof (struct BfunctionArm32));
        ((struct BfunctionArm32 *)(*func)->param)->callee_saved_reg = BITMAP_XMALLOC ();
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
            ;  bb!=NULL
            ;  bb = (basic_block *)List_Next((void *)bb)
            )
        {
            (*bb)->param = (void *) xmalloc (sizeof (struct BblockArm32));
            memset ((*bb)->param, 0, sizeof (struct BblockArm32));
            ((BblockArm32)(*bb)->param)->code = List_Create();
            ((BblockArm32)(*bb)->param)->livein = BITMAP_XMALLOC ();
            ((BblockArm32)(*bb)->param)->liveout = BITMAP_XMALLOC ();
        }
    }
}

static void
backend_finalize (InterCode code)
{
    control_flow_graph *func;
    basic_block *bb;
    void *Cursor;

    for(  func=(control_flow_graph *)List_First(code->funcs)
        ;  func!=NULL
        ;  func = (control_flow_graph *)List_Next((void *)func)
        )
    {
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
            ;  bb!=NULL
            ;  bb = (basic_block *)List_Next((void *)bb)
            )
        {
            for(  Cursor=List_Last ( ((BblockArm32)(*bb)->param)->code)
                ;  Cursor!=NULL
                ;  Cursor = List_Last ( ((BblockArm32)(*bb)->param)->code)
                )
                deleteArmInst((ArmInst)Cursor);
            List_Destroy (&((BblockArm32)(*bb)->param)->code);
            BITMAP_XFREE (((BblockArm32)(*bb)->param)->livein);
            BITMAP_XFREE (((BblockArm32)(*bb)->param)->liveout);
            free ((*bb)->param);
            (*bb)->param = NULL;
        }
        BITMAP_XFREE (((struct BfunctionArm32 *)(*func)->param)->callee_saved_reg);
        free ((*func)->param);
        (*func)->param = NULL;
    }
}

static void stackAllocArm32 (control_flow_graph func, varpool_node_set set)
{
    struct avl_traverser trav;
    varpool_node iter;
    basic_block *bb;
    IRInst *Cursor;

    /* 计算存放在栈上的参数的大小。  */
    for(  bb=(basic_block *)List_First(func->basic_block_info)
        ;  bb!=NULL
        ;  bb = (basic_block *)List_Next((void *)bb)
        )
    {
        for(  Cursor=(IRInst *)List_First((*bb)->insns)
            ;  Cursor!=NULL
            ;  Cursor = (IRInst *)List_Next((void *)Cursor)
            )
        {
            if  ((*Cursor)->opcode == IRINST_OP_call)
            {
                func->arg_size = max(func->arg_size, (GetConstVal(IRInstGetOperand(*Cursor, 2)->var, 0)->cvValue.cvIval - NUM_ARG_REGS) * UNITS_PER_WORD);
            }
        }
    }

    /* 为局部变量分配空间。  */
    for(  iter = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  iter != NULL
       ;  iter = (varpool_node)avl_t_next (&trav)
       )
    {
        if  (iter->var->sdSymKind == SYM_VAR &&
             iter->var->sdVar.sdvLocal &&
             iter->var->sdType->tdTypeKind > TYP_lastIntrins &&
             ! iter->var->sdVar.sdvArgument)
        {
            iter->sdvOffset = func->local_size + func->arg_size;
            if  (iter->var->sdType->tdTypeKind != TYP_VOID)
                func->local_size = func->local_size + type_size(iter->var->sdType);
        }
    }
}

/* 初始化描述符。  */
static void init_descriptors (varpool_node_set set)
{
    struct avl_traverser trav;
    varpool_node iter;

    for(  iter = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  iter != NULL
       ;  iter = (varpool_node)avl_t_next (&trav)
       )
    {
        if  (iter->var->sdSymKind == SYM_VAR)
        {
            location_descriptorArm32 (iter, NULL, NULL, TRUE, FALSE);
            location_descriptorArm32 (iter, NULL, NULL, TRUE, TRUE);
            location_descriptorArm32 (iter, NULL, NULL, FALSE, FALSE);
            location_descriptorArm32 (iter, NULL, NULL, FALSE, TRUE);
        }
    }
}

static void
dump_footer (FILE *file)
{
    fprintf (file, "\t.ident\t\"The right way of Light\"\n");
}

static void
dump_header (const char *name, FILE *file, BOOL debug_info)
{
    fprintf (file, "\t.arch armv7ve\n");
    fprintf (file, "\t.fpu vfp\n");
    fprintf (file, "\t.file\t\"%s\"\n", name ? name : "<unknown>");

    if  (debug_info)
    {
        fprintf (file, "\t.file\t 1 \"%s\"\n", name ? name : "<unknown>");
    }

    fprintf (file, "\n");
}

static void
dump_global_variables (SymDef var, FILE *file)
{
    int size;
    int position;
    struct avl_traverser trav;
    ConstVal iter;

    if  (var->sdSymKind == SYM_VAR &&
         var->sdType->tdTypeKind != TYP_VOID &&
         !var->sdIsImplicit)
    {
        size = type_size(var->sdType);
        fprintf (file, "\t.text\n");
        fprintf (file, var->sdVar.sdvHadInit ? "" : "\t.bss\n");
        fprintf (file, "\t.global\t%s\n", var->sdName);
        fprintf (file, "\t.data\n");
        fprintf (file, "\t.align\t%d\n", 2);
        fprintf (file, "\t.type\t%s, %%object\n", var->sdName);
        fprintf (file, "\t.size\t%s, %d\n", var->sdName, size);
        fprintf (file, "%s:\n", var->sdName);

        /* 输出初始值。  */
        position = 0;
        for(  iter = (ConstVal)avl_t_first (&trav, var->sdVar.sdvCnsVal)
           ;  iter != NULL
           ;  iter = (ConstVal)avl_t_next (&trav)
           )
        {
            if  (iter->index != position)
                fprintf (file, "\t.space\t%d\n", (iter->index - position) * type_size (stGetBaseType (var->sdType)));
            fprintf (file, "\t.word\t%d\n", iter->cvValue.cvIval);
            position = iter->index + 1;
        }
        if  (size != position * type_size (stGetBaseType (var->sdType)))
            fprintf (file, var->sdVar.sdvHadInit ? "\t.space\t%d\n" : "\t.zero\t%d\n", size - position * type_size (stGetBaseType (var->sdType)));
        fprintf (file, "\n");
    }
}

static void
dump_func_header(SymDef func, FILE *file)
{
    fprintf (file, "\t.text\n");
    fprintf (file, "\t.align\t%d\n", 1);
    fprintf (file, "\t.global\t%s\n", stGetSymName(func));
    fprintf (file, "\t.syntax unified\n");
    fprintf (file, "\t.type\t%s, %%function\n", stGetSymName(func));
    fprintf (file, "%s:\n", stGetSymName(func));
}

static void
dump_func_footer(SymDef func, FILE *file)
{
    fprintf (file, "\t.size\t%s, .-%s\n", stGetSymName(func), stGetSymName(func));
    fprintf (file, "\n");
}

static void
dwarfout_end_function_1 (struct dwarf_data *ddata, control_flow_graph func)
{
    dw_cfi_ref cfi;
    dw_fde_ref fde;
    char label[30];
    long int offset;

    fde = &ddata->fde_table[ddata->fde_table_in_use - 1];

    /* 在入口,调用帧地址在堆栈指针寄存器中。  */
    cfi = new_cfi (ddata);
    cfi->dw_cfi_opc = DW_CFA_def_cfa;
    cfi->dw_cfi_oprnd1.dw_cfi_reg_num = SP_REGNUM;
    cfi->dw_cfi_oprnd2.dw_cfi_offset = 0;
    add_cfi (&fde->dw_fde_cfi, cfi);

    /* 将位置计数器设置到函数prolog的末尾。  */
    cfi = new_cfi (ddata);
    cfi->dw_cfi_opc = DW_CFA_advance_loc4;
    sprintf (label, ".L_b%u", func->funcdef_number);
    cfi->dw_cfi_oprnd1.dw_cfi_addr = (char *) List_NewLast (ddata->mem, (unsigned int) strlen (label) + 1);
    strcpy (cfi->dw_cfi_oprnd1.dw_cfi_addr, label);
    add_cfi (&fde->dw_fde_cfi, cfi);

    /* Define the CFA as either an explicit frame pointer register,
       or an offset from the stack pointer.  */
    cfi = new_cfi (ddata);
    cfi->dw_cfi_opc = DW_CFA_offset;
    cfi->dw_cfi_oprnd1.dw_cfi_reg_num = SP_REGNUM;
    offset = func->arg_size + func->callee_saved_reg_size + func->local_size + func->spill_size + func->pad_size;
    cfi->dw_cfi_oprnd2.dw_cfi_offset = offset / (-4);
    add_cfi (&fde->dw_fde_cfi, cfi);

    sprintf (label, ".L_b%u_e", func->funcdef_number);
    cfi = new_cfi (ddata);
    cfi->dw_cfi_opc = DW_CFA_advance_loc4;
    cfi->dw_cfi_oprnd1.dw_cfi_addr = (char *) List_NewLast (ddata->mem, (unsigned int) strlen (label) + 1);
    strcpy (cfi->dw_cfi_oprnd1.dw_cfi_addr, label);
    add_cfi (&fde->dw_fde_cfi, cfi);

    /* Define the rule for restoring the stack pointer.  */
    cfi = new_cfi (ddata);
    cfi->dw_cfi_opc = DW_CFA_def_cfa;
    cfi->dw_cfi_oprnd1.dw_cfi_reg_num = SP_REGNUM;
    add_cfi (&fde->dw_fde_cfi, cfi);
}

/* Generate a DIE to represent a declared function (either file-scope or
   block-local).  */
static void
gen_subprogram_die (SymDef decl, struct dwarf_data *ddata, control_flow_graph func)
{
    char label_id[30];
    dw_die_ref subr_die;
    dw_loc_descr_ref fp_loc = NULL;
    unsigned fp_reg;
    TypDef type;
    dw_die_ref context_die = ddata->comp_unit_die;

    {
      subr_die = new_die (DW_TAG_subprogram, context_die, ddata);
          add_AT_flag (subr_die, DW_AT_external, 1, ddata);
      add_name_and_src_coords_attributes (subr_die, decl, ddata);
      type = decl->sdType;
      add_AT_flag (subr_die, DW_AT_prototyped, 0, ddata);
      add_type_attribute (subr_die, type, context_die, ddata);

    }
    {
      add_AT_lbl_id (subr_die, DW_AT_low_pc, decl->sdName, ddata);
      sprintf (label_id, ".L_f%u_e", func->funcdef_number);
      add_AT_lbl_id (subr_die, DW_AT_high_pc, label_id, ddata);

      add_pubname (decl, subr_die, ddata);
    //   add_arange (decl, subr_die);

      /* Define the frame pointer location for this routine.  */
      fp_reg = SP_REGNUM;
      fp_loc = new_loc_descr (DW_OP_breg0 + fp_reg, func->arg_size + func->callee_saved_reg_size + func->local_size + func->spill_size + func->pad_size, 0, ddata);
      add_AT_loc (subr_die, DW_AT_frame_base, fp_loc, ddata);

    }     

    if (decl->sdIsDefined)
      decls_for_scope (decl, subr_die, ddata);

}

static void
dwarfout_decl (struct dwarf_data *ddata, control_flow_graph func, varpool_node_set set, struct avl_table *virtual_regs)
{
    struct avl_traverser trav;
    varpool_node iter;
    vreg_tArm32 vreg;
    dw_loc_descr_ref loc_result;
    int frame_size = func->arg_size + func->callee_saved_reg_size + func->local_size + func ->spill_size + func ->pad_size; 

    for(  iter = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  iter != NULL
       ;  iter = (varpool_node)avl_t_next (&trav)
       )
    {
        if  (iter->var->sdSymKind == SYM_VAR &&
             !iter->var->sdIsImplicit)
        {
            if  (iter->var->sdType->tdTypeKind <= TYP_lastIntrins ||
                 !iter->var->sdType->tdArr.tdaDims->ddSize)
            {
                /* 不是数组。  */
                vreg = get_registerArm32 (iter, NULL, ALL_REGS, iter->var->sdType->tdTypeKind > TYP_lastIntrins);

                if      (is_global_var(iter->var))
                {
                    loc_result = mem_loc_descriptor (iter->var, ddata);
                    add_loc_descr (&iter->var->loc_descr, loc_result);
                }
                else if (vreg)
                {
                    if  (vreg->spill_slot != -1)
                        loc_result = based_loc_descr (SP_REGNUM, vreg->spill_slot - frame_size, ddata);
                    else
                        /* TODO: 使用宏定义替换48。  */
                        loc_result = reg_loc_descriptor (vreg->hard_num >= FIRST_VFP_REGNUM ? vreg->hard_num + 48 : vreg->hard_num, ddata);
                    add_loc_descr (&iter->var->loc_descr, loc_result);
                }
            }
            else
            {
                /* 是数组。  */
                if  (!is_global_var(iter->var))
                {
                    /* 数组首地址用基址+偏移计算。  */
                    loc_result = based_loc_descr (SP_REGNUM, iter->sdvOffset - frame_size, ddata);
                    add_loc_descr (&iter->var->loc_descr, loc_result);
                }
                else
                {
                    loc_result = mem_loc_descriptor (iter->var, ddata);
                    add_loc_descr (&iter->var->loc_descr, loc_result);
                }
            }

        }
    }
}

BOOL CodeGeneratorArm32(InterCode code, SymTab stab, const char *name, FILE *file, struct dwarf_data *ddata) 
{
    BOOL bSuccess = FALSE;
    control_flow_graph *func;
    varpool_node_set set = NULL;
    struct avl_table *virtual_regs = NULL;
    struct ssa_name tmp_var = {0};

    backend_init (code);

    init_caller_save ();
    init_callee_save ();

    /* 产生汇编头部分。  */
    dump_header (name, file, !!ddata);
    if  (ddata != NULL)
        dwarfout_header (file);

    /* 输出全局变量。  */
    traverse_global_variables (stab, (void (*) (SymDef, void *))dump_global_variables, file);

    /* 输出函数。  */
    for(  func=(control_flow_graph *)List_First(code->funcs)
        ;  func!=NULL
        ;  func = (control_flow_graph *)List_Next((void *)func)
        )
    {
        init_virtual_regs (&virtual_regs);

        /* 同一个变量可能在不同的中间指令中多次出现，我们将中间指令中
           出现的所有变量保存在varpool中，保证一个变量只出现一次。  */
        set = varpool_node_set_new (*func, FALSE);
        tmp_var.var = stCreateIconNode(stab, 0);
        zero = varpool_node_set_add(set, &tmp_var);

        /* 初始化地址描述符。  */
        init_descriptors (set);

        /* 分配栈空间。  */
        stackAllocArm32 (*func, set);

        /* 产生函数头部分。  */
        dump_func_header (IRInstGetOperand(*(IRInst *)List_First((*func)->entry_block_ptr->insns), 0)->var, file);

        /* 指令选择。  */
        if  (!InstSelectorArm32 (*func, set, stab, virtual_regs, ddata))
            goto fail;

        /* 寄存器分配。  */
        ra_colorize_graph (*func, virtual_regs);
/*      LinearScanAllocator (*func, virtual_regs); */

        if  (comp->cmpConfig.optimize)
            if_convertArm32 (*func);

        /* 更新callee_saved_reg、栈大小。  */
        update_frame_layoutArm32 (*func, virtual_regs);

        if  (ddata != NULL)
        {
            dwarfout_end_function_1 (ddata, *func);
            dwarfout_decl (ddata, *func, set, virtual_regs);
            gen_subprogram_die (IRInstGetOperand(*(IRInst *)List_First((*func)->entry_block_ptr->insns), 0)->var, ddata, *func);
        }

        outputArmCode (*func, file);

        if  (ddata != NULL)
            dwarfout_end_epilogue (ddata, *func);

        /* 产生函数尾部分。  */
        dump_func_footer (IRInstGetOperand(*(IRInst *)List_First((*func)->entry_block_ptr->insns), 0)->var, file);

        free_varpool_node_set (set);
        set = NULL;

        finalize_virtual_regs (&virtual_regs);
    }

    /* 产生汇编尾部分。  */
    if  (ddata != NULL)
        dwarfout_footer (file);
    else
        dump_footer (file);

    bSuccess = TRUE;
fail:
    finalize_virtual_regs (&virtual_regs);

    if  (set)
        free_varpool_node_set (set);

    backend_finalize (code);

    return bSuccess;
}
