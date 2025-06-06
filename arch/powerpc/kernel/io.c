// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * I/O string operations
 *    Copyright (C) 1995-1996 Gary Thomas (gdt@linuxppc.org)
 *    Copyright (C) 2006 IBM Corporation
 *
 * Largely rewritten by Cort Dougan (cort@cs.nmt.edu)
 * and Paul Mackerras.
 *
 * Adapted for iSeries by Mike Corrigan (mikejc@us.ibm.com)
 * PPC64 updates by Dave Engebretsen (engebret@us.ibm.com)
 *
 * Rewritten in C by Stephen Rothwell.
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/export.h>

#include <asm/io.h>
#include <asm/firmware.h>
#include <asm/bug.h>

/* See definition in io.h */
bool isa_io_special;

void _insb(const volatile u8 __iomem *port, void *buf, long count)
{
	u8 *tbuf = buf;
	u8 tmp;

	if (unlikely(count <= 0))
		return;

	mb();
	do {
		tmp = *(const volatile u8 __force *)port;
		eieio();
		*tbuf++ = tmp;
	} while (--count != 0);
	data_barrier(tmp);
}
EXPORT_SYMBOL(_insb);

void _outsb(volatile u8 __iomem *port, const void *buf, long count)
{
	const u8 *tbuf = buf;

	if (unlikely(count <= 0))
		return;

	mb();
	do {
		*(volatile u8 __force *)port = *tbuf++;
	} while (--count != 0);
	mb();
}
EXPORT_SYMBOL(_outsb);

void _insw(const volatile u16 __iomem *port, void *buf, long count)
{
	u16 *tbuf = buf;
	u16 tmp;

	if (unlikely(count <= 0))
		return;

	mb();
	do {
		tmp = *(const volatile u16 __force *)port;
		eieio();
		*tbuf++ = tmp;
	} while (--count != 0);
	data_barrier(tmp);
}
EXPORT_SYMBOL(_insw);

void _outsw(volatile u16 __iomem *port, const void *buf, long count)
{
	const u16 *tbuf = buf;

	if (unlikely(count <= 0))
		return;

	mb();
	do {
		*(volatile u16 __force *)port = *tbuf++;
	} while (--count != 0);
	mb();
}
EXPORT_SYMBOL(_outsw);

void _insl(const volatile u32 __iomem *port, void *buf, long count)
{
	u32 *tbuf = buf;
	u32 tmp;

	if (unlikely(count <= 0))
		return;

	mb();
	do {
		tmp = *(const volatile u32 __force *)port;
		eieio();
		*tbuf++ = tmp;
	} while (--count != 0);
	data_barrier(tmp);
}
EXPORT_SYMBOL(_insl);

void _outsl(volatile u32 __iomem *port, const void *buf, long count)
{
	const u32 *tbuf = buf;

	if (unlikely(count <= 0))
		return;

	mb();
	do {
		*(volatile u32 __force *)port = *tbuf++;
	} while (--count != 0);
	mb();
}
EXPORT_SYMBOL(_outsl);

#define IO_CHECK_ALIGN(v,a) ((((unsigned long)(v)) & ((a) - 1)) == 0)

notrace void
_memset_io(volatile void __iomem *addr, int c, unsigned long n)
{
	void *p = (void __force *)addr;
	u32 lc = c;
	lc |= lc << 8;
	lc |= lc << 16;

	mb();
	while(n && !IO_CHECK_ALIGN(p, 4)) {
		*((volatile u8 *)p) = c;
		p++;
		n--;
	}
	while(n >= 4) {
		*((volatile u32 *)p) = lc;
		p += 4;
		n -= 4;
	}
	while(n) {
		*((volatile u8 *)p) = c;
		p++;
		n--;
	}
	mb();
}
EXPORT_SYMBOL(_memset_io);

void _memcpy_fromio(void *dest, const volatile void __iomem *src,
		    unsigned long n)
{
	void *vsrc = (void __force *) src;

	mb();
	while(n && (!IO_CHECK_ALIGN(vsrc, 4) || !IO_CHECK_ALIGN(dest, 4))) {
		*((u8 *)dest) = *((volatile u8 *)vsrc);
		eieio();
		vsrc++;
		dest++;
		n--;
	}
	while(n >= 4) {
		*((u32 *)dest) = *((volatile u32 *)vsrc);
		eieio();
		vsrc += 4;
		dest += 4;
		n -= 4;
	}
	while(n) {
		*((u8 *)dest) = *((volatile u8 *)vsrc);
		eieio();
		vsrc++;
		dest++;
		n--;
	}
	mb();
}
EXPORT_SYMBOL(_memcpy_fromio);

void _memcpy_toio(volatile void __iomem *dest, const void *src, unsigned long n)
{
	void *vdest = (void __force *) dest;

	mb();
	while(n && (!IO_CHECK_ALIGN(vdest, 4) || !IO_CHECK_ALIGN(src, 4))) {
		*((volatile u8 *)vdest) = *((u8 *)src);
		src++;
		vdest++;
		n--;
	}
	while(n >= 4) {
		*((volatile u32 *)vdest) = *((volatile u32 *)src);
		src += 4;
		vdest += 4;
		n-=4;
	}
	while(n) {
		*((volatile u8 *)vdest) = *((u8 *)src);
		src++;
		vdest++;
		n--;
	}
	mb();
}
EXPORT_SYMBOL(_memcpy_toio);
