
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

#ifndef _MDB_LIST
#define _MDB_LIST

struct ACCELERATOR {
	struct ACCELERATOR *accelNext;
	struct ACCELERATOR *accelPrior;
	unsigned long (*accelRoutine)(unsigned long key, void *p,
				struct ACCELERATOR *parser);
	unsigned long (*accelRoutineHelp)(unsigned long key,
				struct ACCELERATOR *parser);
	unsigned long accelFlags;
	unsigned long key;
	unsigned long supervisorCommand;
	unsigned char *shortHelp;
};

struct ALT_DEBUGGER {
	struct ALT_DEBUGGER *altDebugNext;
	struct ALT_DEBUGGER *altDebugPrior;
	int (*AlternateDebugger)(int reason, int error, void *frame);
};


struct DEBUGGER_PARSER {
	struct DEBUGGER_PARSER *debugNext;
	struct DEBUGGER_PARSER *debugPrior;
	unsigned long (*DebugCommandParser)(unsigned char *commandLine,
				StackFrame *stackFrame,
				unsigned long Exception,
				struct DEBUGGER_PARSER *parser);
	unsigned long (*DebugCommandParserHelp)(unsigned char *commandLine,
					struct DEBUGGER_PARSER *parser);
	unsigned long parserFlags;
	unsigned char *debugCommandName;
	unsigned long debugCommandNameLength;
	unsigned long supervisorCommand;
	unsigned char *shortHelp;
	unsigned long controlTransfer;
	unsigned long category;
};

struct DEBUGGER_LIST {
	struct DEBUGGER_PARSER *head;
	struct DEBUGGER_PARSER *tail;
};

#define ACCELERATOR      struct ACCELERATOR
#define ALT_DEBUGGER     struct ALT_DEBUGGER
#define DEBUGGER_PARSER  struct DEBUGGER_PARSER
#define DEBUGGER_LIST    struct DEBUGGER_LIST

extern int AlternateDebuggerRoutine(int reason, int error, void *frame);
extern unsigned long AddAlternateDebugger(ALT_DEBUGGER *Debugger);
extern unsigned long RemoveAlternateDebugger(ALT_DEBUGGER *Debugger);
extern unsigned long DebuggerParserRoutine(unsigned char *command,
		unsigned char *commandLine,
		StackFrame *stackFrame,
		unsigned long Exception);
extern unsigned long DebuggerParserHelpRoutine(unsigned char *command,
		unsigned char *commandLine);
extern unsigned long AddDebuggerCommandParser(DEBUGGER_PARSER *parser);
extern unsigned long RemoveDebuggerCommandParser(DEBUGGER_PARSER *parser);

static inline unsigned long strhash(unsigned char *s, int len, int limit)
{
	register unsigned long h = 0, a = 127, i;

	if (!limit)
	return -1;

	for (i = 0; i < len && *s; s++, i++)
	h = ((a * h) + *s) % limit;

	return h;
}

#endif

