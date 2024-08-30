#include <assert.h>
#include <stdlib.h>
#include <string.h>
#if defined (_WIN32)
#include <direct.h> // _getcwd
#else
#include <unistd.h> // _getcwd
#endif
#include "all.h"

/* Characteristics of base types used by the compiler.  */
static struct base_type_struct
{
    char *bt_name;
    enum dwarf_type bt_type;
    int bt_is_signed;
    int bt_size;
} base_type_table[] =
{
    {"void", DW_ATE_unsigned, 0, 0},
    {"int", DW_ATE_signed, 1, /* INT_TYPE_SIZE */ 4*8},
    {"float", DW_ATE_float, 1, /* FLOAT_TYPE_SIZE */ 4*8},
};
#define NUM_BASE_TYPES (sizeof(base_type_table)/sizeof(base_type_table[0]))


/* Add an attribute/value pair to a DIE */
static void
add_dwarf_attr (dw_die_ref die, dw_attr_ref attr)
{
    if (die != NULL && attr != NULL)
    {
        if (die->die_attr == NULL)
        {
            die->die_attr = attr;
            die->die_attr_last = attr;
        }
        else
        {
            die->die_attr_last->dw_attr_next = attr;
            die->die_attr_last = attr;
        }
    }
}

/* Add an unsigned integer attribute value to a DIE.  */
static void
add_AT_unsigned (dw_die_ref die, enum dwarf_attribute attr_kind, unsigned long unsigned_val, struct dwarf_data *ddata)
{
    dw_attr_ref attr = (dw_attr_ref) List_NewLast (ddata->mem, sizeof (dw_attr_node));
    if (attr != NULL)
    {
        attr->dw_attr_next = NULL;
        attr->dw_attr = attr_kind;
        attr->dw_attr_val.val_class = dw_val_class_unsigned_const;
        attr->dw_attr_val.v.val_unsigned = unsigned_val;
        add_dwarf_attr (die, attr);
    }
}

/* Add a string attribute value to a DIE.  */
static void
add_AT_string (dw_die_ref die, enum dwarf_attribute attr_kind, const char *str, struct dwarf_data *ddata)
{
    dw_attr_ref attr = (dw_attr_ref) List_NewLast (ddata->mem, sizeof (dw_attr_node));
    if (attr != NULL)
    {
        attr->dw_attr_next = NULL;
        attr->dw_attr = attr_kind;
        attr->dw_attr_val.val_class = dw_val_class_str;
        attr->dw_attr_val.v.val_str = (char *) List_NewLast(ddata->mem, (unsigned int)strlen(str) + 1);
        strcpy(attr->dw_attr_val.v.val_str,str);
        add_dwarf_attr (die, attr);
    }
}

/* Add a DIE reference attribute value to a DIE.  */
static void
add_AT_die_ref (dw_die_ref die, enum dwarf_attribute attr_kind, dw_die_ref targ_die, struct dwarf_data *ddata)
{
    dw_attr_ref attr = (dw_attr_ref) List_NewLast (ddata->mem, sizeof (dw_attr_node));
    if (attr != NULL)
    {
        attr->dw_attr_next = NULL;
        attr->dw_attr = attr_kind;
        attr->dw_attr_val.val_class = dw_val_class_die_ref;
        attr->dw_attr_val.v.val_die_ref = targ_die;
        add_dwarf_attr (die, attr);
    }
}

/* Add a child DIE below its parent.  */
static void
add_child_die (dw_die_ref parent_die, dw_die_ref die)
{
    if (parent_die != NULL && die != NULL)
    {
        assert (parent_die != die);
        die->die_parent = parent_die;
        die->die_sib = NULL;
        if (parent_die->die_child == NULL)
        {
            parent_die->die_child = die;
            parent_die->die_child_last = die;
        }
        else
        {
            parent_die->die_child_last->die_sib = die;
            parent_die->die_child_last = die;
        }
    }
}

/* Generate an DW_AT_name attribute given some string value to be included as
   the value of the attribute.  */
static void
add_name_attribute (dw_die_ref die, const char *name_string, struct dwarf_data *ddata)
{
    if (name_string && *name_string)
    {
        add_AT_string (die, DW_AT_name, name_string, ddata);
    }
}

/* Return a pointer to a newly created DIE node.  */
dw_die_ref
new_die (enum dwarf_tag tag_value, dw_die_ref parent_die, struct dwarf_data *ddata)
{
    static int label_num = 0;
    dw_die_ref die = (dw_die_ref) List_NewLast (ddata->mem, sizeof (die_node));
    if (die != NULL)
    {
        die->die_tag = tag_value;
        die->die_abbrev = 0;
        die->die_offset = 0;
        die->die_child = NULL;
        die->die_parent = NULL;
        die->die_sib = NULL;
        die->die_child_last = NULL;
        die->die_attr = NULL;
        die->die_attr_last = NULL;
        die->type = NULL;
        die->label_num = ++label_num;
        if (parent_die != NULL)
        {
            add_child_die (parent_die, die);
        }
    }
    return die;
}

/* Reset the base type to DIE table, and build a special predefined
   base type entry for the "int" signed integer base type.  The
   "int" base type is used to construct subscript index range
   definitions, in situations where an anonymous integer type
   is required.  */
static void
init_base_type_table (struct dwarf_data *ddata)
{
    int i;
    for (i = 0; i < NUM_BASE_TYPES; ++i)
    {
        ddata->base_type_die_table[i] = NULL;
    }
    for (i = 0; i < NUM_BASE_TYPES; ++i)
    {
        if (strcmp (base_type_table[i].bt_name, "int") == 0)
        {
            ddata->base_type_die_table[i] = new_die (DW_TAG_base_type, ddata->comp_unit_die, ddata);
            add_AT_string (ddata->base_type_die_table[i], DW_AT_name, base_type_table[i].bt_name, ddata);
            add_AT_unsigned (ddata->base_type_die_table[i],
                             DW_AT_byte_size, base_type_table[i].bt_size / 8, ddata);
            add_AT_unsigned (ddata->base_type_die_table[i], DW_AT_encoding, base_type_table[i].bt_type, ddata);
            break;
        }
    }
}

/* Given a pointer to a tree node for some base type, return a pointer to
   a DIE that describes the given type.

   This routine must only be called for GCC type nodes that correspond to
   Dwarf base (fundamental) types.  */
static dw_die_ref
base_type_die (enum var_types type, struct dwarf_data *ddata)
{
    dw_die_ref base_type_result = NULL;
    const char *type_name = stIntrinsicTypeName (type);
    int type_index = 0;
    int i;

    for (i = 0; i < NUM_BASE_TYPES; ++i)
    {
        if (strcmp (type_name, base_type_table[i].bt_name) == 0)
        {
            type_index = i;
            break;
        }
    }

    if (type_index == 0)
    {
        base_type_result = NULL;
    }
    else
    {
        base_type_result = ddata->base_type_die_table[type_index];
        if (base_type_result == NULL)
        {
            base_type_result = new_die (DW_TAG_base_type, ddata->comp_unit_die, ddata);
            ddata->base_type_die_table[type_index] = base_type_result;
            add_AT_string (base_type_result, DW_AT_name, base_type_table[type_index].bt_name, ddata);
            add_AT_unsigned (base_type_result, DW_AT_byte_size, base_type_table[type_index].bt_size / 8, ddata);
            add_AT_unsigned (base_type_result, DW_AT_encoding, base_type_table[type_index].bt_type, ddata);
        }

    }

    return base_type_result;
}

/* Add a label identifier attribute value to a DIE.  */
void
add_AT_lbl_id (dw_die_ref die, enum dwarf_attribute attr_kind, const char *lbl_id, struct dwarf_data *ddata)
{
    dw_attr_ref attr = (dw_attr_ref) List_NewLast (ddata->mem, sizeof (dw_attr_node));
    if (attr != NULL)
    {
        attr->dw_attr_next = NULL;
        attr->dw_attr = attr_kind;
        attr->dw_attr_val.val_class = dw_val_class_lbl_id;
        attr->dw_attr_val.v.val_lbl_id = (char *) List_NewLast (ddata->mem, (unsigned int) strlen (lbl_id) + 1);
        strcpy (attr->dw_attr_val.v.val_lbl_id, lbl_id);
        add_dwarf_attr (die, attr);
    }
}

/* Add a section offset attribute value to a DIE.  */
static void
add_AT_section_offset (dw_die_ref die, enum dwarf_attribute attr_kind, const char *section, struct dwarf_data *ddata)
{
    dw_attr_ref attr = (dw_attr_ref) List_NewLast (ddata->mem, sizeof (dw_attr_node));
    if (attr != NULL)
    {
        attr->dw_attr_next = NULL;
        attr->dw_attr = attr_kind;
        attr->dw_attr_val.val_class = dw_val_class_section_offset;
        attr->dw_attr_val.v.val_section = (char *) List_NewLast (ddata->mem, (unsigned int) strlen (section) + 1);
        strcpy (attr->dw_attr_val.v.val_section, section);
        add_dwarf_attr (die, attr);
    }
}

/* Return the DIE associated with the given type specifier.  */
static dw_die_ref
lookup_type_die (TypDef type, struct dwarf_data *ddata)
{
    unsigned type_id;
    for (type_id = 0; type_id < ddata->type_die_table_in_use; type_id++)
    {
        if  (stMatchType2 (ddata->type_die_table[type_id]->type, type))
        {
            break;
        }
    }

    return (type_id < ddata->type_die_table_in_use)
        ? ddata->type_die_table[type_id] : NULL;
}

/* Return a pointer to a newly allocated location description.  Location
   descriptions are simple expression terms that can be strung
   together to form more complicated location (address) descriptions.  */
dw_loc_descr_ref
new_loc_descr (enum dwarf_location_atom op, unsigned long oprnd1, unsigned long oprnd2, struct dwarf_data *ddata)
{
    dw_loc_descr_ref descr =
    (dw_loc_descr_ref) List_NewLast (ddata->mem, sizeof (dw_loc_descr_node));
    if (descr != NULL)
    {
        descr->dw_loc_next = NULL;
        descr->dw_loc_opc = op;
        descr->dw_loc_oprnd1.val_class = dw_val_class_unsigned_const;
        descr->dw_loc_oprnd1.v.val_unsigned = oprnd1;
        descr->dw_loc_oprnd2.val_class = dw_val_class_unsigned_const;
        descr->dw_loc_oprnd2.v.val_unsigned = oprnd2;
    }
    return descr;
}

/* Add a location description term to a location description expression.  */
void
add_loc_descr (dw_loc_descr_ref *list_head, dw_loc_descr_ref descr)
{
    dw_loc_descr_ref *d;
    /* find the end of the chain.  */
    for (d = list_head; (*d) != NULL; d = &(*d)->dw_loc_next)
    {
        /* nothing */ ;
    }
    *d = descr;
}

/* Return a location descriptor that designates a machine register.  */
dw_loc_descr_ref
reg_loc_descriptor (unsigned reg, struct dwarf_data *ddata)
{
    dw_loc_descr_ref loc_result = NULL;
    if (reg >= 0 && reg <= 31)
    {
        loc_result = new_loc_descr (DW_OP_reg0 + reg, 0, 0, ddata);
    }
    else
    {
        loc_result = new_loc_descr (DW_OP_regx, reg, 0, ddata);
    }
    return loc_result;
}

/* Return a location descriptor that designates a base+offset location.  */
dw_loc_descr_ref
based_loc_descr (unsigned reg, unsigned fp_reg, int offset, struct dwarf_data *ddata)
{
    dw_loc_descr_ref loc_result;
    if (reg == fp_reg)
    {
        loc_result = new_loc_descr (DW_OP_fbreg,
                                    offset, 0, ddata);
    }
    else if (reg >= 0 && reg <= 31)
    {
        loc_result = new_loc_descr (DW_OP_breg0 + reg, offset, 0, ddata);
    }
    else
    {
        loc_result = new_loc_descr (DW_OP_bregx, reg, offset, ddata);
    }
    return loc_result;
}

/* 下面的例程将变量或参数(驻留在内存中)的RTL转换为等价的Dwarf表示，
   表示一种将该变量的地址获取到假想的“地址求值”栈顶部的机制。
   在创建内存位置描述符时，我们有效地将内存驻留对象的RTL转换为等价的
   Dwarf后缀表达式。该例程递归地处理RTL树，在运行过程中将其转换为Dwarf后缀代码。  */
dw_loc_descr_ref
mem_loc_descriptor (SymDef rtl, struct dwarf_data *ddata)
{
    dw_loc_descr_ref mem_loc_result = NULL;
    /* 注意，对于动态大小的数组，我们将在这里生成描述的位置将是实际在数组中编号
       最低的位置。这不一定与数组的第0个元素相同。  */
    mem_loc_result = new_loc_descr (DW_OP_addr, 0, 0, ddata);
    mem_loc_result->dw_loc_oprnd1.val_class = dw_val_class_addr;
    mem_loc_result->dw_loc_oprnd1.v.val_addr = (char *) List_NewLast (ddata->mem, (unsigned int) strlen (rtl->sdName) + 1);
    strcpy (mem_loc_result->dw_loc_oprnd1.v.val_addr, rtl->sdName);
    return mem_loc_result;
}

/* Equate a DIE to a given type specifier.  */
static void
equate_type_number_to_die (TypDef type, dw_die_ref type_die, struct dwarf_data *ddata)
{
    unsigned type_id = ddata->type_die_table_in_use;
    unsigned num_allocated;
    dw_die_ref *old_ptr;

    if  (lookup_type_die (type, ddata) == NULL)
    {
        if (type_id >= ddata->type_die_table_allocated)
        {
            num_allocated = (((type_id + 1)
                              + 4096 - 1)
                             / 4096)
              * 4096;
            old_ptr = ddata->type_die_table;
            ddata->type_die_table = (dw_die_ref *) List_NewLast (ddata->mem,
                                             sizeof (dw_die_ref) * num_allocated);
            memcpy (ddata->type_die_table, old_ptr,
                ddata->type_die_table_allocated * sizeof (dw_die_ref));
            memset (&ddata->type_die_table[ddata->type_die_table_allocated], 0,
                (num_allocated - ddata->type_die_table_allocated) * sizeof (dw_die_ref));
            ddata->type_die_table_allocated = num_allocated;
            List_Delete(old_ptr);
        }
        if (type_id >= ddata->type_die_table_in_use)
        {
            ddata->type_die_table_in_use = (type_id + 1);
        }
        ddata->type_die_table[type_id] = type_die;
        type_die->type = type;
    }

}

/* Given a tree node describing an array bound (either lower or upper) output
   a representation for that bound.  */
static void
add_bound_info (dw_die_ref subrange_die, enum dwarf_attribute bound_attr, int bound, struct dwarf_data *ddata)
{
    add_AT_unsigned (subrange_die, bound_attr, bound, ddata);
}

/* Note that the block of subscript information for an array type also
   includes information about the element type of type given array type.  */
static void
add_subscript_info (dw_die_ref type_die, TypDef type, struct dwarf_data *ddata)
{
    unsigned dimension_number;
    unsigned lower, upper;
    dw_die_ref subrange_die;

    /* GNU编译器将多维数组类型表示为一维数组类型的序列，其中的元素类型本身就是数组类型。
       在这里，我们将其压缩，以便每个多维数组类型在Dwarf调试信息中只有一个array_type DIE。
       Dwarf规范草案说，我们允许在C中进行这种压缩(因为在C中，数组或数组与多维数组
       之间没有区别)，但对于其他源语言(例如Ada)，我们可能不应该这样做。  */
    /* ??? 对于const枚举类型的多维数组，SGI dwarf读取器会失败。例如
       const enum machine_mode insn_operand_mode[2][10]。
       我们通过禁用此功能来解决此问题。参见gen_array_type_die。  */
    for (dimension_number = 0;
         type->tdTypeKind == TYP_ARRAY;
         type = type->tdArr.tdaElem, dimension_number++)
    {
        subrange_die = new_die (DW_TAG_subrange_type, type_die, ddata);
        
        /* We have an array type with specified bounds.  */
        lower = type->tdArr.tdaDims->ddLoTree;
        upper = type->tdArr.tdaDims->ddSize - 1;
        add_bound_info (subrange_die, DW_AT_lower_bound, lower, ddata);
        add_bound_info (subrange_die, DW_AT_upper_bound, upper, ddata);
    }
}

static void
gen_array_type_die (TypDef type, dw_die_ref context_die, struct dwarf_data *ddata);

/* Generate a type description DIE.  */
static void
gen_type_die (TypDef type, dw_die_ref context_die, struct dwarf_data *ddata)
{
    switch (type->tdTypeKind)
    {
    case TYP_UNDEF:
        break;

    case TYP_PTR:
        /* For these types, all that is required is that we output a DIE (or a
           set of DIEs) to represent the "basis" type.  */
        gen_type_die (stGetBaseType (type), context_die, ddata);
        break;

    case TYP_FNC:
        /* Force out return type (in case it wasn't forced out already).  */
        gen_type_die (type->tdFnc.tdfRett, context_die, ddata);
/*      gen_subroutine_type_die (type, context_die);*/
        break;

    case TYP_ARRAY:
        gen_array_type_die (type, context_die, ddata);
        break;

    case TYP_VOID:
    case TYP_INT:
    case TYP_DOUBLE:
    case TYP_FLOAT:
    case TYP_BOOL:
        /* No DIEs needed for fundamental types.  */
        break;

    default:
        abort ();
    }
}

/* Given a pointer to an arbitrary ..._TYPE tree node, return a debugging
   entry that chains various modifiers in front of the given type.  */
static dw_die_ref
modified_type_die (TypDef type, dw_die_ref context_die, struct dwarf_data *ddata)
{
    dw_die_ref mod_type_die = NULL;
    dw_die_ref sub_die = NULL;
    TypDef item_type;

    if (type->tdTypeKind == TYP_PTR)
    {
        mod_type_die = new_die (DW_TAG_pointer_type, context_die, ddata);
        add_AT_unsigned (mod_type_die, DW_AT_byte_size, type_size (type), ddata);
        add_AT_unsigned (mod_type_die, DW_AT_address_class, 0, ddata);
        item_type = stGetBaseType (type);
        sub_die = modified_type_die (item_type,
                                     context_die, ddata);
    }
    else if ((type->tdTypeKind <= TYP_lastIntrins))
    {
        mod_type_die = base_type_die (type->tdTypeKind, ddata);
    }
    else
    {
        /* We have to get the type_main_variant here (and pass that to the
           `lookup_type_die' routine) because the ..._TYPE node we have
           might simply be a *copy* of some original type node (where the
           copy was created to help us keep track of typedef names) and
           that copy might have a different TYPE_UID from the original
           ..._TYPE node.  (Note that when `equate_type_number_to_die' is
           labeling a given type DIE for future reference, it always only
           handles DIEs representing *main variants*, and it never even
           knows about non-main-variants.).  */
        mod_type_die = lookup_type_die ( (type), ddata);
    }
    if (sub_die != NULL)
    {
        add_AT_die_ref (mod_type_die, DW_AT_type, sub_die, ddata);
    }
    return mod_type_die;
}


/* Many forms of DIEs require a "type description" attribute.  This
   routine locates the proper "type descriptor" die for the type given
   by 'type', and adds an DW_AT_type attribute below the given die.  */
void
add_type_attribute (dw_die_ref object_die, TypDef type, dw_die_ref context_die, struct dwarf_data *ddata)
{
    dw_die_ref scope_die = NULL;
    dw_die_ref type_die  = NULL;

    /* Handle a special case.  For functions whose return type is void, we
       generate *no* type attribute.  (Note that no object may have type
       `void', so this only applies to function return types).  */
    if (type->tdTypeKind == TYP_VOID)
    {
        return;
    }

    scope_die = ddata->comp_unit_die;
    type_die = modified_type_die (type,
                                  scope_die, ddata);
    if (type_die != NULL)
    {
        add_AT_die_ref (object_die, DW_AT_type, type_die, ddata);
    }
}

/* Don't generate either pointer_type DIEs or reference_type DIEs.
   Use modified type DIE's instead.
   We keep this code here just in case these types of DIEs may be needed to
   represent certain things in other languages (e.g. Pascal) someday.  */
static void
gen_pointer_type_die (TypDef type, dw_die_ref context_die, struct dwarf_data *ddata)
{
    dw_die_ref ptr_die = lookup_type_die (type, ddata);
    if (ptr_die)
        return;
    ptr_die = new_die (DW_TAG_pointer_type, context_die, ddata);
    equate_type_number_to_die (type, ptr_die, ddata);
    add_type_attribute (ptr_die, stGetBaseType (type), context_die, ddata);
}

/* These routines generate the internnal representation of the DIE's for
   the compilation unit.  Debugging information is collected by walking
   the declaration trees passed in from dwarfout_file_scope_decl().  */
static void
gen_array_type_die (TypDef type, dw_die_ref context_die, struct dwarf_data *ddata)
{
    dw_die_ref scope_die = ddata->comp_unit_die;
    dw_die_ref array_die = lookup_type_die (type, ddata);
    TypDef element_type;

    if (array_die)
        return;

    if  (!type->tdArr.tdaDims->ddSize)
    {
        gen_pointer_type_die (type, context_die, ddata);
        return;
    }

    array_die  = new_die (DW_TAG_array_type, scope_die, ddata);
#if 0
    /* We default the array ordering.  SDB will probably do
       the right things even if DW_AT_ordering is not present.  It's not even
       an issue until we start to get into multidimensional arrays anyway.  If
       SDB is ever caught doing the Wrong Thing for multi-dimensional arrays,
       then we'll have to put the DW_AT_ordering attribute back in.  (But if
       and when we find out that we need to put these in, we will only do so
       for multidimensional arrays.  */
    add_AT_unsigned (array_die, DW_AT_ordering, DW_ORD_row_major);
#endif

    add_subscript_info (array_die, type, ddata);

    equate_type_number_to_die (type, array_die, ddata);

    /* Add representation of the type of the elements of this array type.  */
    element_type = type->tdArr.tdaElem;
    /* ??? The SGI dwarf reader fails for multidimensional arrays with a
       const enum type.  E.g. const enum machine_mode insn_operand_mode[2][10].
       We work around this by disabling this feature.  See also
       add_subscript_info.  */
    while (element_type->tdTypeKind == TYP_ARRAY)
    {
        element_type = element_type->tdArr.tdaElem;
    }
    gen_type_die (element_type, context_die, ddata);

    add_type_attribute (array_die, element_type, context_die, ddata);
}

/* Return a pointer to a newly allocated Call Frame Instruction.  */
dw_cfi_ref
new_cfi (struct dwarf_data *ddata)
{
    dw_cfi_ref cfi = (dw_cfi_ref) List_NewLast (ddata->mem, sizeof (dw_cfi_node));
    if (cfi != NULL)
    {
        cfi->dw_cfi_next = NULL;
        cfi->dw_cfi_oprnd1.dw_cfi_reg_num = 0;
        cfi->dw_cfi_oprnd2.dw_cfi_reg_num = 0;
    }
    return cfi;
}

/* Add a Call Frame Instruction to list of instructions.  */
void
add_cfi (dw_cfi_ref *list_head, dw_cfi_ref cfi)
{
    dw_cfi_ref *p;
    /* find the end of the chain.  */
    for (p = list_head; (*p) != NULL; p = &(*p)->dw_cfi_next)
    {
        /* nothing */ ;
    }
    *p = cfi;
}

/* 给定开始和结束范围以及表达式，返回一个新的位置列表。gensym告诉我们是否
   为此位置列表节点生成一个新的内部符号，这仅对列表的头部进行。 */ 
static dw_loc_list_ref
new_loc_list (dw_loc_descr_ref expr, const char *begin, const char *end, const char *section, unsigned gensym, struct dwarf_data *ddata)
{
    char buf[256];
    static int label_num;
    dw_loc_list_ref retlist
      = (dw_loc_list_ref) xcalloc (1, sizeof (dw_loc_list_node));
    retlist->begin = (char *) List_NewLast(ddata->mem, (unsigned int) strlen(begin) + 1);
    strcpy(retlist->begin,begin);
    retlist->end = (char *) List_NewLast(ddata->mem, (unsigned int) strlen(end) + 1);
    strcpy(retlist->end,end);
    retlist->expr = expr;
    retlist->section = (char *) List_NewLast(ddata->mem, (unsigned int) strlen(section) + 1);
    strcpy(retlist->section,section);
    sprintf (buf, "LLST%d", label_num++);
    if (gensym) 
    {
        retlist->ll_symbol = (char *) List_NewLast(ddata->mem, (unsigned int) strlen(buf) + 1);
        strcpy(retlist->ll_symbol,buf);
    }
    return retlist;
}

/* Add a location description expression to a location list */
static void
add_loc_descr_to_loc_list (dw_loc_list_ref *list_head, dw_loc_descr_ref descr, const char *begin, const char *end, const char *section, struct dwarf_data *ddata)
{
    dw_loc_list_ref *d;
    
    /* Find the end of the chain. */
    for (d = list_head; (*d) != NULL; d = &(*d)->dw_loc_next)
        ;
    /* Add a new location list node to the list */
    *d = new_loc_list (descr, begin, end, section, 0, ddata);
}

static void
add_AT_loc_list (dw_die_ref die, enum dwarf_attribute attr_kind, dw_loc_list_ref loc_list, struct dwarf_data *ddata)
{
    dw_attr_ref attr = (dw_attr_ref) List_NewLast (ddata->mem, sizeof (dw_attr_node));
    attr->dw_attr_next = NULL;
    attr->dw_attr = attr_kind;
    attr->dw_attr_val.val_class = dw_val_class_loc_list;
    attr->dw_attr_val.v.val_loc_list = loc_list;
    add_dwarf_attr (die, attr);
}

static dw_loc_list_ref
AT_loc_list (dw_attr_ref a)
{
    if (a && a->dw_attr_val.val_class == dw_val_class_loc_list)
        return a->dw_attr_val.v.val_loc_list;

    abort ();
    return NULL;
}

/* Output the compilation unit that appears at the beginning of the
   .debug_info section, and precedes the DIE descriptions.  */
static void
output_compilation_unit_header (FILE *asm_out_file)
{
    const char *ASM_COMMENT_START = "@";
    fprintf (asm_out_file, "\t%s\t%s",
             ".long", ".Ldebug_info_end-.Ldebug_info_start");
    fprintf (asm_out_file, "\t%s Length of Compilation Unit Info.",
             ASM_COMMENT_START);
    fputc ('\n', asm_out_file);
    fprintf((asm_out_file), ".Ldebug_info_start:\n");
    fprintf (asm_out_file, "\t%s\t0x%x", ".short", 2);
    fprintf (asm_out_file, "\t%s DWARF version number",
             ASM_COMMENT_START);
    fputc ('\n', asm_out_file);
    fprintf (asm_out_file, "\t%s\t%s", ".long",
           ".debug_abbrev");
    fprintf (asm_out_file, "\t%s Offset Into Abbrev. Section",
             ASM_COMMENT_START);
    fputc ('\n', asm_out_file);
    fprintf (asm_out_file, "\t%s\t0x%x", ".byte", 4);
    fprintf (asm_out_file, "\t%s Pointer Size (in bytes)",
             ASM_COMMENT_START);
    fputc ('\n', asm_out_file);
}

/* Output an unsigned LEB128 quantity.  */
static void
output_uleb128 (unsigned long value, struct dwarf_data *ddata)
{
    unsigned long save_value = value;
    fprintf (ddata->asm_out_file, "\t%s\t", ".byte");
    do
    {
        unsigned byte = (value & 0x7f);
        value >>= 7;
        if (value != 0)
        {
            /* More bytes to follow.  */
            byte |= 0x80;
        }
        fprintf (ddata->asm_out_file, "0x%x", byte);
        if (value != 0)
        {
            fprintf (ddata->asm_out_file, ",");
        }
    }
    while (value != 0);
    fprintf (ddata->asm_out_file, "\t%s ULEB128 0x%lx", "@", save_value);
}

/* Output an signed LEB128 quantity.  */
static void
output_sleb128 (long value, struct dwarf_data *ddata)
{
    int more;
    unsigned byte;
    fprintf (ddata->asm_out_file, "\t%s\t", ".byte");
    do
    {
        byte = (value & 0x7f);
        /* arithmetic shift */
        value >>= 7;
        more = !((((value == 0) && ((byte & 0x40) == 0))
                  || ((value == -1) && ((byte & 0x40) != 0))));
        if (more)
        {
            byte |= 0x80;
        }
        fprintf (ddata->asm_out_file, "0x%x", byte);
        if (more)
        {
            fprintf (ddata->asm_out_file, ",");
        }
    }
    while (more);
}

/* Convert a DWARF value form code into its string name.  */
static char *
dwarf_form_name (unsigned form)
{
    switch (form)
    {
    case DW_FORM_addr:
        return "DW_FORM_addr";
    case DW_FORM_block2:
        return "DW_FORM_block2";
    case DW_FORM_block4:
        return "DW_FORM_block4";
    case DW_FORM_data2:
        return "DW_FORM_data2";
    case DW_FORM_data4:
        return "DW_FORM_data4";
    case DW_FORM_data8:
        return "DW_FORM_data8";
    case DW_FORM_string:
        return "DW_FORM_string";
    case DW_FORM_block:
        return "DW_FORM_block";
    case DW_FORM_block1:
        return "DW_FORM_block1";
    case DW_FORM_data1:
        return "DW_FORM_data1";
    case DW_FORM_flag:
        return "DW_FORM_flag";
    case DW_FORM_sdata:
        return "DW_FORM_sdata";
    case DW_FORM_strp:
        return "DW_FORM_strp";
    case DW_FORM_udata:
        return "DW_FORM_udata";
    case DW_FORM_ref_addr:
        return "DW_FORM_ref_addr";
    case DW_FORM_ref1:
        return "DW_FORM_ref1";
    case DW_FORM_ref2:
        return "DW_FORM_ref2";
    case DW_FORM_ref4:
        return "DW_FORM_ref4";
    case DW_FORM_ref8:
        return "DW_FORM_ref8";
    case DW_FORM_ref_udata:
        return "DW_FORM_ref_udata";
    case DW_FORM_indirect:
        return "DW_FORM_indirect";
    default:
        return "DW_FORM_<unknown>";
    }
}

/* Output the encoding of an attribute value.  */
static void
output_value_format (dw_val_ref v, struct dwarf_data *ddata)
{
    enum dwarf_form form;
    switch (v->val_class)
    {
    case dw_val_class_addr:
        form = DW_FORM_addr;
        break;
    case dw_val_class_loc:
        form = DW_FORM_block2;
        break;
    case dw_val_class_const:
        form = DW_FORM_data4;
        break;
    case dw_val_class_unsigned_const:
        form = DW_FORM_data4;
        break;
    case dw_val_class_double_const:
        form = DW_FORM_data8;
        break;
    case dw_val_class_flag:
        form = DW_FORM_flag;
        break;
    case dw_val_class_die_ref:
        form = DW_FORM_ref4;
        break;
    case dw_val_class_fde_ref:
        form = DW_FORM_data4;
        break;
    case dw_val_class_lbl_id:
        form = DW_FORM_addr;
        break;
    case dw_val_class_section_offset:
        form = DW_FORM_data4;
        break;
    case dw_val_class_str:
        form = DW_FORM_string;
        break;
    default:
        abort ();
    }
    output_uleb128 (form, ddata);
        fprintf (ddata->asm_out_file, "\t%s %s",
                 "@", dwarf_form_name (form));
    fputc ('\n', ddata->asm_out_file);
}

/* Convert a DIE tag into its string name.  */
static char *
dwarf_tag_name (unsigned tag)
{
    switch (tag)
    {
    case DW_TAG_padding:
        return "DW_TAG_padding";
    case DW_TAG_array_type:
        return "DW_TAG_array_type";
    case DW_TAG_class_type:
        return "DW_TAG_class_type";
    case DW_TAG_entry_point:
        return "DW_TAG_entry_point";
    case DW_TAG_enumeration_type:
        return "DW_TAG_enumeration_type";
    case DW_TAG_formal_parameter:
        return "DW_TAG_formal_parameter";
    case DW_TAG_imported_declaration:
        return "DW_TAG_imported_declaration";
    case DW_TAG_label:
        return "DW_TAG_label";
    case DW_TAG_lexical_block:
        return "DW_TAG_lexical_block";
    case DW_TAG_member:
        return "DW_TAG_member";
    case DW_TAG_pointer_type:
        return "DW_TAG_pointer_type";
    case DW_TAG_reference_type:
        return "DW_TAG_reference_type";
    case DW_TAG_compile_unit:
        return "DW_TAG_compile_unit";
    case DW_TAG_string_type:
        return "DW_TAG_string_type";
    case DW_TAG_structure_type:
        return "DW_TAG_structure_type";
    case DW_TAG_subroutine_type:
        return "DW_TAG_subroutine_type";
    case DW_TAG_typedef:
        return "DW_TAG_typedef";
    case DW_TAG_union_type:
        return "DW_TAG_union_type";
    case DW_TAG_unspecified_parameters:
        return "DW_TAG_unspecified_parameters";
    case DW_TAG_variant:
        return "DW_TAG_variant";
    case DW_TAG_common_block:
        return "DW_TAG_common_block";
    case DW_TAG_common_inclusion:
        return "DW_TAG_common_inclusion";
    case DW_TAG_inheritance:
        return "DW_TAG_inheritance";
    case DW_TAG_inlined_subroutine:
        return "DW_TAG_inlined_subroutine";
    case DW_TAG_module:
        return "DW_TAG_module";
    case DW_TAG_ptr_to_member_type:
        return "DW_TAG_ptr_to_member_type";
    case DW_TAG_set_type:
        return "DW_TAG_set_type";
    case DW_TAG_subrange_type:
        return "DW_TAG_subrange_type";
    case DW_TAG_with_stmt:
        return "DW_TAG_with_stmt";
    case DW_TAG_access_declaration:
        return "DW_TAG_access_declaration";
    case DW_TAG_base_type:
        return "DW_TAG_base_type";
    case DW_TAG_catch_block:
        return "DW_TAG_catch_block";
    case DW_TAG_const_type:
        return "DW_TAG_const_type";
    case DW_TAG_constant:
        return "DW_TAG_constant";
    case DW_TAG_enumerator:
        return "DW_TAG_enumerator";
    case DW_TAG_file_type:
        return "DW_TAG_file_type";
    case DW_TAG_friend:
        return "DW_TAG_friend";
    case DW_TAG_namelist:
        return "DW_TAG_namelist";
    case DW_TAG_namelist_item:
        return "DW_TAG_namelist_item";
    case DW_TAG_packed_type:
        return "DW_TAG_packed_type";
    case DW_TAG_subprogram:
        return "DW_TAG_subprogram";
    case DW_TAG_template_type_param:
        return "DW_TAG_template_type_param";
    case DW_TAG_template_value_param:
        return "DW_TAG_template_value_param";
    case DW_TAG_thrown_type:
        return "DW_TAG_thrown_type";
    case DW_TAG_try_block:
        return "DW_TAG_try_block";
    case DW_TAG_variant_part:
        return "DW_TAG_variant_part";
    case DW_TAG_variable:
        return "DW_TAG_variable";
    case DW_TAG_volatile_type:
        return "DW_TAG_volatile_type";
    default:
        return "DW_TAG_<unknown>";
    }
}

/* Convert a DWARF attribute code into its string name.  */
static char *
dwarf_attr_name (unsigned attr)
{
    switch (attr)
    {
    case DW_AT_sibling:
        return "DW_AT_sibling";
    case DW_AT_location:
        return "DW_AT_location";
    case DW_AT_name:
        return "DW_AT_name";
    case DW_AT_ordering:
        return "DW_AT_ordering";
    case DW_AT_subscr_data:
        return "DW_AT_subscr_data";
    case DW_AT_byte_size:
        return "DW_AT_byte_size";
    case DW_AT_bit_offset:
        return "DW_AT_bit_offset";
    case DW_AT_bit_size:
        return "DW_AT_bit_size";
    case DW_AT_element_list:
        return "DW_AT_element_list";
    case DW_AT_stmt_list:
        return "DW_AT_stmt_list";
    case DW_AT_low_pc:
        return "DW_AT_low_pc";
    case DW_AT_high_pc:
        return "DW_AT_high_pc";
    case DW_AT_language:
        return "DW_AT_language";
    case DW_AT_member:
        return "DW_AT_member";
    case DW_AT_discr:
        return "DW_AT_discr";
    case DW_AT_discr_value:
        return "DW_AT_discr_value";
    case DW_AT_visibility:
        return "DW_AT_visibility";
    case DW_AT_import:
        return "DW_AT_import";
    case DW_AT_string_length:
        return "DW_AT_string_length";
    case DW_AT_common_reference:
        return "DW_AT_common_reference";
    case DW_AT_comp_dir:
        return "DW_AT_comp_dir";
    case DW_AT_const_value:
        return "DW_AT_const_value";
    case DW_AT_containing_type:
        return "DW_AT_containing_type";
    case DW_AT_default_value:
        return "DW_AT_default_value";
    case DW_AT_inline:
        return "DW_AT_inline";
    case DW_AT_is_optional:
        return "DW_AT_is_optional";
    case DW_AT_lower_bound:
        return "DW_AT_lower_bound";
    case DW_AT_producer:
        return "DW_AT_producer";
    case DW_AT_prototyped:
        return "DW_AT_prototyped";
    case DW_AT_return_addr:
        return "DW_AT_return_addr";
    case DW_AT_start_scope:
        return "DW_AT_start_scope";
    case DW_AT_stride_size:
        return "DW_AT_stride_size";
    case DW_AT_upper_bound:
        return "DW_AT_upper_bound";
    case DW_AT_abstract_origin:
        return "DW_AT_abstract_origin";
    case DW_AT_accessibility:
        return "DW_AT_accessibility";
    case DW_AT_address_class:
        return "DW_AT_address_class";
    case DW_AT_artificial:
        return "DW_AT_artificial";
    case DW_AT_base_types:
        return "DW_AT_base_types";
    case DW_AT_calling_convention:
        return "DW_AT_calling_convention";
    case DW_AT_count:
        return "DW_AT_count";
    case DW_AT_data_member_location:
        return "DW_AT_data_member_location";
    case DW_AT_decl_column:
        return "DW_AT_decl_column";
    case DW_AT_decl_file:
        return "DW_AT_decl_file";
    case DW_AT_decl_line:
        return "DW_AT_decl_line";
    case DW_AT_declaration:
        return "DW_AT_declaration";
    case DW_AT_discr_list:
        return "DW_AT_discr_list";
    case DW_AT_encoding:
        return "DW_AT_encoding";
    case DW_AT_external:
        return "DW_AT_external";
    case DW_AT_frame_base:
        return "DW_AT_frame_base";
    case DW_AT_friend:
        return "DW_AT_friend";
    case DW_AT_identifier_case:
        return "DW_AT_identifier_case";
    case DW_AT_macro_info:
        return "DW_AT_macro_info";
    case DW_AT_namelist_items:
        return "DW_AT_namelist_items";
    case DW_AT_priority:
        return "DW_AT_priority";
    case DW_AT_segment:
        return "DW_AT_segment";
    case DW_AT_specification:
        return "DW_AT_specification";
    case DW_AT_static_link:
        return "DW_AT_static_link";
    case DW_AT_type:
        return "DW_AT_type";
    case DW_AT_use_location:
        return "DW_AT_use_location";
    case DW_AT_variable_parameter:
        return "DW_AT_variable_parameter";
    case DW_AT_virtuality:
        return "DW_AT_virtuality";
    case DW_AT_vtable_elem_location:
        return "DW_AT_vtable_elem_location";
    case DW_AT_sf_names:
        return "DW_AT_sf_names";
    case DW_AT_src_info:
        return "DW_AT_src_info";
    case DW_AT_mac_info:
        return "DW_AT_mac_info";
    case DW_AT_src_coords:
        return "DW_AT_src_coords";
    case DW_AT_body_begin:
        return "DW_AT_body_begin";
    case DW_AT_body_end:
        return "DW_AT_body_end";
    default:
        return "DW_AT_<unknown>";
    }
}
/* Output the .debug_abbrev section which defines the DIE abbreviation
   table.  */
static void
output_abbrev_section (struct dwarf_data *ddata)
{
    const char *ASM_COMMENT_START = "@";
    unsigned long abbrev_id;
    dw_attr_ref a_attr;
    for (abbrev_id = 1; abbrev_id < ddata->abbrev_die_table_in_use; ++abbrev_id)
    {
        register dw_die_ref abbrev = ddata->abbrev_die_table[abbrev_id];
        output_uleb128 (abbrev_id, ddata);
        fprintf (ddata->asm_out_file, "\t%s abbrev code = %lu",
                 ASM_COMMENT_START, abbrev_id);
        fputc ('\n', ddata->asm_out_file);
        output_uleb128 (abbrev->die_tag, ddata);
        fprintf (ddata->asm_out_file, "\t%s TAG: %s",
                 ASM_COMMENT_START, dwarf_tag_name (abbrev->die_tag));
        fputc ('\n', ddata->asm_out_file);
        fprintf (ddata->asm_out_file, "\t%s\t0x%x", ".byte",
                 (abbrev->die_child != NULL)
                 ? DW_children_yes : DW_children_no);
        fprintf (ddata->asm_out_file, "\t%s %s",
                 ASM_COMMENT_START,
                 (abbrev->die_child != NULL)
                 ? "DW_children_yes" : "DW_children_no");
        fputc ('\n', ddata->asm_out_file);
        for (a_attr = abbrev->die_attr; a_attr != NULL;
             a_attr = a_attr->dw_attr_next)
        {
            output_uleb128 (a_attr->dw_attr, ddata);
            fprintf (ddata->asm_out_file, "\t%s %s",
                     ASM_COMMENT_START,
                     dwarf_attr_name (a_attr->dw_attr));
            fputc ('\n', ddata->asm_out_file);
            output_value_format (&a_attr->dw_attr_val, ddata);
        }
        fprintf (ddata->asm_out_file, "\t%s\t0,0\n", ".byte");
    }
    fprintf (ddata->asm_out_file, "\t%s\t0\n", ".byte");
}

/* Output location description stack opcode's operands (if any).  */
static void
output_loc_operands (dw_loc_descr_ref loc, struct dwarf_data *ddata)
{
    dw_val_ref val1 = &loc->dw_loc_oprnd1;
    dw_val_ref val2 = &loc->dw_loc_oprnd2;
    switch (loc->dw_loc_opc)
    {
    case DW_OP_addr:
        fprintf((ddata->asm_out_file), "\t%s\t%s", ".4byte", (val1->v.val_addr));
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_const1u:
    case DW_OP_const1s:
        fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", val1->v.val_flag);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_const2u:
    case DW_OP_const2s:
        fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".2byte", (unsigned)val1->v.val_int);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_const4u:
    case DW_OP_const4s:
        fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".4byte", (unsigned)val1->v.val_int);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_const8u:
    case DW_OP_const8s:
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_constu:
        output_uleb128 (val1->v.val_unsigned, ddata);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_consts:
        output_sleb128 (val1->v.val_unsigned, ddata);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_pick:
        fprintf((ddata->asm_out_file), "\t%s\t0x%lx", ".byte", val1->v.val_int);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_plus_uconst:
        output_uleb128 (val1->v.val_unsigned, ddata);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_skip:
    case DW_OP_bra:
        fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".2byte", (unsigned)val1->v.val_int);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_breg0:
    case DW_OP_breg1:
    case DW_OP_breg2:
    case DW_OP_breg3:
    case DW_OP_breg4:
    case DW_OP_breg5:
    case DW_OP_breg6:
    case DW_OP_breg7:
    case DW_OP_breg8:
    case DW_OP_breg9:
    case DW_OP_breg10:
    case DW_OP_breg11:
    case DW_OP_breg12:
    case DW_OP_breg13:
    case DW_OP_breg14:
    case DW_OP_breg15:
    case DW_OP_breg16:
    case DW_OP_breg17:
    case DW_OP_breg18:
    case DW_OP_breg19:
    case DW_OP_breg20:
    case DW_OP_breg21:
    case DW_OP_breg22:
    case DW_OP_breg23:
    case DW_OP_breg24:
    case DW_OP_breg25:
    case DW_OP_breg26:
    case DW_OP_breg27:
    case DW_OP_breg28:
    case DW_OP_breg29:
    case DW_OP_breg30:
    case DW_OP_breg31:
        output_sleb128 (val1->v.val_int, ddata);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_regx:
        output_uleb128 (val1->v.val_unsigned, ddata);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_fbreg:
        output_sleb128 (val1->v.val_unsigned, ddata);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_bregx:
        output_uleb128 (val1->v.val_unsigned, ddata);
        fputc ('\n', ddata->asm_out_file);
        output_sleb128 (val2->v.val_unsigned, ddata);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_piece:
        output_uleb128 (val1->v.val_unsigned, ddata);
        fputc ('\n', ddata->asm_out_file);
        break;
    case DW_OP_deref_size:
    case DW_OP_xderef_size:
        fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", val1->v.val_flag);
        fputc ('\n', ddata->asm_out_file);
        break;
    default:
        break;
    }
}

/* Convert a DWARF stack opcode into its string name.  */
static char *
dwarf_stack_op_name (unsigned op)
{
    switch (op)
    {
    case DW_OP_addr:
        return "DW_OP_addr";
    case DW_OP_deref:
        return "DW_OP_deref";
    case DW_OP_const1u:
        return "DW_OP_const1u";
    case DW_OP_const1s:
        return "DW_OP_const1s";
    case DW_OP_const2u:
        return "DW_OP_const2u";
    case DW_OP_const2s:
        return "DW_OP_const2s";
    case DW_OP_const4u:
        return "DW_OP_const4u";
    case DW_OP_const4s:
        return "DW_OP_const4s";
    case DW_OP_const8u:
        return "DW_OP_const8u";
    case DW_OP_const8s:
        return "DW_OP_const8s";
    case DW_OP_constu:
        return "DW_OP_constu";
    case DW_OP_consts:
        return "DW_OP_consts";
    case DW_OP_dup:
        return "DW_OP_dup";
    case DW_OP_drop:
        return "DW_OP_drop";
    case DW_OP_over:
        return "DW_OP_over";
    case DW_OP_pick:
        return "DW_OP_pick";
    case DW_OP_swap:
        return "DW_OP_swap";
    case DW_OP_rot:
        return "DW_OP_rot";
    case DW_OP_xderef:
        return "DW_OP_xderef";
    case DW_OP_abs:
        return "DW_OP_abs";
    case DW_OP_and:
        return "DW_OP_and";
    case DW_OP_div:
        return "DW_OP_div";
    case DW_OP_minus:
        return "DW_OP_minus";
    case DW_OP_mod:
        return "DW_OP_mod";
    case DW_OP_mul:
        return "DW_OP_mul";
    case DW_OP_neg:
        return "DW_OP_neg";
    case DW_OP_not:
        return "DW_OP_not";
    case DW_OP_or:
        return "DW_OP_or";
    case DW_OP_plus:
        return "DW_OP_plus";
    case DW_OP_plus_uconst:
        return "DW_OP_plus_uconst";
    case DW_OP_shl:
        return "DW_OP_shl";
    case DW_OP_shr:
        return "DW_OP_shr";
    case DW_OP_shra:
        return "DW_OP_shra";
    case DW_OP_xor:
        return "DW_OP_xor";
    case DW_OP_bra:
        return "DW_OP_bra";
    case DW_OP_eq:
        return "DW_OP_eq";
    case DW_OP_ge:
        return "DW_OP_ge";
    case DW_OP_gt:
        return "DW_OP_gt";
    case DW_OP_le:
        return "DW_OP_le";
    case DW_OP_lt:
        return "DW_OP_lt";
    case DW_OP_ne:
        return "DW_OP_ne";
    case DW_OP_skip:
        return "DW_OP_skip";
    case DW_OP_lit0:
        return "DW_OP_lit0";
    case DW_OP_lit1:
        return "DW_OP_lit1";
    case DW_OP_lit2:
        return "DW_OP_lit2";
    case DW_OP_lit3:
        return "DW_OP_lit3";
    case DW_OP_lit4:
        return "DW_OP_lit4";
    case DW_OP_lit5:
        return "DW_OP_lit5";
    case DW_OP_lit6:
        return "DW_OP_lit6";
    case DW_OP_lit7:
        return "DW_OP_lit7";
    case DW_OP_lit8:
        return "DW_OP_lit8";
    case DW_OP_lit9:
        return "DW_OP_lit9";
    case DW_OP_lit10:
        return "DW_OP_lit10";
    case DW_OP_lit11:
        return "DW_OP_lit11";
    case DW_OP_lit12:
        return "DW_OP_lit12";
    case DW_OP_lit13:
        return "DW_OP_lit13";
    case DW_OP_lit14:
        return "DW_OP_lit14";
    case DW_OP_lit15:
        return "DW_OP_lit15";
    case DW_OP_lit16:
        return "DW_OP_lit16";
    case DW_OP_lit17:
        return "DW_OP_lit17";
    case DW_OP_lit18:
        return "DW_OP_lit18";
    case DW_OP_lit19:
        return "DW_OP_lit19";
    case DW_OP_lit20:
        return "DW_OP_lit20";
    case DW_OP_lit21:
        return "DW_OP_lit21";
    case DW_OP_lit22:
        return "DW_OP_lit22";
    case DW_OP_lit23:
        return "DW_OP_lit23";
    case DW_OP_lit24:
        return "DW_OP_lit24";
    case DW_OP_lit25:
        return "DW_OP_lit25";
    case DW_OP_lit26:
        return "DW_OP_lit26";
    case DW_OP_lit27:
        return "DW_OP_lit27";
    case DW_OP_lit28:
        return "DW_OP_lit28";
    case DW_OP_lit29:
        return "DW_OP_lit29";
    case DW_OP_lit30:
        return "DW_OP_lit30";
    case DW_OP_lit31:
        return "DW_OP_lit31";
    case DW_OP_reg0:
        return "DW_OP_reg0";
    case DW_OP_reg1:
        return "DW_OP_reg1";
    case DW_OP_reg2:
        return "DW_OP_reg2";
    case DW_OP_reg3:
        return "DW_OP_reg3";
    case DW_OP_reg4:
        return "DW_OP_reg4";
    case DW_OP_reg5:
        return "DW_OP_reg5";
    case DW_OP_reg6:
        return "DW_OP_reg6";
    case DW_OP_reg7:
        return "DW_OP_reg7";
    case DW_OP_reg8:
        return "DW_OP_reg8";
    case DW_OP_reg9:
        return "DW_OP_reg9";
    case DW_OP_reg10:
        return "DW_OP_reg10";
    case DW_OP_reg11:
        return "DW_OP_reg11";
    case DW_OP_reg12:
        return "DW_OP_reg12";
    case DW_OP_reg13:
        return "DW_OP_reg13";
    case DW_OP_reg14:
        return "DW_OP_reg14";
    case DW_OP_reg15:
        return "DW_OP_reg15";
    case DW_OP_reg16:
        return "DW_OP_reg16";
    case DW_OP_reg17:
        return "DW_OP_reg17";
    case DW_OP_reg18:
        return "DW_OP_reg18";
    case DW_OP_reg19:
        return "DW_OP_reg19";
    case DW_OP_reg20:
        return "DW_OP_reg20";
    case DW_OP_reg21:
        return "DW_OP_reg21";
    case DW_OP_reg22:
        return "DW_OP_reg22";
    case DW_OP_reg23:
        return "DW_OP_reg23";
    case DW_OP_reg24:
        return "DW_OP_reg24";
    case DW_OP_reg25:
        return "DW_OP_reg25";
    case DW_OP_reg26:
        return "DW_OP_reg26";
    case DW_OP_reg27:
        return "DW_OP_reg27";
    case DW_OP_reg28:
        return "DW_OP_reg28";
    case DW_OP_reg29:
        return "DW_OP_reg29";
    case DW_OP_reg30:
        return "DW_OP_reg30";
    case DW_OP_reg31:
        return "DW_OP_reg31";
    case DW_OP_breg0:
        return "DW_OP_breg0";
    case DW_OP_breg1:
        return "DW_OP_breg1";
    case DW_OP_breg2:
        return "DW_OP_breg2";
    case DW_OP_breg3:
        return "DW_OP_breg3";
    case DW_OP_breg4:
        return "DW_OP_breg4";
    case DW_OP_breg5:
        return "DW_OP_breg5";
    case DW_OP_breg6:
        return "DW_OP_breg6";
    case DW_OP_breg7:
        return "DW_OP_breg7";
    case DW_OP_breg8:
        return "DW_OP_breg8";
    case DW_OP_breg9:
        return "DW_OP_breg9";
    case DW_OP_breg10:
        return "DW_OP_breg10";
    case DW_OP_breg11:
        return "DW_OP_breg11";
    case DW_OP_breg12:
        return "DW_OP_breg12";
    case DW_OP_breg13:
        return "DW_OP_breg13";
    case DW_OP_breg14:
        return "DW_OP_breg14";
    case DW_OP_breg15:
        return "DW_OP_breg15";
    case DW_OP_breg16:
        return "DW_OP_breg16";
    case DW_OP_breg17:
        return "DW_OP_breg17";
    case DW_OP_breg18:
        return "DW_OP_breg18";
    case DW_OP_breg19:
        return "DW_OP_breg19";
    case DW_OP_breg20:
        return "DW_OP_breg20";
    case DW_OP_breg21:
        return "DW_OP_breg21";
    case DW_OP_breg22:
        return "DW_OP_breg22";
    case DW_OP_breg23:
        return "DW_OP_breg23";
    case DW_OP_breg24:
        return "DW_OP_breg24";
    case DW_OP_breg25:
        return "DW_OP_breg25";
    case DW_OP_breg26:
        return "DW_OP_breg26";
    case DW_OP_breg27:
        return "DW_OP_breg27";
    case DW_OP_breg28:
        return "DW_OP_breg28";
    case DW_OP_breg29:
        return "DW_OP_breg29";
    case DW_OP_breg30:
        return "DW_OP_breg30";
    case DW_OP_breg31:
        return "DW_OP_breg31";
    case DW_OP_regx:
        return "DW_OP_regx";
    case DW_OP_fbreg:
        return "DW_OP_fbreg";
    case DW_OP_bregx:
        return "DW_OP_bregx";
    case DW_OP_piece:
        return "DW_OP_piece";
    case DW_OP_deref_size:
        return "DW_OP_deref_size";
    case DW_OP_xderef_size:
        return "DW_OP_xderef_size";
    case DW_OP_nop:
        return "DW_OP_nop";
    default:
        return "OP_<unknown>";
    }
}

/* Output a sequence of location operations.  */
static void
output_loc_sequence (dw_loc_descr_ref loc, struct dwarf_data *ddata)
{
    const char *ASM_COMMENT_START = "@";
    for (; loc != NULL; loc = loc->dw_loc_next)
    {
        /* Output the opcode.  */
        fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".byte", loc->dw_loc_opc);
        fprintf (ddata->asm_out_file, "\t%s %s",
                 ASM_COMMENT_START, dwarf_stack_op_name (loc->dw_loc_opc));
        fputc ('\n', ddata->asm_out_file);

        /* Output the operand(s) (if any).  */
        output_loc_operands (loc, ddata);
    }
}

/* Output the location list given to us */
static void
output_loc_list (dw_loc_list_ref list_head, struct dwarf_data *ddata)
{
    const char *ASM_COMMENT_START = "@";
    dw_loc_list_ref curr = list_head;
    fprintf((ddata->asm_out_file), "%s:\n", list_head->ll_symbol);
    if (strcmp (curr->section, ".text") == 0)
    {
        fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".4byte", 0xffffffff);
        fprintf (ddata->asm_out_file, "\t%s Location list base address specifier fake entry",
                 ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);

        fprintf(ddata->asm_out_file, "\t%s\t%s", ".4byte", curr->section);
        fprintf (ddata->asm_out_file, "\t%s Location list base address specifier base",
                 ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);
    }
    for (curr = list_head; curr != NULL; curr=curr->dw_loc_next)
    {
#if 0
        int size;
#endif
        fprintf(ddata->asm_out_file, "\t%s\t%s-%s", ".4byte", curr->begin, curr->section);
        fprintf (ddata->asm_out_file, "\t%s Location list begin address (%s)",
                 ASM_COMMENT_START, list_head->ll_symbol);
        fputc ('\n', ddata->asm_out_file);

        fprintf(ddata->asm_out_file, "\t%s\t%s-%s", ".4byte", curr->end, curr->section);
        fprintf (ddata->asm_out_file, "\t%s Location list end address (%s)",
                 ASM_COMMENT_START, list_head->ll_symbol);
        fputc ('\n', ddata->asm_out_file);
#if 0
        size = size_of_locs (curr->expr);
      
        /* Output the block length for this list of location operations.  */
        fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".4byte", size);
        fprintf (ddata->asm_out_file, "\t%s Location expression size",
                 ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);
#endif

        output_loc_sequence (curr->expr, ddata);
    }
    fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".4byte", 0);
    fprintf (ddata->asm_out_file, "\t%s Location list terminator begin (%s)",
             ASM_COMMENT_START, list_head->ll_symbol);
    fputc ('\n', ddata->asm_out_file);

    fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".4byte", 0);
    fprintf (ddata->asm_out_file, "\t%s Location list terminator begin (%s)",
             ASM_COMMENT_START, list_head->ll_symbol);
    fputc ('\n', ddata->asm_out_file);
}

/* Output all location lists for the DIE and it's children */
static void
output_location_lists (dw_die_ref die, struct dwarf_data *ddata)
{
    dw_die_ref c;
    dw_attr_ref d_attr;
    for (d_attr = die->die_attr; d_attr; d_attr = d_attr->dw_attr_next)
    {
        if (d_attr->dw_attr_val.val_class == dw_val_class_loc_list)
        {
            output_loc_list (AT_loc_list (d_attr), ddata);
        }
    }
    for (c = die->die_child; c != NULL; c = c->die_sib)
        output_location_lists (c, ddata);

}

static void
output_line_info (struct dwarf_data *ddata)
{
    char line_label[30];
    char prev_line_label[30];
    unsigned opc;
    unsigned n_op_args;
    dw_line_info_ref line_info;
    unsigned long lt_index;
    unsigned long current_line;
    long line_offset;
    long line_delta;
    unsigned long current_file;

    const char *ASM_COMMENT_START = "@";

    fprintf((ddata->asm_out_file), "\t%s\t%s-%s", ".4byte", ".LTEND", ".LTSTART");
    fprintf (ddata->asm_out_file, "\t%s Length of Source Line Info.",
       ASM_COMMENT_START);

    fputc ('\n', ddata->asm_out_file);
    fprintf((ddata->asm_out_file), "%s:\n", ".LTSTART");

    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".2byte", (unsigned)2);
    fprintf (ddata->asm_out_file, "\t%s DWARF Version",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);

    fprintf((ddata->asm_out_file), "\t%s\t%s-%s", ".4byte", ".LELTP", ".LASLTP");
    fprintf (ddata->asm_out_file, "\t%s Prolog Length",
       ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);

    fprintf((ddata->asm_out_file), "%s:\n", ".LASLTP");

    /* FIXME: architecture-dependent.  */
    fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".byte", 4);
    fprintf (ddata->asm_out_file, "\t%s Minimum Instruction Length",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".byte", 1);
    fprintf (ddata->asm_out_file, "\t%s Default is_stmt_start flag",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    fprintf (ddata->asm_out_file, "\t%s\t%d", ".byte", - 10);
    fprintf (ddata->asm_out_file, "\t%s Line Base Value (Special Opcodes)",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    fprintf (ddata->asm_out_file, "\t%s\t%u", ".byte", (254 - 10 + 1));
    fprintf (ddata->asm_out_file, "\t%s Line Range Value (Special Opcodes)",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    fprintf (ddata->asm_out_file, "\t%s\t%u", ".byte", 10);
    fprintf (ddata->asm_out_file, "\t%s Special Opcode Base",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    for (opc = 1; opc < 10; ++opc)
    {
        switch (opc)
        {
        case DW_LNS_advance_pc:
        case DW_LNS_advance_line:
        case DW_LNS_set_file:
        case DW_LNS_set_column:
        case DW_LNS_fixed_advance_pc:
            n_op_args = 1;
            break;
        default:
            n_op_args = 0;
            break;
        }
        fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".byte", n_op_args);
        fprintf (ddata->asm_out_file, "\t%s opcode: 0x%x has %d args",
                 ASM_COMMENT_START, opc, n_op_args);
        fputc ('\n', ddata->asm_out_file);
    }
    fprintf (ddata->asm_out_file, "%s Include Directory Table\n",
             ASM_COMMENT_START);
    /* Include directory table is empty, at present */
    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", 0);
    fputc ('\n', ddata->asm_out_file);
    fprintf (ddata->asm_out_file, "%s File Name Table\n", ASM_COMMENT_START);
    {
        do {
            int slen = (int) strlen(ddata->main_input_filename);
            char *p = ddata->main_input_filename;
            int i;
            fprintf(ddata->asm_out_file, "\t.ascii \"");
            for (i = 0; i < slen; i++) {
                int c = p[i];
                if (c == '\"' || c == '\\')
                    putc('\\', ddata->asm_out_file);
                if (c >= ' ' && c < 0177)
                    putc(c, ddata->asm_out_file);
                else {
                    fprintf(ddata->asm_out_file, "\\%o", c);
                }
            }
            fprintf(ddata->asm_out_file, "\\0\"");
        } while (0);
        fprintf (ddata->asm_out_file, "%s File Entry: 0x%x",
                 ASM_COMMENT_START, 1);
        fputc ('\n', ddata->asm_out_file);
        /* Include directory index */
        output_uleb128 (0, ddata);
        fputc ('\n', ddata->asm_out_file);
        /* Modification time */
        output_uleb128 (0, ddata);
        fputc ('\n', ddata->asm_out_file);
        /* File length in bytes */
        output_uleb128 (0, ddata);
        fputc ('\n', ddata->asm_out_file);
    }
    /* Terminate the file name table */
    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", 0);
    fputc ('\n', ddata->asm_out_file);

    /* Set the address register to the first location in the text section */
    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", 0);
        fprintf (ddata->asm_out_file, "\t%s DW_LNE_set_address", ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    output_uleb128 (1 + 4, ddata);
/*  output_uleb128 (1 + PTR_SIZE); */
    fputc ('\n', ddata->asm_out_file);
    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", DW_LNE_set_address);
    fputc ('\n', ddata->asm_out_file);
    fprintf((ddata->asm_out_file), "\t%s\t%s", ".4byte", ".L_text_b");
    fputc ('\n', ddata->asm_out_file);

    fprintf((ddata->asm_out_file), "%s:\n", ".LELTP");

    /* Generate the line number to PC correspondence table, encoded as
       a series of state machine operations.  */
    current_file = 1;
    current_line = 1;
    strcpy (prev_line_label, ".L_text_b");
    for (lt_index = 1; lt_index < ddata->line_info_table_in_use; ++lt_index)
    {
        line_info = &ddata->line_info_table[lt_index];

        sprintf (line_label, ".L_LC%lu", lt_index);
        if (0)
        {
            /* This can handle deltas up to 0xffff.  This takes 3 bytes.  */
            fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", DW_LNS_fixed_advance_pc);
            fprintf (ddata->asm_out_file, "\t%s DW_LNS_fixed_advance_pc",
                     ASM_COMMENT_START);
            fputc ('\n', ddata->asm_out_file);
            fprintf((ddata->asm_out_file), "\t%s\t%s-%s", ".2byte", line_label, prev_line_label);
            fprintf (ddata->asm_out_file, "\t%s from %s to %s",
                     ASM_COMMENT_START, prev_line_label, line_label);
            fputc ('\n', ddata->asm_out_file);
        }
        else
        {
            /* This can handle any delta.  This takes
               4+DWARF2_ADDR_SIZE bytes.  */
            fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", 0);
            fprintf (ddata->asm_out_file, "\t%s DW_LNE_set_address",
                     ASM_COMMENT_START);
            fputc ('\n', ddata->asm_out_file);
/*          output_uleb128 (1 + PTR_SIZE); */
            output_uleb128 (1 + 4, ddata);
            fputc ('\n', ddata->asm_out_file);
            fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", DW_LNE_set_address);
            fputc ('\n', ddata->asm_out_file);
            fprintf((ddata->asm_out_file), "\t%s\t", ".4byte");
            fprintf((ddata->asm_out_file), "%s", line_label);
            fputc ('\n', ddata->asm_out_file);
        }

        strcpy (prev_line_label, line_label);

        if (line_info->dw_file_num != current_file)
        {
            current_file = line_info->dw_file_num;
            fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", DW_LNS_set_file);
            fprintf (ddata->asm_out_file,
                     "\t%s DW_LNS_set_file", ASM_COMMENT_START);
            fputc ('\n', ddata->asm_out_file);
            output_uleb128 (current_file, ddata);
            fprintf (ddata->asm_out_file, "\t%s \"%s\"",
                     ASM_COMMENT_START, ddata->main_input_filename);
            fputc ('\n', ddata->asm_out_file);
        }

        if (line_info->dw_line_num != current_line)
        {
            line_offset = line_info->dw_line_num - current_line;
            line_delta = line_offset - (- 10);
            current_line = line_info->dw_line_num;
            if (line_delta >= 0 && line_delta < ((254 - 10 + 1) - 1))
            {
                /* This can handle deltas from -10 to 234, using the current
                   definitions of DWARF_LINE_BASE and DWARF_LINE_RANGE.  This
                   takes 1 byte.  */
                fprintf((ddata->asm_out_file), "\t%s\t0x%lx", ".byte", 10 + line_delta);
                fprintf (ddata->asm_out_file,
                         "\t%s line %lu", ASM_COMMENT_START, current_line);
                fputc ('\n', ddata->asm_out_file);
            }
            else
            {
                /* This can handle any delta.  This takes at least 4 bytes,
                     depending on the value being encoded.  */
                fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", DW_LNS_advance_line);
                fprintf (ddata->asm_out_file,
                         "\t%s advance to line %lu",
                         ASM_COMMENT_START, current_line);
                fputc ('\n', ddata->asm_out_file);
                output_sleb128 (line_offset, ddata);
                fputc ('\n', ddata->asm_out_file);
                fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".byte", DW_LNS_copy);
                fprintf (ddata->asm_out_file,
                         "\t%s DW_LNS_copy", ASM_COMMENT_START);
                fputc ('\n', ddata->asm_out_file);
            }
        }
        else
        {
            /* We still need to start a new row, so output a copy insn.  */
            fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".byte", DW_LNS_copy);
            fprintf (ddata->asm_out_file,
                     "\t%s copy line %lu", ASM_COMMENT_START, current_line);
            fputc ('\n', ddata->asm_out_file);
        }
    }

    fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".byte", 0);
    fprintf (ddata->asm_out_file, "\t%s DW_LNE_set_address", ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
/*  output_uleb128 (1 + PTR_SIZE); */
    output_uleb128 (1 + 4, ddata);
    fputc ('\n', ddata->asm_out_file);
    fprintf(ddata->asm_out_file, "\t%s\t0x%x", ".byte", DW_LNE_set_address);
    fputc ('\n', ddata->asm_out_file);
    fprintf(ddata->asm_out_file, "\t%s\t%s", ".4byte", ".L_text_e");
    fputc ('\n', ddata->asm_out_file);
    /* Output the marker for the end of the line number info.  */
    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", 0);
    fprintf (ddata->asm_out_file, "\t%s DW_LNE_end_sequence", ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    output_uleb128 (1, ddata);
    fputc ('\n', ddata->asm_out_file);
    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", DW_LNE_end_sequence);
    fputc ('\n', ddata->asm_out_file);

    /* Output the marker for the end of the line number info.  */
    fprintf((ddata->asm_out_file), "%s:\n", ".LTEND");
}

/* Return the size of an unsigned LEB128 quantity.  */
static unsigned long
size_of_uleb128 (unsigned long value)
{
    unsigned long size = 0;
    do
    {
        value >>= 7;
        size += 1;
    }
    while (value != 0);
    return size;
}

/* Return the size of a signed LEB128 quantity.  */
static unsigned long
size_of_sleb128 (long value)
{
    unsigned long size = 0;
    unsigned byte;
    do
    {
        byte = (value & 0x7f);
        value >>= 7;
        size += 1;
    }
    while (!(((value == 0) && ((byte & 0x40) == 0))
           || ((value == -1) && ((byte & 0x40) != 0))));
    return size;
}

/* Return the size of a location descriptor.  */
static unsigned long
size_of_loc_descr (dw_loc_descr_ref loc)
{
    unsigned long size = 1;
    switch (loc->dw_loc_opc)
    {
    case DW_OP_addr:
/*      size += PTR_SIZE; */
        size += 4;
        break;
    case DW_OP_const1u:
    case DW_OP_const1s:
        size += 1;
        break;
    case DW_OP_const2u:
    case DW_OP_const2s:
        size += 2;
        break;
    case DW_OP_const4u:
    case DW_OP_const4s:
        size += 4;
        break;
    case DW_OP_const8u:
    case DW_OP_const8s:
        size += 8;
        break;
    case DW_OP_constu:
        size += size_of_uleb128 (loc->dw_loc_oprnd1.v.val_unsigned);
        break;
    case DW_OP_consts:
        size += size_of_sleb128 (loc->dw_loc_oprnd1.v.val_int);
        break;
    case DW_OP_pick:
        size += 1;
        break;
    case DW_OP_plus_uconst:
        size += size_of_uleb128 (loc->dw_loc_oprnd1.v.val_unsigned);
        break;
    case DW_OP_skip:
    case DW_OP_bra:
        size += 2;
        break;
    case DW_OP_breg0:
    case DW_OP_breg1:
    case DW_OP_breg2:
    case DW_OP_breg3:
    case DW_OP_breg4:
    case DW_OP_breg5:
    case DW_OP_breg6:
    case DW_OP_breg7:
    case DW_OP_breg8:
    case DW_OP_breg9:
    case DW_OP_breg10:
    case DW_OP_breg11:
    case DW_OP_breg12:
    case DW_OP_breg13:
    case DW_OP_breg14:
    case DW_OP_breg15:
    case DW_OP_breg16:
    case DW_OP_breg17:
    case DW_OP_breg18:
    case DW_OP_breg19:
    case DW_OP_breg20:
    case DW_OP_breg21:
    case DW_OP_breg22:
    case DW_OP_breg23:
    case DW_OP_breg24:
    case DW_OP_breg25:
    case DW_OP_breg26:
    case DW_OP_breg27:
    case DW_OP_breg28:
    case DW_OP_breg29:
    case DW_OP_breg30:
    case DW_OP_breg31:
        size += size_of_sleb128 (loc->dw_loc_oprnd1.v.val_int);
        break;
    case DW_OP_regx:
        size += size_of_uleb128 (loc->dw_loc_oprnd1.v.val_unsigned);
        break;
    case DW_OP_fbreg:
        size += size_of_sleb128 (loc->dw_loc_oprnd1.v.val_int);
        break;
    case DW_OP_bregx:
        size += size_of_uleb128 (loc->dw_loc_oprnd1.v.val_unsigned);
        size += size_of_sleb128 (loc->dw_loc_oprnd2.v.val_int);
        break;
    case DW_OP_piece:
        size += size_of_uleb128 (loc->dw_loc_oprnd1.v.val_unsigned);
        break;
    case DW_OP_deref_size:
    case DW_OP_xderef_size:
        size += 1;
        break;
    default:
        break;
    }
    return size;
}

/* Output the DIE and its attributes.  Called recursively to generate
   the definitions of each child DIE.  */
static void
output_die (dw_die_ref die, struct dwarf_data *ddata)
{
    const char *ASM_COMMENT_START = "@";
    dw_attr_ref a;
    dw_die_ref c;
    unsigned long size;
    dw_loc_descr_ref loc;
    char *sym;

    fprintf (ddata->asm_out_file, ".LDIE%u:\n", die->label_num);

    output_uleb128 (die->die_abbrev, ddata);
    fprintf (ddata->asm_out_file, "\t%s DIE (0x%lx) %s",
             ASM_COMMENT_START,
             die->die_offset,
             dwarf_tag_name (die->die_tag));
    fputc ('\n', ddata->asm_out_file);
    for (a = die->die_attr; a != NULL; a = a->dw_attr_next)
    {
        switch (a->dw_attr_val.val_class)
        {
        case dw_val_class_addr:
            fprintf((ddata->asm_out_file), "\t%s\t%s", ".4byte", (a->dw_attr_val.v.val_addr));
            break;
        case dw_val_class_loc:
            size = 0;
            for (loc = a->dw_attr_val.v.val_loc; loc != NULL;
                 loc = loc->dw_loc_next)
            {
                size += size_of_loc_descr (loc);
            }
            /* Output the block length for this list of location operations.  */
            fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".2byte", (unsigned)size);
            fprintf (ddata->asm_out_file, "\t%s %s",
                     ASM_COMMENT_START, dwarf_attr_name (a->dw_attr));
            fputc ('\n', ddata->asm_out_file);
            for (loc = a->dw_attr_val.v.val_loc; loc != NULL;
                 loc = loc->dw_loc_next)
            {
                /* Output the opcode.  */
                fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", loc->dw_loc_opc);
                fprintf (ddata->asm_out_file, "\t%s %s",
                         ASM_COMMENT_START,
                         dwarf_stack_op_name (loc->dw_loc_opc));
                fputc ('\n', ddata->asm_out_file);
                /* Output the operand(s) (if any).  */
                output_loc_operands (loc, ddata);
            }
            break;
        case dw_val_class_const:
            fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".4byte",
                    (unsigned)a->dw_attr_val.v.val_int);
            break;
        case dw_val_class_unsigned_const:
            fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".4byte",
                    (unsigned)a->dw_attr_val.v.val_unsigned);
            break;
        case dw_val_class_double_const:
            break;
        case dw_val_class_flag:
            fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", a->dw_attr_val.v.val_flag);
            break;
        case dw_val_class_loc_list:
            sym = AT_loc_list (a)->ll_symbol;
            if (sym == 0)
                abort();
            fprintf((ddata->asm_out_file), "\t%s\t%s-%s", ".4byte", sym, "Ldebug_loc");
            break;
        case dw_val_class_die_ref:
            fprintf((ddata->asm_out_file), "\t%s\t.LDIE%u", ".4byte", a->dw_attr_val.v.val_die_ref->label_num);
            break;
        case dw_val_class_fde_ref:
#if 0
            ref_offset = fde_table[a->dw_attr_val.v.val_fde_index].dw_fde_offset;
            fprintf (asm_out_file, "\t%s\t%s+0x%x", UNALIGNED_INT_ASM_OP,
                     stripattributes (FRAME_SECTION), ref_offset);
#endif
            break;
        case dw_val_class_lbl_id:
            fprintf(ddata->asm_out_file, "\t%s\t", ".4byte");
            fprintf(ddata->asm_out_file, "%s", a->dw_attr_val.v.val_lbl_id);
            break;
        case dw_val_class_section_offset:
            fprintf((ddata->asm_out_file), "\t%s\t%s", ".4byte", a->dw_attr_val.v.val_section);
            break;
        case dw_val_class_str:
            do {
                int slen = (int) strlen(a->dw_attr_val.v.val_str);
                char *p = (a->dw_attr_val.v.val_str);
                int i;
                fprintf(ddata->asm_out_file, "\t.ascii \"");
                for (i = 0; i < slen; i++) {
                    int c = p[i];
                    if (c == '\"' || c == '\\')
                      putc('\\', ddata->asm_out_file);
                    if (c >= ' ' && c < 0177)
                      putc(c, ddata->asm_out_file);
                    else {
                      fprintf(ddata->asm_out_file, "\\%o", c);
                    }
                }
                fprintf(ddata->asm_out_file, "\\0\"");
            } while (0);
            break;
        default:
            abort ();
        }
        if (a->dw_attr_val.val_class != dw_val_class_loc)
        {
            fprintf (ddata->asm_out_file, "\t%s %s",
                     ASM_COMMENT_START, dwarf_attr_name (a->dw_attr));
            fputc ('\n', ddata->asm_out_file);
        }
    }
    for (c = die->die_child; c != NULL; c = c->die_sib)
    {
        output_die (c, ddata);
    }
    if (die->die_child != NULL)
    {
        /* Add null byte to terminate sibling list. */
        fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", 0);
        fputc ('\n', ddata->asm_out_file);
    }
}

/* Output a marker (i.e. a label) for the absolute end of the generated code
   for a function definition.  This gets called *after* the epilogue code has
   been generated.  */
void
dwarfout_end_epilogue (struct dwarf_data *ddata, control_flow_graph func)
{
    dw_fde_ref fde;
    char label[30];
    /* Output a label to mark the endpoint of the code generated for this
       function.        */
    sprintf (label, ".L_f%u_e", func->funcdef_number);
    fprintf((ddata->asm_out_file), "%s:\n", label);
    fde = &ddata->fde_table[ddata->fde_table_in_use - 1];
    fde->dw_fde_end = (char *) List_NewLast(ddata->mem, (int) strlen(label) + 1);
    strcpy(fde->dw_fde_end,label);
}

/* Convert a DWARF call frame info. operation to its string name */
static char *
dwarf_cfi_name (unsigned cfi_opc)
{
    switch (cfi_opc)
    {
    case DW_CFA_advance_loc:
        return "DW_CFA_advance_loc";
    case DW_CFA_offset:
        return "DW_CFA_offset";
    case DW_CFA_restore:
        return "DW_CFA_restore";
    case DW_CFA_nop:
        return "DW_CFA_nop";
    case DW_CFA_set_loc:
        return "DW_CFA_set_loc";
    case DW_CFA_advance_loc1:
        return "DW_CFA_advance_loc1";
    case DW_CFA_advance_loc2:
        return "DW_CFA_advance_loc2";
    case DW_CFA_advance_loc4:
        return "DW_CFA_advance_loc4";
    case DW_CFA_offset_extended:
        return "DW_CFA_offset_extended";
    case DW_CFA_restore_extended:
        return "DW_CFA_restore_extended";
    case DW_CFA_undefined:
        return "DW_CFA_undefined";
    case DW_CFA_same_value:
        return "DW_CFA_same_value";
    case DW_CFA_register:
        return "DW_CFA_register";
    case DW_CFA_remember_state:
        return "DW_CFA_remember_state";
    case DW_CFA_restore_state:
        return "DW_CFA_restore_state";
    case DW_CFA_def_cfa:
        return "DW_CFA_def_cfa";
    case DW_CFA_def_cfa_register:
        return "DW_CFA_def_cfa_register";
    case DW_CFA_def_cfa_offset:
        return "DW_CFA_def_cfa_offset";
    /* SGI/MIPS specific */
    case DW_CFA_MIPS_advance_loc8:
        return "DW_CFA_MIPS_advance_loc8";
    default:
        return "DW_CFA_<unknown>";
    }
}

/* Output a Call Frame Information opcode and its operand(s).  */
static void
output_cfi (dw_cfi_ref cfi, dw_fde_ref fde, struct dwarf_data *ddata)
{
    const char *ASM_COMMENT_START = "@";
    if (cfi->dw_cfi_opc == DW_CFA_advance_loc)
    {
        fprintf((ddata->asm_out_file), "\t%s\t0x%lx", ".byte",
                cfi->dw_cfi_opc | (cfi->dw_cfi_oprnd1.dw_cfi_offset & 0x3f));
        fprintf (ddata->asm_out_file, "\t%s DW_CFA_advance_loc", ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);
    }
    else if (cfi->dw_cfi_opc == DW_CFA_offset)
    {
        fprintf((ddata->asm_out_file), "\t%s\t0x%lx", ".byte",
                cfi->dw_cfi_opc | (cfi->dw_cfi_oprnd1.dw_cfi_reg_num & 0x3f));
        fprintf (ddata->asm_out_file, "\t%s DW_CFA_offset", ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);
        output_uleb128(cfi->dw_cfi_oprnd2.dw_cfi_offset, ddata);
        fputc ('\n', ddata->asm_out_file);
    }
    else if (cfi->dw_cfi_opc == DW_CFA_restore)
    {
        fprintf((ddata->asm_out_file), "\t%s\t0x%lx", ".byte",
                cfi->dw_cfi_opc | (cfi->dw_cfi_oprnd1.dw_cfi_reg_num & 0x3f));
        fprintf (ddata->asm_out_file, "\t%s DW_CFA_restore", ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);
    }
    else
    {
        fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", cfi->dw_cfi_opc);
        fprintf (ddata->asm_out_file, "\t%s %s",
                 ASM_COMMENT_START,
                 dwarf_cfi_name (cfi->dw_cfi_opc));
        fputc ('\n', ddata->asm_out_file);
        switch (cfi->dw_cfi_opc)
        {
        case DW_CFA_set_loc:
            fprintf((ddata->asm_out_file), "\t%s\t", ".4byte");
            fprintf((ddata->asm_out_file), "%s", cfi->dw_cfi_oprnd1.dw_cfi_addr);
            fputc ('\n', ddata->asm_out_file);
            break;
        case DW_CFA_advance_loc1:
            fprintf((ddata->asm_out_file), "\t%s\t", ".byte");
            fprintf((ddata->asm_out_file), "%s", cfi->dw_cfi_oprnd1.dw_cfi_addr);
            fprintf(ddata->asm_out_file, "-");
            fprintf((ddata->asm_out_file), "%s", fde->dw_fde_current_label);
            fde->dw_fde_current_label = cfi->dw_cfi_oprnd1.dw_cfi_addr;
            fputc ('\n', ddata->asm_out_file);
            break;
        case DW_CFA_advance_loc2:
            fprintf((ddata->asm_out_file), "\t%s\t", ".2byte");
            fprintf((ddata->asm_out_file), "%s", cfi->dw_cfi_oprnd1.dw_cfi_addr);
            fprintf(ddata->asm_out_file, "-");
            fprintf((ddata->asm_out_file), "%s", fde->dw_fde_current_label);
            fde->dw_fde_current_label = cfi->dw_cfi_oprnd1.dw_cfi_addr;
            fputc ('\n', ddata->asm_out_file);
            break;
        case DW_CFA_advance_loc4:
            fprintf((ddata->asm_out_file), "\t%s\t", ".4byte");
            fprintf((ddata->asm_out_file), "%s", cfi->dw_cfi_oprnd1.dw_cfi_addr);
            fprintf(ddata->asm_out_file, "-");
            fprintf((ddata->asm_out_file), "%s",  fde->dw_fde_current_label);
            fde->dw_fde_current_label = cfi->dw_cfi_oprnd1.dw_cfi_addr;
            fputc ('\n', ddata->asm_out_file);
            break;
        case DW_CFA_offset_extended:
        case DW_CFA_def_cfa:
            output_uleb128(cfi->dw_cfi_oprnd1.dw_cfi_reg_num, ddata);
            fputc ('\n', ddata->asm_out_file);
            output_uleb128(cfi->dw_cfi_oprnd2.dw_cfi_offset, ddata);
            fputc ('\n', ddata->asm_out_file);
            break;
        case DW_CFA_restore_extended:
        case DW_CFA_undefined:
            output_uleb128(cfi->dw_cfi_oprnd1.dw_cfi_reg_num, ddata);
            fputc ('\n', ddata->asm_out_file);
            break;
        case DW_CFA_same_value:
        case DW_CFA_def_cfa_register:
            output_uleb128(cfi->dw_cfi_oprnd1.dw_cfi_reg_num, ddata);
            fputc ('\n', ddata->asm_out_file);
            break;
        case DW_CFA_register:
            output_uleb128(cfi->dw_cfi_oprnd1.dw_cfi_reg_num, ddata);
            fputc ('\n', ddata->asm_out_file);
            output_uleb128(cfi->dw_cfi_oprnd2.dw_cfi_reg_num, ddata);
            fputc ('\n', ddata->asm_out_file);
            break;
        case DW_CFA_def_cfa_offset:
            output_uleb128(cfi->dw_cfi_oprnd1.dw_cfi_offset, ddata);
            fputc ('\n', ddata->asm_out_file);
            break;
        default:
            break;
        }
     }
}

/* Output the call frame information used to used to record information
   that relates to calculating the frame pointer, and records the
   location of saved registers.  */
static void
output_call_frame_info (struct dwarf_data *ddata)
{
    unsigned long i;
    dw_fde_ref fde;
    dw_cfi_ref cfi;
    const char *ASM_COMMENT_START = "@";
    char l1[20], l2[20];

    /* Output the CIE. */
    sprintf (l1, ".%s%d", "LSCIE", 0);
    sprintf (l2, ".%s%d", "LECIE", 0);
    fprintf((ddata->asm_out_file), "\t%s\t", ".4byte");
    fprintf (ddata->asm_out_file, "%s", l2);
    fprintf (ddata->asm_out_file, "-");
    fprintf (ddata->asm_out_file, "%s", l1);
    fprintf (ddata->asm_out_file, "\t%s Length of Common Information Entry",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);

    fprintf((ddata->asm_out_file), "%s:\n", l1);

    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".4byte", DW_CIE_ID);
    fprintf (ddata->asm_out_file, "\t%s CIE Identifier Tag",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", DW_CIE_VERSION);
    fprintf (ddata->asm_out_file, "\t%s CIE Version",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte",  0);
    fprintf (ddata->asm_out_file, "\t%s CIE Augmentation (none)",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    output_uleb128 (1, ddata);
    fprintf (ddata->asm_out_file, "\t%s CIE Code Alignment Factor",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    output_sleb128 (-4, ddata);
    fprintf (ddata->asm_out_file, "\t%s CIE Data Alignment Factor",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);
    fprintf((ddata->asm_out_file), "\t%s\t0x%x", ".byte", 14);
    fprintf (ddata->asm_out_file, "\t%s CIE RA Column",
             ASM_COMMENT_START);
    fputc ('\n', ddata->asm_out_file);

    /* Pad the CIE out to an address sized boundary.  */
    fprintf((ddata->asm_out_file), "\t.align\t%d\n", 2);

    fprintf((ddata->asm_out_file), "%s:\n", l2);

    /* Loop through all of the FDE's.  */
    for (i = 0; i < ddata->fde_table_in_use; ++i)
    {
        fde = &ddata->fde_table[i];
        sprintf (l1, ".%s%lu", "LSFDE", 2*i);
        sprintf (l2, ".%s%lu", "LEFDE", 2*i);
        fprintf((ddata->asm_out_file), "\t%s\t", ".4byte");
        fprintf (ddata->asm_out_file, "%s", l2);
        fprintf (ddata->asm_out_file, "-");
        fprintf (ddata->asm_out_file, "%s", l1);

        fprintf (ddata->asm_out_file, "\t%s FDE Length",
                 ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);

        fprintf((ddata->asm_out_file), "%s:\n", l1);

        do {
            fprintf((ddata->asm_out_file), "\t%s\t", ".4byte");
            fprintf((ddata->asm_out_file), ".debug_frame");
        } while (0);
        fprintf (ddata->asm_out_file, "\t%s FDE CIE offset",
                 ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);
        do {
            fprintf((ddata->asm_out_file), "\t%s\t", ".4byte");
            fprintf((ddata->asm_out_file), "%s", fde->dw_fde_begin);
        } while (0);
        fprintf (ddata->asm_out_file, "\t%s FDE initial location",
                 ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);
        do {
            fprintf((ddata->asm_out_file), "\t%s\t", ".4byte");
            fprintf((ddata->asm_out_file), "%s", fde->dw_fde_end);
            fprintf(ddata->asm_out_file, "-");
            fprintf((ddata->asm_out_file), "%s", fde->dw_fde_begin);
        } while (0);
        fprintf (ddata->asm_out_file, "\t%s FDE address range",
                 ASM_COMMENT_START);
        fputc ('\n', ddata->asm_out_file);

        /* Loop through the Call Frame Instructions associated with
           this FDE.  */
        fde->dw_fde_current_label = fde->dw_fde_begin;
        for (cfi = fde->dw_fde_cfi; cfi != NULL; cfi = cfi->dw_cfi_next)
        {
            output_cfi (cfi, fde, ddata);
        }

        /* Pad to a double word boundary.  */
        fprintf((ddata->asm_out_file), "\t.align\t%d\n", 2);
        fprintf((ddata->asm_out_file), "%s:\n", l2);
    }
}

/* Traverse the DIE, and add a sibling attribute if it may have the
   effect of speeding up access to siblings.  To save some space,
   avoid generating sibling attributes for DIE's without children.  */
static void
add_sibling_attributes(dw_die_ref die, struct dwarf_data *ddata)
{
    dw_die_ref c;
    dw_attr_ref attr;
    if (die != ddata->comp_unit_die
        && die->die_sib && die->die_child != NULL)
    {
        attr = (dw_attr_ref) List_NewLast (ddata->mem, sizeof (dw_attr_node));
        if (attr != NULL)
        {
            attr->dw_attr_next = NULL;
            attr->dw_attr = DW_AT_sibling;
            attr->dw_attr_val.val_class = dw_val_class_die_ref;
            attr->dw_attr_val.v.val_die_ref = die->die_sib;
        }
        /* add the sibling link to the front of the attribute list.  */
        attr->dw_attr_next = die->die_attr;
        if (die->die_attr == NULL)
        {
            die->die_attr_last = attr;
        }
        die->die_attr = attr;
    }
    for (c = die->die_child; c != NULL; c = c->die_sib)
    {
        add_sibling_attributes (c, ddata);
    }
}

/* 每个DIE（及其属性值对）的格式都编码在一个缩写表中。这个例程
   构建缩写表，并为每个缩写条目分配一个唯一的缩写ID。每个DIE的
   子节点都会递归地被访问。  */
static void
build_abbrev_table (dw_die_ref die, struct dwarf_data *ddata)
{
    unsigned long abbrev_id;
    unsigned long n_alloc;
    dw_die_ref c;
    dw_attr_ref d_attr, a_attr;
    dw_die_ref *old_ptr;
    for (abbrev_id = 1; abbrev_id < ddata->abbrev_die_table_in_use; ++abbrev_id)
    {
        dw_die_ref abbrev = ddata->abbrev_die_table[abbrev_id];
        if (abbrev->die_tag == die->die_tag)
        {
            if ((abbrev->die_child != NULL) == (die->die_child != NULL))
            {
                a_attr = abbrev->die_attr;
                d_attr = die->die_attr;
                while (a_attr != NULL && d_attr != NULL)
                {
                    if ((a_attr->dw_attr != d_attr->dw_attr)
                        || (a_attr->dw_attr_val.val_class
                            != d_attr->dw_attr_val.val_class))
                    {
                        break;
                    }
                    a_attr = a_attr->dw_attr_next;
                    d_attr = d_attr->dw_attr_next;
                }
                if (a_attr == NULL && d_attr == NULL)
                {
                    break;
                }
            }
        }
    }
    if (abbrev_id >= ddata->abbrev_die_table_in_use)
    {
        if (ddata->abbrev_die_table_in_use >= ddata->abbrev_die_table_allocated)
        {
            n_alloc = ddata->abbrev_die_table_allocated + 256;
            old_ptr = ddata->abbrev_die_table;
            ddata->abbrev_die_table = (dw_die_ref *)
              List_NewLast (ddata->mem,
                       sizeof (dw_die_ref) * n_alloc);
            memcpy (ddata->abbrev_die_table, old_ptr,
                ddata->abbrev_die_table_allocated * sizeof (dw_die_ref));
            memset (&ddata->abbrev_die_table[ddata->abbrev_die_table_allocated], 0,
                (n_alloc - ddata->abbrev_die_table_allocated) * sizeof (dw_die_ref));
            ddata->abbrev_die_table_allocated = n_alloc;
            List_Delete(old_ptr);
        }
        ++ddata->abbrev_die_table_in_use;
        ddata->abbrev_die_table[abbrev_id] = die;
    }
    die->die_abbrev = abbrev_id;
    for (c = die->die_child; c != NULL; c = c->die_sib)
    {
        build_abbrev_table (c, ddata);
    }
}

/* Add an DW_AT_name attribute and source coordinate attribute for the
   given decl, but only if it actually has a name.  */
void
add_name_and_src_coords_attributes (dw_die_ref die, SymDef decl, struct dwarf_data *ddata)
{
    char *decl_name = decl->sdName;
    unsigned file_index;
    if (decl_name)
    {
        add_name_attribute (die, decl_name, ddata);
        file_index = 1;
        add_AT_unsigned (die, DW_AT_decl_file, file_index, ddata);
        add_AT_unsigned (die, DW_AT_decl_line, decl->line, ddata);
    }
}

/* Add a flag value attribute to a DIE.  */
void
add_AT_flag (dw_die_ref die, enum dwarf_attribute attr_kind, unsigned flag, struct dwarf_data *ddata)
{
    dw_attr_ref attr = (dw_attr_ref) List_NewLast (ddata->mem, sizeof (dw_attr_node));
    if (attr != NULL)
    {
        attr->dw_attr_next = NULL;
        attr->dw_attr = attr_kind;
        attr->dw_attr_val.val_class = dw_val_class_flag;
        attr->dw_attr_val.v.val_flag = flag;
        add_dwarf_attr (die, attr);
    }
}

/* Add a location description attribute value to a DIE.  */
void
add_AT_loc (dw_die_ref die, enum dwarf_attribute attr_kind, dw_loc_descr_ref loc, struct dwarf_data *ddata)
{
    dw_attr_ref attr = (dw_attr_ref) List_NewLast (ddata->mem, sizeof (dw_attr_node));
    if (attr != NULL)
    {
        attr->dw_attr_next = NULL;
        attr->dw_attr = attr_kind;
        attr->dw_attr_val.val_class = dw_val_class_loc;
        attr->dw_attr_val.v.val_loc = loc;
        add_dwarf_attr (die, attr);
    }
}

/* Add a new entry to .debug_pubnames if appropriate.  */
void
add_pubname (SymDef decl, dw_die_ref die, struct dwarf_data *ddata)
{
    pubname_ref p;
    pubname_ref old_ptr;

    if (! is_global_var (decl))
        return;

    if (ddata->pubname_table_in_use == ddata->pubname_table_allocated)
    {
        ddata->pubname_table_allocated += 64;
        old_ptr = ddata->pubname_table;
        ddata->pubname_table = (pubname_ref) List_NewLast
          (ddata->mem, ddata->pubname_table_allocated * sizeof (pubname_entry));
        if (old_ptr)
        {
            memcpy (ddata->pubname_table, old_ptr, ddata->pubname_table_in_use * sizeof (pubname_entry));
            List_Delete(old_ptr);
        }
    }
    p = &ddata->pubname_table[ddata->pubname_table_in_use++];
    p->die = die;
    p->name = decl->sdName;
}

/* Generate a DIE to represent a declared data object.  */
static void
gen_variable_die (SymDef decl, dw_die_ref context_die, struct dwarf_data *ddata)
{
    dw_die_ref var_die = new_die (DW_TAG_variable, context_die, ddata);
    dw_loc_descr_ref loc_descr = NULL;
  
    add_name_and_src_coords_attributes (var_die, decl, ddata);
    add_type_attribute (var_die, decl->sdType,context_die, ddata);
    if (is_global_var (decl))
    {
        add_AT_flag (var_die, DW_AT_external, 1, ddata);
    }

    if  (is_global_var (decl))
    {
        loc_descr = mem_loc_descriptor (decl, ddata);
        add_loc_descr(&decl->loc_descr, loc_descr);
    }

    add_AT_loc (var_die, DW_AT_location, decl->loc_descr, ddata);

#if 0
    add_location_or_const_value_attribute (var_die, decl);
#endif
    add_pubname (decl, var_die, ddata);
}

/* Generate a DIE to represent either a real live formal parameter decl or to
   represent just the type of some formal parameter position in some function
   type.  */
static void
gen_formal_parameter_die (SymDef node, dw_die_ref context_die, struct dwarf_data *ddata)
{
    dw_die_ref parm_die = new_die (DW_TAG_formal_parameter,
                                          context_die, ddata);

    add_name_and_src_coords_attributes (parm_die, node, ddata);
    add_type_attribute (parm_die, node->sdType,
                        context_die, ddata);
    if (node->sdIsImplicit)
        add_AT_flag (parm_die, DW_AT_artificial, 1, ddata);

    add_AT_loc (parm_die, DW_AT_location, node->loc_descr, ddata);
}

/* Generate Dwarf debug information for a decl described by DECL.  */
void
gen_decl_die (SymDef decl, dw_die_ref context_die, struct dwarf_data *ddata)
{
    if      (decl->sdSymKind == SYM_FNC)
    {
        /* Before we describe the FUNCTION_DECL itself, make sure that we have
           described its return type.  */
        gen_type_die (decl->sdType->tdFnc.tdfRett, context_die, ddata);

#if 0
        /* Now output a DIE to represent the function itself.  */
        gen_subprogram_die (decl, context_die);
#endif
    }
    else if (decl->sdSymKind  == SYM_VAR &&
             decl->sdIsImplicit == FALSE   &&
             !decl->sdVar.sdvArgument)
    {
        /* Output any DIEs that are needed to specify the type of this data
           object.  */
        gen_type_die (decl->sdType, context_die, ddata);

        /* Now output the DIE to represent the data object itself.  This gets
           complicated because of the possibility that the VAR_DECL really
           represents an inlined instance of a formal parameter for an inline
           function.  */
        gen_variable_die (decl, context_die, ddata);
    }
    else if (decl->sdSymKind  == SYM_VAR &&
             decl->sdVar.sdvArgument)
    {
        gen_type_die (decl->sdType, context_die, ddata);
        gen_formal_parameter_die (decl, context_die, ddata);
    }
}

void
dwarfout_file_scope_decl (SymDef decl, struct dwarf_data *ddata)
{
    if  (!decl->sdIsImplicit)
    {
        gen_decl_die (decl, ddata->comp_unit_die, ddata);
    }

}

/* Generate a DIE for a lexical block.  */
static void
gen_lexical_block_die (SymDef stmt, dw_die_ref context_die, struct dwarf_data *ddata)
{
    dw_die_ref stmt_die = new_die (DW_TAG_lexical_block, context_die, ddata);
    char label[30];
    sprintf (label, ".L_B%u", stmt->uid);
    add_AT_lbl_id (stmt_die, DW_AT_low_pc, label, ddata);
    sprintf (label, ".L_B%u_e", stmt->uid);
    add_AT_lbl_id (stmt_die, DW_AT_high_pc, label, ddata);
    decls_for_scope (stmt, stmt_die, ddata);
}


/* Generate a DW_TAG_lexical_block DIE followed by DIEs to represent all of the
   things which are local to the given block.  */
static void
gen_block_die (SymDef stmt, dw_die_ref context_die, struct dwarf_data *ddata)
{
    int must_output_die = 0;
    SymDef decl;
    struct avl_traverser trav;

  /* Determine if we need to output any Dwarf DIEs at all to represent this
     block.  */
    for(  decl = (SymDef)avl_t_first (&trav, stmt->sdScope.sdScope)
       ;  decl != NULL
       ;  decl = avl_t_next (&trav)
       )
        if (decl->sdSymKind == SYM_VAR
            && !decl->sdIsImplicit
            && !comp->cmpConfig.optimize)
          {
            must_output_die = 1;
            break;
          }

    /* It would be a waste of space to generate a Dwarf DW_TAG_lexical_block
       DIE for any block which contains no significant local declarations at
       all.  Rather, in such cases we just call `decls_for_scope' so that any
       needed Dwarf info for any sub-blocks will get properly generated. Note
       that in terse mode, our definition of what constitutes a "significant"
       local declaration gets restricted to include only inlined function
       instances and local (nested) function definitions.  */
    if (must_output_die)
    {
        gen_lexical_block_die (stmt, context_die, ddata);
    }
    else
        decls_for_scope (stmt, context_die, ddata);
}

/* Generate all of the decls declared within a given scope and (recursively)
   all of it's sub-blocks.  */
void
decls_for_scope (SymDef stmt, dw_die_ref context_die, struct dwarf_data *ddata)
{
    SymDef subblocks;

    SymDef decl;
    struct avl_traverser trav;

    /* Output the DIEs to represent all of the data objects, functions,
       typedefs, and tagged types declared directly within this block but not
       within any nested sub-blocks.  */
    for(  decl = (SymDef)avl_t_first (&trav, stmt->sdScope.sdScope)
       ;  decl != NULL
       ;  decl = avl_t_next (&trav)
       )
    {
        gen_decl_die (decl, context_die, ddata);
    }

  /* Output the DIEs to represent all sub-blocks (and the items declared
     therein) of this block.  */
    for(  subblocks = (SymDef)avl_t_first (&trav, stmt->sdScope.sdScope)
       ;  subblocks != NULL
       ;  subblocks = avl_t_next (&trav)
       )
        if  (sdHasScope(subblocks->sdSymKind))
        {
            gen_block_die (subblocks, context_die, ddata);
        }
}

/* Generate the DIE for the compilation unit.  */
static void
gen_compile_unit_die (const char *main_input_filename, struct dwarf_data *ddata)
{
    dyn_string_t producer;
    dyn_string_t full_src_name;
#if defined (_WIN32)
    char *wd = _getcwd ( NULL, 0 );
#else
    char *wd = getcwd ( NULL, 0 );
#endif

    producer = dyn_string_new (3);
    ddata->comp_unit_die = new_die (DW_TAG_compile_unit, NULL, ddata);

    /* MIPS/SGI requires the full pathname of the input file.  */
    if (main_input_filename[0] == '/' || main_input_filename[0] == '\\')
    {
        add_name_attribute (ddata->comp_unit_die, main_input_filename, ddata);
    }
    else
    {
        full_src_name = dyn_string_new (1024);
        dyn_string_insert_cstr (full_src_name, 0, wd);
        dyn_string_append_char (full_src_name, '/');
        dyn_string_append_cstr (full_src_name, main_input_filename);
        add_name_attribute (ddata->comp_unit_die, dyn_string_buf (full_src_name), ddata);
        dyn_string_delete (full_src_name);
    }

    dyn_string_insert_cstr (producer, 0, "The right way of Light");
    add_AT_string (ddata->comp_unit_die, DW_AT_producer, dyn_string_buf (producer), ddata);
    dyn_string_delete (producer);

    add_AT_unsigned (ddata->comp_unit_die, DW_AT_language, 0x0002, ddata);

    add_AT_lbl_id (ddata->comp_unit_die, DW_AT_low_pc, ".L_text_b", ddata);
    add_AT_lbl_id (ddata->comp_unit_die, DW_AT_high_pc, ".L_text_e", ddata);

    if (wd)
    {
        add_AT_string (ddata->comp_unit_die, DW_AT_comp_dir, wd, ddata);
    }
    add_AT_section_offset (ddata->comp_unit_die, DW_AT_stmt_list, ".debug_line", ddata);

    free (wd);
}

void
dwarfout_header (FILE *asm_out_file)
{
    /* Output a starting label for the .text section.  */
    fputc ('\n', asm_out_file);
    fprintf(asm_out_file, "%s:\n", ".L_text_b");
}

void
dwarfout_footer (FILE *asm_out_file)
{
    /* Output a terminator label for the .text section.  */
    fputc ('\n', asm_out_file);
    fprintf(asm_out_file, "%s:\n", ".L_text_e");
}

/* Set up for Dwarf output at the start of compilation.  */
struct dwarf_data *
dwarfout_init (SymTab stab, FILE *asm_out_file, const char *main_input_filename)
{
    struct dwarf_data *ddata = (struct dwarf_data *) xmalloc (sizeof (struct dwarf_data));
    memset (ddata, 0, sizeof (struct dwarf_data));
    ddata->mem = List_Create ();

    ddata->base_type_die_table = (dw_die_ref *) List_NewLast (ddata->mem, sizeof (dw_die_ref) * NUM_BASE_TYPES);

    ddata->asm_out_file = asm_out_file;
    ddata->main_input_filename = (char *)main_input_filename;

    /* Allocate the initial hunk of the type_die_table.  */
    ddata->type_die_table
      = (dw_die_ref *) List_NewLast (ddata->mem, 4096 * sizeof (dw_die_ref));
    memset (ddata->type_die_table, 0,  4096 * sizeof (dw_die_ref));
    ddata->type_die_table_allocated = 4096;
    ddata->type_die_table_in_use = 0;

    /* Allocate the initial hunk of the abbrev_die_table.  */
    ddata->abbrev_die_table
      = (dw_die_ref *) List_NewLast (ddata->mem, 256
                                * sizeof (dw_die_ref));
    memset (ddata->abbrev_die_table, 0, 256 * sizeof (dw_die_ref));
    ddata->abbrev_die_table_allocated = 256;
    /* zero-th entry is allocated, but unused */
    ddata->abbrev_die_table_in_use = 1;

    /* Allocate the initial hunk of the line_info_table.  */
    ddata->line_info_table
      = (dw_line_info_ref) List_NewLast (ddata->mem, 1024
                                    * sizeof (dw_line_info_entry));
    memset (ddata->line_info_table, 0, 1024
           * sizeof (dw_line_info_entry));
    ddata->line_info_table_allocated = 1024;
    /* zero-th entry is allocated, but unused */
    ddata->line_info_table_in_use = 1;

    /* Allocate the initial hunk of the fde_table.  */
    ddata->fde_table = (dw_fde_ref) List_NewLast (ddata->mem, 256 * sizeof (dw_fde_node));
    memset (ddata->fde_table, 0, 256 * sizeof (dw_fde_node));
    ddata->fde_table_allocated = 256;
    ddata->fde_table_in_use = 0;

    /* Generate the initial DIE for the .debug section.  Note that the (string) 
       value given in the DW_AT_name attribute of the DW_TAG_compile_unit DIE
       will (typically) be a relative pathname and that this pathname should be 
       taken as being relative to the directory from which the compiler was
       invoked when the given (base) source file was compiled.  */
    gen_compile_unit_die (main_input_filename, ddata);

    init_base_type_table (ddata);

    return ddata;
}

void dwarfout_deinit(struct dwarf_data *ddata)
{
    List_Destroy (&ddata->mem);
    free (ddata);
}

/* Output stuff that dwarf requires at the end of every file,
   and generate the DWARF-2 debugging info.  */
void
dwarfout_finish (SymTab stab, struct dwarf_data *ddata)
{
    traverse_global_variables (stab, (void (*) (SymDef, void *))dwarfout_file_scope_decl, ddata);

    /* Traverse the DIE tree and add sibling attributes to those DIE's
       that have children.  */
    add_sibling_attributes (ddata->comp_unit_die, ddata);

    /* Output the abbreviation table.  */
    fputc ('\n', ddata->asm_out_file);
    fprintf((ddata->asm_out_file), "\t%s\t%s\n", ".section", ".debug_abbrev");
    build_abbrev_table (ddata->comp_unit_die, ddata);
    output_abbrev_section (ddata);

    /* Output the source line correspondence table.  */
    fputc ('\n', ddata->asm_out_file);
    fprintf((ddata->asm_out_file), "\t%s\t%s\n", ".section", ".debug_line");
    output_line_info (ddata);

    /* Output debugging information.  */
    fputc ('\n', ddata->asm_out_file);
    fprintf(ddata->asm_out_file, "\t%s\t%s\n", ".section", ".debug_info");
    output_compilation_unit_header (ddata->asm_out_file);
    output_die (ddata->comp_unit_die, ddata);
    fprintf((ddata->asm_out_file), ".Ldebug_info_end:\n");

    if (ddata->fde_table_in_use)
    {
        /* Output call frame information.  */
        fputc ('\n', ddata->asm_out_file);
        fprintf(ddata->asm_out_file, "\t%s\t%s\n", ".section", ".debug_frame");
        output_call_frame_info (ddata);
    }

    /* Output the location lists info. */
    fprintf(ddata->asm_out_file, "\t%s\t%s\n", ".section", ".debug_loc");
    output_location_lists (ddata->comp_unit_die, ddata);
}

BOOL codegen (InterCode code, SymTab stab, const char *main_input_filename, FILE *asm_out_file, BOOL debug_info) 
{
    BOOL bSuccess = FALSE;
    struct dwarf_data *ddata = NULL;
    struct Backend* backend;

    backend = (struct Backend*) xmalloc (sizeof (*backend));
    if  (debug_info)
        ddata = dwarfout_init (stab, asm_out_file, main_input_filename);

    if  (!CodeGeneratorArm32 (code, stab, main_input_filename, asm_out_file, backend, ddata))
        goto fail;

    if  (debug_info)
        dwarfout_finish (stab, ddata);

    bSuccess = TRUE;
fail:
    if  (debug_info)
        dwarfout_deinit (ddata);
    free (backend);
    return bSuccess;
}

