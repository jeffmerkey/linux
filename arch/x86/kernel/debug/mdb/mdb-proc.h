
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

#ifndef _MDB_PROC_H
#define _MDB_PROC_H

/* mdb-main.c */
extern void mdb_watchdogs(void);

/* mdb-base.c */
extern unsigned long needs_proceed;
extern unsigned long jmp_active;
extern unsigned long trap_disable;
extern unsigned long general_toggle;
extern unsigned long line_info_toggle;
extern unsigned long control_toggle;
extern unsigned long segment_toggle;
extern unsigned long numeric_toggle;
extern unsigned long reason_toggle;
extern unsigned long lockup_toggle;
extern unsigned long user_toggle;
extern unsigned long toggle_user_break;

extern unsigned char *IA32Flags[];
extern unsigned char *BreakDescription[];
extern unsigned char *BreakLengthDescription[];
extern unsigned char *ExceptionDescription[];

// mdb-ia.c
extern atomic_t focusActive;
extern atomic_t debuggerActive;
extern atomic_t debuggerProcessors;
extern atomic_t nmiProcessors;
extern atomic_t traceProcessors;
extern unsigned long ProcessorHold;
extern unsigned long ProcessorState;
extern unsigned char *procState[];
extern unsigned char *lastDumpAddress;
extern unsigned char *lastLinkAddress;
extern unsigned long lastUnasmAddress;
extern unsigned long displayLength;
extern unsigned long lastCommand;
extern unsigned long lastCommandEntry;
extern unsigned char lastDebugCommand[100];
extern unsigned long lastDisplayLength;
extern unsigned char debugCommand[100];
extern unsigned long nextUnasmAddress;
extern unsigned long pic1Value;
extern unsigned long pic2Value;
extern unsigned long BreakReserved[4];
extern unsigned long BreakPoints[4];
extern unsigned long BreakType[4];
extern unsigned long BreakLength[4];
extern unsigned long BreakTemp[4];
extern unsigned long BreakGo[4];
extern unsigned long BreakProceed[4];
extern unsigned long ConditionalBreakpoint[4];
extern unsigned char BreakCondition[4][256];
extern StackFrame lastStackFrame;
extern unsigned long lastCR0;
extern unsigned long lastCR2;
extern unsigned long lastCR4;
extern unsigned long CurrentDR7;
extern unsigned long CurrentDR6;
extern StackFrame CurrentStackFrame;
extern unsigned long BreakMask;
extern unsigned long repeatCommand;
extern unsigned long totalLines;
extern unsigned long debuggerInitialized;
extern unsigned long ssbmode;
extern int nextline;
extern unsigned char *category_strings[13];

extern unsigned long disassemble(StackFrame *stackFrame, unsigned long p, 
                                 unsigned long count, unsigned long use, 
                                 unsigned long type);
extern void ClearDebuggerState(void);
extern void displayMTRRRegisters(void);
extern void DisplayGDT(unsigned char *GDT_ADDRESS);
extern void DisplayIDT(unsigned char *IDT_ADDRESS);
extern void SetDebugRegisters(void);
extern void LoadDebugRegisters(void);
extern void ClearTempBreakpoints(void);
extern unsigned long ValidBreakpoint(unsigned long address);
extern unsigned char *dump(unsigned char *p, unsigned long count,
				unsigned long physical);
extern unsigned char *dumpWord(unsigned char *p, unsigned long count,
				unsigned long physical);
extern unsigned char *dumpDouble(unsigned char *p, unsigned long count,
				unsigned long physical);
extern unsigned char *dumpLinkedList(unsigned char *p, unsigned long count, 
                                     unsigned long offset);
extern unsigned char *dumpDoubleStack(StackFrame *stackFrame, 
                                      unsigned char *p, 
                                      unsigned long count);
extern unsigned char *dumpStack(StackFrame *stackFrame, 
                                unsigned char *p, 
                                unsigned long count);
extern unsigned long dumpBacktrace(unsigned char *p, unsigned long count);
extern unsigned long debugger_setup(unsigned long processor, 
                                    unsigned long Exception, 
                                    StackFrame *stackFrame, 
                                    unsigned char *panicMsg);
extern unsigned long debugger_entry(unsigned long Exception, 
                                    StackFrame *stackFrame, 
                                    unsigned long processor);
extern unsigned long debugger_command_entry(unsigned long processor, 
                                            unsigned long Exception, 
                                            StackFrame *stackFrame);
extern unsigned long ConsoleDisplayBreakReason(StackFrame *stackFrame,
			                       unsigned long reason, 
                                               unsigned long processor,
			                       unsigned long lastCommand);
extern uint64_t EvaluateExpression(StackFrame *stackFrame, 
                                   unsigned char **p, 
                                   unsigned long *type);
extern uint64_t EvaluateNumericExpression(StackFrame *stackFrame, 
                                          unsigned char **p, 
                                          unsigned long *type);
extern uint64_t EvaluateDisassemblyExpression(StackFrame *stackFrame, 
                                              unsigned char **p, 
                                              unsigned long *type, 
                                              int sizeflag, 
                                              unsigned char **result);
extern unsigned long unassemble(StackFrame *stackFrame, unsigned long ip, 
				unsigned long use, unsigned long *ret, 
				unsigned long type);
extern void DisplayASCIITable(void);
extern unsigned char *UpcaseString(unsigned char *);
extern unsigned long validate_address(unsigned long addr);
extern unsigned long ScreenInputFromKeyboard(unsigned char *buffer, unsigned long Start, 
				unsigned long Length);

extern unsigned long GetIP(StackFrame *);
extern unsigned long GetStackAddress(StackFrame *);
extern unsigned long GetStackSegment(StackFrame *);
extern unsigned short read_memory(void *, void *, unsigned);
extern unsigned long SSBUpdate(StackFrame *stackFrame, unsigned long processor);
extern void mdb_breakpoint(void);

#endif
