
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

#ifndef _MDB_BASE_H
#define _MDB_BASE_H

unsigned long enterKeyACC(unsigned long key,
			  void *stackFrame,
			  ACCELERATOR *accel);
unsigned long activateRegisterDisplayACC(unsigned long key,
					 void *stackFrame,
					 ACCELERATOR *accel);

unsigned long displayDebuggerHelpHelp(unsigned char *commandLine,
				      DEBUGGER_PARSER *parser);
unsigned long displayDebuggerHelp(unsigned char *commandLine,
				  StackFrame *stackFrame,
				  unsigned long Exception,
				  DEBUGGER_PARSER *parser);

unsigned long ascTableHelp(unsigned char *commandLine,
			   DEBUGGER_PARSER *parser);
unsigned long displayASCTable(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);

unsigned long displayToggleHelp(unsigned char *commandLine,
				DEBUGGER_PARSER *parser);
unsigned long displayToggleAll(unsigned char *cmd,
			       StackFrame *stackFrame,
			       unsigned long Exception,
			       DEBUGGER_PARSER *parser);
unsigned long ProcessTUToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long ProcessTBToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long ProcessTDToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long ProcessTLToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long ProcessTGToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long ProcessTCToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long ProcessTNToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long ProcessTRToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long ProcessToggleUser(unsigned char *cmd,
				StackFrame *stackFrame,
				unsigned long Exception,
				DEBUGGER_PARSER *parser);
unsigned long ProcessTSToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long ProcessTAToggle(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);

unsigned long displayDebuggerVersionHelp(unsigned char *commandLine,
					 DEBUGGER_PARSER *parser);
unsigned long DisplayDebuggerVersion(unsigned char *cmd,
				     StackFrame *stackFrame,
				     unsigned long Exception,
				     DEBUGGER_PARSER *parser);

unsigned long displayKernelProcessHelp(unsigned char *commandLine,
				       DEBUGGER_PARSER *parser);
unsigned long displayKernelProcess(unsigned char *cmd,
				   StackFrame *stackFrame,
				   unsigned long Exception,
				   DEBUGGER_PARSER *parser);

unsigned long displayKernelQueueHelp(unsigned char *commandLine,
				     DEBUGGER_PARSER *parser);
unsigned long displayKernelQueue(unsigned char *cmd,
				 StackFrame *stackFrame,
				 unsigned long Exception,
				 DEBUGGER_PARSER *parser);

unsigned long displaySymbolsHelp(unsigned char *commandLine,
				 DEBUGGER_PARSER *parser);
unsigned long displaySymbols(unsigned char *cmd,
			     StackFrame *stackFrame,
			     unsigned long Exception,
			     DEBUGGER_PARSER *parser);

unsigned long displayLoaderMapHelp(unsigned char *commandLine,
				   DEBUGGER_PARSER *parser);
unsigned long displayLoaderMap(unsigned char *cmd,
			       StackFrame *stackFrame,
			       unsigned long Exception,
			       DEBUGGER_PARSER *parser);

unsigned long displayModuleHelp(unsigned char *commandLine,
				DEBUGGER_PARSER *parser);
unsigned long displayModuleInfo(unsigned char *cmd,
				StackFrame *stackFrame,
				unsigned long Exception,
				DEBUGGER_PARSER *parser);

unsigned long displayProcessesHelp(unsigned char *commandLine,
				   DEBUGGER_PARSER *parser);
unsigned long displayProcesses(unsigned char *cmd,
			       StackFrame *stackFrame,
			       unsigned long Exception,
			       DEBUGGER_PARSER *parser);

unsigned long displayRegistersHelp(unsigned char *commandLine,
				   DEBUGGER_PARSER *parser);
unsigned long displayControlRegisters(unsigned char *cmd,
				      StackFrame *stackFrame,
				      unsigned long Exception,
				      DEBUGGER_PARSER *parser);
unsigned long displayAllRegisters(unsigned char *cmd,
				  StackFrame *stackFrame,
				  unsigned long Exception,
				  DEBUGGER_PARSER *parser);
unsigned long displaySegmentRegisters(unsigned char *cmd,
				      StackFrame *stackFrame,
				      unsigned long Exception,
				      DEBUGGER_PARSER *parser);
unsigned long displayNumericRegisters(unsigned char *cmd,
				      StackFrame *stackFrame,
				      unsigned long Exception,
				      DEBUGGER_PARSER *parser);
unsigned long displayGeneralRegisters(unsigned char *cmd,
				      StackFrame *stackFrame,
				      unsigned long Exception,
				      DEBUGGER_PARSER *parser);
unsigned long displayDefaultRegisters(unsigned char *cmd,
				      StackFrame *stackFrame,
				      unsigned long Exception,
				      DEBUGGER_PARSER *parser);

unsigned long displayAPICHelp(unsigned char *commandLine,
			      DEBUGGER_PARSER *parser);
unsigned long displayAPICInfo(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);

unsigned long listProcessors(unsigned char *cmd,
			     StackFrame *stackFrame,
			     unsigned long Exception,
			     DEBUGGER_PARSER *parser);
unsigned long listProcessorFrame(unsigned char *cmd,
				 StackFrame *stackFrame,
				 unsigned long Exception,
				 DEBUGGER_PARSER *parser);

unsigned long ReasonHelp(unsigned char *commandLine,
			 DEBUGGER_PARSER *parser);
unsigned long ReasonDisplay(unsigned char *cmd,
			    StackFrame *stackFrame,
			    unsigned long Exception,
			    DEBUGGER_PARSER *parser);

unsigned long displayMPSHelp(unsigned char *commandLine,
			     DEBUGGER_PARSER *parser);
unsigned long displayMPS(unsigned char *cmd,
			 StackFrame *stackFrame,
			 unsigned long Exception,
			 DEBUGGER_PARSER *parser);

unsigned long clearScreenHelp(unsigned char *commandLine,
			      DEBUGGER_PARSER *parser);
unsigned long clearDebuggerScreen(unsigned char *cmd,
				  StackFrame *stackFrame,
				  unsigned long Exception,
				  DEBUGGER_PARSER *parser);

unsigned long SearchMemoryHelp(unsigned char *commandLine,
			       DEBUGGER_PARSER *parser);
unsigned long SearchMemory(unsigned char *cmd,
			   StackFrame *stackFrame,
			   unsigned long Exception,
			   DEBUGGER_PARSER *parser);
unsigned long SearchMemoryB(unsigned char *cmd,
			    StackFrame *stackFrame,
			    unsigned long Exception,
			    DEBUGGER_PARSER *parser);
unsigned long SearchMemoryW(unsigned char *cmd,
			    StackFrame *stackFrame,
			    unsigned long Exception,
			    DEBUGGER_PARSER *parser);
unsigned long SearchMemoryD(unsigned char *cmd,
			    StackFrame *stackFrame,
			    unsigned long Exception,
			    DEBUGGER_PARSER *parser);
unsigned long SearchMemoryQ(unsigned char *cmd,
			    StackFrame *stackFrame,
			    unsigned long Exception,
			    DEBUGGER_PARSER *parser);

unsigned long changeMemoryHelp(unsigned char *commandLine,
			       DEBUGGER_PARSER *parser);
unsigned long changeWordValue(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long changeDoubleValue(unsigned char *cmd,
				StackFrame *stackFrame,
				unsigned long Exception,
				DEBUGGER_PARSER *parser);
unsigned long changeQuadValue(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long changeByteValue(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long changeDefaultValue(unsigned char *cmd,
				 StackFrame *stackFrame,
				 unsigned long Exception,
				 DEBUGGER_PARSER *parser);

unsigned long displayCloseHelp(unsigned char *commandLine,
			       DEBUGGER_PARSER *parser);
unsigned long displayCloseSymbols(unsigned char *cmd,
				  StackFrame *stackFrame,
				  unsigned long Exception,
				  DEBUGGER_PARSER *parser);

unsigned long displayINTRHelp(unsigned char *commandLine,
			      DEBUGGER_PARSER *parser);
unsigned long displayInterruptTable(unsigned char *cmd,
				    StackFrame *stackFrame,
				    unsigned long Exception,
				    DEBUGGER_PARSER *parser);

unsigned long viewScreensHelp(unsigned char *commandLine,
			      DEBUGGER_PARSER *parser);
unsigned long displayScreenList(unsigned char *cmd,
				StackFrame *stackFrame,
				unsigned long Exception,
				DEBUGGER_PARSER *parser);

unsigned long displayIOAPICHelp(unsigned char *commandLine,
				DEBUGGER_PARSER *parser);
unsigned long displayIOAPICInfo(unsigned char *cmd,
				StackFrame *stackFrame,
				unsigned long Exception,
				DEBUGGER_PARSER *parser);

unsigned long displayDumpHelp(unsigned char *commandLine,
			      DEBUGGER_PARSER *parser);
unsigned long debuggerWalkStack(unsigned char *cmd,
				StackFrame *stackFrame,
				unsigned long Exception,
				DEBUGGER_PARSER *parser);
unsigned long debuggerDumpLinkedList(unsigned char *cmd,
				     StackFrame *stackFrame,
				     unsigned long Exception,
				     DEBUGGER_PARSER *parser);
unsigned long debuggerDumpStack(unsigned char *cmd,
				StackFrame *stackFrame,
				unsigned long Exception,
				DEBUGGER_PARSER *parser);
unsigned long debuggerDumpDoubleStack(unsigned char *cmd,
				      StackFrame *stackFrame,
				      unsigned long Exception,
				      DEBUGGER_PARSER *parser);
unsigned long debuggerDumpQuadStack(unsigned char *cmd,
				    StackFrame *stackFrame,
				    unsigned long Exception,
				    DEBUGGER_PARSER *parser);

unsigned long debuggerDumpQuad(unsigned char *cmd,
			       StackFrame *stackFrame,
			       unsigned long Exception,
			       DEBUGGER_PARSER *parser);
unsigned long debuggerDumpDouble(unsigned char *cmd,
				 StackFrame *stackFrame,
				 unsigned long Exception,
				 DEBUGGER_PARSER *parser);
unsigned long debuggerDumpWord(unsigned char *cmd,
			       StackFrame *stackFrame,
			       unsigned long Exception,
			       DEBUGGER_PARSER *parser);
unsigned long debuggerDumpByte(unsigned char *cmd,
			       StackFrame *stackFrame,
			       unsigned long Exception,
			       DEBUGGER_PARSER *parser);

unsigned long debuggerDumpQuadPhys(unsigned char *cmd,
				   StackFrame *stackFrame,
				   unsigned long Exception,
				   DEBUGGER_PARSER *parser);
unsigned long debuggerDumpDoublePhys(unsigned char *cmd,
				     StackFrame *stackFrame,
				     unsigned long Exception,
				     DEBUGGER_PARSER *parser);
unsigned long debuggerDumpWordPhys(unsigned char *cmd,
				   StackFrame *stackFrame,
				   unsigned long Exception,
				   DEBUGGER_PARSER *parser);
unsigned long debuggerDumpBytePhys(unsigned char *cmd,
				   StackFrame *stackFrame,
				   unsigned long Exception,
				   DEBUGGER_PARSER *parser);

unsigned long displayDisassembleHelp(unsigned char *commandLine,
				     DEBUGGER_PARSER *parser);
unsigned long processDisassemble16(unsigned char *cmd,
				   StackFrame *stackFrame,
				   unsigned long Exception,
				   DEBUGGER_PARSER *parser);
unsigned long processDisassemble32(unsigned char *cmd,
				   StackFrame *stackFrame,
				   unsigned long Exception,
				   DEBUGGER_PARSER *parser);
unsigned long processDisassembleAny(unsigned char *cmd,
				    StackFrame *stackFrame,
				    unsigned long Exception,
				    DEBUGGER_PARSER *parser);
unsigned long processDisassembleATT(unsigned char *cmd,
				    StackFrame *stackFrame,
				    unsigned long Exception,
				    DEBUGGER_PARSER *parser);

unsigned long rebootSystemHelp(unsigned char *commandLine,
			       DEBUGGER_PARSER *parser);
unsigned long rebootSystem(unsigned char *cmd,
			   StackFrame *stackFrame,
			   unsigned long Exception,
			   DEBUGGER_PARSER *parser);

unsigned long displaySectionsHelp(unsigned char *commandLine,
				  DEBUGGER_PARSER *parser);
unsigned long displaySections(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long displayKernelProcessHelp(unsigned char *commandLine,
				       DEBUGGER_PARSER *parser);
unsigned long displayKernelProcess(unsigned char *cmd,
				   StackFrame *stackFrame,
				   unsigned long Exception,
				   DEBUGGER_PARSER *parser);
unsigned long displayProcessorStatusHelp(unsigned char *commandLine,
					 DEBUGGER_PARSER *parser);
unsigned long displayProcessorStatus(unsigned char *cmd,
				     StackFrame *stackFrame,
				     unsigned long Exception,
				     DEBUGGER_PARSER *parser);

unsigned long backTraceHelp(unsigned char *commandLine,
			    DEBUGGER_PARSER *parser);
unsigned long backTraceAllPID(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long backTracePID(unsigned char *cmd,
			   StackFrame *stackFrame,
			   unsigned long Exception,
			   DEBUGGER_PARSER *parser);
unsigned long backTraceStack(unsigned char *cmd,
			     StackFrame *stackFrame,
			     unsigned long Exception,
			     DEBUGGER_PARSER *parser);
unsigned long timedBreakpointHelp(unsigned char *commandLine,
				  DEBUGGER_PARSER *parser);
unsigned long timerBreakpoint(unsigned char *cmd,
			      StackFrame *stackFrame,
			      unsigned long Exception,
			      DEBUGGER_PARSER *parser);
unsigned long timerBreakpointClear(unsigned char *cmd,
				   StackFrame *stackFrame,
				   unsigned long Exception,
				   DEBUGGER_PARSER *parser);

unsigned long displayProcessSwitchHelp(unsigned char *commandLine,
				       DEBUGGER_PARSER *parser);
unsigned long switchKernelProcess(unsigned char *cmd,
				  StackFrame *stackFrame,
				  unsigned long Exception,
				  DEBUGGER_PARSER *parser);

unsigned long percpuHelp(unsigned char *commandLine,
			 DEBUGGER_PARSER *parser);
unsigned long perCpu(unsigned char *cmd,
		     StackFrame *stackFrame,
		     unsigned long Exception,
		     DEBUGGER_PARSER *parser);

#if defined(CONFIG_MODULES)
unsigned long listModulesHelp(unsigned char *commandLine,
			      DEBUGGER_PARSER *parser);
unsigned long listModules(unsigned char *cmd,
			  StackFrame *stackFrame,
			  unsigned long Exception,
			  DEBUGGER_PARSER *parser);
unsigned long unloadModule(unsigned char *cmd,
			   StackFrame *stackFrame,
			   unsigned long Exception,
			   DEBUGGER_PARSER *parser);
#endif

#endif
