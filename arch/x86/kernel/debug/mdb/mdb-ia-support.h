
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

/* Print i386 instructions for GDB, the GNU debugger.
   Copyright 1988, 1989, 1991, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
   2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
   GNU General Public License for more details.

 * Extracted from binutils 2.16.91.0.2 (OpenSUSE 10.0) and modified for kdb use.
 * Run through col -b to remove trailing whitespace and various #ifdef/ifndef
 * __KERNEL__ added.
 * Keith Owens <kaos@sgi.com> 15 May 2006
 */

/* 80386 instruction printer by Pace Willisson (pace@prep.ai.mit.edu)
   July 1988
    modified by John Hassey (hassey@dg-rtp.dg.com)
    x86-64 support added by Jan Hubicka (jh@suse.cz)
    VIA PadLock support by Michal Ludvig (mludvig@suse.cz).  */

/* The main tables describing the instructions is essentially a copy
   of the "Opcode Map" chapter (Appendix A) of the Intel 80386
   Programmers Manual.	Usually, there is a capital letter, followed
   by a small letter.  The capital letter tell the addressing mode,
   and the small letter tells about the operand size.  Refer to
   the Intel manual for details.  */

/* 06-26-2010 Jeff V. Merkey - port GDB debugger to MDB and add memory
   dereferencing, jump labeling, and opcode output.  Convert MDB to 
   conform to Linux hex and decimal notation and display.  */

#ifndef DIS_ASM_H
#define DIS_ASM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ansidecl.h"
#include "bfd.h"
typedef void FILE;

typedef int (*fprintf_ftype) (void *, const char*, ...) ATTRIBUTE_FPTR_PRINTF_2;

enum dis_insn_type {
	dis_noninsn,
	dis_nonbranch,
	dis_branch,
	dis_condbranch,
	dis_jsr,
	dis_condjsr,
	dis_dref,
	dis_dref2
};

typedef struct disassemble_info {
	fprintf_ftype fprintf_func;
	void *stream;
	void *application_data;
	enum bfd_flavour flavour;
	enum bfd_architecture arch;
	unsigned long mach;
	enum bfd_endian endian;
	unsigned long insn_sets;
	asection *section;
	asymbol **symbols;
	int num_symbols;

	unsigned long flags;
#define INSN_HAS_RELOC	0x80000000
	void *private_data;

	int (*read_memory_func)
	(bfd_vma memaddr, bfd_byte *myaddr, unsigned int length,
	struct disassemble_info *info);

	void (*memory_error_func)
	(int status, bfd_vma memaddr, struct disassemble_info *info);

	void (*print_address_func)
	(bfd_vma addr, struct disassemble_info *info);

	int (* symbol_at_address_func)
	(bfd_vma addr, struct disassemble_info * info);

	bfd_boolean (* symbol_is_valid)
	(asymbol *, struct disassemble_info * info);

	bfd_byte *buffer;
	bfd_vma buffer_vma;
	unsigned int buffer_length;

	int bytes_per_line;
	int bytes_per_chunk;
	enum bfd_endian display_endian;
	unsigned int octets_per_byte;
	unsigned int skip_zeroes;
	unsigned int skip_zeroes_at_end;

	char insn_info_valid;
	char branch_delay_insns;
	char data_size;
	enum dis_insn_type insn_type;
	bfd_vma target;
	bfd_vma target2;

	char * disassembler_options;

} disassemble_info;


typedef int (*disassembler_ftype) (bfd_vma, disassemble_info *);

extern int print_insn_big_mips		(bfd_vma, disassemble_info *);
extern int print_insn_little_mips	(bfd_vma, disassemble_info *);
extern int print_insn_ia64		(bfd_vma, disassemble_info *);
extern int print_insn_i370		(bfd_vma, disassemble_info *);
extern int print_insn_m68hc11		(bfd_vma, disassemble_info *);
extern int print_insn_m68hc12		(bfd_vma, disassemble_info *);
extern int print_insn_m68k		(bfd_vma, disassemble_info *);
extern int print_insn_z8001		(bfd_vma, disassemble_info *);
extern int print_insn_z8002		(bfd_vma, disassemble_info *);
extern int print_insn_h8300		(bfd_vma, disassemble_info *);
extern int print_insn_h8300h		(bfd_vma, disassemble_info *);
extern int print_insn_h8300s		(bfd_vma, disassemble_info *);
extern int print_insn_h8500		(bfd_vma, disassemble_info *);
extern int print_insn_alpha		(bfd_vma, disassemble_info *);
extern int print_insn_big_arm		(bfd_vma, disassemble_info *);
extern int print_insn_little_arm	(bfd_vma, disassemble_info *);
extern int print_insn_sparc		(bfd_vma, disassemble_info *);
extern int print_insn_big_a29k		(bfd_vma, disassemble_info *);
extern int print_insn_little_a29k	(bfd_vma, disassemble_info *);
extern int print_insn_avr		(bfd_vma, disassemble_info *);
extern int print_insn_d10v		(bfd_vma, disassemble_info *);
extern int print_insn_d30v		(bfd_vma, disassemble_info *);
extern int print_insn_dlx 		(bfd_vma, disassemble_info *);
extern int print_insn_fr30		(bfd_vma, disassemble_info *);
extern int print_insn_hppa		(bfd_vma, disassemble_info *);
extern int print_insn_i860		(bfd_vma, disassemble_info *);
extern int print_insn_i960		(bfd_vma, disassemble_info *);
extern int print_insn_ip2k		(bfd_vma, disassemble_info *);
extern int print_insn_m32r		(bfd_vma, disassemble_info *);
extern int print_insn_m88k		(bfd_vma, disassemble_info *);
extern int print_insn_maxq_little	(bfd_vma, disassemble_info *);
extern int print_insn_maxq_big		(bfd_vma, disassemble_info *);
extern int print_insn_mcore		(bfd_vma, disassemble_info *);
extern int print_insn_mmix		(bfd_vma, disassemble_info *);
extern int print_insn_mn10200		(bfd_vma, disassemble_info *);
extern int print_insn_mn10300		(bfd_vma, disassemble_info *);
extern int print_insn_ms1               (bfd_vma, disassemble_info *);
extern int print_insn_msp430		(bfd_vma, disassemble_info *);
extern int print_insn_ns32k		(bfd_vma, disassemble_info *);
extern int print_insn_crx               (bfd_vma, disassemble_info *);
extern int print_insn_openrisc		(bfd_vma, disassemble_info *);
extern int print_insn_big_or32		(bfd_vma, disassemble_info *);
extern int print_insn_little_or32	(bfd_vma, disassemble_info *);
extern int print_insn_pdp11		(bfd_vma, disassemble_info *);
extern int print_insn_pj		(bfd_vma, disassemble_info *);
extern int print_insn_big_powerpc	(bfd_vma, disassemble_info *);
extern int print_insn_little_powerpc	(bfd_vma, disassemble_info *);
extern int print_insn_rs6000		(bfd_vma, disassemble_info *);
extern int print_insn_s390		(bfd_vma, disassemble_info *);
extern int print_insn_sh		(bfd_vma, disassemble_info *);
extern int print_insn_tic30		(bfd_vma, disassemble_info *);
extern int print_insn_tic4x		(bfd_vma, disassemble_info *);
extern int print_insn_tic54x		(bfd_vma, disassemble_info *);
extern int print_insn_tic80		(bfd_vma, disassemble_info *);
extern int print_insn_v850		(bfd_vma, disassemble_info *);
extern int print_insn_vax		(bfd_vma, disassemble_info *);
extern int print_insn_w65		(bfd_vma, disassemble_info *);
extern int print_insn_xstormy16		(bfd_vma, disassemble_info *);
extern int print_insn_xtensa		(bfd_vma, disassemble_info *);
extern int print_insn_sh64		(bfd_vma, disassemble_info *);
extern int print_insn_sh64x_media	(bfd_vma, disassemble_info *);
extern int print_insn_frv		(bfd_vma, disassemble_info *);
extern int print_insn_iq2000		(bfd_vma, disassemble_info *);
extern int print_insn_m32c	(bfd_vma, disassemble_info *);

extern disassembler_ftype arc_get_disassembler (void *);
extern disassembler_ftype cris_get_disassembler (bfd *);

extern void print_mips_disassembler_options (FILE *);
extern void print_ppc_disassembler_options (FILE *);
extern void print_arm_disassembler_options (FILE *);
extern void parse_arm_disassembler_option (char *);
extern int get_arm_regname_num_options (void);
extern int set_arm_regname_option (int);
extern int get_arm_regnames (int, const char **, const char **, const char *const **);
extern bfd_boolean arm_symbol_is_valid (asymbol *, struct disassemble_info *);
extern disassembler_ftype disassembler (bfd *);
extern void disassemble_init_for_target (struct disassemble_info * info);
extern void disassembler_usage (FILE *);
extern int buffer_read_memory
	(bfd_vma, bfd_byte *, unsigned int, struct disassemble_info *);
extern void perror_memory (int, bfd_vma, struct disassemble_info *);
extern void generic_print_address
	(bfd_vma, struct disassemble_info *);
extern int generic_symbol_at_address
	(bfd_vma, struct disassemble_info *);
extern bfd_boolean generic_symbol_is_valid
	(asymbol *, struct disassemble_info *);
extern void init_disassemble_info (struct disassemble_info *info, void *stream,
					fprintf_ftype fprintf_func);

#define INIT_DISASSEMBLE_INFO(INFO, STREAM, FPRINTF_FUNC) \
	init_disassemble_info (&(INFO), (STREAM), (fprintf_ftype) (FPRINTF_FUNC))
#define INIT_DISASSEMBLE_INFO_NO_ARCH(INFO, STREAM, FPRINTF_FUNC) \
	init_disassemble_info (&(INFO), (STREAM), (fprintf_ftype) (FPRINTF_FUNC))


#ifdef __cplusplus
}
#endif

#endif

