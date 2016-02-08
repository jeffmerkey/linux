
/***************************************************************************
*
*   Copyright (c) 2000-2015 Jeff V. Merkey  All Rights Reserved.
*   jeffmerkey@gmail.com
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation, version 2.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   AUTHOR   :  Jeff V. Merkey
*   DESCRIP  :  Minimal Linux Debugger
*
***************************************************************************/

#ifndef _MDB_IA32_PROC_H
#define _MDB_IA32_PROC_H

#define EXT_NMI_PORT             0x0461
#define NMI_IO_PORT              0x0462
#define NMI_CONTROL_PORT         0x0C6E
#define NMI_PORT                 0x61
#define PIC1_DEBUG_MASK          0xFC
#define PIC2_DEBUG_MASK          0xFF
#define EXCEPTION_ENTRIES        19
#define RESUME                   0x00010000
#define NESTED_TASK              0x00004000
#define SINGLE_STEP              0x00000100
#define INVALID_EXPRESSION       0
#define NUMERIC_EXPRESSION       1
#define BOOLEAN_EXPRESSION       2

/* DR7 Breakpoint Type and Length Fields */

#define BREAK_EXECUTE     0
#define BREAK_WRITE       1
#define BREAK_IOPORT      2
#define BREAK_READWRITE   3
#define ONE_BYTE_FIELD    0
#define TWO_BYTE_FIELD    1
#define EIGHT_BYTE_FIELD  2
#define FOUR_BYTE_FIELD   3

/* DR7 Register */

#define L0_BIT   0x00000001
#define G0_BIT   0x00000002
#define L1_BIT   0x00000004
#define G1_BIT   0x00000008
#define L2_BIT   0x00000010
#define G2_BIT   0x00000020
#define L3_BIT   0x00000040
#define G3_BIT   0x00000080
#define LEXACT   0x00000100
#define GEXACT   0x00000200
#define GDETECT  0x00002000
#define DR7DEF   0x00000400

/* DR6 Register */

#define B0_BIT   0x00000001
#define B1_BIT   0x00000002
#define B2_BIT   0x00000004
#define B3_BIT   0x00000008

#define BD_BIT   0x00002000
#define BS_BIT   0x00004000
#define BT_BIT   0x00008000

/* Memory Type Range Registers (MTRR) */

#define MTRR_PHYS_BASE_0    0x200
#define MTRR_PHYS_MASK_0    0x201
#define MTRR_PHYS_BASE_1    0x202
#define MTRR_PHYS_MASK_1    0x203
#define MTRR_PHYS_BASE_2    0x204
#define MTRR_PHYS_MASK_2    0x205
#define MTRR_PHYS_BASE_3    0x206
#define MTRR_PHYS_MASK_3    0x207
#define MTRR_PHYS_BASE_4    0x208
#define MTRR_PHYS_MASK_4    0x209
#define MTRR_PHYS_BASE_5    0x20A
#define MTRR_PHYS_MASK_5    0x20B
#define MTRR_PHYS_BASE_6    0x20C
#define MTRR_PHYS_MASK_6    0x20D
#define MTRR_PHYS_BASE_7    0x20E
#define MTRR_PHYS_MASK_7    0x20F

/* IA32 flags settings */

#define   CF_FLAG      0x00000001
#define   PF_FLAG      0x00000004
#define   AF_FLAG      0x00000010
#define   ZF_FLAG      0x00000040
#define   SF_FLAG      0x00000080
#define   TF_FLAG      0x00000100  /* ss flag */
#define   IF_FLAG      0x00000200
#define   DF_FLAG      0x00000400
#define   OF_FLAG      0x00000800
#define   NT_FLAG      0x00004000
#define   RF_FLAG      0x00010000  /* resume flag */
#define   VM_FLAG      0x00020000
#define   AC_FLAG      0x00040000
#define   VIF_FLAG     0x00080000
#define   VIP_FLAG     0x00100000
#define   ID_FLAGS     0x00200000

#ifdef CONFIG_X86_64

struct GATE64 {
	u16 offset_low;
	u16 segment;
	unsigned ist : 3, zero0 : 5, type : 5, dpl : 2, p : 1;
	u16 offset_middle;
	u32 offset_high;
	u32 zero1;
} __packed;

struct LDTTSS64 {
	u16 limit0;
	u16 base0;
	unsigned base1 : 8, type : 5, dpl : 2, p : 1;
	unsigned limit1 : 4, zero0 : 3, g : 1, base2 : 8;
	u32 base3;
	u32 zero1;
} __packed;

#define GDT       struct GATE64
#define IDT       struct GATE64
#define LDT       struct LDTTSS64
#define TSS       struct LDTTSS64
#define TSS_GATE  struct LDTTSS64

#else

struct DESC {
	union {
		struct	{
			unsigned int a;
			unsigned int b;
		};
		struct	{
			u16 limit0;
			u16 base0;
			unsigned base1: 8, type: 4, s: 1, dpl: 2, p: 1;
			unsigned limit: 4, avl: 1, l: 1, d: 1, g: 1, base2: 8;
		};
	};
} __packed;

struct GDT32 {
	u16 Limit;    /*	0xFFFF */
	u16 Base1;    /*  0 */
	u8  Base2;     /*	0 */
	u8  GDTType;   /*	10010010b */
	u8  OtherType; /*	11001111b */
	u8  Base3;     /*	0 */
} __packed;

struct IDT32 {
	u16 IDTLow;     /*	0 */
	u16 IDTSegment; /*	0x08 */
	u8  IDTSkip;     /*	0 */
	u8  IDTFlags;    /*	10001110b */
	u16 IDTHigh;    /*	0 */
} __packed;

struct TSS32 {
	u16 TSSLimit;	/* 0x0080 */
	u16 TSSBase1;	/* 0 */
	u8  TSSBase2;	/* 0 */
	u8  TSSType;	/* 10001001b */
	u8  TSSOtherType;	/* 00000000b */
	u8  TSSBase3;	/* 0 */
} __packed;

struct TSS_GATE32 {
	u16 TSSRes1;	/* 0 */
	u16 TSSSelector;	/* 0 */
	u8  TSSRes2;	/* 0 */
	u8  TSSFlags;	/* 10000101b */
	u16 TSSRes3;	/* 0 */
} __packed;

struct LDT32 {
	u16 LDTLimit;	/* 0xFFFF */
	u16 LDTBase1;	/* 0 */
	u8  LDTBase2;	/* 0 */
	u8  LDTGDTType;	/* 10000010b */
	u8  LDTOtherType;	/* 10001111b */
	u8  LDTBase3;	/* 0 */
} __packed;

#define GDT       struct GDT32
#define LDT       struct LDT32
#define IDT       struct IDT32
#define TSS       struct TSS32
#define TSS_GATE  struct TSS_GATE32

#endif

unsigned long is_processor_held(unsigned long cpu);
unsigned long ReadDR0(void);
unsigned long ReadDR1(void);
unsigned long ReadDR2(void);
unsigned long ReadDR3(void);
unsigned long ReadDR6(void);
unsigned long ReadDR7(void);
void WriteDR0(unsigned long);
void WriteDR1(unsigned long);
void WriteDR2(unsigned long);
void WriteDR3(unsigned long);
void WriteDR6(unsigned long);
void WriteDR7(unsigned long);
unsigned long ReadCR0(void);
unsigned long ReadCR2(void);
unsigned long ReadCR3(void);
unsigned long ReadCR4(void);
void ReadGDTR(unsigned long *);
void ReadIDTR(unsigned long *);
unsigned long ReadLDTR(void);
unsigned long ReadTR(void);

void ReadMSR(unsigned long msr,
	     unsigned long *val1,
	unsigned long *val2);
void WriteMSR(unsigned long msr,
	      unsigned long *val1,
	unsigned long *val2);
void MTRROpen(void);
void MTRRClose(void);
void save_npx(NUMERIC_FRAME *npx);
void load_npx(NUMERIC_FRAME *npx);

unsigned long get_processor_id(void);
unsigned long get_physical_processor(void);
unsigned long fpu_present(void);

void ReadTaskFrame(StackFrame *sf,
		   struct task_struct *p);
void DisplayTSS(StackFrame *stackFrame);
void DisplayGeneralRegisters(StackFrame *stackFrame);
void DisplaySegmentRegisters(StackFrame *stackFrame);
void DisplayControlRegisters(unsigned long processor,
			     StackFrame *stackFrame);
double ldexp(double v, int e);
void DisplayNPXRegisters(StackFrame *stackFrame);

unsigned long processProceedACC(unsigned long key,
				void *stackFrame,
		ACCELERATOR *accel);
unsigned long processTraceACC(unsigned long key,
			      void *stackFrame,
		ACCELERATOR *accel);
unsigned long processTraceSSBACC(unsigned long key,
				 void *stackFrame,
		ACCELERATOR *accel);
unsigned long processGoACC(unsigned long key,
			   void *stackFrame,
		ACCELERATOR *accel);

unsigned long executeCommandHelp(unsigned char *commandLine,
				 DEBUGGER_PARSER *parser);
unsigned long processProceed(unsigned char *cmd,
			     StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long processTrace(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long processTraceSSB(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long processGo(unsigned char *cmd,
			StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long processorCommandHelp(unsigned char *commandLine,
				   DEBUGGER_PARSER *parser);
unsigned long breakProcessor(unsigned char *cmd,
			     StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long TSSDisplayHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long TSSDisplay(unsigned char *cmd,
			 StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayEAXHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeEAXRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long ChangeORIGEAXRegister(unsigned char *cmd,
				    StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayEBXHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeEBXRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayECXHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeECXRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayEDXHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeEDXRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayESIHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeESIRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayEDIHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeEDIRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayEBPHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeEBPRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayESPHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeESPRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayEIPHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeEIPRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayCSHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeCSRegister(unsigned char *cmd,
			       StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayDSHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeDSRegister(unsigned char *cmd,
			       StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayESHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeESRegister(unsigned char *cmd,
			       StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayFSHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeFSRegister(unsigned char *cmd,
			       StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayGSHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeGSRegister(unsigned char *cmd,
			       StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displaySSHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeSSRegister(unsigned char *cmd,
			       StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayRFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeRFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayTFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeTFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayZFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeZFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displaySFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeSFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayPFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangePFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayCFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeCFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayOFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeOFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayIFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeIFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayIDHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeIDFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayDFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeDFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayNTHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeNTFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayVMHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeVMFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayVIFHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeVIFFlag(unsigned char *cmd,
			    StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayVIPHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeVIPFlag(unsigned char *cmd,
			    StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayAFHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeAFFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayACHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeACFlag(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayMTRRHelp(unsigned char *commandLine,
			      DEBUGGER_PARSER *parser);
unsigned long DisplayMTRRRegisters(unsigned char *cmd,
				   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayGDTHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long displayGDT(unsigned char *cmd,
			 StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayIDTHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long displayIDT(unsigned char *cmd,
			 StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long evaluateExpressionHelp(unsigned char *commandLine,
				     DEBUGGER_PARSER *parser);
unsigned long evaluateExpression(unsigned char *cmd,
				 StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayDOSTableHelp(unsigned char *commandLine,
				  DEBUGGER_PARSER *parser);
unsigned long displayDOSTable(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long portCommandHelp(unsigned char *commandLine,
			      DEBUGGER_PARSER *parser);
unsigned long inputWordPort(unsigned char *cmd,
			    StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long inputDoublePort(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long inputBytePort(unsigned char *cmd,
			    StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long inputPort(unsigned char *cmd,
			StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long outputWordPort(unsigned char *cmd,
			     StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long outputDoublePort(unsigned char *cmd,
			       StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long outputBytePort(unsigned char *cmd,
			     StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long outputPort(unsigned char *cmd,
			 StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long breakpointCommandHelp(unsigned char *commandLine,
				    DEBUGGER_PARSER *parser);
unsigned long breakpointClearAll(unsigned char *cmd,
				 StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointClear(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointMask(unsigned char *cmd,
			     StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointWord1(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointWord2(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointWord4(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
#ifdef CONFIG_X86_64
unsigned long breakpointWord8(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
#endif
unsigned long breakpointWord(unsigned char *cmd,
			     StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointRead1(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointRead2(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointRead4(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
#ifdef CONFIG_X86_64
unsigned long breakpointRead8(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
#endif
unsigned long breakpointRead(unsigned char *cmd,
			     StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointIO1(unsigned char *cmd,
			    StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointIO2(unsigned char *cmd,
			    StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointIO4(unsigned char *cmd,
			    StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointIO(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointExecute(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long breakpointShowTemp(unsigned char *cmd,
				 StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
void mdb_breakpoint(void);

#if defined(CONFIG_SMP)
unsigned long displayAPICHelp(unsigned char *commandLine,
			      DEBUGGER_PARSER *parser);
unsigned long displayAPICInfo(unsigned char *cmd,
			      StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long displayIOAPICInfo(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long nmiProcessor(unsigned char *cmd,
			   StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long apic_directed_nmi(unsigned long cpu);
unsigned long apic_xcall(unsigned long cpu,
			 unsigned long command,
		unsigned long type);
#endif

#ifdef CONFIG_X86_64
unsigned long displayRAXHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeRAXRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long ChangeORIGRAXRegister(unsigned char *cmd,
				    StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayRBXHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeRBXRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayRCXHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeRCXRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayRDXHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeRDXRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayRSIHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeRSIRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayRDIHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeRDIRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayRBPHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeRBPRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayRSPHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeRSPRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayRIPHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeRIPRegister(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);

unsigned long displayR8Help(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeR8Register(unsigned char *cmd,
			       StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long displayR9Help(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long ChangeR9Register(unsigned char *cmd,
			       StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long displayR10Help(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeR10Register(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long displayR11Help(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeR11Register(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long displayR12Help(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeR12Register(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long displayR13Help(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeR13Register(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long displayR14Help(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeR14Register(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
unsigned long displayR15Help(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long ChangeR15Register(unsigned char *cmd,
				StackFrame *stackFrame,
		unsigned long Exception,
		DEBUGGER_PARSER *parser);
#endif

#endif

