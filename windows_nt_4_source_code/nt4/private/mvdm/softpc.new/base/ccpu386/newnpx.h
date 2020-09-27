/*
 * SccsID = @(#)newnpx.h	1.4 10/06/94
 */

/* Function prototypes - everything returns void */
IMPORT VOID F2XM1(); 
IMPORT VOID FABS(); 
IMPORT VOID FADD(); 
IMPORT VOID FBLD(); 
IMPORT VOID FBSTP(); 
IMPORT VOID FCHS(); 
IMPORT VOID FCLEX(); 
IMPORT VOID FCOM(); 
IMPORT VOID FCOS(); 
IMPORT VOID FDECSTP(); 
IMPORT VOID FDIV();
IMPORT VOID FFREE();
IMPORT VOID FILD();
IMPORT VOID FLD();
IMPORT VOID FINCSTP();
IMPORT VOID FINIT();
IMPORT VOID FIST();
IMPORT VOID FLDCONST();
IMPORT VOID FLDCW();
IMPORT VOID FMUL();
IMPORT VOID FNOP();
IMPORT VOID FPATAN();
IMPORT VOID FPREM();
IMPORT VOID FPREM1();
IMPORT VOID FPTAN();
IMPORT VOID FRNDINT();
IMPORT VOID FSTCW();
IMPORT VOID FRSTOR();
IMPORT VOID FSAVE();
IMPORT VOID FSCALE();
IMPORT VOID FSIN();
IMPORT VOID FSINCOS();
IMPORT VOID FSQRT();
IMPORT VOID FST();
IMPORT VOID FSTENV();
IMPORT VOID FSTSW();
IMPORT VOID FSTCW();
IMPORT VOID FSUB();
IMPORT VOID FTST();
IMPORT VOID FXAM();
IMPORT VOID FXCH();
IMPORT VOID FXTRACT();
IMPORT VOID FYL2X();
IMPORT VOID FYL2XP1();

#define SAVE_PTRS()	\
	if (!NPX_PROT_MODE) {	\
		NpxFOP = NpxInstr;	\
	}				\
	NpxFCS = GET_CS_SELECTOR();	\
	NpxFIP = GET_EIP();

#define SAVE_DPTRS()	\
	NpxFEA = m_off[0];	\
	NpxFDS = GET_SR_SELECTOR(m_seg[0])

typedef enum 
{
FPSTACK,
M16I,
M32I,
M64I,
M32R,
M64R,
M80R
} NPXOPTYPE;

IMPORT IU8 FPtype;
