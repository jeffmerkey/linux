
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/mm.h>
#include <linux/cdrom.h>
#include <linux/pagemap.h>
#include <linux/swap.h>
#include <linux/smp.h>
#include <linux/ctype.h>
#include <linux/keyboard.h>
#include <linux/console.h>
#include <linux/serial_reg.h>
#include <linux/uaccess.h>
#include <asm/segment.h>
#include <asm/atomic.h>
#include <asm/msr.h>
#include <linux/io.h>

#define __KERNEL_SYSCALLS__
#include <linux/unistd.h>
#include <linux/kallsyms.h>

#include "mdb.h"
#include "mdb-ia.h"
#include "mdb-list.h"
#include "mdb-ia-proc.h"
#include "mdb-base.h"
#include "mdb-proc.h"
#include "mdb-os.h"
#include "mdb-keyboard.h"

#ifdef CONFIG_X86_64
#define PROCESSOR_WIDTH     64
#else
#define PROCESSOR_WIDTH     32
#endif

#define DEBUG_EXPRESS        0
#define DEBUG_BOOL           0
#define DEBUG_LOGICAL        0
#define DEBUG_LOGICAL_STACK  0
#define DEBUG_BOOL_STACK     0

#define INVALID_EXPRESSION  0
#define NUMERIC_EXPRESSION  1
#define BOOLEAN_EXPRESSION  2

#define SEG_STACK_SIZE      256
#define NUM_STACK_SIZE      256
#define CONTEXT_STACK_SIZE  1024
#define BOOL_STACK_SIZE     256
#define LOGICAL_STACK_SIZE  256

// 0-127 token values, 8 high bit is reserved as dereference flag
#define NULL_TOKEN            0
#define NUMBER_TOKEN          1
#define MINUS_TOKEN           2
#define PLUS_TOKEN            3
#define MULTIPLY_TOKEN        4
#define DIVIDE_TOKEN          5
#define GREATER_TOKEN         6
#define LESS_TOKEN            7
#define XOR_TOKEN             8
#define AND_TOKEN             9
#define OR_TOKEN             10
#define NOT_TOKEN            11
#define NEG_TOKEN            12
#define EQUAL_TOKEN          13
#define LEFT_SHIFT_TOKEN     14
#define RIGHT_SHIFT_TOKEN    15
#define SPACE_TOKEN          16
#define FLAGS_TOKEN          17
#define AX_TOKEN            18
#define BX_TOKEN            19
#define CX_TOKEN            20
#define DX_TOKEN            21
#define SI_TOKEN            22
#define DI_TOKEN            23
#define BP_TOKEN            24
#define SP_TOKEN            25
#define CS_TOKEN             26
#define DS_TOKEN             27
#define ES_TOKEN             28
#define FS_TOKEN             29
#define GS_TOKEN             30
#define SS_TOKEN             31
#define DREF_OPEN_TOKEN      32
#define DREF_CLOSE_TOKEN     33
#define MOD_TOKEN            34
#define NUMBER_END           35
#define GREATER_EQUAL_TOKEN  36
#define LESS_EQUAL_TOKEN     37
#define IP_TOKEN             38
#define ASSIGNMENT_TOKEN     39
#define DWORD_TOKEN          40
#define WORD_TOKEN           41
#define BYTE_TOKEN           42
#define LOGICAL_AND_TOKEN    43
#define LOGICAL_OR_TOKEN     44
#define CF_TOKEN             45
#define PF_TOKEN             46
#define AF_TOKEN             47
#define ZF_TOKEN             48
#define SF_TOKEN             49
#define IF_TOKEN             50
#define DF_TOKEN             51
#define OF_TOKEN             52
#define VM_TOKEN             53
#define AC_TOKEN             54
#define BB_TOKEN             55
#define EB_TOKEN             56
#define NOT_EQUAL_TOKEN      57
#define INVALID_NUMBER_TOKEN 58
#define QWORD_TOKEN          59
#define R8_TOKEN             60
#define R9_TOKEN             61
#define R10_TOKEN            62
#define R11_TOKEN            63
#define R12_TOKEN            64
#define R13_TOKEN            65
#define R14_TOKEN            66
#define R15_TOKEN            67
#define CS_ADDR_TOKEN        68
#define DS_ADDR_TOKEN        69
#define ES_ADDR_TOKEN        70
#define FS_ADDR_TOKEN        71
#define GS_ADDR_TOKEN        72
#define SS_ADDR_TOKEN        73
#define EAX_TOKEN            74
#define EBX_TOKEN            75
#define ECX_TOKEN            76
#define EDX_TOKEN            77
#define RAX_TOKEN            78
#define RBX_TOKEN            79
#define RCX_TOKEN            80
#define RDX_TOKEN            81
#define AL_TOKEN             82
#define BL_TOKEN             83
#define CL_TOKEN             84
#define DL_TOKEN             85
#define ESI_TOKEN            86
#define EDI_TOKEN	     87	
#define EBP_TOKEN            88
#define ESP_TOKEN            89
#define RSI_TOKEN            90
#define RDI_TOKEN            91
#define RBP_TOKEN            92
#define RSP_TOKEN            93

// limit is 0-127

#define CF_FLAG   0x00000001
#define PF_FLAG   0x00000004
#define AF_FLAG   0x00000010
#define ZF_FLAG   0x00000040
#define SF_FLAG   0x00000080
#define TF_FLAG   0x00000100  /* ss flag */
#define IF_FLAG   0x00000200
#define DF_FLAG   0x00000400
#define OF_FLAG   0x00000800
#define NT_FLAG   0x00004000
#define RF_FLAG   0x00010000  /* resume flag */
#define VM_FLAG   0x00020000
#define AC_FLAG   0x00040000
#define VIF_FLAG  0x00080000
#define VIP_FLAG  0x00100000
#define ID_FLAG   0x00200000

#define   ARCH_PTR          0
#define   ULONG_PTR         1
#define   WORD_PTR          2
#define   BYTE_PTR          3
#define   ULONGLONG_PTR     4

#define   CLASS_DATA        1
#define   CLASS_ASSIGN      2
#define   CLASS_PARTITION   3
#define   CLASS_ARITHMETIC  4
#define   CLASS_BOOLEAN     5

unsigned char *exprDescription[]={
     "INVALID",
     "NUMERIC",
     "BOOLEAN",
     "???????",
};

unsigned char *parserDescription[]={
     "NULL_TOKEN",
     "NUMBER_TOKEN",
     "MINUS_TOKEN",
     "PLUS_TOKEN",
     "MULTIPLY_TOKEN",
     "DIVIDE_TOKEN",
     "GREATER_TOKEN",
     "LESS_TOKEN",
     "XOR_TOKEN",
     "AND_TOKEN",
     "OR_TOKEN",
     "NOT_TOKEN",
     "NEG_TOKEN",
     "EQUAL_TOKEN",
     "LEFT_SHIFT_TOKEN",
     "RIGHT_SHIFT_TOKEN",
     "SPACE_TOKEN",
     "FLAGS_TOKEN",
     "AX_TOKEN",
     "BX_TOKEN",
     "CX_TOKEN",
     "DX_TOKEN",
     "SI_TOKEN",
     "DI_TOKEN",
     "BP_TOKEN",
     "SP_TOKEN",
     "CS_TOKEN",
     "DS_TOKEN",
     "ES_TOKEN",
     "FS_TOKEN",
     "GS_TOKEN",
     "SS_TOKEN",
     "DREF_OPEN_TOKEN",
     "DREF_CLOSE_TOKEN",
     "MOD_TOKEN",
     "NUMBER_END",
     "GREATER_EQUAL_TOKEN",
     "LESS_EQUAL_TOKEN",
     "IP_TOKEN",
     "ASSIGNMENT_TOKEN",
     "DWORD_TOKEN",
     "WORD_TOKEN",
     "BYTE_TOKEN",
     "LOGICAL_AND_TOKEN",
     "LOGICAL_OR_TOKEN",
     "CF_TOKEN",
     "PF_TOKEN",
     "AF_TOKEN",
     "ZF_TOKEN",
     "SF_TOKEN",
     "IF_TOKEN",
     "DF_TOKEN",
     "OF_TOKEN",
     "VM_TOKEN",
     "AC_TOKEN",
     "BEGIN_BRACKET",
     "END_BRACKET",
     "NOT_EQUAL_TOKEN",
     "INVALID_NUMBER_TOKEN",
     "QWORD_TOKEN",
     "R8_TOKEN",             
     "R9_TOKEN",            
     "R10_TOKEN",           
     "R11_TOKEN",           
     "R12_TOKEN",           
     "R13_TOKEN",           
     "R14_TOKEN",           
     "R15_TOKEN",          
     "CS_ADDR_TOKEN",
     "DS_ADDR_TOKEN",
     "ES_ADDR_TOKEN",
     "FS_ADDR_TOKEN",
     "GS_ADDR_TOKEN",
     "SS_ADDR_TOKEN",
     "EAX_TOKEN",
     "EBX_TOKEN",
     "ECX_TOKEN",
     "EDX_TOKEN",
     "RAX_TOKEN",
     "RBX_TOKEN",
     "RCX_TOKEN",
     "RDX_TOKEN",
     "AL_TOKEN",
     "BL_TOKEN",
     "CL_TOKEN",
     "DL_TOKEN",
     "ESI_TOKEN",
     "EDI_TOKEN",
     "EBP_TOKEN",
     "ESP_TOKEN",
     "RSI_TOKEN",
     "RDI_TOKEN",
     "RBP_TOKEN",
     "RSP_TOKEN",
};

struct stackframe_symbols {	
	char *symbol;
 	int symbol_len;
	unsigned long type;
};

struct stackframe_symbols stackframeSymbols[] = {
	{ "CS",  2, CS_TOKEN },
	{ "CS:", 3, CS_ADDR_TOKEN },
	{ "DS",  2, DS_TOKEN },
	{ "DS:", 3, DS_ADDR_TOKEN },
	{ "ES",  2, ES_TOKEN },
	{ "ES:", 3, ES_ADDR_TOKEN },
	{ "FS",  2, FS_TOKEN },
	{ "FS:", 3, FS_ADDR_TOKEN },
	{ "GS",  2, GS_TOKEN },
	{ "GS:", 3, GS_ADDR_TOKEN },
	{ "SS",  2, SS_TOKEN },
	{ "SS:", 3, SS_ADDR_TOKEN },

	{ "AL", 2, AL_TOKEN },
	{ "BL", 2, BL_TOKEN },
	{ "CL", 2, CL_TOKEN },
	{ "DL", 2, DL_TOKEN },

	{ "AX", 2, AX_TOKEN },
	{ "BX", 2, BX_TOKEN },
	{ "CX", 2, CX_TOKEN },
	{ "DX", 2, DX_TOKEN },
	{ "SI", 2, SI_TOKEN },
	{ "DI", 2, DI_TOKEN },
	{ "BP", 2, BP_TOKEN },
	{ "SP", 2, SP_TOKEN },
	{ "IP", 2, IP_TOKEN },

	{ "EAX", 3, EAX_TOKEN },
	{ "EBX", 3, EBX_TOKEN },
	{ "ECX", 3, ECX_TOKEN },
	{ "EDX", 3, EDX_TOKEN },
	{ "ESI", 3, ESI_TOKEN },
	{ "EDI", 3, EDI_TOKEN },
	{ "EBP", 3, EBP_TOKEN },
	{ "ESP", 3, ESP_TOKEN },

	{ "EIP", 3, IP_TOKEN },

#ifdef CONFIG_X86_64
	{ "RAX", 3, RAX_TOKEN },
	{ "RBX", 3, RBX_TOKEN },
	{ "RCX", 3, RCX_TOKEN },
	{ "RDX", 3, RDX_TOKEN },
	{ "RSI", 3, RSI_TOKEN },
	{ "RDI", 3, RDI_TOKEN },
	{ "RBP", 3, RBP_TOKEN },
	{ "RSP", 3, RSP_TOKEN },
	{ "RIP", 3, IP_TOKEN },
	{ "R8",  2, R8_TOKEN },
	{ "R9",  2, R9_TOKEN },
	{ "R10", 3, R10_TOKEN },
	{ "R11", 3, R11_TOKEN },
	{ "R12", 3, R12_TOKEN },
	{ "R13", 3, R13_TOKEN },
	{ "R14", 3, R14_TOKEN },
	{ "R15", 3, R15_TOKEN },
#endif
	{ "PF", 2, PF_TOKEN },
	{ "ZF", 2, ZF_TOKEN },
	{ "IF", 2, IF_TOKEN },
	{ "OF", 2, OF_TOKEN },
	{ "VM",	2, VM_TOKEN }, 
	{ "AF",	2, AF_TOKEN }, 
	{ "AC",	2, AC_TOKEN },
	{ "CF",	2, CF_TOKEN }, 
	{ "DF",	2, DF_TOKEN },
	{ "SF",	2, SF_TOKEN },
};

static unsigned char TokenIndex[256];
static unsigned char TokenType[256];
static unsigned long long TokenValue[256];
static unsigned long long TokenCount;

static unsigned long long numStack[NUM_STACK_SIZE];
static unsigned long long *sp;
static unsigned long long *tos;
static unsigned long long *bos;

static unsigned long long segStack[SEG_STACK_SIZE];
static unsigned long long *s_sp;
static unsigned long long *s_tos;
static unsigned long long *s_bos;

static unsigned long long contextStack[CONTEXT_STACK_SIZE];
static unsigned long long *c_sp;
static unsigned long long *c_tos;
static unsigned long long *c_bos;

static unsigned long long booleanStack[BOOL_STACK_SIZE];
static unsigned long long *b_sp;
static unsigned long long *b_tos;
static unsigned long long *b_bos;

static unsigned long long logicalStack[LOGICAL_STACK_SIZE];
static unsigned long long *l_sp;
static unsigned long long *l_tos;
static unsigned long long *l_bos;

extern unsigned long GetValueFromSymbol(unsigned char *symbolName);
extern unsigned char delim_table[256];

static unsigned long token_stackframe_value(StackFrame *stackFrame, unsigned long type)
{
	if (!stackFrame)
	   return 0;

	switch (type) {
		case PF_TOKEN:
		    return stackFrame->tSystemFlags & PF_FLAG;
		case ZF_TOKEN:
		    return stackFrame->tSystemFlags & ZF_FLAG;
		case IF_TOKEN:
		    return stackFrame->tSystemFlags & IF_FLAG;
		case OF_TOKEN:
		    return stackFrame->tSystemFlags & OF_FLAG;
		case DF_TOKEN:
		    return stackFrame->tSystemFlags & DF_FLAG;
		case VM_TOKEN:
		    return stackFrame->tSystemFlags & VM_FLAG;
		case AF_TOKEN:
		    return stackFrame->tSystemFlags & AF_FLAG;
		case AC_TOKEN:
		    return stackFrame->tSystemFlags & AC_FLAG;
		case SF_TOKEN:
		    return stackFrame->tSystemFlags & SF_FLAG;
		case CF_TOKEN:
		    return stackFrame->tSystemFlags & CF_FLAG;

		case CS_ADDR_TOKEN:
		case CS_TOKEN:
		    return stackFrame->tCS;
		case DS_ADDR_TOKEN:
		case DS_TOKEN:
		    return stackFrame->tDS;
		case ES_ADDR_TOKEN:
		case ES_TOKEN:
		    return stackFrame->tES;
		case FS_ADDR_TOKEN:
		case FS_TOKEN:
		    return stackFrame->tFS;
		case GS_ADDR_TOKEN:
		case GS_TOKEN:
		    return stackFrame->tGS;
		case SS_ADDR_TOKEN:
		case SS_TOKEN:
		    return stackFrame->tSS;

		case AL_TOKEN:
		    return stackFrame->tAX & 0xFF;
		case BL_TOKEN:
		    return stackFrame->tBX & 0xFF;
		case CL_TOKEN:
		    return stackFrame->tCX & 0xFF;
		case DL_TOKEN:
		    return stackFrame->tDX & 0xFF;

		case AX_TOKEN:
		    return stackFrame->tAX & 0xFFFF;
		case BX_TOKEN:
		    return stackFrame->tBX & 0xFFFF;
		case CX_TOKEN:
		    return stackFrame->tCX & 0xFFFF;
		case DX_TOKEN:
		    return stackFrame->tDX & 0xFFFF;
		case SI_TOKEN:
		    return stackFrame->tSI & 0xFFFF;
		case DI_TOKEN:
		    return stackFrame->tDI & 0xFFFF;
		case BP_TOKEN:
		    return stackFrame->tBP & 0xFFFF;
		case SP_TOKEN:
 	   	    return stackFrame->tSP & 0xFFFF;

#ifdef CONFIG_X86_64
		case RAX_TOKEN:
		    return stackFrame->tAX;
		case RBX_TOKEN:
		    return stackFrame->tBX;
		case RCX_TOKEN:
		    return stackFrame->tCX;
		case RDX_TOKEN:
		    return stackFrame->tDX;
		case RSI_TOKEN:
		    return stackFrame->tSI;
		case RDI_TOKEN:
		    return stackFrame->tDI;
		case RBP_TOKEN:
		    return stackFrame->tBP;
		case RSP_TOKEN:
 	   	    return stackFrame->tSP;

		case EAX_TOKEN:
		    return stackFrame->tAX & 0xFFFFFFFF;
		case EBX_TOKEN:
		    return stackFrame->tBX & 0xFFFFFFFF;
		case ECX_TOKEN:
		    return stackFrame->tCX & 0xFFFFFFFF;
		case EDX_TOKEN:
		    return stackFrame->tDX & 0xFFFFFFFF;
		case ESI_TOKEN:
		    return stackFrame->tSI & 0xFFFFFFFF;
		case EDI_TOKEN:
		    return stackFrame->tDI & 0xFFFFFFFF;
		case EBP_TOKEN:
		    return stackFrame->tBP & 0xFFFFFFFF;
		case ESP_TOKEN:
 	   	    return stackFrame->tSP & 0xFFFFFFFF;

		case R8_TOKEN:
		    return stackFrame->r8;
		case R9_TOKEN:
		    return stackFrame->r9;
		case R10_TOKEN:
		    return stackFrame->r10;
		case R11_TOKEN:
		    return stackFrame->r11;
		case R12_TOKEN:
		    return stackFrame->r12;
		case R13_TOKEN:
		    return stackFrame->r13;
		case R14_TOKEN:
		    return stackFrame->r14;
		case R15_TOKEN:
		    return stackFrame->r15;
#else
		case EAX_TOKEN:
		    return stackFrame->tAX;
		case EBX_TOKEN:
		    return stackFrame->tBX;
		case ECX_TOKEN:
		    return stackFrame->tCX;
		case EDX_TOKEN:
		    return stackFrame->tDX;
		case ESI_TOKEN:
		    return stackFrame->tSI;
		case EDI_TOKEN:
		    return stackFrame->tDI;
		case EBP_TOKEN:
		    return stackFrame->tBP;
		case ESP_TOKEN:
 	   	    return stackFrame->tSP;
#endif
		case IP_TOKEN:
		    return stackFrame->tIP;

		default:
		   break;
	}
	return 0;
}

static inline unsigned long long GetNumber(unsigned char *p, 
                                           unsigned char **rp, 
                                           unsigned long *opl, 
                                           unsigned long *retCode, 
                                           unsigned long mode)
{

    unsigned char *op, *pp = NULL;
    unsigned long long c = 0;
    unsigned long decimal = 0, hex_found = 0, invalid = 0, valid = 0;

    if (rp)
       *rp = p;
    pp = op = p;

    if (!strncasecmp(p, "0x", 2))
    {
       hex_found = 1;
       p++;
       p++;
       pp = p;
    } 
    else
    if (*p == 'X' || *p == 'x')
    {
       hex_found = 1;
       p++;
       pp = p;
    }

    while (*p)
    {
       if (*p >= '0' && *p <= '9')
       {
          valid++;
	  p++;
       }
       else
       if (*p >= 'A' && *p <= 'F')
       {
	  hex_found = 1;
          valid++;
	  p++;
       }
       else
       if (*p >= 'a' && *p <= 'f')
       {
	  hex_found = 1;
          valid++;
	  p++;
       }
       else
       if (delim_table[((*p) & 0xFF)])
	  break;
       else
       {
          invalid = 1;
          break;
       }
    }

    if (invalid)
    {
       if (retCode)
          *retCode = -1;   /* invalid string */
       return 0;
    }

    if (rp)
       *rp = p;
    if (opl)
       *opl = (unsigned long)((unsigned long)p - (unsigned long) op);

    p = pp;

    if (mode)
       decimal = 1;

    if (hex_found)
    {
       /* parse as hex number */
       while (*p)
       {
	  if (*p >= '0' && *p <= '9')
	     c = (c << 4) | (*p - '0');
	  else if (*p >= 'A' && *p <= 'F')
	     c = (c << 4) | (*p - 'A' + 10);
	  else if (*p >= 'a' && *p <= 'f')
	     c = (c << 4) | (*p - 'a' + 10);
	  else
	     break;
	  p++;
       }
    }
    else
    if (decimal)
    {
       /* parse as decimal number */
       while (*p)
       {
	     if (*p >= '0' && *p <= '9')
		c = (c * 10) + (*p - '0');
	     else
		break;
	  p++;
       }
    }
    else  /* default parses as decimal */
    {
       /* parse as decimal number */
       while (*p)
       {
	     if (*p >= '0' && *p <= '9')
		c = (c * 10) + (*p - '0');
	     else
		break;
	  p++;
       }
    }

    if (retCode)
       *retCode = 0;

    return (c);

}

unsigned long GetValueFromToken(unsigned char *symbol, StackFrame *stackFrame,
				unsigned long *retCode, unsigned long *type)
{
    register int i, len;

    if (retCode)
       *retCode = -1;

	if (!stackFrame)
	   return 0;

    len = strlen(symbol);
    for (i = 0; len && i < ARRAY_SIZE(stackframeSymbols); 
         i++) 
    {
       if ((len == stackframeSymbols[i].symbol_len) && 
           (!strcasecmp(symbol, stackframeSymbols[i].symbol)))
       {
          if (type)
             *type = stackframeSymbols[i].type;
          if (retCode)
             *retCode = 0;
          return token_stackframe_value(stackFrame, stackframeSymbols[i].type);
       }
    }
    return 0;
}

static inline unsigned char *parseTokens(StackFrame *stackFrame, 
                                         unsigned char *p, 
                                         unsigned long mode)
{
    register unsigned long i, value;
    unsigned char symbol[256];
    unsigned char *op, *prev, *next;
    unsigned long delta, retCode = 0, type;

    op = p;
    TokenCount = 0;
    while (TokenCount < 255 && (unsigned long)p - (unsigned long)op < 255)
    {
       for (prev = p, i = 0; i < 255; i++) {
           if (delim_table[((p[i]) & 0xFF)])
                break;
           if (!isprint(p[i]))
                break;
	   symbol[i] = p[i];
       }
       symbol[i] = '\0';
       next = &p[i];

       if (*symbol) {
#if DEBUG_EXPRESS
          DBGPrint("symbol: [%s]\n", symbol);
#endif
          value = GetValueFromToken(symbol, stackFrame, &retCode, &type);
          if (retCode)
          {
	     value = GetNumber(symbol, NULL, &delta, &retCode, mode);
             if (retCode) 
             {
                value = GetValueFromSymbol(symbol);
                if (!value)
                {
	           TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
	           TokenValue[TokenCount] = value;
	           TokenType[TokenCount++] = INVALID_NUMBER_TOKEN;
                }
                else
                {
                   TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                   TokenValue[TokenCount] = value;
                   TokenType[TokenCount++] = NUMBER_TOKEN;
                }
             }
             else
             {
	        TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
	        TokenValue[TokenCount] = value;
	        TokenType[TokenCount++] = NUMBER_TOKEN;
             }
          }
          else
          {
	     TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
	     TokenValue[TokenCount] = value;
	     TokenType[TokenCount++] = type;
          }
          p = next;  
       }

       if (delim_table[*p & 0xFF]) {
#if DEBUG_EXPRESS
          if (*p)
             DBGPrint("delim: [%c]\n", *p);
          else
             DBGPrint("delim: [NULL]\n", *p);
#endif
          switch (*p) {

          case '\0':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = NULL_TOKEN;
	      return (p);

	  case ']':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = DREF_CLOSE_TOKEN;
	      p++;
	      break;

	  case '(':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = BB_TOKEN;
	      p++;
	      break;

	  case ')':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = EB_TOKEN;
	      p++;
	     break;

	  case '+':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = PLUS_TOKEN;
	      p++;
	      break;

	  case '-':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = MINUS_TOKEN;
	      p++;
	      break;

	  case '*':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = MULTIPLY_TOKEN;
	      p++;
	      break;

	  case '/':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = DIVIDE_TOKEN;
	      p++;
	      break;

	  case '%':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = MOD_TOKEN;
	      p++;
	      break;

	  case '~':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = NEG_TOKEN;
	      p++;
	      break;

	  case '^':
	      TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
              TokenValue[TokenCount] = 0;
	      TokenType[TokenCount++] = XOR_TOKEN;
	      p++;
	      break;

	  case '!':
	     p++;
	     if (*p == '=')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = NOT_EQUAL_TOKEN;
		p++;
		break;
	     }
	     TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
             TokenValue[TokenCount] = 0;
	     TokenType[TokenCount++] = NOT_TOKEN;
	     break;

	  case ' ':   /* drop spaces on the floor */
	     p++;
	     break;

	  case '[':
	     TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
             TokenValue[TokenCount] = 0;
	     TokenType[TokenCount++] = DREF_OPEN_TOKEN;
	     p++;
	     if (tolower(*p) == 'q' && *(p + 1) == ' ')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = QWORD_TOKEN;
		p++;
		break;
	     }
	     if (tolower(*p) == 'd' && *(p + 1) == ' ')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = DWORD_TOKEN;
		p++;
		break;
	     }
	     if (tolower(*p) == 'w' && *(p + 1) == ' ')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = WORD_TOKEN;
		p++;
		break;
	     }
	     if (tolower(*p) == 'b' && *(p + 1) == ' ')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = BYTE_TOKEN;
		p++;
		break;
	     }
	     break;

	  case '=':
	     p++;
	     if (*p == '=')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = EQUAL_TOKEN;
		p++;
		break;
	     }
	     TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
             TokenValue[TokenCount] = 0;
	     TokenType[TokenCount++] = ASSIGNMENT_TOKEN;
	     break;

	  case '<':
	     p++;
	     if (*p == '<')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = LEFT_SHIFT_TOKEN;
		p++;
		break;
	     }
	     if (*p == '=')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = LESS_EQUAL_TOKEN;
		p++;
		break;
	     }
	     TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
             TokenValue[TokenCount] = 0;
	     TokenType[TokenCount++] = LESS_TOKEN;
	     break;

	  case '>':
	     p++;
	     if (*p == '>')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = RIGHT_SHIFT_TOKEN;
		p++;
		break;
	     }
	     if (*p == '=')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = GREATER_EQUAL_TOKEN;
		p++;
		break;
	     }
	     TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
             TokenValue[TokenCount] = 0;
	     TokenType[TokenCount++] = GREATER_TOKEN;
	     break;

	  case '|':
	     p++;
	     if (*p == '|')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = LOGICAL_OR_TOKEN;
		p++;
		break;
	     }
	     TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
             TokenValue[TokenCount] = 0;
	     TokenType[TokenCount++] = OR_TOKEN;
	     break;

	  case '&':
	     p++;
	     if (*p == '&')
	     {
		TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
                TokenValue[TokenCount] = 0;
		TokenType[TokenCount++] = LOGICAL_AND_TOKEN;
		p++;
		break;
	     }
	     TokenIndex[TokenCount] = (unsigned long) ((unsigned long) p - (unsigned long) op);
             TokenValue[TokenCount] = 0;
	     TokenType[TokenCount++] = AND_TOKEN;
	     break;

	  default: 
	     break;

          }
       }

       /* if the string did not advance, bump the pointer. */
       if (*p && prev >= p) 
          p++;

    }  /* while */
    return p;

}

void displayExpressionHelp(void)
{
#ifdef CONFIG_X86_64
       if (DBGPrint("Arithmetic Operators\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("+   add\n")) return;
       if (DBGPrint("-   subtract\n")) return;
       if (DBGPrint("*   multiply\n")) return;
       if (DBGPrint("/   divide\n")) return;
       if (DBGPrint("<<  bit shift left\n")) return;
       if (DBGPrint(">>  bit shift right\n")) return;
       if (DBGPrint("|   OR operator\n")) return;
       if (DBGPrint("&   AND operator\n")) return;
       if (DBGPrint("^   XOR operator\n")) return;
       if (DBGPrint("~   NEG operator\n")) return;
       if (DBGPrint("%%   MODULO operator\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 1:\n")) return;
       if (DBGPrint("(0)> .e (0x100 + 0x100)\n")) return;
       if (DBGPrint("(0)> result = 0x200 (512)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 2:\n")) return;
       if (DBGPrint("(0)> .e (1 << 20)\n")) return;
       if (DBGPrint("(0)> result = 0x00100000 (1,024,000)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 3:\n")) return;
       if (DBGPrint("(0)> .e (0xFEF023 & 0x100F)\n")) return;
       if (DBGPrint("(0)> result = 0x1003 (4099)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Boolean Operators (Conditional Breakpoint)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("==      is equal to\n")) return;
       if (DBGPrint("!=      is not equal to\n")) return;
       if (DBGPrint("!<expr> is not\n")) return;
       if (DBGPrint(">       is greater than\n")) return;
       if (DBGPrint("<       is less than\n")) return;
       if (DBGPrint(">=      is greater than or equal to\n")) return;
       if (DBGPrint("<=      if less than or equal to\n")) return;
       if (DBGPrint("||      logical OR operator\n")) return;
       if (DBGPrint("&&      logical AND operator\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("all breakpoint conditions must be enclosed in brackets () to\n")) return;
       if (DBGPrint("evaluate correctly\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 1 (Execute Breakpoint):\n")) return;
       if (DBGPrint("(0)> b 0x37000 (RAX == 20 && RBX <= 4000)\n")) return;
       if (DBGPrint("breakpoint will activate if condition is true (returns 1)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 2 (RW Breakpoint):\n")) return;
       if (DBGPrint("(0)> br 0x3D4 (!RBX && [d RSI+40] != 2000)\n")) return;
       if (DBGPrint("breakpoint will activate if condition is true (returns 1)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Register Operators\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("RAX, RBX, RCX, RDX        - general registers\n")) return;
       if (DBGPrint(" R8,  R9, R10, R11        - general registers\n")) return;
       if (DBGPrint("R12, R13, R14, R15        - general registers\n")) return;
       if (DBGPrint("RSI, RDI, RBP, RSP        - pointer registers\n")) return;
       if (DBGPrint("RIP, <symbol>             - instruction pointer or symbol\n")) return;
       if (DBGPrint("CS, DS, ES, FS, GS, SS    - segment registers\n")) return;
       if (DBGPrint("CF, PF, AF, ZF, SF, IF    - flags\n")) return;
       if (DBGPrint("DF, OF, VM, AC\n")) return;
       if (DBGPrint("=                         - set equal to\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 1:\n")) return;
       if (DBGPrint("(0)> RAX = 0x0032700 \n")) return;
       if (DBGPrint("RAX changed to 0x0032700\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 2:\n")) return;
       if (DBGPrint("(0)> u thread_switch\n")) return;
       if (DBGPrint("unassembles function thread_switch\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 3 (Dump):\n")) return;
       if (DBGPrint("(0)> d RBP+RCX\n")) return;
       if (DBGPrint("(dumps [RBP + RCX])\n")) return;
       if (DBGPrint("[addr] 00 00 00 01 02 04 07 ...\n")) return;
       if (DBGPrint("[addr] 00 34 56 00 7A 01 00 ...\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Bracket Operators\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("(       begin expression bracket\n")) return;
       if (DBGPrint(")       end expression bracket\n")) return;
       if (DBGPrint("[       begin pointer\n")) return;
       if (DBGPrint("]       end pointer\n")) return;
       if (DBGPrint("q       QWORD reference\n")) return;
       if (DBGPrint("d       DWORD reference\n")) return;
       if (DBGPrint("w       WORD reference\n")) return;
       if (DBGPrint("b       BYTE reference\n")) return;
       if (DBGPrint("Note - QWORD, DWORD, WORD, and BYTE dereference operators must\n"))          return;
       if (DBGPrint("immediately follow pointer brackets (no spaces)\n"))                 return;
       if (DBGPrint("i.e.  [d <addr/symbol>] or [w <addr/symbol>] or\n"))
           return;
       if (DBGPrint("[b <addr/symbol>], etc.\n"))
           return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 1 (dump):\n")) return;
       if (DBGPrint("(0)> d [d RAX+100] \n")) return;
       if (DBGPrint("[rax + 100 (dec)] 00 00 00 01 02 04 07 00\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 2 (dump):\n")) return;
       if (DBGPrint("(0)> d [w 0x003400] \n")) return;
       if (DBGPrint("[addr (hex)] 00 22 00 01 02 04 07 ...\n")) return;
       if (DBGPrint("[addr (hex)] 00 31 A1 00 6A 05 00 ...\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 3 (break):\n")) return;
       if (DBGPrint("(0)> b = 0x7A000 (RAX + RCX == 30) && ([d 0xB8000+50]  == 0x07)\n")) return;
       if (DBGPrint("breakpoint will activate if condition is true (returns 1)\n")) return;
       if (DBGPrint("\n")) return;
#else
       if (DBGPrint("Arithmetic Operators\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("+   add\n")) return;
       if (DBGPrint("-   subtract\n")) return;
       if (DBGPrint("*   multiply\n")) return;
       if (DBGPrint("/   divide\n")) return;
       if (DBGPrint("<<  bit shift left\n")) return;
       if (DBGPrint(">>  bit shift right\n")) return;
       if (DBGPrint("|   OR operator\n")) return;
       if (DBGPrint("&   AND operator\n")) return;
       if (DBGPrint("^   XOR operator\n")) return;
       if (DBGPrint("~   NEG operator\n")) return;
       if (DBGPrint("%%   MODULO operator\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 1:\n")) return;
       if (DBGPrint("(0)> .e (0x100 + 0x100)\n")) return;
       if (DBGPrint("(0)> result = 0x200 (512)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 2:\n")) return;
       if (DBGPrint("(0)> .e (1 << 20)\n")) return;
       if (DBGPrint("(0)> result = 0x00100000 (1,024,000)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 3:\n")) return;
       if (DBGPrint("(0)> .e (0xFEF023 & 0x100F)\n")) return;
       if (DBGPrint("(0)> result = 0x1003 (4099)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Boolean Operators (Conditional Breakpoint)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("==      is equal to\n")) return;
       if (DBGPrint("!=      is not equal to\n")) return;
       if (DBGPrint("!<expr> is not\n")) return;
       if (DBGPrint(">       is greater than\n")) return;
       if (DBGPrint("<       is less than\n")) return;
       if (DBGPrint(">=      is greater than or equal to\n")) return;
       if (DBGPrint("<=      if less than or equal to\n")) return;
       if (DBGPrint("||      logical OR operator\n")) return;
       if (DBGPrint("&&      logical AND operator\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("all breakpoint conditions must be enclosed in brackets () to\n")) return;
       if (DBGPrint("evaluate correctly\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 1 (Execute Breakpoint):\n")) return;
       if (DBGPrint("(0)> b 37000 (EAX == 20 && EBX <= 4000)\n")) return;
       if (DBGPrint("breakpoint will activate if condition is true (returns 1)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 2 (RW Breakpoint):\n")) return;
       if (DBGPrint("(0)> br 3D4 (!EBX && [d ESI+40] != 2000)\n")) return;
       if (DBGPrint("breakpoint will activate if condition is true (returns 1)\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Register Operators\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("EAX, EBX, ECX, EDX        - general registers\n")) return;
       if (DBGPrint("ESI, EDI, EBP, ESP        - pointer registers\n")) return;
       if (DBGPrint("EIP, <symbol>             - instruction pointer or symbol\n")) return;
       if (DBGPrint("CS, DS, ES, FS, GS, SS    - segment registers\n")) return;
       if (DBGPrint("CF, PF, AF, ZF, SF, IF    - flags\n")) return;
       if (DBGPrint("DF, OF, VM, AC\n")) return;
       if (DBGPrint("=                         - set equal to\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 1:\n")) return;
       if (DBGPrint("(0)> EAX = 0x0032700 \n")) return;
       if (DBGPrint("EAX changed to 0x0032700\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 2:\n")) return;
       if (DBGPrint("(0)> u thread_switch\n")) return;
       if (DBGPrint("unassembles function thread_switch\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 3 (Dump):\n")) return;
       if (DBGPrint("(0)> d EBP+ECX\n")) return;
       if (DBGPrint("(dumps [d EBP + ECX])\n")) return;
       if (DBGPrint("[addr] 00 00 00 01 02 04 07 ...\n")) return;
       if (DBGPrint("[addr] 00 34 56 00 7A 01 00 ...\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Bracket Operators\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("(       begin expression bracket\n")) return;
       if (DBGPrint(")       end expression bracket\n")) return;
       if (DBGPrint("[       begin pointer\n")) return;
       if (DBGPrint("]       end pointer\n")) return;
       if (DBGPrint("d       DWORD reference\n")) return;
       if (DBGPrint("w       WORD reference\n")) return;
       if (DBGPrint("b       BYTE reference\n")) return;
       if (DBGPrint("Note - DWORD, WORD, and BYTE dereference operators must\n"))          return;
       if (DBGPrint("immediately follow pointer brackets (no spaces)\n"))                 return;
       if (DBGPrint("i.e.  [d <addr/symbol>] or [w <addr/symbol>] or\n"))
           return;
       if (DBGPrint("[b <addr/symbol>], etc.\n"))
           return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 1 (dump):\n")) return;
       if (DBGPrint("(0)> d [d EAX+100] \n")) return;
       if (DBGPrint("[eax + 100 (dec)] 00 00 00 01 02 04 07 00\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 2 (dump):\n")) return;
       if (DBGPrint("(0)> d [w 0x003400] \n")) return;
       if (DBGPrint("[addr (hex)] 00 22 00 01 02 04 07 ...\n")) return;
       if (DBGPrint("[addr (hex)] 00 31 A1 00 6A 05 00 ...\n")) return;
       if (DBGPrint("\n")) return;
       if (DBGPrint("Example 3 (break):\n")) return;
       if (DBGPrint("(0)> b = 0x7A000 (EAX + ECX == 30) && ([d 0xB8000+50]  == 0x07)\n")) return;
       if (DBGPrint("breakpoint will activate if condition is true (returns 1)\n")) return;
       if (DBGPrint("\n")) return;
#endif
       return;

}

static inline uint64_t deref(unsigned long type, 
                             unsigned long value,
                             int sizeflag, 
                             unsigned char **result,
                             unsigned long seg,
                             unsigned long sv)
{
   uint64_t *pq;
   unsigned long *pd;
   unsigned short *pw;
   unsigned char *pb;

#if (DEBUG_EXPRESS)
   DBGPrint("DEREF: %04X -> %04X:%lX\n", seg, sv, value);
#endif

   // if a sizeflag was specified, override the type field
   if (sizeflag)
   {
      switch (sizeflag)
      {
           // BYTE
           case 1:
	     pb = (unsigned char *) value;
	     return (unsigned char) mdb_segment_getword(sv,
                                                       (unsigned long)pb, 1);

           // WORD
           case 2:
	     pw = (unsigned short *) value;
	     return (unsigned short) mdb_segment_getword(sv,
                                                        (unsigned long)pw, 2);

           // DWORD
           case 4:
	     pd = (unsigned long *) value;
	     return (unsigned long) mdb_segment_getword(sv, 
                                                       (unsigned long)pd, 4);

           // QWORD
           case 8:
	     pq = (uint64_t *) value;
	     return (uint64_t) mdb_segment_getqword(sv, (uint64_t *)pq, 8);

           // FWORD
           case 6:
           // TBYTE (floating point)
           case 10:
           // VMMWORD 
           case 16:
             if (result)
                *result = (unsigned char *)value;
             return 0;

           // if an unknown sizeflag is passed, then default to type
           default:
              break;
      } 
   }

   switch (type)
   {
      case ARCH_PTR:
#ifdef CONFIG_X86_64 
	 pq = (uint64_t *) value;
	 return (uint64_t) mdb_segment_getqword(sv, (uint64_t *)pq, 8);
#else
	 pd = (unsigned long *) value;
	 return (unsigned long) mdb_segment_getword(sv, (unsigned long)pd, 4);
#endif
      case ULONGLONG_PTR:
	 pq = (uint64_t *) value;
	 return (uint64_t) mdb_segment_getqword(sv, (uint64_t *)pq, 8);

      case ULONG_PTR:
	 pd = (unsigned long *) value;
	 return (unsigned long) mdb_segment_getword(sv, (unsigned long)pd, 4);

      case WORD_PTR:
	 pw = (unsigned short *) value;
	 return (unsigned short) mdb_segment_getword(sv, (unsigned long)pw, 2);

      case BYTE_PTR:
	 pb = (unsigned char *) value;
	 return (unsigned char) mdb_segment_getword(sv, (unsigned long)pb, 1);

      default:
	 return 0;
   }

}

static inline unsigned long SegmentPush(unsigned long i)
{
     if (s_sp > s_bos)
     {
#if (DEBUG_EXPRESS)
	DBGPrint("spush : <err>\n");
#endif
	return 0;
     }
     *s_sp = i;
#if (DEBUG_EXPRESS)
     DBGPrint("spush : %lX (%d)\n", *s_sp, *s_sp);
#endif
     s_sp++;
     return 1;
}

static inline unsigned long SegmentPop(void)
{
    s_sp--;
    if (s_sp < s_tos)
    {
       s_sp++;
#if (DEBUG_EXPRESS)
       DBGPrint("spop  : <err>\n");
#endif
       return -1;
    }
#if (DEBUG_EXPRESS)
    DBGPrint("spop  : %lX (%d)\n", *s_sp, *s_sp);
#endif
    return *s_sp;

}

static inline unsigned long ExpressPush(unsigned long long i)
{
     if (sp > bos)
     {
#if (DEBUG_EXPRESS)
	DBGPrint("push : <err>\n");
#endif
	return 0;
     }
     *sp = i;
#if (DEBUG_EXPRESS)
     DBGPrint("push : %lX (%d)\n", *sp, *sp);
#endif
     sp++;
     return 1;
}

static inline unsigned long long ExpressPop(void)
{
    sp--;
    if (sp < tos)
    {
       sp++;
#if (DEBUG_EXPRESS)
       DBGPrint("pop  : <err>\n");
#endif
       return 0;
    }
#if (DEBUG_EXPRESS)
    DBGPrint("pop  : %lX (%d)\n", *sp, *sp);
#endif
    return *sp;

}

static inline unsigned long ContextPush(unsigned long long i)
{
     if (c_sp > c_bos)
     {
#if (DEBUG_EXPRESS)
	DBGPrint("cpush: <err>\n");
#endif
	return 0;
     }
     *c_sp = i;
#if (DEBUG_EXPRESS)
     DBGPrint("cpush: %lX (%d)\n", *c_sp, *c_sp);
#endif
     c_sp++;
     return 1;
}

static inline unsigned long long ContextPop(void)
{
    c_sp--;
    if (c_sp < c_tos)
    {
       c_sp++;
#if (DEBUG_EXPRESS)
       DBGPrint("cpop : <err>\n");
#endif
       return 0;
    }
#if (DEBUG_EXPRESS)
    DBGPrint("cpop : %lX (%d)\n", *c_sp, *c_sp);
#endif
    return *c_sp;

}

static inline unsigned long BooleanPush(unsigned long long i)
{
     if (b_sp > b_bos)
     {
#if (DEBUG_BOOL_STACK)
	DBGPrint("bpush: <err>\n");
#endif
	return 0;
     }
     *b_sp = i;
#if (DEBUG_BOOL_STACK)
     DBGPrint("bpush: %lX (%d)\n", *b_sp, *b_sp);
#endif
     b_sp++;
     return 1;
}

static inline unsigned long long BooleanPop(void)
{
    b_sp--;
    if (b_sp < b_tos)
    {
       b_sp++;
#if (DEBUG_BOOL_STACK)
       DBGPrint("bpop : <err>\n");
#endif
       return 0;
    }
#if (DEBUG_BOOL_STACK)
    DBGPrint("bpop : %lX (%d)\n", *b_sp, *b_sp);
#endif
    return *b_sp;

}

static inline unsigned long LogicalPush(unsigned long long i)
{
     if (l_sp > l_bos)
     {
#if (DEBUG_LOGICAL_STACK)
	DBGPrint("lpush: <err>\n");
#endif
	return 0;
     }
     *l_sp = i;
#if (DEBUG_LOGICAL_STACK)
     DBGPrint("lpush: %lX (%d)\n", *l_sp, *l_sp);
#endif
     l_sp++;
     return 1;
}

static inline unsigned long long LogicalPop(void)
{
    l_sp--;
    if (l_sp < l_tos)
    {
       l_sp++;
#if (DEBUG_LOGICAL_STACK)
       DBGPrint("lpop : <err>\n");
#endif
       return 0;
    }
#if (DEBUG_LOGICAL_STACK)
    DBGPrint("lpop : %lX (%d)\n", *l_sp, *l_sp);
#endif
    return *l_sp;

}

static inline void initNumericStacks(void)
{

    sp = numStack;
    tos = sp;
    bos = sp + NUM_STACK_SIZE - 1;

    s_sp = segStack;
    s_tos = s_sp;
    s_bos = s_sp + SEG_STACK_SIZE - 1;

    c_sp = contextStack;
    c_tos = c_sp;
    c_bos = c_sp + CONTEXT_STACK_SIZE - 1;

    b_sp = booleanStack;
    b_tos = b_sp;
    b_bos = b_sp + BOOL_STACK_SIZE - 1;

    l_sp = logicalStack;
    l_tos = l_sp;
    l_bos = l_sp + LOGICAL_STACK_SIZE - 1;

}

static inline unsigned long ProcessOperator(unsigned long oper)
{
    unsigned long a, b;

    b = ExpressPop();
    a = ExpressPop();
    switch(oper)
    {
       case NEG_TOKEN:
	  break;

       case LEFT_SHIFT_TOKEN:
	  ExpressPush(a << (b % PROCESSOR_WIDTH));  /* mod (b) to base */
	  break;

       case RIGHT_SHIFT_TOKEN:
	  ExpressPush(a >> (b % PROCESSOR_WIDTH));  /* mob (b) to base */
	  break;

       case PLUS_TOKEN:
	  ExpressPush(a + b);
	  break;

       case XOR_TOKEN:
	  ExpressPush(a ^ b);
	  break;

       case AND_TOKEN:
	  ExpressPush(a & b);
	  break;

       case MOD_TOKEN:
	  if (b) /* if modulo by zero, drop value on the floor */
	     ExpressPush(a % b);
	  else
	     ExpressPush(0);
	  break;

       case OR_TOKEN:
	  ExpressPush(a | b);
	  break;

       case MINUS_TOKEN:
	  ExpressPush(a - b);
	  break;

       case MULTIPLY_TOKEN:
	  ExpressPush(a * b);
	  break;

       case DIVIDE_TOKEN:
	  if (b) /* if divide by zero, drop value on the floor */
	     ExpressPush(a / b);
	  else
	     ExpressPush(0);
	  break;

    }
    return 0;

}

static inline unsigned long ProcessBoolean(unsigned long oper)
{

    unsigned long a, b;

    b = ExpressPop();
    a = ExpressPop();
    switch(oper)
    {
       case NOT_TOKEN:
	  ExpressPush(a == b); /* we pushed an imaginary zero on the stack */
	  break;             /* this operation returns the boolean for (!x) */

       case GREATER_TOKEN:
	  ExpressPush(a > b);
	  break;

       case LESS_TOKEN:
	  ExpressPush(a < b);
	  break;

       case GREATER_EQUAL_TOKEN:
	  ExpressPush(a >= b);
	  break;

       case LESS_EQUAL_TOKEN:
	  ExpressPush(a <= b);
	  break;

       case EQUAL_TOKEN:
	  ExpressPush(a == b);
	  break;

       case NOT_EQUAL_TOKEN:
	  ExpressPush(a != b);
	  break;
    }
    return 0;

}

static inline unsigned long ProcessLogical(unsigned long oper)
{

    unsigned long a, b;

    b = ExpressPop();
    a = ExpressPop();
    switch(oper)
    {
       case LOGICAL_AND_TOKEN:
	  ExpressPush(a && b);
	  break;

       case LOGICAL_OR_TOKEN:
	  ExpressPush(a || b);
	  break;
    }
    return 0;

}

static inline unsigned long ParseLogical(unsigned long logicalCount)
{

    register int i, r;
    unsigned long a;
    unsigned long c = 0, lastClass = 0, oper = 0;

    for (i = 0; i < logicalCount; i++)
       ExpressPush(LogicalPop());

    for (i = 0, r = 0; i < (logicalCount / 2); i++)
    {
       a = ExpressPop();
       TokenType[r] = NUMBER_TOKEN;
       TokenValue[r++] = a;
       a = ExpressPop();
       TokenType[r] = a;  /* get the operator type */
       TokenValue[r++] = 0;
    }

    initNumericStacks();

#if (DEBUG_LOGICAL)
     DBGPrint("\n");
#endif
    for (i = 0; i < logicalCount; i++)
    {
#if DEBUG_LOGICAL
       DBGPrint("token: %02X  value: %lX  type: %s\n", TokenType[i],
	      TokenValue[i], parserDescription[TokenType[i]]);
#endif
       switch (TokenType[i])
       {
	  case LOGICAL_AND_TOKEN:
	  case LOGICAL_OR_TOKEN:
	     if (lastClass != CLASS_BOOLEAN)
	     {
		lastClass = CLASS_BOOLEAN;
		oper = TokenType[i];
	     }
	     continue;

	  case NUMBER_TOKEN:
	     if (lastClass == CLASS_DATA)
	     {
		c = ExpressPop();
		return c;
	     }
	     lastClass = CLASS_DATA;
	     c = TokenValue[i];
	     ExpressPush(c);
	     if (oper)
		oper = ProcessLogical(oper);
	     continue;

	  case NULL_TOKEN:
	     c = ExpressPop();
	     return c;

	  default:
	     continue;
       }
    }
    return c;

}

static inline unsigned long ParseBoolean(unsigned long booleanCount)
{

    register int i, r;
    unsigned long a, oper = 0;
    unsigned long c = 0, lastClass = 0, logicalCount = 0;

    for (i = 0; i < booleanCount; i++)
       ExpressPush(BooleanPop());

    for (i = 0, r = 0; i < (booleanCount / 2); i++)
    {
       a = ExpressPop();
       TokenType[r] = NUMBER_TOKEN;
       TokenValue[r++] = a;
       a = ExpressPop();
       TokenType[r] = a;  /* get the operator type */
       TokenValue[r++] = 0;
    }

    initNumericStacks();

#if (DEBUG_BOOL)
     DBGPrint("\n");
#endif
    for (i = 0; i < booleanCount; i++)
    {
#if DEBUG_BOOL
       DBGPrint("token: %02X  value: %lX  type: %s\n", TokenType[i],
	      TokenValue[i], parserDescription[TokenType[i]]);
#endif
       switch (TokenType[i])
       {
	  /* partition operators */
	  case LOGICAL_AND_TOKEN:
	  case LOGICAL_OR_TOKEN:
	     c = ExpressPop();
	     LogicalPush(c);
	     logicalCount++;
	     LogicalPush(TokenType[i]);
	     logicalCount++;
	     ExpressPush(c);
	     oper = 0;
	     lastClass = 0;
	     continue;

	  /* boolean operators */
	  case NOT_TOKEN:
	     if (lastClass != CLASS_BOOLEAN)
	     {
		ExpressPush(0);
		lastClass = CLASS_BOOLEAN;
		oper = TokenType[i];
	     }
	     continue;

	  case GREATER_TOKEN:
	  case LESS_TOKEN:
	  case GREATER_EQUAL_TOKEN:
	  case LESS_EQUAL_TOKEN:
	  case EQUAL_TOKEN:
	  case NOT_EQUAL_TOKEN:
	     if (lastClass != CLASS_BOOLEAN)
	     {
		lastClass = CLASS_BOOLEAN;
		oper = TokenType[i];
	     }
	     continue;

	  case NUMBER_TOKEN:
	     if (lastClass == CLASS_DATA)
	     {
		c = ExpressPop();
		if (logicalCount)
		{
		   LogicalPush(c);
		   logicalCount++;
		   LogicalPush(0); /* push null token */
		   logicalCount++;
		   c = ParseLogical(logicalCount);
		   return c;
		}
		return c;
	     }
	     lastClass = CLASS_DATA;
	     c = TokenValue[i];
	     ExpressPush(c);
	     if (oper)
		oper = ProcessBoolean(oper);
	     continue;

	  case NULL_TOKEN:
	     c = ExpressPop();
	     if (logicalCount)
	     {
		LogicalPush(c);
		logicalCount++;
		LogicalPush(0); /* push null token */
		logicalCount++;
		c = ParseLogical(logicalCount);
		return c;
	     }
	     return c;

	  default:
	     continue;
       }
    }
    return c;

}

uint64_t Evaluate(StackFrame *stackFrame, 
                                unsigned char **p, 
                                unsigned long *type, 
                                unsigned long mode,
                                int sizeflag, 
                                unsigned char **result)
{
     register int i;
     unsigned long oper = 0, dref = 0, bracket = 0;
     unsigned long dref_type = ARCH_PTR, lastClass = 0, lastToken = 0;
     unsigned long neg_flag = 0, negative_flag = 0;
     uint64_t c;
     unsigned long booleanCount = 0, segment = -1, segmentCount = 0,
                   segment_value = -1;

#ifdef MDB_ATOMIC
     spin_lock_irqsave(&expressLock, flags);
#endif

     if (type)
	*type = INVALID_EXPRESSION;
#if (DEBUG_BOOL)
     DBGPrint("\n");
#endif
#if (DEBUG_EXPRESS)
     DBGPrint("\np: %lX  %s\n", *p, *p);
#endif
     parseTokens(stackFrame, *p, mode);
     if (TokenCount)
     {
	initNumericStacks();
	for (i = 0; i < TokenCount; i++)
	{
#if (DEBUG_EXPRESS)
	   DBGPrint("token: %s value %0lX lastClass: %d\n", 
                    parserDescription[TokenType[i]], 
                    TokenValue[i],
                    lastClass);
#endif
	   switch (TokenType[i])
	   {
	      case INVALID_NUMBER_TOKEN:
                 goto evaluate_error_exit;

	      case NOT_TOKEN:
		 if (lastClass != CLASS_DATA)
		 {
		    if (oper)
		       oper = ProcessOperator(oper);
		    c = ExpressPop();
		    BooleanPush(c);
		    booleanCount++;
		    BooleanPush(TokenType[i]);
		    booleanCount++;
		    dref_type = ARCH_PTR;
		    lastClass = 0;
		    neg_flag  = 0;
		    negative_flag = 0;
		 }
		 lastToken = NOT_TOKEN;
		 continue;

	      /* assignment operators */
	      case ASSIGNMENT_TOKEN:
		 if (lastClass == CLASS_DATA)
                 {
		    ExpressPop();
		    dref_type = ARCH_PTR;
		    lastClass = 0;
		    neg_flag  = 0;
		    negative_flag = 0;
                 }
		 lastToken = 0;
		 continue;

	      /* boolean operators */
	      case GREATER_TOKEN:
	      case LESS_TOKEN:
	      case GREATER_EQUAL_TOKEN:
	      case LESS_EQUAL_TOKEN:
	      case LOGICAL_AND_TOKEN:
	      case LOGICAL_OR_TOKEN:
	      case EQUAL_TOKEN:
	      case NOT_EQUAL_TOKEN:
		 if (oper)
		    oper = ProcessOperator(oper);
		 c = ExpressPop();
		 BooleanPush(c);
		 booleanCount++;
		 BooleanPush(TokenType[i]);
		 booleanCount++;
		 dref_type = ARCH_PTR;
		 lastClass = 0;
		 neg_flag  = 0;
		 negative_flag = 0;
		 lastToken = 0;
		 continue;

	      /* partition operators */
	      case QWORD_TOKEN:
		 if (dref)
		    dref_type = ULONGLONG_PTR;
		 lastToken = 0;
		 continue;

	      case DWORD_TOKEN:
		 if (dref)
		    dref_type = ULONG_PTR;
		 lastToken = 0;
		 continue;

	      case WORD_TOKEN:
		 if (dref)
		    dref_type = WORD_PTR;
		 lastToken = 0;
		 continue;

	      case BYTE_TOKEN:
		 if (dref)
		    dref_type = BYTE_PTR;
		 lastToken = 0;
		 continue;

	      case DREF_OPEN_TOKEN:   /* push state and nest for de-reference */
		 if (lastClass == CLASS_DATA)
		 {
		    *p = (unsigned char *)((unsigned long)*p + (unsigned long)TokenIndex[i]);
		    if (type)
		    {
		       if (booleanCount)
			  *type = BOOLEAN_EXPRESSION;
		       else
			  *type = NUMERIC_EXPRESSION;
		    }
		    c = ExpressPop();
		    if (booleanCount)
		    {
		       BooleanPush(c);
		       booleanCount++;
		       BooleanPush(0); /* last operator is the null token */
		       booleanCount++;
		       c = ParseBoolean(booleanCount);
#if (DEBUG_BOOL)
		       DBGPrint("be_N : (%d) = (%s)\n", c, c ? "TRUE" : "FALSE");
#endif
#ifdef MDB_ATOMIC
		       spin_unlock_irqrestore(&expressLock, flags);
#endif
		       return c;
		    }
#if (DEBUG_EXPRESS)
		    DBGPrint("ee_N : %lX (%d)\n", c, c);
#endif
#ifdef MDB_ATOMIC
		    spin_unlock_irqrestore(&expressLock, flags);
#endif
		    return c;
		 }
		 dref++;
		 ContextPush(dref_type);
		 ContextPush(oper);
		 ContextPush(lastClass);
		 ContextPush(neg_flag);
		 ContextPush(negative_flag);
		 dref_type = ARCH_PTR;
		 oper      = 0;
		 lastClass = 0;
		 neg_flag  = 0;
		 negative_flag = 0;
		 lastToken = 0;
		 continue;

	      case DREF_CLOSE_TOKEN: /* pop state,restore,and complete oper */
		 if (!dref)
		    continue;

		 c = deref(dref_type, ExpressPop(), sizeflag, result, 
                           segment, segment_value);
		 ExpressPush(c);
		 negative_flag  = ContextPop();
		 neg_flag  = ContextPop();
		 ContextPop();
		 oper      = ContextPop();
		 dref_type = ContextPop();
		 if (dref)
		    dref--;
		 lastClass = CLASS_DATA;

		 c = ExpressPop();
		 if (negative_flag)
		    c = 0 - c;
		 if (neg_flag)
		    c = ~c;
		 neg_flag = 0;
		 negative_flag = 0;
		 ExpressPush(c);

		 if (oper)
		    oper = ProcessOperator(oper);
		 lastToken = 0;
		 continue;

	      case BB_TOKEN:
		 if (lastClass == CLASS_DATA)
		 {
		    *p = (unsigned char *)((unsigned long)*p + (unsigned long)TokenIndex[i]);
		    if (type)
		    {
		       if (booleanCount)
			  *type = BOOLEAN_EXPRESSION;
		       else
			  *type = NUMERIC_EXPRESSION;
		    }
		    c = ExpressPop();
		    if (booleanCount)
		    {
		       BooleanPush(c);
		       booleanCount++;
		       BooleanPush(0); /* last operator is the null token */
		       booleanCount++;
		       c = ParseBoolean(booleanCount);
#if (DEBUG_BOOL)
		       DBGPrint("be_N : (%d) = (%s)\n", c, c ? "TRUE" : "FALSE");
#endif
#ifdef MDB_ATOMIC
		       spin_unlock_irqrestore(&expressLock, flags);
#endif
		       return c;
		    }
#if (DEBUG_EXPRESS)
		    DBGPrint("ee_N : %lX (%d)\n", c, c);
#endif
#ifdef MDB_ATOMIC
		    spin_unlock_irqrestore(&expressLock, flags);
#endif
		    return c;
		 }
		 bracket++;
		 ContextPush(oper);
		 ContextPush(lastClass);
		 ContextPush(neg_flag);
		 ContextPush(negative_flag);
		 oper      = 0;
		 lastClass = 0;
		 neg_flag  = 0;
		 negative_flag = 0;
		 lastToken = 0;
		 continue;

	      case EB_TOKEN:
		 if (!bracket)
		    continue;
		 negative_flag  = ContextPop();
		 neg_flag  = ContextPop();
		 ContextPop();
		 oper      = ContextPop();
		 if (bracket)
		    bracket--;
		 lastClass = CLASS_DATA;
		 c = ExpressPop();
		 if (negative_flag)
		    c = 0 - c;
		 if (neg_flag)
		    c = ~c;
		 neg_flag = 0;
		 negative_flag = 0;
		 ExpressPush(c);
		 if (oper)
		    oper = ProcessOperator(oper);
		 lastToken = 0;
		 continue;

	      /* arithmetic operators */
	      case NEG_TOKEN:
		 neg_flag = 1;
		 lastToken = 0;
		 continue;

	      case MINUS_TOKEN:
		 if (lastClass == CLASS_ARITHMETIC)
		 {
		    lastToken = MINUS_TOKEN;
		    negative_flag = 1;
		    continue;
		 }
		 if (lastClass != CLASS_ARITHMETIC)
		 {
		    lastClass = CLASS_ARITHMETIC;
		    oper = TokenType[i];
		 }
		 lastToken = 0;
		 continue;

	      case PLUS_TOKEN:
	      case LEFT_SHIFT_TOKEN:
	      case RIGHT_SHIFT_TOKEN:
	      case XOR_TOKEN:
	      case AND_TOKEN:
	      case MOD_TOKEN:
	      case OR_TOKEN:
	      case MULTIPLY_TOKEN:
	      case DIVIDE_TOKEN:
		 if (lastClass != CLASS_ARITHMETIC)
		 {
		    lastClass = CLASS_ARITHMETIC;
		    oper = TokenType[i];
		 }
		 lastToken = 0;
		 continue;

	      /* data operators */
	      case CF_TOKEN:
	      case PF_TOKEN:
	      case AF_TOKEN:
	      case ZF_TOKEN:
	      case SF_TOKEN:
	      case IF_TOKEN:
	      case DF_TOKEN:
	      case OF_TOKEN:
	      case VM_TOKEN:
	      case AC_TOKEN:
	      case IP_TOKEN:
	      case FLAGS_TOKEN:
	      case AL_TOKEN:
	      case BL_TOKEN:
	      case CL_TOKEN:
	      case DL_TOKEN:
	      case AX_TOKEN:
	      case BX_TOKEN:
	      case CX_TOKEN:
	      case DX_TOKEN:
	      case SI_TOKEN:
	      case DI_TOKEN:
	      case BP_TOKEN:
	      case SP_TOKEN:
	      case R8_TOKEN:
	      case R9_TOKEN:
	      case R10_TOKEN:
	      case R11_TOKEN:
	      case R12_TOKEN:
	      case R13_TOKEN:
	      case R14_TOKEN:
	      case R15_TOKEN:
	      case NUMBER_TOKEN:
	      case EAX_TOKEN:
	      case EBX_TOKEN:
	      case ECX_TOKEN:
	      case EDX_TOKEN:
	      case RAX_TOKEN:
	      case RBX_TOKEN:
	      case RCX_TOKEN:
	      case RDX_TOKEN:
	      case ESI_TOKEN:
	      case EDI_TOKEN:	
	      case EBP_TOKEN:
	      case ESP_TOKEN:
	      case RSI_TOKEN:
	      case RDI_TOKEN:
	      case RBP_TOKEN:
	      case RSP_TOKEN:
                 // get the last segment associated with this data token
                 if (segmentCount)
                 {
                    segment = SegmentPop();
                    segment_value = SegmentPop();
                    segmentCount--;
                 }

		 if (lastClass == CLASS_DATA)
		 {
		    *p = (unsigned char *)((unsigned long)*p + (unsigned long)TokenIndex[i]);
		    if (type)
		    {
		       if (booleanCount)
			  *type = BOOLEAN_EXPRESSION;
		       else
			  *type = NUMERIC_EXPRESSION;
		    }
		    c = ExpressPop();
		    if (booleanCount)
		    {
		       BooleanPush(c);
		       booleanCount++;
		       BooleanPush(0); /* last operator is the null token */
		       booleanCount++;
		       c = ParseBoolean(booleanCount);
#if (DEBUG_BOOL)
		       DBGPrint("be_N : (%d) = (%s)\n", c, c ? "TRUE" : "FALSE");
#endif
#ifdef MDB_ATOMIC
		       spin_unlock_irqrestore(&expressLock, flags);
#endif
		       return c;
		    }
#if (DEBUG_EXPRESS)
		    DBGPrint("ee_N : %lX (%d)\n", c, c);
#endif
#ifdef MDB_ATOMIC
		    spin_unlock_irqrestore(&expressLock, flags);
#endif
		    return c;
		 }
		 lastClass = CLASS_DATA;
		 c = TokenValue[i];
		 if (negative_flag)
		    c = 0 - c;
		 if (neg_flag)
		    c = ~TokenValue[i];
		 neg_flag = 0;
		 negative_flag = 0;
		 ExpressPush(c);
		 if (oper)
		    oper = ProcessOperator(oper);
		 lastToken = 0;
		 continue;

              // if a segment token is tagged for derefence, push segment
	      case CS_ADDR_TOKEN:
	      case DS_ADDR_TOKEN:
	      case ES_ADDR_TOKEN:
	      case FS_ADDR_TOKEN:
	      case GS_ADDR_TOKEN:
	      case SS_ADDR_TOKEN:
                 SegmentPush(TokenValue[i]);
                 SegmentPush(TokenType[i]);
                 segmentCount++;
		 lastToken = 0;
		 continue;

	      case CS_TOKEN:
	      case DS_TOKEN:
	      case ES_TOKEN:
	      case FS_TOKEN:
	      case GS_TOKEN:
	      case SS_TOKEN:
		 if (lastClass == CLASS_DATA)
		 {
		    *p = (unsigned char *)((unsigned long)*p + (unsigned long)TokenIndex[i]);
		    if (type)
		    {
		       if (booleanCount)
			  *type = BOOLEAN_EXPRESSION;
		       else
			  *type = NUMERIC_EXPRESSION;
		    }
		    c = ExpressPop();
		    if (booleanCount)
		    {
		       BooleanPush(c);
		       booleanCount++;
		       BooleanPush(0); /* last operator is the null token */
		       booleanCount++;
		       c = ParseBoolean(booleanCount);
#if (DEBUG_BOOL)
		       DBGPrint("be_N : (%d) = (%s)\n", c, c ? "TRUE" : "FALSE");
#endif
#ifdef MDB_ATOMIC
		       spin_unlock_irqrestore(&expressLock, flags);
#endif
		       return c;
		    }
#if (DEBUG_EXPRESS)
		    DBGPrint("ee_N : %lX (%d)\n", c, c);
#endif
#ifdef MDB_ATOMIC
		    spin_unlock_irqrestore(&expressLock, flags);
#endif
		    return c;
		 }
		 lastClass = CLASS_DATA;
		 c = TokenValue[i];
		 if (negative_flag)
		    c = 0 - c;
		 if (neg_flag)
		    c = ~TokenValue[i];
		 neg_flag = 0;
		 negative_flag = 0;
		 ExpressPush(c);
		 if (oper)
		    oper = ProcessOperator(oper);
		 lastToken = 0;
		 continue;

	      case NULL_TOKEN:
		 *p = (unsigned char *)((unsigned long)*p + (unsigned long)TokenIndex[i]);
		 if (TokenCount > 1 && type)
		 {
		    if (booleanCount)
		       *type = BOOLEAN_EXPRESSION;
		    else
		       *type = NUMERIC_EXPRESSION;
		 }
		 c = ExpressPop();
		 if (booleanCount)
		 {
		    BooleanPush(c);
		    booleanCount++;
		    BooleanPush(0); /* last operator is the null token */
		    booleanCount++;
		    c = ParseBoolean(booleanCount);
#if (DEBUG_BOOL)
		    DBGPrint("be_N : (%d) = (%s)\n", c, c ? "TRUE" : "FALSE");
#endif
#ifdef MDB_ATOMIC
		    spin_unlock_irqrestore(&expressLock, flags);
#endif
		    return c;
		 }
#if (DEBUG_EXPRESS)
		 DBGPrint("ee_N : %lX (%d)\n", c, c);
#endif
#ifdef MDB_ATOMIC
		 spin_unlock_irqrestore(&expressLock, flags);
#endif
		 return c;

	      default:
		 lastToken = 0;
		 continue;
	   }
	}
     }

evaluate_error_exit:
     if (type)
	*type = INVALID_EXPRESSION;

     if (lastToken) {}

#ifdef MDB_ATOMIC
     spin_unlock_irqrestore(&expressLock, flags);
#endif
     return 0;

}

uint64_t EvaluateDisassemblyExpression(StackFrame *stackFrame, unsigned char **p, unsigned long *type, int sizeflag, unsigned char **result)
{
     register uint64_t c;
#if DEBUG_EXPRESS
     unsigned char *s = *p;
#endif
     if (result)
        *result = NULL;
     c = Evaluate(stackFrame, p, type, 1, sizeflag, result);
#if DEBUG_EXPRESS
     DBGPrint("EDE expr: [%s]\n", s);
#endif
     return c;
}

uint64_t EvaluateNumericExpression(StackFrame *stackFrame, unsigned char **p, unsigned long *type)
{
     register uint64_t c;
#if DEBUG_EXPRESS
     unsigned char *s = *p;
#endif
     c = Evaluate(stackFrame, p, type, 1, 0, NULL);
#if DEBUG_EXPRESS
     DBGPrint("ENE expr: [%s]\n", s);
#endif
     return c;
}

uint64_t EvaluateExpression(StackFrame *stackFrame, unsigned char **p, unsigned long *type)
{
     register uint64_t c;
#if DEBUG_EXPRESS
     unsigned char *s = *p;
#endif
     c = Evaluate(stackFrame, p, type, 0, 0, NULL);
#if DEBUG_EXPRESS
     DBGPrint("EE expr: [%s] ret %0llX type %0lX\n", s, c, type ? *type : 0);
#endif
     return c;
}

void EvaluateCommandExpression(StackFrame *stackFrame, unsigned char *p)
{
     unsigned char *expr;
     unsigned long type;
     uint64_t c;

#if DEBUG_EXPRESS
     DBGPrint("expr: [%s]\n", p);
#endif
     expr = p;
     c = EvaluateExpression(stackFrame, &p, &type);
     if (type)
     {
	DBGPrint("expr: %s = 0x%llX (%lld) (%s) bool(%i) = %s\n",
		    expr, c, c, exprDescription[type & 3],
		    (c) ? 1 : 0, (c) ? "TRUE" : "FALSE");
     }
     else
     {
        DBGPrint("expression parameters invalid\n");
	DBGPrint("expr: %s = 0x%llX (%lld) (results invalid) (%s)"
                 " bool(%i) = %s\n",
		 expr, c, c, exprDescription[type & 3],
		 (c) ? 1 : 0, (c) ? "TRUE" : "FALSE");
     }
     return;

}
