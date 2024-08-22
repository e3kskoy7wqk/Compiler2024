#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "all.h"

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
	0,	/* 1 */
	0,	/* 2 */
	0,	/* 3 */
	burmArm32_nts_0,	/* 4 */
	burmArm32_nts_1,	/* 5 */
	burmArm32_nts_2,	/* 6 */
	burmArm32_nts_1,	/* 7 */
	burmArm32_nts_0,	/* 8 */
	0,	/* 9 */
	burmArm32_nts_2,	/* 10 */
	burmArm32_nts_3,	/* 11 */
	burmArm32_nts_2,	/* 12 */
	burmArm32_nts_2,	/* 13 */
	burmArm32_nts_1,	/* 14 */
	burmArm32_nts_2,	/* 15 */
	0,	/* 16 */
	0,	/* 17 */
	burmArm32_nts_2,	/* 18 */
	burmArm32_nts_2,	/* 19 */
	burmArm32_nts_1,	/* 20 */
	burmArm32_nts_3,	/* 21 */
	burmArm32_nts_3,	/* 22 */
	burmArm32_nts_2,	/* 23 */
	burmArm32_nts_1,	/* 24 */
	0,	/* 25 */
	0,	/* 26 */
	0,	/* 27 */
	0,	/* 28 */
	0,	/* 29 */
	0,	/* 30 */
	burmArm32_nts_0,	/* 31 */
	0,	/* 32 */
	0,	/* 33 */
	burmArm32_nts_0,	/* 34 */
	burmArm32_nts_2,	/* 35 */
	burmArm32_nts_1,	/* 36 */
	0,	/* 37 */
	0,	/* 38 */
	0,	/* 39 */
	0,	/* 40 */
	0,	/* 41 */
	0,	/* 42 */
	0,	/* 43 */
	0,	/* 44 */
	0,	/* 45 */
	0,	/* 46 */
	0,	/* 47 */
	0,	/* 48 */
	0,	/* 49 */
	0,	/* 50 */
	0,	/* 51 */
	burmArm32_nts_1,	/* 52 */
	0,	/* 53 */
	burmArm32_nts_0,	/* 54 */
	burmArm32_nts_1,	/* 55 */
	burmArm32_nts_1,	/* 56 */
	burmArm32_nts_0,	/* 57 */
	burmArm32_nts_0,	/* 58 */
	burmArm32_nts_1,	/* 59 */
	burmArm32_nts_0,	/* 60 */
	burmArm32_nts_0,	/* 61 */
	burmArm32_nts_0,	/* 62 */
	burmArm32_nts_1,	/* 63 */
	burmArm32_nts_2,	/* 64 */
	burmArm32_nts_2,	/* 65 */
	burmArm32_nts_2,	/* 66 */
	burmArm32_nts_2,	/* 67 */
	burmArm32_nts_1,	/* 68 */
	burmArm32_nts_1,	/* 69 */
	burmArm32_nts_1,	/* 70 */
	burmArm32_nts_2,	/* 71 */
	burmArm32_nts_1,	/* 72 */
	burmArm32_nts_0,	/* 73 */
	burmArm32_nts_1,	/* 74 */
	burmArm32_nts_1,	/* 75 */
	burmArm32_nts_2,	/* 76 */
	burmArm32_nts_2,	/* 77 */
	burmArm32_nts_0,	/* 78 */
	burmArm32_nts_0,	/* 79 */
	burmArm32_nts_1,	/* 80 */
	burmArm32_nts_1,	/* 81 */
	burmArm32_nts_2,	/* 82 */
	burmArm32_nts_3,	/* 83 */
	burmArm32_nts_2,	/* 84 */
	burmArm32_nts_2,	/* 85 */
	burmArm32_nts_2,	/* 86 */
	burmArm32_nts_2,	/* 87 */
	burmArm32_nts_2,	/* 88 */
	burmArm32_nts_2,	/* 89 */
	burmArm32_nts_2,	/* 90 */
	burmArm32_nts_2,	/* 91 */
	burmArm32_nts_2,	/* 92 */
	burmArm32_nts_1,	/* 93 */
	burmArm32_nts_1,	/* 94 */
	burmArm32_nts_1,	/* 95 */
};

char burmArm32_arity[] = {
	0,	/* 0 */
	0,	/* 1=imm12 */
	0,	/* 2=simm8 */
	0,	/* 3 */
	0,	/* 4=CNSTI4 */
	0,	/* 5 */
	0,	/* 6 */
	1,	/* 7=INDIRI4 */
	0,	/* 8 */
	2,	/* 9=ADD */
	2,	/* 10=SDIV */
	2,	/* 11=MUL */
	2,	/* 12=SUB */
	0,	/* 13 */
	0,	/* 14 */
	0,	/* 15 */
	0,	/* 16 */
	0,	/* 17 */
	0,	/* 18 */
	1,	/* 19=JUMPV */
	0,	/* 20=LABELV */
	1,	/* 21=LDR */
	0,	/* 22 */
	0,	/* 23 */
	2,	/* 24=STR */
	0,	/* 25 */
	0,	/* 26 */
	0,	/* 27 */
	0,	/* 28=REGISTER */
	2,	/* 29=RSB */
	2,	/* 30=CMP */
	0,	/* 31=imm16 */
	0,	/* 32 */
	0,	/* 33 */
	0,	/* 34 */
	0,	/* 35 */
	0,	/* 36 */
	0,	/* 37 */
	1,	/* 38=CLZ */
	2,	/* 39=LSR */
	1,	/* 40=MOV */
	1,	/* 41=BL */
	0,	/* 42=LABEL */
	1,	/* 43=BX */
	0,	/* 44=PUSH */
	0,	/* 45=POP */
	0,	/* 46=STRING */
	1,	/* 47=VMOV */
	2,	/* 48=VMUL */
	2,	/* 49=VADD */
	2,	/* 50=VSUB */
	2,	/* 51=VDIV */
	1,	/* 52=vcvt_signedToFloatingPoint */
	1,	/* 53=vcvt_floatingPointToSigned */
	1,	/* 54=VNEG */
	2,	/* 55=VCMP */
	1,	/* 56=VCMPz */
	0,	/* 57=VMRS */
	2,	/* 58=VSTR */
	1,	/* 59=VLDR */
	0,	/* 60=imm10 */
	0,	/* 61=VPUSH */
	0,	/* 62=VPOP */
	2,	/* 63=LSL */
	2,	/* 64=ASR */
	0,	/* 65=imm5 */
};

char *burmArm32_opname[] = {
	/* 0 */	0,
	/* 1 */	"imm12",
	/* 2 */	"simm8",
	/* 3 */	0,
	/* 4 */	"CNSTI4",
	/* 5 */	0,
	/* 6 */	0,
	/* 7 */	"INDIRI4",
	/* 8 */	0,
	/* 9 */	"ADD",
	/* 10 */	"SDIV",
	/* 11 */	"MUL",
	/* 12 */	"SUB",
	/* 13 */	0,
	/* 14 */	0,
	/* 15 */	0,
	/* 16 */	0,
	/* 17 */	0,
	/* 18 */	0,
	/* 19 */	"JUMPV",
	/* 20 */	"LABELV",
	/* 21 */	"LDR",
	/* 22 */	0,
	/* 23 */	0,
	/* 24 */	"STR",
	/* 25 */	0,
	/* 26 */	0,
	/* 27 */	0,
	/* 28 */	"REGISTER",
	/* 29 */	"RSB",
	/* 30 */	"CMP",
	/* 31 */	"imm16",
	/* 32 */	0,
	/* 33 */	0,
	/* 34 */	0,
	/* 35 */	0,
	/* 36 */	0,
	/* 37 */	0,
	/* 38 */	"CLZ",
	/* 39 */	"LSR",
	/* 40 */	"MOV",
	/* 41 */	"BL",
	/* 42 */	"LABEL",
	/* 43 */	"BX",
	/* 44 */	"PUSH",
	/* 45 */	"POP",
	/* 46 */	"STRING",
	/* 47 */	"VMOV",
	/* 48 */	"VMUL",
	/* 49 */	"VADD",
	/* 50 */	"VSUB",
	/* 51 */	"VDIV",
	/* 52 */	"vcvt_signedToFloatingPoint",
	/* 53 */	"vcvt_floatingPointToSigned",
	/* 54 */	"VNEG",
	/* 55 */	"VCMP",
	/* 56 */	"VCMPz",
	/* 57 */	"VMRS",
	/* 58 */	"VSTR",
	/* 59 */	"VLDR",
	/* 60 */	"imm10",
	/* 61 */	"VPUSH",
	/* 62 */	"VPOP",
	/* 63 */	"LSL",
	/* 64 */	"ASR",
	/* 65 */	"imm5",
};

short burmArm32_cost[][4] = {
	{ 0 },	/* 0 */
	{ 0 },	/* 1 */
	{ 0 },	/* 2 */
	{ 0 },	/* 3 */
	{ 1 },	/* 4 = reg: LDR(CNSTI4) */
	{ 1 },	/* 5 = reg: LDR(INDIRI4(reg)) */
	{ 1 },	/* 6 = reg: LDR(INDIRI4(ADD(reg,reg))) */
	{ 1 },	/* 7 = reg: LDR(INDIRI4(ADD(reg,imm12))) */
	{ 1 },	/* 8 = reg: LDR(LABELV) */
	{ 0 },	/* 9 */
	{ 1 },	/* 10 = stm: STR(reg,INDIRI4(reg)) */
	{ 1 },	/* 11 = stm: STR(reg,INDIRI4(ADD(reg,reg))) */
	{ 1 },	/* 12 = stm: STR(reg,INDIRI4(ADD(reg,imm12))) */
	{ 1 },	/* 13 = reg: ADD(reg,reg) */
	{ 1 },	/* 14 = reg: ADD(reg,simm8) */
	{ 1 },	/* 15 = reg: SDIV(reg,reg) */
	{ 0 },	/* 16 */
	{ 0 },	/* 17 */
	{ 1 },	/* 18 = reg: MUL(reg,reg) */
	{ 1 },	/* 19 = reg: SUB(reg,reg) */
	{ 1 },	/* 20 = reg: SUB(reg,simm8) */
	{ 1 },	/* 21 = reg: ADD(MUL(reg,reg),reg) */
	{ 1 },	/* 22 = reg: SUB(reg,MUL(reg,reg)) */
	{ 1 },	/* 23 = stm: CMP(reg,reg) */
	{ 1 },	/* 24 = stm: CMP(reg,simm8) */
	{ 0 },	/* 25 */
	{ 0 },	/* 26 */
	{ 0 },	/* 27 */
	{ 0 },	/* 28 */
	{ 0 },	/* 29 */
	{ 0 },	/* 30 */
	{ 1 },	/* 31 = stm: JUMPV(CNSTI4) */
	{ 0 },	/* 32 */
	{ 0 },	/* 33 */
	{ 0 },	/* 34 = reg: REGISTER */
	{ 1 },	/* 35 = reg: RSB(reg,reg) */
	{ 1 },	/* 36 = reg: RSB(reg,simm8) */
	{ 0 },	/* 37 */
	{ 0 },	/* 38 */
	{ 0 },	/* 39 */
	{ 0 },	/* 40 */
	{ 0 },	/* 41 */
	{ 0 },	/* 42 */
	{ 0 },	/* 43 */
	{ 0 },	/* 44 */
	{ 0 },	/* 45 */
	{ 0 },	/* 46 */
	{ 0 },	/* 47 */
	{ 0 },	/* 48 */
	{ 0 },	/* 49 */
	{ 0 },	/* 50 */
	{ 0 },	/* 51 */
	{ 1 },	/* 52 = reg: MOV(reg) */
	{ 0 },	/* 53 */
	{ 1 },	/* 54 = reg: MOV(imm16) */
	{ 1 },	/* 55 = reg: CLZ(reg) */
	{ 1 },	/* 56 = reg: LSR(reg,imm5) */
	{ 1 },	/* 57 = stm: BL(LABELV) */
	{ 0 },	/* 58 = stm: LABEL */
	{ 1 },	/* 59 = stm: BX(reg) */
	{ 1 },	/* 60 = stm: PUSH */
	{ 1 },	/* 61 = stm: POP */
	{ 0 },	/* 62 = stm: STRING */
	{ 1 },	/* 63 = reg: VMOV(reg) */
	{ 1 },	/* 64 = reg: VMUL(reg,reg) */
	{ 1 },	/* 65 = reg: VADD(reg,reg) */
	{ 1 },	/* 66 = reg: VSUB(reg,reg) */
	{ 1 },	/* 67 = reg: VDIV(reg,reg) */
	{ 1 },	/* 68 = reg: vcvt_signedToFloatingPoint(reg) */
	{ 1 },	/* 69 = reg: vcvt_floatingPointToSigned(reg) */
	{ 1 },	/* 70 = reg: VNEG(reg) */
	{ 1 },	/* 71 = stm: VCMP(reg,reg) */
	{ 1 },	/* 72 = stm: VCMPz(reg) */
	{ 1 },	/* 73 = stm: VMRS */
	{ 1 },	/* 74 = reg: VLDR(INDIRI4(reg)) */
	{ 1 },	/* 75 = reg: VLDR(INDIRI4(ADD(reg,imm10))) */
	{ 1 },	/* 76 = stm: VSTR(reg,INDIRI4(reg)) */
	{ 1 },	/* 77 = stm: VSTR(reg,INDIRI4(ADD(reg,imm10))) */
	{ 1 },	/* 78 = stm: VPUSH */
	{ 1 },	/* 79 = stm: VPOP */
	{ 1 },	/* 80 = reg: LSL(reg,imm5) */
	{ 1 },	/* 81 = reg: ASR(reg,imm5) */
	{ 1 },	/* 82 = reg: LDR(INDIRI4(ADD(reg,LSL(reg,imm5)))) */
	{ 1 },	/* 83 = stm: STR(reg,INDIRI4(ADD(reg,LSL(reg,imm5)))) */
	{ 1 },	/* 84 = reg: ADD(reg,LSL(reg,imm5)) */
	{ 1 },	/* 85 = reg: ADD(reg,LSR(reg,imm5)) */
	{ 1 },	/* 86 = reg: ADD(reg,ASR(reg,imm5)) */
	{ 1 },	/* 87 = reg: SUB(reg,LSL(reg,imm5)) */
	{ 1 },	/* 88 = reg: SUB(reg,LSR(reg,imm5)) */
	{ 1 },	/* 89 = reg: SUB(reg,ASR(reg,imm5)) */
	{ 1 },	/* 90 = stm: CMP(reg,LSL(reg,imm5)) */
	{ 1 },	/* 91 = stm: CMP(reg,LSR(reg,imm5)) */
	{ 1 },	/* 92 = stm: CMP(reg,ASR(reg,imm5)) */
	{ 1 },	/* 93 = reg: MOV(LSL(reg,imm5)) */
	{ 1 },	/* 94 = reg: MOV(LSR(reg,imm5)) */
	{ 1 },	/* 95 = reg: MOV(ASR(reg,imm5)) */
};

char *burmArm32_string[] = {
	/* 0 */	0,
	/* 1 */	0,
	/* 2 */	0,
	/* 3 */	0,
	/* 4 */	"reg: LDR(CNSTI4)",
	/* 5 */	"reg: LDR(INDIRI4(reg))",
	/* 6 */	"reg: LDR(INDIRI4(ADD(reg,reg)))",
	/* 7 */	"reg: LDR(INDIRI4(ADD(reg,imm12)))",
	/* 8 */	"reg: LDR(LABELV)",
	/* 9 */	0,
	/* 10 */	"stm: STR(reg,INDIRI4(reg))",
	/* 11 */	"stm: STR(reg,INDIRI4(ADD(reg,reg)))",
	/* 12 */	"stm: STR(reg,INDIRI4(ADD(reg,imm12)))",
	/* 13 */	"reg: ADD(reg,reg)",
	/* 14 */	"reg: ADD(reg,simm8)",
	/* 15 */	"reg: SDIV(reg,reg)",
	/* 16 */	0,
	/* 17 */	0,
	/* 18 */	"reg: MUL(reg,reg)",
	/* 19 */	"reg: SUB(reg,reg)",
	/* 20 */	"reg: SUB(reg,simm8)",
	/* 21 */	"reg: ADD(MUL(reg,reg),reg)",
	/* 22 */	"reg: SUB(reg,MUL(reg,reg))",
	/* 23 */	"stm: CMP(reg,reg)",
	/* 24 */	"stm: CMP(reg,simm8)",
	/* 25 */	0,
	/* 26 */	0,
	/* 27 */	0,
	/* 28 */	0,
	/* 29 */	0,
	/* 30 */	0,
	/* 31 */	"stm: JUMPV(CNSTI4)",
	/* 32 */	0,
	/* 33 */	0,
	/* 34 */	"reg: REGISTER",
	/* 35 */	"reg: RSB(reg,reg)",
	/* 36 */	"reg: RSB(reg,simm8)",
	/* 37 */	0,
	/* 38 */	0,
	/* 39 */	0,
	/* 40 */	0,
	/* 41 */	0,
	/* 42 */	0,
	/* 43 */	0,
	/* 44 */	0,
	/* 45 */	0,
	/* 46 */	0,
	/* 47 */	0,
	/* 48 */	0,
	/* 49 */	0,
	/* 50 */	0,
	/* 51 */	0,
	/* 52 */	"reg: MOV(reg)",
	/* 53 */	0,
	/* 54 */	"reg: MOV(imm16)",
	/* 55 */	"reg: CLZ(reg)",
	/* 56 */	"reg: LSR(reg,imm5)",
	/* 57 */	"stm: BL(LABELV)",
	/* 58 */	"stm: LABEL",
	/* 59 */	"stm: BX(reg)",
	/* 60 */	"stm: PUSH",
	/* 61 */	"stm: POP",
	/* 62 */	"stm: STRING",
	/* 63 */	"reg: VMOV(reg)",
	/* 64 */	"reg: VMUL(reg,reg)",
	/* 65 */	"reg: VADD(reg,reg)",
	/* 66 */	"reg: VSUB(reg,reg)",
	/* 67 */	"reg: VDIV(reg,reg)",
	/* 68 */	"reg: vcvt_signedToFloatingPoint(reg)",
	/* 69 */	"reg: vcvt_floatingPointToSigned(reg)",
	/* 70 */	"reg: VNEG(reg)",
	/* 71 */	"stm: VCMP(reg,reg)",
	/* 72 */	"stm: VCMPz(reg)",
	/* 73 */	"stm: VMRS",
	/* 74 */	"reg: VLDR(INDIRI4(reg))",
	/* 75 */	"reg: VLDR(INDIRI4(ADD(reg,imm10)))",
	/* 76 */	"stm: VSTR(reg,INDIRI4(reg))",
	/* 77 */	"stm: VSTR(reg,INDIRI4(ADD(reg,imm10)))",
	/* 78 */	"stm: VPUSH",
	/* 79 */	"stm: VPOP",
	/* 80 */	"reg: LSL(reg,imm5)",
	/* 81 */	"reg: ASR(reg,imm5)",
	/* 82 */	"reg: LDR(INDIRI4(ADD(reg,LSL(reg,imm5))))",
	/* 83 */	"stm: STR(reg,INDIRI4(ADD(reg,LSL(reg,imm5))))",
	/* 84 */	"reg: ADD(reg,LSL(reg,imm5))",
	/* 85 */	"reg: ADD(reg,LSR(reg,imm5))",
	/* 86 */	"reg: ADD(reg,ASR(reg,imm5))",
	/* 87 */	"reg: SUB(reg,LSL(reg,imm5))",
	/* 88 */	"reg: SUB(reg,LSR(reg,imm5))",
	/* 89 */	"reg: SUB(reg,ASR(reg,imm5))",
	/* 90 */	"stm: CMP(reg,LSL(reg,imm5))",
	/* 91 */	"stm: CMP(reg,LSR(reg,imm5))",
	/* 92 */	"stm: CMP(reg,ASR(reg,imm5))",
	/* 93 */	"reg: MOV(LSL(reg,imm5))",
	/* 94 */	"reg: MOV(LSR(reg,imm5))",
	/* 95 */	"reg: MOV(ASR(reg,imm5))",
};

static short burmArm32_decode_stm[] = {
	0,
	10,
	11,
	12,
	23,
	24,
	31,
	57,
	59,
	58,
	60,
	61,
	62,
	71,
	72,
	73,
	76,
	77,
	78,
	79,
	83,
	90,
	91,
	92,
};

static short burmArm32_decode_reg[] = {
	0,
	4,
	5,
	6,
	7,
	8,
	13,
	14,
	15,
	18,
	19,
	20,
	21,
	22,
	34,
	35,
	36,
	52,
	54,
	55,
	56,
	63,
	64,
	65,
	66,
	67,
	68,
	69,
	70,
	74,
	75,
	80,
	81,
	82,
	84,
	85,
	86,
	87,
	88,
	89,
	93,
	94,
	95,
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
	case 4: /* CNSTI4 */
		{
			static struct burmArm32_state z = { 4, 0, 0,
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
	case 7: /* INDIRI4 */
		assert(l);
		break;
	case 9: /* ADD */
		assert(l && r);
		if (	/* reg: ADD(reg,ASR(reg,imm5)) */
			r->op == 64 && /* ASR */
			r->right->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 36;
			}
		}
		if (	/* reg: ADD(reg,LSR(reg,imm5)) */
			r->op == 39 && /* LSR */
			r->right->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 35;
			}
		}
		if (	/* reg: ADD(reg,LSL(reg,imm5)) */
			r->op == 63 && /* LSL */
			r->right->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 34;
			}
		}
		if (	/* reg: ADD(MUL(reg,reg),reg) */
			l->op == 11 /* MUL */
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
	case 10: /* SDIV */
		assert(l && r);
		{	/* reg: SDIV(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 8;
			}
		}
		break;
	case 11: /* MUL */
		assert(l && r);
		{	/* reg: MUL(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 9;
			}
		}
		break;
	case 12: /* SUB */
		assert(l && r);
		if (	/* reg: SUB(reg,ASR(reg,imm5)) */
			r->op == 64 && /* ASR */
			r->right->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 39;
			}
		}
		if (	/* reg: SUB(reg,LSR(reg,imm5)) */
			r->op == 39 && /* LSR */
			r->right->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 38;
			}
		}
		if (	/* reg: SUB(reg,LSL(reg,imm5)) */
			r->op == 63 && /* LSL */
			r->right->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 37;
			}
		}
		if (	/* reg: SUB(reg,MUL(reg,reg)) */
			r->op == 11 /* MUL */
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
	case 19: /* JUMPV */
		assert(l);
		if (	/* stm: JUMPV(CNSTI4) */
			l->op == 4 /* CNSTI4 */
		) {
			c = 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 6;
			}
		}
		break;
	case 20: /* LABELV */
		{
			static struct burmArm32_state z = { 20, 0, 0,
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
	case 21: /* LDR */
		assert(l);
		if (	/* reg: LDR(INDIRI4(ADD(reg,LSL(reg,imm5)))) */
			l->op == 7 && /* INDIRI4 */
			l->left->op == 9 && /* ADD */
			l->left->right->op == 63 && /* LSL */
			l->left->right->right->op == 65 /* imm5 */
		) {
			c = l->left->left->cost[burmArm32_reg_NT] + l->left->right->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 33;
			}
		}
		if (	/* reg: LDR(LABELV) */
			l->op == 20 /* LABELV */
		) {
			c = 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 5;
			}
		}
		if (	/* reg: LDR(INDIRI4(ADD(reg,imm12))) */
			l->op == 7 && /* INDIRI4 */
			l->left->op == 9 && /* ADD */
			l->left->right->op == 1 /* imm12 */
		) {
			c = l->left->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 4;
			}
		}
		if (	/* reg: LDR(INDIRI4(ADD(reg,reg))) */
			l->op == 7 && /* INDIRI4 */
			l->left->op == 9 /* ADD */
		) {
			c = l->left->left->cost[burmArm32_reg_NT] + l->left->right->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 3;
			}
		}
		if (	/* reg: LDR(INDIRI4(reg)) */
			l->op == 7 /* INDIRI4 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 2;
			}
		}
		if (	/* reg: LDR(CNSTI4) */
			l->op == 4 /* CNSTI4 */
		) {
			c = 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 1;
			}
		}
		break;
	case 24: /* STR */
		assert(l && r);
		if (	/* stm: STR(reg,INDIRI4(ADD(reg,LSL(reg,imm5)))) */
			r->op == 7 && /* INDIRI4 */
			r->left->op == 9 && /* ADD */
			r->left->right->op == 63 && /* LSL */
			r->left->right->right->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->left->cost[burmArm32_reg_NT] + r->left->right->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 20;
			}
		}
		if (	/* stm: STR(reg,INDIRI4(ADD(reg,imm12))) */
			r->op == 7 && /* INDIRI4 */
			r->left->op == 9 && /* ADD */
			r->left->right->op == 1 /* imm12 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 3;
			}
		}
		if (	/* stm: STR(reg,INDIRI4(ADD(reg,reg))) */
			r->op == 7 && /* INDIRI4 */
			r->left->op == 9 /* ADD */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->left->cost[burmArm32_reg_NT] + r->left->right->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 2;
			}
		}
		if (	/* stm: STR(reg,INDIRI4(reg)) */
			r->op == 7 /* INDIRI4 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 1;
			}
		}
		break;
	case 28: /* REGISTER */
		{
			static struct burmArm32_state z = { 28, 0, 0,
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
	case 29: /* RSB */
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
	case 30: /* CMP */
		assert(l && r);
		if (	/* stm: CMP(reg,ASR(reg,imm5)) */
			r->op == 64 && /* ASR */
			r->right->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 23;
			}
		}
		if (	/* stm: CMP(reg,LSR(reg,imm5)) */
			r->op == 39 && /* LSR */
			r->right->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 22;
			}
		}
		if (	/* stm: CMP(reg,LSL(reg,imm5)) */
			r->op == 63 && /* LSL */
			r->right->op == 65 /* imm5 */
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
	case 31: /* imm16 */
		{
			static struct burmArm32_state z = { 31, 0, 0,
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
	case 38: /* CLZ */
		assert(l);
		{	/* reg: CLZ(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 19;
			}
		}
		break;
	case 39: /* LSR */
		assert(l && r);
		if (	/* reg: LSR(reg,imm5) */
			r->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 20;
			}
		}
		break;
	case 40: /* MOV */
		assert(l);
		if (	/* reg: MOV(ASR(reg,imm5)) */
			l->op == 64 && /* ASR */
			l->right->op == 65 /* imm5 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 42;
			}
		}
		if (	/* reg: MOV(LSR(reg,imm5)) */
			l->op == 39 && /* LSR */
			l->right->op == 65 /* imm5 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 41;
			}
		}
		if (	/* reg: MOV(LSL(reg,imm5)) */
			l->op == 63 && /* LSL */
			l->right->op == 65 /* imm5 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 40;
			}
		}
		if (	/* reg: MOV(imm16) */
			l->op == 31 /* imm16 */
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
	case 41: /* BL */
		assert(l);
		if (	/* stm: BL(LABELV) */
			l->op == 20 /* LABELV */
		) {
			c = 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 7;
			}
		}
		break;
	case 42: /* LABEL */
		{
			static struct burmArm32_state z = { 42, 0, 0,
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
	case 43: /* BX */
		assert(l);
		{	/* stm: BX(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 8;
			}
		}
		break;
	case 44: /* PUSH */
		{
			static struct burmArm32_state z = { 44, 0, 0,
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
	case 45: /* POP */
		{
			static struct burmArm32_state z = { 45, 0, 0,
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
	case 46: /* STRING */
		{
			static struct burmArm32_state z = { 46, 0, 0,
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
	case 47: /* VMOV */
		assert(l);
		{	/* reg: VMOV(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 21;
			}
		}
		break;
	case 48: /* VMUL */
		assert(l && r);
		{	/* reg: VMUL(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 22;
			}
		}
		break;
	case 49: /* VADD */
		assert(l && r);
		{	/* reg: VADD(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 23;
			}
		}
		break;
	case 50: /* VSUB */
		assert(l && r);
		{	/* reg: VSUB(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 24;
			}
		}
		break;
	case 51: /* VDIV */
		assert(l && r);
		{	/* reg: VDIV(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 25;
			}
		}
		break;
	case 52: /* vcvt_signedToFloatingPoint */
		assert(l);
		{	/* reg: vcvt_signedToFloatingPoint(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 26;
			}
		}
		break;
	case 53: /* vcvt_floatingPointToSigned */
		assert(l);
		{	/* reg: vcvt_floatingPointToSigned(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 27;
			}
		}
		break;
	case 54: /* VNEG */
		assert(l);
		{	/* reg: VNEG(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 28;
			}
		}
		break;
	case 55: /* VCMP */
		assert(l && r);
		{	/* stm: VCMP(reg,reg) */
			c = l->cost[burmArm32_reg_NT] + r->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 13;
			}
		}
		break;
	case 56: /* VCMPz */
		assert(l);
		{	/* stm: VCMPz(reg) */
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 14;
			}
		}
		break;
	case 57: /* VMRS */
		{
			static struct burmArm32_state z = { 57, 0, 0,
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
	case 58: /* VSTR */
		assert(l && r);
		if (	/* stm: VSTR(reg,INDIRI4(ADD(reg,imm10))) */
			r->op == 7 && /* INDIRI4 */
			r->left->op == 9 && /* ADD */
			r->left->right->op == 60 /* imm10 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 17;
			}
		}
		if (	/* stm: VSTR(reg,INDIRI4(reg)) */
			r->op == 7 /* INDIRI4 */
		) {
			c = l->cost[burmArm32_reg_NT] + r->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_stm_NT]) {
				p->cost[burmArm32_stm_NT] = c + 0;
				p->rule.burmArm32_stm = 16;
			}
		}
		break;
	case 59: /* VLDR */
		assert(l);
		if (	/* reg: VLDR(INDIRI4(ADD(reg,imm10))) */
			l->op == 7 && /* INDIRI4 */
			l->left->op == 9 && /* ADD */
			l->left->right->op == 60 /* imm10 */
		) {
			c = l->left->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 30;
			}
		}
		if (	/* reg: VLDR(INDIRI4(reg)) */
			l->op == 7 /* INDIRI4 */
		) {
			c = l->left->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 29;
			}
		}
		break;
	case 60: /* imm10 */
		{
			static struct burmArm32_state z = { 60, 0, 0,
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
	case 61: /* VPUSH */
		{
			static struct burmArm32_state z = { 61, 0, 0,
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
	case 62: /* VPOP */
		{
			static struct burmArm32_state z = { 62, 0, 0,
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
	case 63: /* LSL */
		assert(l && r);
		if (	/* reg: LSL(reg,imm5) */
			r->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 31;
			}
		}
		break;
	case 64: /* ASR */
		assert(l && r);
		if (	/* reg: ASR(reg,imm5) */
			r->op == 65 /* imm5 */
		) {
			c = l->cost[burmArm32_reg_NT] + 1;
			if (c + 0 < p->cost[burmArm32_reg_NT]) {
				p->cost[burmArm32_reg_NT] = c + 0;
				p->rule.burmArm32_reg = 32;
			}
		}
		break;
	case 65: /* imm5 */
		{
			static struct burmArm32_state z = { 65, 0, 0,
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
	case 79: /* stm: VPOP */
	case 78: /* stm: VPUSH */
	case 73: /* stm: VMRS */
	case 62: /* stm: STRING */
	case 61: /* stm: POP */
	case 60: /* stm: PUSH */
	case 58: /* stm: LABEL */
	case 57: /* stm: BL(LABELV) */
	case 54: /* reg: MOV(imm16) */
	case 34: /* reg: REGISTER */
	case 31: /* stm: JUMPV(CNSTI4) */
	case 8: /* reg: LDR(LABELV) */
	case 4: /* reg: LDR(CNSTI4) */
		break;
	case 95: /* reg: MOV(ASR(reg,imm5)) */
	case 94: /* reg: MOV(LSR(reg,imm5)) */
	case 93: /* reg: MOV(LSL(reg,imm5)) */
	case 74: /* reg: VLDR(INDIRI4(reg)) */
	case 5: /* reg: LDR(INDIRI4(reg)) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(p));
		break;
	case 6: /* reg: LDR(INDIRI4(ADD(reg,reg))) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(LEFT_CHILD(p)));
		kids[1] = RIGHT_CHILD(LEFT_CHILD(LEFT_CHILD(p)));
		break;
	case 75: /* reg: VLDR(INDIRI4(ADD(reg,imm10))) */
	case 7: /* reg: LDR(INDIRI4(ADD(reg,imm12))) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(LEFT_CHILD(p)));
		break;
	case 92: /* stm: CMP(reg,ASR(reg,imm5)) */
	case 91: /* stm: CMP(reg,LSR(reg,imm5)) */
	case 90: /* stm: CMP(reg,LSL(reg,imm5)) */
	case 89: /* reg: SUB(reg,ASR(reg,imm5)) */
	case 88: /* reg: SUB(reg,LSR(reg,imm5)) */
	case 87: /* reg: SUB(reg,LSL(reg,imm5)) */
	case 86: /* reg: ADD(reg,ASR(reg,imm5)) */
	case 85: /* reg: ADD(reg,LSR(reg,imm5)) */
	case 84: /* reg: ADD(reg,LSL(reg,imm5)) */
	case 76: /* stm: VSTR(reg,INDIRI4(reg)) */
	case 10: /* stm: STR(reg,INDIRI4(reg)) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(RIGHT_CHILD(p));
		break;
	case 11: /* stm: STR(reg,INDIRI4(ADD(reg,reg))) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(LEFT_CHILD(RIGHT_CHILD(p)));
		kids[2] = RIGHT_CHILD(LEFT_CHILD(RIGHT_CHILD(p)));
		break;
	case 77: /* stm: VSTR(reg,INDIRI4(ADD(reg,imm10))) */
	case 12: /* stm: STR(reg,INDIRI4(ADD(reg,imm12))) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(LEFT_CHILD(RIGHT_CHILD(p)));
		break;
	case 71: /* stm: VCMP(reg,reg) */
	case 67: /* reg: VDIV(reg,reg) */
	case 66: /* reg: VSUB(reg,reg) */
	case 65: /* reg: VADD(reg,reg) */
	case 64: /* reg: VMUL(reg,reg) */
	case 35: /* reg: RSB(reg,reg) */
	case 23: /* stm: CMP(reg,reg) */
	case 19: /* reg: SUB(reg,reg) */
	case 18: /* reg: MUL(reg,reg) */
	case 15: /* reg: SDIV(reg,reg) */
	case 13: /* reg: ADD(reg,reg) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = RIGHT_CHILD(p);
		break;
	case 81: /* reg: ASR(reg,imm5) */
	case 80: /* reg: LSL(reg,imm5) */
	case 72: /* stm: VCMPz(reg) */
	case 70: /* reg: VNEG(reg) */
	case 69: /* reg: vcvt_floatingPointToSigned(reg) */
	case 68: /* reg: vcvt_signedToFloatingPoint(reg) */
	case 63: /* reg: VMOV(reg) */
	case 59: /* stm: BX(reg) */
	case 56: /* reg: LSR(reg,imm5) */
	case 55: /* reg: CLZ(reg) */
	case 52: /* reg: MOV(reg) */
	case 36: /* reg: RSB(reg,simm8) */
	case 24: /* stm: CMP(reg,simm8) */
	case 20: /* reg: SUB(reg,simm8) */
	case 14: /* reg: ADD(reg,simm8) */
		kids[0] = LEFT_CHILD(p);
		break;
	case 21: /* reg: ADD(MUL(reg,reg),reg) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(p));
		kids[1] = RIGHT_CHILD(LEFT_CHILD(p));
		kids[2] = RIGHT_CHILD(p);
		break;
	case 22: /* reg: SUB(reg,MUL(reg,reg)) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(RIGHT_CHILD(p));
		kids[2] = RIGHT_CHILD(RIGHT_CHILD(p));
		break;
	case 82: /* reg: LDR(INDIRI4(ADD(reg,LSL(reg,imm5)))) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(LEFT_CHILD(p)));
		kids[1] = LEFT_CHILD(RIGHT_CHILD(LEFT_CHILD(LEFT_CHILD(p))));
		break;
	case 83: /* stm: STR(reg,INDIRI4(ADD(reg,LSL(reg,imm5)))) */
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
