#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "all.h"
#include "CodeGeneratorArm32.h"

extern LIST mempool;
#define ALLOC(n) List_NewLast(mempool, n)
#include <limits.h>
#include <stdlib.h>
#ifndef STATE_TYPE
#define STATE_TYPE int
#endif
#ifndef ALLOC
#define ALLOC(n) malloc(n)
#endif
#ifndef burmArm32_assert
#define burmArm32_assert(x,y) if (!(x)) { y; abort(); }
#endif

#define burmArm32_stm_NT 1
#define burmArm32_reg_NT 2
int burmArm32_max_nt = 2;

char *burmArm32_ntname[] = {
	0,
	"stm",
	"reg",
	0
};

struct burmArm32_state {
	int op;
	struct burmArm32_state *left, *right;
	short cost[3];
	struct {
		unsigned burmArm32_stm:5;
		unsigned burmArm32_reg:6;
	} rule;
};

static short burmArm32_nts_0[] = { 0 };
static short burmArm32_nts_1[] = { burmArm32_reg_NT, 0 };
static short burmArm32_nts_2[] = { burmArm32_reg_NT, burmArm32_reg_NT, 0 };
static short burmArm32_nts_3[] = { burmArm32_reg_NT, burmArm32_reg_NT, burmArm32_reg_NT, 0 };

short *burmArm32_nts[] = {
	0,	/* 0 */
	burmArm32_nts_0,	/* 1 */
	burmArm32_nts_1,	/* 2 */
	burmArm32_nts_2,	/* 3 */
	burmArm32_nts_1,	/* 4 */
	burmArm32_nts_0,	/* 5 */
	burmArm32_nts_2,	/* 6 */
	burmArm32_nts_3,	/* 7 */
	burmArm32_nts_2,	/* 8 */
	burmArm32_nts_2,	/* 9 */
	burmArm32_nts_1,	/* 10 */
	burmArm32_nts_2,	/* 11 */
	burmArm32_nts_2,	/* 12 */
	burmArm32_nts_2,	/* 13 */
	burmArm32_nts_1,	/* 14 */
	burmArm32_nts_3,	/* 15 */
	burmArm32_nts_3,	/* 16 */
	burmArm32_nts_2,	/* 17 */
	burmArm32_nts_1,	/* 18 */
	burmArm32_nts_0,	/* 19 */
	burmArm32_nts_0,	/* 20 */
	burmArm32_nts_2,	/* 21 */
	burmArm32_nts_1,	/* 22 */
	burmArm32_nts_1,	/* 23 */
	burmArm32_nts_0,	/* 24 */
	burmArm32_nts_1,	/* 25 */
	burmArm32_nts_1,	/* 26 */
	burmArm32_nts_0,	/* 27 */
	burmArm32_nts_0,	/* 28 */
	burmArm32_nts_1,	/* 29 */
	burmArm32_nts_0,	/* 30 */
	burmArm32_nts_0,	/* 31 */
	burmArm32_nts_0,	/* 32 */
	burmArm32_nts_1,	/* 33 */
	burmArm32_nts_2,	/* 34 */
	burmArm32_nts_2,	/* 35 */
	burmArm32_nts_2,	/* 36 */
	burmArm32_nts_2,	/* 37 */
	burmArm32_nts_1,	/* 38 */
	burmArm32_nts_1,	/* 39 */
	burmArm32_nts_1,	/* 40 */
	burmArm32_nts_2,	/* 41 */
	burmArm32_nts_1,	/* 42 */
	burmArm32_nts_0,	/* 43 */
	burmArm32_nts_1,	/* 44 */
	burmArm32_nts_1,	/* 45 */
	burmArm32_nts_2,	/* 46 */
	burmArm32_nts_2,	/* 47 */
	burmArm32_nts_0,	/* 48 */
	burmArm32_nts_0,	/* 49 */
	burmArm32_nts_1,	/* 50 */
	burmArm32_nts_1,	/* 51 */
	burmArm32_nts_2,	/* 52 */
	burmArm32_nts_3,	/* 53 */
	burmArm32_nts_2,	/* 54 */
	burmArm32_nts_2,	/* 55 */
	burmArm32_nts_2,	/* 56 */
	burmArm32_nts_2,	/* 57 */
	burmArm32_nts_2,	/* 58 */
	burmArm32_nts_2,	/* 59 */
	burmArm32_nts_2,	/* 60 */
	burmArm32_nts_2,	/* 61 */
	burmArm32_nts_2,	/* 62 */
	burmArm32_nts_1,	/* 63 */
	burmArm32_nts_1,	/* 64 */
	burmArm32_nts_1,	/* 65 */
};

char burmArm32_arity[] = {
	0,	/* 0 */
	0,	/* 1=imm12 */
	0,	/* 2=simm8 */
	0,	/* 3=CNSTI4 */
	1,	/* 4=INDIRI4 */
	2,	/* 5=ADD */
	2,	/* 6=SDIV */
	2,	/* 7=MUL */
	2,	/* 8=SUB */
	1,	/* 9=JUMPV */
	0,	/* 10=LABELV */
	1,	/* 11=LDR */
	2,	/* 12=STR */
	0,	/* 13=REGISTER */
	2,	/* 14=RSB */
	2,	/* 15=CMP */
	0,	/* 16=imm16 */
	1,	/* 17=CLZ */
	2,	/* 18=LSR */
	1,	/* 19=MOV */
	1,	/* 20=BL */
	0,	/* 21=LABEL */
	1,	/* 22=BX */
	0,	/* 23=PUSH */
	0,	/* 24=POP */
	0,	/* 25=STRING */
	1,	/* 26=VMOV */
	2,	/* 27=VMUL */
	2,	/* 28=VADD */
	2,	/* 29=VSUB */
	2,	/* 30=VDIV */
	1,	/* 31=vcvt_signedToFloatingPoint */
	1,	/* 32=vcvt_floatingPointToSigned */
	1,	/* 33=VNEG */
	2,	/* 34=VCMP */
	1,	/* 35=VCMPz */
	0,	/* 36=VMRS */
	2,	/* 37=VSTR */
	1,	/* 38=VLDR */
	0,	/* 39=imm10 */
	0,	/* 40=VPUSH */
	0,	/* 41=VPOP */
	2,	/* 42=LSL */
	2,	/* 43=ASR */
	0,	/* 44=imm5 */
};

char *burmArm32_opname[] = {
	/* 0 */	0,
	/* 1 */	"imm12",
	/* 2 */	"simm8",
	/* 3 */	"CNSTI4",
	/* 4 */	"INDIRI4",
	/* 5 */	"ADD",
	/* 6 */	"SDIV",
	/* 7 */	"MUL",
	/* 8 */	"SUB",
	/* 9 */	"JUMPV",
	/* 10 */	"LABELV",
	/* 11 */	"LDR",
	/* 12 */	"STR",
	/* 13 */	"REGISTER",
	/* 14 */	"RSB",
	/* 15 */	"CMP",
	/* 16 */	"imm16",
	/* 17 */	"CLZ",
	/* 18 */	"LSR",
	/* 19 */	"MOV",
	/* 20 */	"BL",
	/* 21 */	"LABEL",
	/* 22 */	"BX",
	/* 23 */	"PUSH",
	/* 24 */	"POP",
	/* 25 */	"STRING",
	/* 26 */	"VMOV",
	/* 27 */	"VMUL",
	/* 28 */	"VADD",
	/* 29 */	"VSUB",
	/* 30 */	"VDIV",
	/* 31 */	"vcvt_signedToFloatingPoint",
	/* 32 */	"vcvt_floatingPointToSigned",
	/* 33 */	"VNEG",
	/* 34 */	"VCMP",
	/* 35 */	"VCMPz",
	/* 36 */	"VMRS",
	/* 37 */	"VSTR",
	/* 38 */	"VLDR",
	/* 39 */	"imm10",
	/* 40 */	"VPUSH",
	/* 41 */	"VPOP",
	/* 42 */	"LSL",
	/* 43 */	"ASR",
	/* 44 */	"imm5",
};

short burmArm32_cost[][4] = {
	{ 0 },	/* 0 */
	{ 1 },	/* 1 = reg: LDR(CNSTI4) */
	{ 1 },	/* 2 = reg: LDR(INDIRI4(reg)) */
	{ 1 },	/* 3 = reg: LDR(INDIRI4(ADD(reg,reg))) */
	{ 1 },	/* 4 = reg: LDR(INDIRI4(ADD(reg,imm12))) */
	{ 1 },	/* 5 = reg: LDR(LABELV) */
	{ 1 },	/* 6 = stm: STR(reg,INDIRI4(reg)) */
	{ 1 },	/* 7 = stm: STR(reg,INDIRI4(ADD(reg,reg))) */
	{ 1 },	/* 8 = stm: STR(reg,INDIRI4(ADD(reg,imm12))) */
	{ 1 },	/* 9 = reg: ADD(reg,reg) */
	{ 1 },	/* 10 = reg: ADD(reg,simm8) */
	{ 1 },	/* 11 = reg: SDIV(reg,reg) */
	{ 1 },	/* 12 = reg: MUL(reg,reg) */
	{ 1 },	/* 13 = reg: SUB(reg,reg) */
	{ 1 },	/* 14 = reg: SUB(reg,simm8) */
	{ 1 },	/* 15 = reg: ADD(MUL(reg,reg),reg) */
	{ 1 },	/* 16 = reg: SUB(reg,MUL(reg,reg)) */
	{ 1 },	/* 17 = stm: CMP(reg,reg) */
	{ 1 },	/* 18 = stm: CMP(reg,simm8) */
	{ 1 },	/* 19 = stm: JUMPV(CNSTI4) */
	{ 0 },	/* 20 = reg: REGISTER */
	{ 1 },	/* 21 = reg: RSB(reg,reg) */
	{ 1 },	/* 22 = reg: RSB(reg,simm8) */
	{ 1 },	/* 23 = reg: MOV(reg) */
	{ 1 },	/* 24 = reg: MOV(imm16) */
	{ 1 },	/* 25 = reg: CLZ(reg) */
	{ 1 },	/* 26 = reg: LSR(reg,imm5) */
	{ 1 },	/* 27 = stm: BL(LABELV) */
	{ 0 },	/* 28 = stm: LABEL */
	{ 1 },	/* 29 = stm: BX(reg) */
	{ 1 },	/* 30 = stm: PUSH */
	{ 1 },	/* 31 = stm: POP */
	{ 0 },	/* 32 = stm: STRING */
	{ 1 },	/* 33 = reg: VMOV(reg) */
	{ 1 },	/* 34 = reg: VMUL(reg,reg) */
	{ 1 },	/* 35 = reg: VADD(reg,reg) */
	{ 1 },	/* 36 = reg: VSUB(reg,reg) */
	{ 1 },	/* 37 = reg: VDIV(reg,reg) */
	{ 1 },	/* 38 = reg: vcvt_signedToFloatingPoint(reg) */
	{ 1 },	/* 39 = reg: vcvt_floatingPointToSigned(reg) */
	{ 1 },	/* 40 = reg: VNEG(reg) */
	{ 1 },	/* 41 = stm: VCMP(reg,reg) */
	{ 1 },	/* 42 = stm: VCMPz(reg) */
	{ 1 },	/* 43 = stm: VMRS */
	{ 1 },	/* 44 = reg: VLDR(INDIRI4(reg)) */
	{ 1 },	/* 45 = reg: VLDR(INDIRI4(ADD(reg,imm10))) */
	{ 1 },	/* 46 = stm: VSTR(reg,INDIRI4(reg)) */
	{ 1 },	/* 47 = stm: VSTR(reg,INDIRI4(ADD(reg,imm10))) */
	{ 1 },	/* 48 = stm: VPUSH */
	{ 1 },	/* 49 = stm: VPOP */
	{ 1 },	/* 50 = reg: LSL(reg,imm5) */
	{ 1 },	/* 51 = reg: ASR(reg,imm5) */
	{ 1 },	/* 52 = reg: LDR(INDIRI4(ADD(reg,LSL(reg,imm5)))) */
	{ 1 },	/* 53 = stm: STR(reg,INDIRI4(ADD(reg,LSL(reg,imm5)))) */
	{ 1 },	/* 54 = reg: ADD(reg,LSL(reg,imm5)) */
	{ 1 },	/* 55 = reg: ADD(reg,LSR(reg,imm5)) */
	{ 1 },	/* 56 = reg: ADD(reg,ASR(reg,imm5)) */
	{ 1 },	/* 57 = reg: SUB(reg,LSL(reg,imm5)) */
	{ 1 },	/* 58 = reg: SUB(reg,LSR(reg,imm5)) */
	{ 1 },	/* 59 = reg: SUB(reg,ASR(reg,imm5)) */
	{ 1 },	/* 60 = stm: CMP(reg,LSL(reg,imm5)) */
	{ 1 },	/* 61 = stm: CMP(reg,LSR(reg,imm5)) */
	{ 1 },	/* 62 = stm: CMP(reg,ASR(reg,imm5)) */
	{ 1 },	/* 63 = reg: MOV(LSL(reg,imm5)) */
	{ 1 },	/* 64 = reg: MOV(LSR(reg,imm5)) */
	{ 1 },	/* 65 = reg: MOV(ASR(reg,imm5)) */
};

char *burmArm32_string[] = {
	/* 0 */	0,
	/* 1 */	"reg: LDR(CNSTI4)",
	/* 2 */	"reg: LDR(INDIRI4(reg))",
	/* 3 */	"reg: LDR(INDIRI4(ADD(reg,reg)))",
	/* 4 */	"reg: LDR(INDIRI4(ADD(reg,imm12)))",
	/* 5 */	"reg: LDR(LABELV)",
	/* 6 */	"stm: STR(reg,INDIRI4(reg))",
	/* 7 */	"stm: STR(reg,INDIRI4(ADD(reg,reg)))",
	/* 8 */	"stm: STR(reg,INDIRI4(ADD(reg,imm12)))",
	/* 9 */	"reg: ADD(reg,reg)",
	/* 10 */	"reg: ADD(reg,simm8)",
	/* 11 */	"reg: SDIV(reg,reg)",
	/* 12 */	"reg: MUL(reg,reg)",
	/* 13 */	"reg: SUB(reg,reg)",
	/* 14 */	"reg: SUB(reg,simm8)",
	/* 15 */	"reg: ADD(MUL(reg,reg),reg)",
	/* 16 */	"reg: SUB(reg,MUL(reg,reg))",
	/* 17 */	"stm: CMP(reg,reg)",
	/* 18 */	"stm: CMP(reg,simm8)",
	/* 19 */	"stm: JUMPV(CNSTI4)",
	/* 20 */	"reg: REGISTER",
	/* 21 */	"reg: RSB(reg,reg)",
	/* 22 */	"reg: RSB(reg,simm8)",
	/* 23 */	"reg: MOV(reg)",
	/* 24 */	"reg: MOV(imm16)",
	/* 25 */	"reg: CLZ(reg)",
	/* 26 */	"reg: LSR(reg,imm5)",
	/* 27 */	"stm: BL(LABELV)",
	/* 28 */	"stm: LABEL",
	/* 29 */	"stm: BX(reg)",
	/* 30 */	"stm: PUSH",
	/* 31 */	"stm: POP",
	/* 32 */	"stm: STRING",
	/* 33 */	"reg: VMOV(reg)",
	/* 34 */	"reg: VMUL(reg,reg)",
	/* 35 */	"reg: VADD(reg,reg)",
	/* 36 */	"reg: VSUB(reg,reg)",
	/* 37 */	"reg: VDIV(reg,reg)",
	/* 38 */	"reg: vcvt_signedToFloatingPoint(reg)",
	/* 39 */	"reg: vcvt_floatingPointToSigned(reg)",
	/* 40 */	"reg: VNEG(reg)",
	/* 41 */	"stm: VCMP(reg,reg)",
	/* 42 */	"stm: VCMPz(reg)",
	/* 43 */	"stm: VMRS",
	/* 44 */	"reg: VLDR(INDIRI4(reg))",
	/* 45 */	"reg: VLDR(INDIRI4(ADD(reg,imm10)))",
	/* 46 */	"stm: VSTR(reg,INDIRI4(reg))",
	/* 47 */	"stm: VSTR(reg,INDIRI4(ADD(reg,imm10)))",
	/* 48 */	"stm: VPUSH",
	/* 49 */	"stm: VPOP",
	/* 50 */	"reg: LSL(reg,imm5)",
	/* 51 */	"reg: ASR(reg,imm5)",
	/* 52 */	"reg: LDR(INDIRI4(ADD(reg,LSL(reg,imm5))))",
	/* 53 */	"stm: STR(reg,INDIRI4(ADD(reg,LSL(reg,imm5))))",
	/* 54 */	"reg: ADD(reg,LSL(reg,imm5))",
	/* 55 */	"reg: ADD(reg,LSR(reg,imm5))",
	/* 56 */	"reg: ADD(reg,ASR(reg,imm5))",
	/* 57 */	"reg: SUB(reg,LSL(reg,imm5))",
	/* 58 */	"reg: SUB(reg,LSR(reg,imm5))",
	/* 59 */	"reg: SUB(reg,ASR(reg,imm5))",
	/* 60 */	"stm: CMP(reg,LSL(reg,imm5))",
	/* 61 */	"stm: CMP(reg,LSR(reg,imm5))",
	/* 62 */	"stm: CMP(reg,ASR(reg,imm5))",
	/* 63 */	"reg: MOV(LSL(reg,imm5))",
	/* 64 */	"reg: MOV(LSR(reg,imm5))",
	/* 65 */	"reg: MOV(ASR(reg,imm5))",
};

static short burmArm32_decode_stm[] = {
	0,
	6,
	7,
	8,
	17,
	18,
	19,
	27,
	29,
	28,
	30,
	31,
	32,
	41,
	42,
	43,
	46,
	47,
	48,
	49,
	53,
	60,
	61,
	62,
};

static short burmArm32_decode_reg[] = {
	0,
	1,
	2,
	3,
	4,
	5,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	40,
	44,
	45,
	50,
	51,
	52,
	54,
	55,
	56,
	57,
	58,
	59,
	63,
	64,
	65,
};

int burmArm32_rule(STATE_TYPE state, int goalnt) {
	burmArm32_assert(goalnt >= 1 && goalnt <= 2, PANIC("Bad goal nonterminal %d in burmArm32_rule\n", goalnt));
	if (!state)
		return 0;
	switch (goalnt) {
	case burmArm32_stm_NT:	return burmArm32_decode_stm[((struct burmArm32_state *)state)->rule.burmArm32_stm];
	case burmArm32_reg_NT:	return burmArm32_decode_reg[((struct burmArm32_state *)state)->rule.burmArm32_reg];
	default:
		burmArm32_assert(0, PANIC("Bad goal nonterminal %d in burmArm32_rule\n", goalnt));
	}
	return 0;
}


STATE_TYPE burmArm32_state(int op, STATE_TYPE left, STATE_TYPE right) {
	int c;
	struct burmArm32_state *p, *l = (struct burmArm32_state *)left,
		*r = (struct burmArm32_state *)right;

	assert(sizeof (STATE_TYPE) >= sizeof (void *));
	if (burmArm32_arity[op] > 0) {
		p = ALLOC(sizeof *p);
		burmArm32_assert(p, PANIC("ALLOC returned NULL in burmArm32_state\n"));
		p->op = op;
		p->left = l;
		p->right = r;
		p->rule.burmArm32_stm = 0;
		p->cost[1] =
		p->cost[2] =
			32767;
	}
	switch (op) {
	case 1: /* imm12 */
		{
			static struct burmArm32_state z = { 1, 0, 0,
				{	0,
					32767,
					32767,
				},{
					0,
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 2: /* simm8 */
		{
			static struct burmArm32_state z = { 2, 0, 0,
				{	0,
					32767,
					32767,
				},{
					0,
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 3: /* CNSTI4 */
		{
			static struct burmArm32_state z = { 3, 0, 0,
				{	0,
					32767,
					32767,
				},{
					0,
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 4: /* INDIRI4 */
		assert(l);
		break;
	case 5: /* ADD */
		assert(l && r);
		if (	/* reg: ADD(reg,ASR(reg,imm5)) */
			r->op == 43 && /* ASR */
			r->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 36;
			}
		}
		if (	/* reg: ADD(reg,LSR(reg,imm5)) */
			r->op == 18 && /* LSR */
			r->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 35;
			}
		}
		if (	/* reg: ADD(reg,LSL(reg,imm5)) */
			r->op == 42 && /* LSL */
			r->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 34;
			}
		}
		if (	/* reg: ADD(MUL(reg,reg),reg) */
			l->op == 7 /* MUL */
		) {
			c = l->left->cost[burmArm32_reg_NT] + l->right->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 12;
			}
		}
		if (	/* reg: ADD(reg,simm8) */
			r->op == 2 /* simm8 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 7;
			}
		}
		{	/* reg: ADD(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 6;
			}
		}
		break;
	case 6: /* SDIV */
		assert(l && r);
		{	/* reg: SDIV(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 8;
			}
		}
		break;
	case 7: /* MUL */
		assert(l && r);
		{	/* reg: MUL(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 9;
			}
		}
		break;
	case 8: /* SUB */
		assert(l && r);
		if (	/* reg: SUB(reg,ASR(reg,imm5)) */
			r->op == 43 && /* ASR */
			r->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 39;
			}
		}
		if (	/* reg: SUB(reg,LSR(reg,imm5)) */
			r->op == 18 && /* LSR */
			r->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 38;
			}
		}
		if (	/* reg: SUB(reg,LSL(reg,imm5)) */
			r->op == 42 && /* LSL */
			r->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 37;
			}
		}
		if (	/* reg: SUB(reg,MUL(reg,reg)) */
			r->op == 7 /* MUL */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + r->right->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 13;
			}
		}
		if (	/* reg: SUB(reg,simm8) */
			r->op == 2 /* simm8 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 11;
			}
		}
		{	/* reg: SUB(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 10;
			}
		}
		break;
	case 9: /* JUMPV */
		assert(l);
		if (	/* stm: JUMPV(CNSTI4) */
			l->op == 3 /* CNSTI4 */
		) {
			c = 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 6;
			}
		}
		break;
	case 10: /* LABELV */
		{
			static struct burmArm32_state z = { 10, 0, 0,
				{	0,
					32767,
					32767,
				},{
					0,
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 11: /* LDR */
		assert(l);
		if (	/* reg: LDR(INDIRI4(ADD(reg,LSL(reg,imm5)))) */
			l->op == 4 && /* INDIRI4 */
			l->left->op == 5 && /* ADD */
			l->left->right->op == 42 && /* LSL */
			l->left->right->right->op == 44 /* imm5 */
		) {
			c = l->left->left->cost[burmArm32_reg_NT] + l->left->right->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 33;
			}
		}
		if (	/* reg: LDR(LABELV) */
			l->op == 10 /* LABELV */
		) {
			c = 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 5;
			}
		}
		if (	/* reg: LDR(INDIRI4(ADD(reg,imm12))) */
			l->op == 4 && /* INDIRI4 */
			l->left->op == 5 && /* ADD */
			l->left->right->op == 1 /* imm12 */
		) {
			c = l->left->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 4;
			}
		}
		if (	/* reg: LDR(INDIRI4(ADD(reg,reg))) */
			l->op == 4 && /* INDIRI4 */
			l->left->op == 5 /* ADD */
		) {
			c = l->left->left->cost[burmArm32_reg_NT] + l->left->right->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 3;
			}
		}
		if (	/* reg: LDR(INDIRI4(reg)) */
			l->op == 4 /* INDIRI4 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 2;
			}
		}
		if (	/* reg: LDR(CNSTI4) */
			l->op == 3 /* CNSTI4 */
		) {
			c = 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 1;
			}
		}
		break;
	case 12: /* STR */
		assert(l && r);
		if (	/* stm: STR(reg,INDIRI4(ADD(reg,LSL(reg,imm5)))) */
			r->op == 4 && /* INDIRI4 */
			r->left->op == 5 && /* ADD */
			r->left->right->op == 42 && /* LSL */
			r->left->right->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->left->cost[burmArm32_reg_NT] + r->left->right->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 20;
			}
		}
		if (	/* stm: STR(reg,INDIRI4(ADD(reg,imm12))) */
			r->op == 4 && /* INDIRI4 */
			r->left->op == 5 && /* ADD */
			r->left->right->op == 1 /* imm12 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 3;
			}
		}
		if (	/* stm: STR(reg,INDIRI4(ADD(reg,reg))) */
			r->op == 4 && /* INDIRI4 */
			r->left->op == 5 /* ADD */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->left->cost[burmArm32_reg_NT] + r->left->right->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 2;
			}
		}
		if (	/* stm: STR(reg,INDIRI4(reg)) */
			r->op == 4 /* INDIRI4 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 1;
			}
		}
		break;
	case 13: /* REGISTER */
		{
			static struct burmArm32_state z = { 13, 0, 0,
				{	0,
					32767,
					0,	/* reg: REGISTER */
				},{
					0,
					14,	/* reg: REGISTER */
				}
			};
			return (STATE_TYPE)&z;
		}
	case 14: /* RSB */
		assert(l && r);
		if (	/* reg: RSB(reg,simm8) */
			r->op == 2 /* simm8 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 16;
			}
		}
		{	/* reg: RSB(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 15;
			}
		}
		break;
	case 15: /* CMP */
		assert(l && r);
		if (	/* stm: CMP(reg,ASR(reg,imm5)) */
			r->op == 43 && /* ASR */
			r->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 23;
			}
		}
		if (	/* stm: CMP(reg,LSR(reg,imm5)) */
			r->op == 18 && /* LSR */
			r->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 22;
			}
		}
		if (	/* stm: CMP(reg,LSL(reg,imm5)) */
			r->op == 42 && /* LSL */
			r->right->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 21;
			}
		}
		if (	/* stm: CMP(reg,simm8) */
			r->op == 2 /* simm8 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 5;
			}
		}
		{	/* stm: CMP(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 4;
			}
		}
		break;
	case 16: /* imm16 */
		{
			static struct burmArm32_state z = { 16, 0, 0,
				{	0,
					32767,
					32767,
				},{
					0,
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 17: /* CLZ */
		assert(l);
		{	/* reg: CLZ(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 19;
			}
		}
		break;
	case 18: /* LSR */
		assert(l && r);
		if (	/* reg: LSR(reg,imm5) */
			r->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 20;
			}
		}
		break;
	case 19: /* MOV */
		assert(l);
		if (	/* reg: MOV(ASR(reg,imm5)) */
			l->op == 43 && /* ASR */
			l->right->op == 44 /* imm5 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 42;
			}
		}
		if (	/* reg: MOV(LSR(reg,imm5)) */
			l->op == 18 && /* LSR */
			l->right->op == 44 /* imm5 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 41;
			}
		}
		if (	/* reg: MOV(LSL(reg,imm5)) */
			l->op == 42 && /* LSL */
			l->right->op == 44 /* imm5 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 40;
			}
		}
		if (	/* reg: MOV(imm16) */
			l->op == 16 /* imm16 */
		) {
			c = 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 18;
			}
		}
		{	/* reg: MOV(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 17;
			}
		}
		break;
	case 20: /* BL */
		assert(l);
		if (	/* stm: BL(LABELV) */
			l->op == 10 /* LABELV */
		) {
			c = 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 7;
			}
		}
		break;
	case 21: /* LABEL */
		{
			static struct burmArm32_state z = { 21, 0, 0,
				{	0,
					0,	/* stm: LABEL */
					32767,
				},{
					9,	/* stm: LABEL */
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 22: /* BX */
		assert(l);
		{	/* stm: BX(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 8;
			}
		}
		break;
	case 23: /* PUSH */
		{
			static struct burmArm32_state z = { 23, 0, 0,
				{	0,
					1,	/* stm: PUSH */
					32767,
				},{
					10,	/* stm: PUSH */
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 24: /* POP */
		{
			static struct burmArm32_state z = { 24, 0, 0,
				{	0,
					1,	/* stm: POP */
					32767,
				},{
					11,	/* stm: POP */
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 25: /* STRING */
		{
			static struct burmArm32_state z = { 25, 0, 0,
				{	0,
					0,	/* stm: STRING */
					32767,
				},{
					12,	/* stm: STRING */
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 26: /* VMOV */
		assert(l);
		{	/* reg: VMOV(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 21;
			}
		}
		break;
	case 27: /* VMUL */
		assert(l && r);
		{	/* reg: VMUL(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 22;
			}
		}
		break;
	case 28: /* VADD */
		assert(l && r);
		{	/* reg: VADD(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 23;
			}
		}
		break;
	case 29: /* VSUB */
		assert(l && r);
		{	/* reg: VSUB(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 24;
			}
		}
		break;
	case 30: /* VDIV */
		assert(l && r);
		{	/* reg: VDIV(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 25;
			}
		}
		break;
	case 31: /* vcvt_signedToFloatingPoint */
		assert(l);
		{	/* reg: vcvt_signedToFloatingPoint(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 26;
			}
		}
		break;
	case 32: /* vcvt_floatingPointToSigned */
		assert(l);
		{	/* reg: vcvt_floatingPointToSigned(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 27;
			}
		}
		break;
	case 33: /* VNEG */
		assert(l);
		{	/* reg: VNEG(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 28;
			}
		}
		break;
	case 34: /* VCMP */
		assert(l && r);
		{	/* stm: VCMP(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 13;
			}
		}
		break;
	case 35: /* VCMPz */
		assert(l);
		{	/* stm: VCMPz(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 14;
			}
		}
		break;
	case 36: /* VMRS */
		{
			static struct burmArm32_state z = { 36, 0, 0,
				{	0,
					1,	/* stm: VMRS */
					32767,
				},{
					15,	/* stm: VMRS */
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 37: /* VSTR */
		assert(l && r);
		if (	/* stm: VSTR(reg,INDIRI4(ADD(reg,imm10))) */
			r->op == 4 && /* INDIRI4 */
			r->left->op == 5 && /* ADD */
			r->left->right->op == 39 /* imm10 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 17;
			}
		}
		if (	/* stm: VSTR(reg,INDIRI4(reg)) */
			r->op == 4 /* INDIRI4 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 16;
			}
		}
		break;
	case 38: /* VLDR */
		assert(l);
		if (	/* reg: VLDR(INDIRI4(ADD(reg,imm10))) */
			l->op == 4 && /* INDIRI4 */
			l->left->op == 5 && /* ADD */
			l->left->right->op == 39 /* imm10 */
		) {
			c = l->left->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 30;
			}
		}
		if (	/* reg: VLDR(INDIRI4(reg)) */
			l->op == 4 /* INDIRI4 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 29;
			}
		}
		break;
	case 39: /* imm10 */
		{
			static struct burmArm32_state z = { 39, 0, 0,
				{	0,
					32767,
					32767,
				},{
					0,
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 40: /* VPUSH */
		{
			static struct burmArm32_state z = { 40, 0, 0,
				{	0,
					1,	/* stm: VPUSH */
					32767,
				},{
					18,	/* stm: VPUSH */
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 41: /* VPOP */
		{
			static struct burmArm32_state z = { 41, 0, 0,
				{	0,
					1,	/* stm: VPOP */
					32767,
				},{
					19,	/* stm: VPOP */
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	case 42: /* LSL */
		assert(l && r);
		if (	/* reg: LSL(reg,imm5) */
			r->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 31;
			}
		}
		break;
	case 43: /* ASR */
		assert(l && r);
		if (	/* reg: ASR(reg,imm5) */
			r->op == 44 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 32;
			}
		}
		break;
	case 44: /* imm5 */
		{
			static struct burmArm32_state z = { 44, 0, 0,
				{	0,
					32767,
					32767,
				},{
					0,
					0,
				}
			};
			return (STATE_TYPE)&z;
		}
	default:
		burmArm32_assert(0, PANIC("Bad operator %d in burmArm32_state\n", op));
	}
	return (STATE_TYPE)p;
}

#ifdef STATE_LABEL
static void burmArm32_label1(NODEPTR_TYPE p) {
	burmArm32_assert(p, PANIC("NULL tree in burmArm32_label\n"));
	switch (burmArm32_arity[OP_LABEL(p)]) {
	case 0:
		STATE_LABEL(p) = burmArm32_state(OP_LABEL(p), 0, 0);
		break;
	case 1:
		burmArm32_label1(LEFT_CHILD(p));
		STATE_LABEL(p) = burmArm32_state(OP_LABEL(p),
			STATE_LABEL(LEFT_CHILD(p)), 0);
		break;
	case 2:
		burmArm32_label1(LEFT_CHILD(p));
		burmArm32_label1(RIGHT_CHILD(p));
		STATE_LABEL(p) = burmArm32_state(OP_LABEL(p),
			STATE_LABEL(LEFT_CHILD(p)),
			STATE_LABEL(RIGHT_CHILD(p)));
		break;
	}
}

STATE_TYPE burmArm32_label(NODEPTR_TYPE p) {
	burmArm32_label1(p);
	return ((struct burmArm32_state *)STATE_LABEL(p))->rule.burmArm32_stm ? STATE_LABEL(p) : 0;
}

NODEPTR_TYPE *burmArm32_kids(NODEPTR_TYPE p, int eruleno, NODEPTR_TYPE kids[]) {
	burmArm32_assert(p, PANIC("NULL tree in burmArm32_kids\n"));
	burmArm32_assert(kids, PANIC("NULL kids in burmArm32_kids\n"));
	switch (eruleno) {
	case 49: /* stm: VPOP */
	case 48: /* stm: VPUSH */
	case 43: /* stm: VMRS */
	case 32: /* stm: STRING */
	case 31: /* stm: POP */
	case 30: /* stm: PUSH */
	case 28: /* stm: LABEL */
	case 27: /* stm: BL(LABELV) */
	case 24: /* reg: MOV(imm16) */
	case 20: /* reg: REGISTER */
	case 19: /* stm: JUMPV(CNSTI4) */
	case 5: /* reg: LDR(LABELV) */
	case 1: /* reg: LDR(CNSTI4) */
		break;
	case 65: /* reg: MOV(ASR(reg,imm5)) */
	case 64: /* reg: MOV(LSR(reg,imm5)) */
	case 63: /* reg: MOV(LSL(reg,imm5)) */
	case 44: /* reg: VLDR(INDIRI4(reg)) */
	case 2: /* reg: LDR(INDIRI4(reg)) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(p));
		break;
	case 3: /* reg: LDR(INDIRI4(ADD(reg,reg))) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(LEFT_CHILD(p)));
		kids[1] = RIGHT_CHILD(LEFT_CHILD(LEFT_CHILD(p)));
		break;
	case 45: /* reg: VLDR(INDIRI4(ADD(reg,imm10))) */
	case 4: /* reg: LDR(INDIRI4(ADD(reg,imm12))) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(LEFT_CHILD(p)));
		break;
	case 62: /* stm: CMP(reg,ASR(reg,imm5)) */
	case 61: /* stm: CMP(reg,LSR(reg,imm5)) */
	case 60: /* stm: CMP(reg,LSL(reg,imm5)) */
	case 59: /* reg: SUB(reg,ASR(reg,imm5)) */
	case 58: /* reg: SUB(reg,LSR(reg,imm5)) */
	case 57: /* reg: SUB(reg,LSL(reg,imm5)) */
	case 56: /* reg: ADD(reg,ASR(reg,imm5)) */
	case 55: /* reg: ADD(reg,LSR(reg,imm5)) */
	case 54: /* reg: ADD(reg,LSL(reg,imm5)) */
	case 46: /* stm: VSTR(reg,INDIRI4(reg)) */
	case 6: /* stm: STR(reg,INDIRI4(reg)) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(RIGHT_CHILD(p));
		break;
	case 7: /* stm: STR(reg,INDIRI4(ADD(reg,reg))) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(LEFT_CHILD(RIGHT_CHILD(p)));
		kids[2] = RIGHT_CHILD(LEFT_CHILD(RIGHT_CHILD(p)));
		break;
	case 47: /* stm: VSTR(reg,INDIRI4(ADD(reg,imm10))) */
	case 8: /* stm: STR(reg,INDIRI4(ADD(reg,imm12))) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(LEFT_CHILD(RIGHT_CHILD(p)));
		break;
	case 41: /* stm: VCMP(reg,reg) */
	case 37: /* reg: VDIV(reg,reg) */
	case 36: /* reg: VSUB(reg,reg) */
	case 35: /* reg: VADD(reg,reg) */
	case 34: /* reg: VMUL(reg,reg) */
	case 21: /* reg: RSB(reg,reg) */
	case 17: /* stm: CMP(reg,reg) */
	case 13: /* reg: SUB(reg,reg) */
	case 12: /* reg: MUL(reg,reg) */
	case 11: /* reg: SDIV(reg,reg) */
	case 9: /* reg: ADD(reg,reg) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = RIGHT_CHILD(p);
		break;
	case 51: /* reg: ASR(reg,imm5) */
	case 50: /* reg: LSL(reg,imm5) */
	case 42: /* stm: VCMPz(reg) */
	case 40: /* reg: VNEG(reg) */
	case 39: /* reg: vcvt_floatingPointToSigned(reg) */
	case 38: /* reg: vcvt_signedToFloatingPoint(reg) */
	case 33: /* reg: VMOV(reg) */
	case 29: /* stm: BX(reg) */
	case 26: /* reg: LSR(reg,imm5) */
	case 25: /* reg: CLZ(reg) */
	case 23: /* reg: MOV(reg) */
	case 22: /* reg: RSB(reg,simm8) */
	case 18: /* stm: CMP(reg,simm8) */
	case 14: /* reg: SUB(reg,simm8) */
	case 10: /* reg: ADD(reg,simm8) */
		kids[0] = LEFT_CHILD(p);
		break;
	case 15: /* reg: ADD(MUL(reg,reg),reg) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(p));
		kids[1] = RIGHT_CHILD(LEFT_CHILD(p));
		kids[2] = RIGHT_CHILD(p);
		break;
	case 16: /* reg: SUB(reg,MUL(reg,reg)) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(RIGHT_CHILD(p));
		kids[2] = RIGHT_CHILD(RIGHT_CHILD(p));
		break;
	case 52: /* reg: LDR(INDIRI4(ADD(reg,LSL(reg,imm5)))) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(LEFT_CHILD(p)));
		kids[1] = LEFT_CHILD(RIGHT_CHILD(LEFT_CHILD(LEFT_CHILD(p))));
		break;
	case 53: /* stm: STR(reg,INDIRI4(ADD(reg,LSL(reg,imm5)))) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(LEFT_CHILD(RIGHT_CHILD(p)));
		kids[2] = LEFT_CHILD(RIGHT_CHILD(LEFT_CHILD(RIGHT_CHILD(p))));
		break;
	default:
		burmArm32_assert(0, PANIC("Bad external rule number %d in burmArm32_kids\n", eruleno));
	}
	return kids;
}

int burmArm32_op_label(NODEPTR_TYPE p) {
	burmArm32_assert(p, PANIC("NULL tree in burmArm32_op_label\n"));
	return OP_LABEL(p);
}

STATE_TYPE burmArm32_state_label(NODEPTR_TYPE p) {
	burmArm32_assert(p, PANIC("NULL tree in burmArm32_state_label\n"));
	return STATE_LABEL(p);
}

NODEPTR_TYPE burmArm32_child(NODEPTR_TYPE p, int index) {
	burmArm32_assert(p, PANIC("NULL tree in burmArm32_child\n"));
	switch (index) {
	case 0:	return LEFT_CHILD(p);
	case 1:	return RIGHT_CHILD(p);
	}
	burmArm32_assert(0, PANIC("Bad index %d in burmArm32_child\n", index));
	return 0;
}

#endif
