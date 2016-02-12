
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

#ifndef _MDB_OS_H
#define _MDB_OS_H

#define MAX_SYMBOL_LEN  KSYM_NAME_LEN + 1

extern atomic_t inmdb;
extern int pause_mode;
extern struct task_struct *mdb_current_task;
extern unsigned char *mdb_oops;
extern unsigned char *last_mdb_oops;
extern unsigned char symbuf[MAX_SYMBOL_LEN];
extern unsigned char modbuf[MAX_SYMBOL_LEN];
extern unsigned char workbuf[MAX_SYMBOL_LEN];

unsigned long mdb_kallsyms_lookup_name(char *str);
int mdb_kallsyms(char *str,
		int (*print)(char *s, ...));
int mdb_modules(char *str,
		int (*print)(char *s, ...));
int mdb_getkey(void);
int mdb_getlword(uint64_t *word,
		unsigned long addr,
		size_t size);
int mdb_putword(unsigned long addr,
		unsigned long word,
		size_t size);
int mdb_copy(void *to, void *from,
		size_t size);
unsigned long mdb_phys_getword(unsigned long addr,
		size_t size);
unsigned long mdb_getword(unsigned long addr,
		size_t size);
uint64_t mdb_getqword(uint64_t *addr, size_t size);
uint64_t mdb_phys_getqword(uint64_t *addr, size_t size);
int mdb_putqword(uint64_t *addr, uint64_t word,
		size_t size);
unsigned long mdb_segment_getword(unsigned long seg,
		unsigned long addr,
		size_t size);
uint64_t mdb_segment_getqword(unsigned long segment,
		uint64_t *addr,
		size_t size);
int mdb_verify_rw(void *addr, size_t size);
unsigned long ValidateAddress(unsigned long addr,
		unsigned long length);
int DisplayClosestSymbol(unsigned long address);
void DumpOSSymbolTableMatch(unsigned char *symbol);
void DumpOSSymbolTable(void);
unsigned long GetValueFromSymbol(unsigned char *symbol);
unsigned char *GetModuleInfoFromSymbolValue(unsigned long value,
		unsigned char *buf,
		unsigned long len);
unsigned char *GetSymbolFromValue(unsigned long value,
		unsigned char *buf,
		unsigned long len);
unsigned char *GetSymbolFromValueWithOffset(unsigned long value,
		unsigned long *sym_offset,
		unsigned char *buf,
		unsigned long len);
unsigned char *GetSymbolFromValueOffsetModule(unsigned long value,
		unsigned long *sym_offset,
		unsigned char **module,
		unsigned char *buf,
		unsigned long len);
unsigned long get_processor_id(void);
unsigned long get_physical_processor(void);
unsigned long fpu_present(void);
unsigned long cpu_mttr_on(void);
unsigned char *UpcaseString(unsigned char *s);
void ClearScreen(void);
unsigned long ReadDS(void);
unsigned long ReadES(void);
unsigned long ReadFS(void);
unsigned long ReadGS(void);
unsigned long ReadDR(unsigned long regnum);
void WriteDR(int regnum, unsigned long contents);
unsigned long ReadCR(int regnum);
void WriteCR(int regnum, unsigned long contents);
unsigned long ReadTR(void);
unsigned long ReadLDTR(void);
void ReadGDTR(unsigned long *v);
void ReadIDTR(unsigned long *v);
void save_npx(NUMERIC_FRAME *v);
void load_npx(NUMERIC_FRAME *v);
unsigned long ReadDR0(void);
unsigned long ReadDR1(void);
unsigned long ReadDR2(void);
unsigned long ReadDR3(void);
unsigned long ReadDR6(void);
unsigned long ReadDR7(void);
void WriteDR0(unsigned long v);
void WriteDR1(unsigned long v);
void WriteDR2(unsigned long v);
void WriteDR3(unsigned long v);
void WriteDR6(unsigned long v);
void WriteDR7(unsigned long v);
unsigned long ReadCR0(void);
unsigned long ReadCR2(void);
unsigned long ReadCR3(void);
unsigned long ReadCR4(void);
void WriteCR0(unsigned long v);
void WriteCR2(unsigned long v);
void WriteCR3(unsigned long v);
void WriteCR4(unsigned long v);
void ReadMSR(unsigned long r, unsigned long *v1,
		unsigned long *v2);
void WriteMSR(unsigned long r, unsigned long *v1,
		unsigned long *v2);
#endif
