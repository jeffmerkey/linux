
/***************************************************************************
 *
 *   Copyright (c) 2000-2016 Jeff V. Merkey  All Rights Reserved.
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
#include <linux/nmi.h>
#include <asm/processor.h>
#include <asm/segment.h>
#include <linux/atomic.h>
#include <asm/msr.h>
#include <asm-generic/kmap_types.h>
#include <asm/stacktrace.h>
#include <linux/io.h>
#include <linux/delay.h>

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

/* module symbol workspace */
unsigned char symbuf[MAX_SYMBOL_LEN];
unsigned char modbuf[MAX_SYMBOL_LEN];
unsigned char workbuf[MAX_SYMBOL_LEN];

#if IS_ENABLED(CONFIG_X86_64)

#include <asm/stacktrace.h>

static char *exception_stack_names[N_EXCEPTION_STACKS] = {
		[ DOUBLEFAULT_STACK-1	]	= "#DF",
		[ NMI_STACK-1		]	= "NMI",
		[ DEBUG_STACK-1		]	= "#DB",
		[ MCE_STACK-1		]	= "#MC",
};

static unsigned long exception_stack_sizes[N_EXCEPTION_STACKS] = {
	[0 ... N_EXCEPTION_STACKS - 1]		= EXCEPTION_STKSZ,
	[DEBUG_STACK - 1]			= DEBUG_STKSZ
};

void stack_type_str(enum stack_type type, const char **begin, const char **end)
{
	BUILD_BUG_ON(N_EXCEPTION_STACKS != 4);

	switch (type) {
	case STACK_TYPE_IRQ:
		*begin = "IRQ";
		*end   = "EOI";
		break;
	case STACK_TYPE_EXCEPTION ... STACK_TYPE_EXCEPTION_LAST:
		*begin = exception_stack_names[type - STACK_TYPE_EXCEPTION];
		*end   = "EOE";
		break;
	default:
		*begin = NULL;
		*end   = NULL;
	}
}

static bool in_exception_stack(unsigned long *stack, struct stack_info *info)
{
	unsigned long *begin, *end;
	struct pt_regs *regs;
	unsigned k;

	BUILD_BUG_ON(N_EXCEPTION_STACKS != 4);

	for (k = 0; k < N_EXCEPTION_STACKS; k++) {
		end   = (unsigned long *)raw_cpu_ptr(&orig_ist)->ist[k];
		begin = end - (exception_stack_sizes[k] / sizeof(long));
		regs  = (struct pt_regs *)end - 1;

		if (stack < begin || stack >= end)
			continue;

		info->type	= STACK_TYPE_EXCEPTION + k;
		info->begin	= begin;
		info->end	= end;
		info->next_sp	= (unsigned long *)regs->sp;

		return true;
	}

	return false;
}

static bool in_irq_stack(unsigned long *stack, struct stack_info *info)
{
	unsigned long *end   = (unsigned long *)this_cpu_read(irq_stack_ptr);
	unsigned long *begin = end - (IRQ_STACK_SIZE / sizeof(long));

	/*
	 * This is a software stack, so 'end' can be a valid stack pointer.
	 * It just means the stack is empty.
	 */
	if (stack < begin || stack > end)
		return false;

	info->type	= STACK_TYPE_IRQ;
	info->begin	= begin;
	info->end	= end;

	/*
	 * The next stack pointer is the first thing pushed by the entry code
	 * after switching to the irq stack.
	 */
	info->next_sp = (unsigned long *)*(end - 1);

	return true;
}

int get_stack_info(unsigned long *stack, struct task_struct *task,
		   struct stack_info *info, unsigned long *visit_mask)
{
	if (!stack)
		goto unknown;

	task = task ? : current;

	if (in_task_stack(stack, task, info))
		goto recursion_check;

	if (task != current)
		goto unknown;

	if (in_exception_stack(stack, info))
		goto recursion_check;

	if (in_irq_stack(stack, info))
		goto recursion_check;

	goto unknown;

recursion_check:
	/*
	 * Make sure we don't iterate through any given stack more than once.
	 * If it comes up a second time then there's something wrong going on:
	 * just break out and report an unknown stack type.
	 */
	if (visit_mask) {
		if (*visit_mask & (1UL << info->type))
			goto unknown;
		*visit_mask |= 1UL << info->type;
	}

	return 0;

unknown:
	info->type = STACK_TYPE_UNKNOWN;
	return -EINVAL;
}

void show_stack_log_lvl(struct task_struct *task, struct pt_regs *regs,
			unsigned long *sp, char *log_lvl)
{
	unsigned long *irq_stack_end;
	unsigned long *irq_stack;
	unsigned long *stack;
	int i;

	if (!try_get_task_stack(task))
		return;

	irq_stack_end = (unsigned long *)this_cpu_read(irq_stack_ptr);
	irq_stack     = irq_stack_end - (IRQ_STACK_SIZE / sizeof(long));

	sp = sp ? : get_stack_pointer(task, regs);

	stack = sp;
	for (i = 0; i < kstack_depth_to_print; i++) {
		unsigned long word;

		if (stack >= irq_stack && stack <= irq_stack_end) {
			if (stack == irq_stack_end) {
				stack = (unsigned long *) (irq_stack_end[-1]);
				pr_cont(" <EOI> ");
			}
		} else {
		if (kstack_end(stack))
			break;
		}

		if (probe_kernel_address(stack, word))
			break;

		if ((i % STACKSLOTS_PER_LINE) == 0) {
			if (i != 0)
				dbg_pr("\n");
			dbg_pr("%s %016lx", log_lvl, word);
		} else
			dbg_pr(" %016lx", word);

		stack++;
		mdb_watchdogs();
	}

	dbg_pr("\n");
	show_trace_log_lvl(task, regs, sp, log_lvl);

	put_task_stack(task);
}

int is_valid_bugaddr(unsigned long ip)
{
	unsigned short ud2;

	if (__copy_from_user(&ud2, (const void __user *) ip, sizeof(ud2)))
		return 0;

	return ud2 == 0x0b0f;
}

static inline unsigned long fixup_bp_irq_link(unsigned long bp,
					      unsigned long *stack,
					      unsigned long *irq_stack,
					      unsigned long *irq_stack_end)
{
#if IS_ENABLED(CONFIG_FRAME_POINTER)
	struct stack_frame *frame = (struct stack_frame *)bp;
	unsigned long next;

	if (!in_irq_stack(stack, irq_stack, irq_stack_end)) {
		if (!probe_kernel_address(&frame->next_frame, next))
			return next;
		dbg_pr("MDB: bad frame pointer = %p in chain\n",
		       &frame->next_frame);
	}
#endif
	return bp;
}

void dbg_pr_address(unsigned long address, int reliable)
{
	dbg_pr("[<%p>] %s%pS\n", (void *)address,
	       reliable ? "" : "? ", (void *)address);
}

static inline int valid_stack_ptr(struct thread_info *tinfo,
				  void *p, unsigned int size, void *end)
{
	void *t = tinfo;

	if (end) {
		if (p < end && p >= (end - THREAD_SIZE))
			return 1;
		else
			return 0;
	}
	return p > t && p < t + THREAD_SIZE - size;
}

static inline unsigned long
print_context(struct thread_info *tinfo, unsigned long *stack,
	      unsigned long bp, unsigned long *end)
{
	struct stack_frame *frame = (struct stack_frame *)bp;

	while (valid_stack_ptr(tinfo, stack, sizeof(*stack), end)) {
		unsigned long addr;

		addr = *stack;
		if (__kernel_text_address(addr)) {
			if ((unsigned long)stack == bp + sizeof(long)) {
				mdb_watchdogs();
				dbg_pr_address(addr, 1);
				frame = frame->next_frame;
				bp = (unsigned long)frame;
			} else {
				mdb_watchdogs();
				dbg_pr_address(addr, bp == 0);
			}
		}
		stack++;
	}
	return bp;
}

void bt_stack(struct task_struct *task, struct pt_regs *regs,
	      unsigned long *stack)
{
	unsigned long bp = 0;
	const unsigned int cpu = get_cpu();
//	unsigned long *irq_stack_end = get_irq_stack_end(cpu);
	unsigned long *irq_stack_end = NULL;
	unsigned int used = 0;
	struct thread_info *tinfo;

	if (!task)
		task = current;

	if (!stack) {
		unsigned long dummy;

		stack = &dummy;
		if (task && task != current)
			stack = (unsigned long *)task->thread.sp;
	}

#if IS_ENABLED(CONFIG_FRAME_POINTER)
	if (!bp) {
//		if (task == current)
//			get_bp(bp);
//		else
		bp = *(unsigned long *)task->thread.sp;
	}
#endif

	tinfo = task_thread_info(task);
	for (;;) {
		char *id;
		unsigned long *estack_end = NULL;

//		estack_end = in_exception_stack(cpu, (unsigned long)stack,
//						&used, &id);

		if (estack_end) {
			if (dbg_pr("%s", id))
				break;

			bp = print_context(tinfo, stack, bp, estack_end);
			dbg_pr("%s", "<EOE>");

			stack = (unsigned long *)estack_end[-2];
			continue;
		}

		if (irq_stack_end) {
			unsigned long *irq_stack;

			irq_stack = irq_stack_end -
				(IRQ_STACK_SIZE - 64) / sizeof(*irq_stack);

			if (in_irq_stack(stack, irq_stack, irq_stack_end)) {
				if (dbg_pr("%s", "IRQ"))
					break;

				bp = print_context(tinfo, stack, bp,
						   irq_stack_end);

				stack = (unsigned long *)(irq_stack_end[-1]);
				bp = fixup_bp_irq_link(bp, stack, irq_stack,
						       irq_stack_end);
				irq_stack_end = NULL;
				dbg_pr("%s", "EOI");
				continue;
			}
		}
		break;
	}

	bp = print_context(tinfo, stack, bp, NULL);
	put_cpu();
}

#else

#include <linux/kdebug.h>

#include <asm/stacktrace.h>

void stack_type_str(enum stack_type type, const char **begin, const char **end)
{
	switch (type) {
	case STACK_TYPE_IRQ:
	case STACK_TYPE_SOFTIRQ:
		*begin = "IRQ";
		*end   = "EOI";
		break;
	default:
		*begin = NULL;
		*end   = NULL;
	}
}

static bool in_hardirq_stack(unsigned long *stack, struct stack_info *info)
{
	unsigned long *begin = (unsigned long *)this_cpu_read(hardirq_stack);
	unsigned long *end   = begin + (THREAD_SIZE / sizeof(long));

	/*
	 * This is a software stack, so 'end' can be a valid stack pointer.
	 * It just means the stack is empty.
	 */
	if (stack < begin || stack > end)
		return false;

	info->type	= STACK_TYPE_IRQ;
	info->begin	= begin;
	info->end	= end;

	/*
	 * See irq_32.c -- the next stack pointer is stored at the beginning of
	 * the stack.
	 */
	info->next_sp	= (unsigned long *)*begin;

	return true;
}

static bool in_softirq_stack(unsigned long *stack, struct stack_info *info)
{
	unsigned long *begin = (unsigned long *)this_cpu_read(softirq_stack);
	unsigned long *end   = begin + (THREAD_SIZE / sizeof(long));

	/*
	 * This is a software stack, so 'end' can be a valid stack pointer.
	 * It just means the stack is empty.
	 */
	if (stack < begin || stack > end)
		return false;

	info->type	= STACK_TYPE_SOFTIRQ;
	info->begin	= begin;
	info->end	= end;

	/*
	 * The next stack pointer is stored at the beginning of the stack.
	 * See irq_32.c.
	 */
	info->next_sp	= (unsigned long *)*begin;

	return true;
}

int get_stack_info(unsigned long *stack, struct task_struct *task,
		   struct stack_info *info, unsigned long *visit_mask)
{
	if (!stack)
		goto unknown;

	task = task ? : current;

	if (in_task_stack(stack, task, info))
		goto recursion_check;

	if (task != current)
		goto unknown;

	if (in_hardirq_stack(stack, info))
		goto recursion_check;

	if (in_softirq_stack(stack, info))
		goto recursion_check;

	goto unknown;

recursion_check:
	/*
	 * Make sure we don't iterate through any given stack more than once.
	 * If it comes up a second time then there's something wrong going on:
	 * just break out and report an unknown stack type.
	 */
	if (visit_mask) {
		if (*visit_mask & (1UL << info->type))
			goto unknown;
		*visit_mask |= 1UL << info->type;
	}

	return 0;

unknown:
	info->type = STACK_TYPE_UNKNOWN;
	return -EINVAL;
}

void show_stack_log_lvl(struct task_struct *task, struct pt_regs *regs,
			unsigned long *sp, char *log_lvl)
{
	unsigned long *stack;
	int i;

	if (!try_get_task_stack(task))
		return;

	sp = sp ? : get_stack_pointer(task, regs);

	stack = sp;
	for (i = 0; i < kstack_depth_to_print; i++) {
		if (kstack_end(stack))
			break;
		if ((i % STACKSLOTS_PER_LINE) == 0) {
			if (i != 0)
				pr_cont("\n");
			dbg_pr("%s %08lx", log_lvl, *stack++);
		} else
			dbg_pr(" %08lx", *stack++);
		mdb_watchdogs();
	}
	dbg_pr("\n");
	show_trace_log_lvl(task, regs, sp, log_lvl);

	put_task_stack(task);
}


int is_valid_bugaddr(unsigned long ip)
{
	unsigned short ud2;

	if (ip < PAGE_OFFSET)
		return 0;
	if (probe_kernel_address((unsigned short *)ip, ud2))
		return 0;

	return ud2 == 0x0b0f;
}

static inline int valid_stack_ptr(struct thread_info *tinfo,
				  void *p, unsigned int size, void *end)
{
	void *t = tinfo;

	if (end) {
		if (p < end && p >= (end - THREAD_SIZE))
			return 1;
		else
			return 0;
	}
	return p > t && p < t + THREAD_SIZE - size;
}

static void print_stack_address(unsigned long address, int reliable)
{
	dbg_pr("[<%p>] %s%pB\n",
	       (void *)address, reliable ? "" : "? ",
	       (void *)address);
}

unsigned long
print_context(struct thread_info *tinfo,
	      unsigned long *stack, unsigned long bp,
	      unsigned long *end)
{
	struct stack_frame *frame = (struct stack_frame *)bp;

	while (valid_stack_ptr(tinfo, stack, sizeof(*stack), end)) {
		unsigned long addr;

		addr = *stack;
		if (__kernel_text_address(addr)) {
			if ((unsigned long)stack == bp + sizeof(long)) {
				mdb_watchdogs();
				print_stack_address(addr, 1);
				frame = frame->next_frame;
				bp = (unsigned long)frame;
			} else {
				mdb_watchdogs();
				print_stack_address(addr, 0);
			}
		}
		stack++;
	}
	return bp;
}

static int print_trace(char *name)
{
	dbg_pr("<%s> ", name);
	return 0;
}

void bt_stack(struct task_struct *task, struct pt_regs *regs,
	      unsigned long *stack)
{
	const unsigned int cpu = get_cpu();
	unsigned long bp = 0;
	u32 *prev_esp;

	if (cpu == cpu) {};

	if (!task)
		task = current;

	if (!stack) {
		unsigned long dummy;

		stack = &dummy;
		if (task != current)
			stack = (unsigned long *)task->thread.sp;
	}

//	if (!bp)
//		bp = stack_frame(task, regs);

	for (;;) {
		struct thread_info *context;
		void *end_stack = NULL;

//		end_stack = is_hardirq_stack(stack, cpu);
//		if (!end_stack)
//			end_stack = is_softirq_stack(stack, cpu);
//
		context = task_thread_info(task);

		bp = print_context(context, stack, bp, end_stack);

		/* Stop if not on irq stack */
		if (!end_stack)
			break;

		/* The previous esp is saved on the bottom of the stack */
		prev_esp = (u32 *)(end_stack - THREAD_SIZE);
		stack = (unsigned long *)*prev_esp;
		if (!stack)
			break;

		if (print_trace("IRQ") < 0)
			break;

		mdb_watchdogs();
	}
	put_cpu();
}

#endif

#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <linux/uaccess.h>
#include <linux/utsname.h>
#include <linux/hardirq.h>
#include <linux/kdebug.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/ftrace.h>
#include <linux/kexec.h>
#include <linux/bug.h>
#include <linux/nmi.h>
#include <linux/sysfs.h>

#include <asm/processor.h>
#include <asm/stacktrace.h>
#include <asm/unwind.h>

unsigned int code_bytes = 64;
int kstack_depth_to_print = 3 * STACKSLOTS_PER_LINE;

bool in_task_stack(unsigned long *stack, struct task_struct *task,
		   struct stack_info *info)
{
	unsigned long *begin = task_stack_page(task);
	unsigned long *end   = task_stack_page(task) + THREAD_SIZE;

	if (stack < begin || stack >= end)
		return false;

	info->type	= STACK_TYPE_TASK;
	info->begin	= begin;
	info->end	= end;
	info->next_sp	= NULL;

	return true;
}

static void printk_stack_address(unsigned long address, int reliable,
				 char *log_lvl)
{
	mdb_watchdogs();
	dbg_pr("%s [<%p>] %s%pB\n",
		log_lvl, (void *)address, reliable ? "" : "? ",
		(void *)address);
}

void printk_address(unsigned long address)
{
	dbg_pr(" [<%p>] %pS\n", (void *)address, (void *)address);
}

void show_trace_log_lvl(struct task_struct *task, struct pt_regs *regs,
			unsigned long *stack, char *log_lvl)
{
	struct unwind_state state;
	struct stack_info stack_info = {0};
	unsigned long visit_mask = 0;
	int graph_idx = 0;

	dbg_pr("%sCall Trace:\n", log_lvl);

	unwind_start(&state, task, regs, stack);

	/*
	 * Iterate through the stacks, starting with the current stack pointer.
	 * Each stack has a pointer to the next one.
	 *
	 * x86-64 can have several stacks:
	 * - task stack
	 * - interrupt stack
	 * - HW exception stacks (double fault, nmi, debug, mce)
	 *
	 * x86-32 can have up to three stacks:
	 * - task stack
	 * - softirq stack
	 * - hardirq stack
	 */
	for (; stack; stack = stack_info.next_sp) {
		const char *str_begin, *str_end;

		/*
		 * If we overflowed the task stack into a guard page, jump back
		 * to the bottom of the usable stack.
		 */
		if (task_stack_page(task) - (void *)stack < PAGE_SIZE)
			stack = task_stack_page(task);

		if (get_stack_info(stack, task, &stack_info, &visit_mask))
			break;

		stack_type_str(stack_info.type, &str_begin, &str_end);
		if (str_begin)
			dbg_pr("%s <%s> ", log_lvl, str_begin);

		/*
		 * Scan the stack, printing any text addresses we find.  At the
		 * same time, follow proper stack frames with the unwinder.
		 *
		 * Addresses found during the scan which are not reported by
		 * the unwinder are considered to be additional clues which are
		 * sometimes useful for debugging and are prefixed with '?'.
		 * This also serves as a failsafe option in case the unwinder
		 * goes off in the weeds.
		 */
		for (; stack < stack_info.end; stack++) {
			unsigned long real_addr;
			int reliable = 0;
			unsigned long addr = READ_ONCE_NOCHECK(*stack);
			unsigned long *ret_addr_p =
				unwind_get_return_address_ptr(&state);

			if (!__kernel_text_address(addr))
				continue;

			if (stack == ret_addr_p)
				reliable = 1;

			/*
			 * When function graph tracing is enabled for a
			 * function, its return address on the stack is
			 * replaced with the address of an ftrace handler
			 * (return_to_handler).  In that case, before printing
			 * the "real" address, we want to print the handler
			 * address as an "unreliable" hint that function graph
			 * tracing was involved.
			 */
			real_addr = ftrace_graph_ret_addr(task, &graph_idx,
							  addr, stack);
			if (real_addr != addr)
				printk_stack_address(addr, 0, log_lvl);
			printk_stack_address(real_addr, reliable, log_lvl);

			if (!reliable)
				continue;

			/*
			 * Get the next frame from the unwinder.  No need to
			 * check for an error: if anything goes wrong, the rest
			 * of the addresses will just be printed as unreliable.
			 */
			unwind_next_frame(&state);
		}

		if (str_end)
			dbg_pr("%s <%s> ", log_lvl, str_end);
	}
}

void show_stack(struct task_struct *task, unsigned long *sp)
{
	task = task ? : current;

	/*
	 * Stack frames below this one aren't interesting.  Don't show them
	 * if we're printing for %current.
	 */
	if (!sp && task == current)
		sp = get_stack_pointer(current, NULL);

	show_stack_log_lvl(task, NULL, sp, "");
}

void show_stack_regs(struct pt_regs *regs)
{
	show_stack_log_lvl(current, regs, NULL, "");
}


/* keyboard and serial interface functions */

unsigned char *mdbprompt =   "more (q to quit)>";
int linecount = 23;
int nextline;
int pause_mode = 1;
int mdb_suppress_crlf;

rlock_t mdb_mutex = { -1, 0 };
DEFINE_SPINLOCK(mdb_lock);

char mdb_buffer[256];
char mdb_keystroke[16];
unsigned long consolestate;

int mdb_printf(char *fmt, ...)
{
	va_list	ap;
	struct console *con;

	debug_rlock(&mdb_lock, &mdb_mutex, get_processor_id());

#if IS_ENABLED(CONFIG_VT_CONSOLE) && IS_ENABLED(CONFIG_MDB_CONSOLE_REDIRECTION)
	consolestate = vt_kmsg_redirect(-1);
	if (consolestate)
		consolestate = vt_kmsg_redirect(0);
#endif
	va_start(ap, fmt);
	vsprintf(mdb_buffer, fmt, ap);
	va_end(ap);

	for (con = console_drivers; con; con = con->next) {
		if ((con->flags & CON_ENABLED) && con->write &&
		    (cpu_online(get_processor_id()) ||
		     (con->flags & CON_ANYTIME))) {
			con->write(con, mdb_buffer, strlen(mdb_buffer));
			mdb_watchdogs();
		}
	}

	if (strchr(mdb_buffer, '\n'))
		nextline++;

	if (pause_mode && (nextline >= linecount)) {
		if (nextline > linecount)
			nextline = linecount;

		for (con = console_drivers; con; con = con->next) {
			if ((con->flags & CON_ENABLED) && con->write &&
			    (cpu_online(get_processor_id()) ||
			     (con->flags & CON_ANYTIME))) {
				con->write(con, mdbprompt, strlen(mdbprompt));
				mdb_watchdogs();
			}
		}

		mdb_suppress_crlf = 1;
		mdb_keystroke[0] = (char)mdb_getkey();
		mdb_suppress_crlf = 0;

		if (mdb_keystroke[0] == ' ')
			nextline = 0;

		for (con = console_drivers; con; con = con->next) {
			if ((con->flags & CON_ENABLED) && con->write &&
			    (cpu_online(get_processor_id()) ||
			     (con->flags & CON_ANYTIME))) {
				memset(mdb_buffer, 0, 256);

				memset(mdb_buffer, '\b', strlen(mdbprompt));
				con->write(con, mdb_buffer, strlen(mdb_buffer));
				mdb_watchdogs();

				memset(mdb_buffer, ' ', strlen(mdbprompt));
				con->write(con, mdb_buffer, strlen(mdb_buffer));
				mdb_watchdogs();

				memset(mdb_buffer, '\b', strlen(mdbprompt));
				con->write(con, mdb_buffer, strlen(mdb_buffer));
				mdb_watchdogs();
			}
		}

		if ((mdb_keystroke[0] == 'q') || (mdb_keystroke[0] == 'Q')) {
			nextline = 0;

#if IS_ENABLED(CONFIG_VT_CONSOLE) && IS_ENABLED(CONFIG_MDB_CONSOLE_REDIRECTION)
			if (consolestate)
				vt_kmsg_redirect(consolestate);
#endif

			debug_unrlock(&mdb_lock, &mdb_mutex,
				      get_processor_id());
			return 1;
		}
	}

#if IS_ENABLED(CONFIG_VT_CONSOLE) && IS_ENABLED(CONFIG_MDB_CONSOLE_REDIRECTION)
	if (consolestate)
		vt_kmsg_redirect(consolestate);
#endif

	debug_unrlock(&mdb_lock, &mdb_mutex, get_processor_id());
	return 0;
}

unsigned int mdb_serial_port;
module_param(mdb_serial_port, uint, 0644);
MODULE_PARM_DESC(mdb_serial_port,
		 "MDB serial port address.  i.e. 0x3f8(ttyS0), 0x2f8(ttyS1), 0x3e8(ttyS2), 0x2e8(ttyS3)");

int get_modem_char(void)
{
	unsigned char ch;
	int status;

	if (mdb_serial_port == 0)
		return -1;

	status = inb(mdb_serial_port + UART_LSR);
	if (status & UART_LSR_DR) {
		ch = inb(mdb_serial_port + UART_RX);
		switch (ch) {
		case 0x7f:
			ch = 8;
			break;

		case '\t':
			ch = ' ';
			break;

		case 8:  /* backspace */
			break;

		case 13: /* enter */
			if (!mdb_suppress_crlf)
				dbg_pr("\n");
			break;

		default:
			if (!isprint(ch))
				return -1;
			if (!mdb_suppress_crlf)
				dbg_pr("%c", ch);
			break;
		}
		return ch;
	}
	return -1;
}

u_short m_plain_map[NR_KEYS] = {
	0xf200,	0xf01b,	0xf031,	0xf032,	0xf033,	0xf034,	0xf035,	0xf036,
	0xf037,	0xf038,	0xf039,	0xf030,	0xf02d,	0xf03d,	0xf07f,	0xf009,
	0xfb71,	0xfb77,	0xfb65,	0xfb72,	0xfb74,	0xfb79,	0xfb75,	0xfb69,
	0xfb6f,	0xfb70,	0xf05b,	0xf05d,	0xf201,	0xf702,	0xfb61,	0xfb73,
	0xfb64,	0xfb66,	0xfb67,	0xfb68,	0xfb6a,	0xfb6b,	0xfb6c,	0xf03b,
	0xf027,	0xf060,	0xf700,	0xf05c,	0xfb7a,	0xfb78,	0xfb63,	0xfb76,
	0xfb62,	0xfb6e,	0xfb6d,	0xf02c,	0xf02e,	0xf02f,	0xf700,	0xf30c,
	0xf703,	0xf020,	0xf207,	0xf100,	0xf101,	0xf102,	0xf103,	0xf104,
	0xf105,	0xf106,	0xf107,	0xf108,	0xf109,	0xf208,	0xf209,	0xf307,
	0xf308,	0xf309,	0xf30b,	0xf304,	0xf305,	0xf306,	0xf30a,	0xf301,
	0xf302,	0xf303,	0xf300,	0xf310,	0xf206,	0xf200,	0xf03c,	0xf10a,
	0xf10b,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf30e,	0xf702,	0xf30d,	0xf01c,	0xf701,	0xf205,	0xf114,	0xf603,
	0xf118,	0xf601,	0xf602,	0xf117,	0xf600,	0xf119,	0xf115,	0xf116,
	0xf11a,	0xf10c,	0xf10d,	0xf11b,	0xf11c,	0xf110,	0xf311,	0xf11d,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
};

u_short m_shift_map[NR_KEYS] = {
	0xf200,	0xf01b,	0xf021,	0xf040,	0xf023,	0xf024,	0xf025,	0xf05e,
	0xf026,	0xf02a,	0xf028,	0xf029,	0xf05f,	0xf02b,	0xf07f,	0xf009,
	0xfb51,	0xfb57,	0xfb45,	0xfb52,	0xfb54,	0xfb59,	0xfb55,	0xfb49,
	0xfb4f,	0xfb50,	0xf07b,	0xf07d,	0xf201,	0xf702,	0xfb41,	0xfb53,
	0xfb44,	0xfb46,	0xfb47,	0xfb48,	0xfb4a,	0xfb4b,	0xfb4c,	0xf03a,
	0xf022,	0xf07e,	0xf700,	0xf07c,	0xfb5a,	0xfb58,	0xfb43,	0xfb56,
	0xfb42,	0xfb4e,	0xfb4d,	0xf03c,	0xf03e,	0xf03f,	0xf700,	0xf30c,
	0xf703,	0xf020,	0xf207,	0xf10a,	0xf10b,	0xf10c,	0xf10d,	0xf10e,
	0xf10f,	0xf110,	0xf111,	0xf112,	0xf113,	0xf213,	0xf203,	0xf307,
	0xf308,	0xf309,	0xf30b,	0xf304,	0xf305,	0xf306,	0xf30a,	0xf301,
	0xf302,	0xf303,	0xf300,	0xf310,	0xf206,	0xf200,	0xf03e,	0xf10a,
	0xf10b,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf30e,	0xf702,	0xf30d,	0xf200,	0xf701,	0xf205,	0xf114,	0xf603,
	0xf20b,	0xf601,	0xf602,	0xf117,	0xf600,	0xf20a,	0xf115,	0xf116,
	0xf11a,	0xf10c,	0xf10d,	0xf11b,	0xf11c,	0xf110,	0xf311,	0xf11d,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
};

u_short m_ctrl_map[NR_KEYS] = {
	0xf200,	0xf200,	0xf200,	0xf000,	0xf01b,	0xf01c,	0xf01d,	0xf01e,
	0xf01f,	0xf07f,	0xf200,	0xf200,	0xf01f,	0xf200,	0xf008,	0xf200,
	0xf011,	0xf017,	0xf005,	0xf012,	0xf014,	0xf019,	0xf015,	0xf009,
	0xf00f,	0xf010,	0xf01b,	0xf01d,	0xf201,	0xf702,	0xf001,	0xf013,
	0xf004,	0xf006,	0xf007,	0xf008,	0xf00a,	0xf00b,	0xf00c,	0xf200,
	0xf007,	0xf000,	0xf700,	0xf01c,	0xf01a,	0xf018,	0xf003,	0xf016,
	0xf002,	0xf00e,	0xf00d,	0xf200,	0xf20e,	0xf07f,	0xf700,	0xf30c,
	0xf703,	0xf000,	0xf207,	0xf100,	0xf101,	0xf102,	0xf103,	0xf104,
	0xf105,	0xf106,	0xf107,	0xf108,	0xf109,	0xf208,	0xf204,	0xf307,
	0xf308,	0xf309,	0xf30b,	0xf304,	0xf305,	0xf306,	0xf30a,	0xf301,
	0xf302,	0xf303,	0xf300,	0xf310,	0xf206,	0xf200,	0xf200,	0xf10a,
	0xf10b,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf30e,	0xf702,	0xf30d,	0xf01c,	0xf701,	0xf205,	0xf114,	0xf603,
	0xf118,	0xf601,	0xf602,	0xf117,	0xf600,	0xf119,	0xf115,	0xf116,
	0xf11a,	0xf10c,	0xf10d,	0xf11b,	0xf11c,	0xf110,	0xf311,	0xf11d,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
};

static int get_kbd_char(void)
{
	int	scancode, scanstatus;
	static int shift_lock;	/* CAPS LOCK state (0-off, 1-on) */
	static int shift_key;	/* Shift next keypress */
	static int ctrl_key;
	u_short keychar;

	if ((inb(KBD_STATUS_REG) & KBD_STAT_OBF) == 0)
		return -1;
	/* Fetch the scancode */
	scancode = inb(KBD_DATA_REG);
	scanstatus = inb(KBD_STATUS_REG);

	/* Ignore mouse events. */
	if (scanstatus & KBD_STAT_MOUSE_OBF)
		return -1;

	/* Ignore release, trigger on make
	 * (except for shift keys, where we want to
	 *  keep the shift state so long as the key is
	 *  held down).
	 */

	if (((scancode & 0x7f) == 0x2a) ||
	    ((scancode & 0x7f) == 0x36)) {
		/* Next key may use shift table */
		if ((scancode & 0x80) == 0)
			shift_key = 1;
		else
			shift_key = 0;
		return -1;
	}

	if ((scancode & 0x7f) == 0x1d) {
		/* Left ctrl key */
		if ((scancode & 0x80) == 0)
			ctrl_key = 1;
		else
			ctrl_key = 0;
		return -1;
	}

	if ((scancode & 0x80) != 0)
		return -1;

	scancode &= 0x7f;

	/* Translate scancode */

	if (scancode == 0x3a) {
		/* toggle caps lock */
		shift_lock ^= 1;
		return -1;
	}

	if (scancode == 0x0e) {
		/* Backspace */
		return 8;
	}

	if (scancode == 0xe0)
		return -1;

	/* For Japanese 86/106 keyboards
	 *	See comment in drivers/char/pc_keyb.c.
	 *	- Masahiro Adegawa
	 */
	if (scancode == 0x73)
		scancode = 0x59;
	else if (scancode == 0x7d)
		scancode = 0x7c;

	if (!shift_lock && !shift_key && !ctrl_key) {
		keychar = m_plain_map[scancode];
	} else if (shift_lock || shift_key) {
		keychar = m_shift_map[scancode];
	} else if (ctrl_key) {
		keychar = m_ctrl_map[scancode];
	} else {
		keychar = 0x0020;
		dbg_pr("Unknown state/scancode (%d)\n", scancode);
	}

	keychar &= 0x0fff;
	if (keychar == '\t')
		keychar = ' ';

	switch (keychar) {
	case K_F1:
	case K_F2:
	case K_F3:
	case K_F4:
	case K_F5:
	case K_F6:
	case K_F7:
	case K_F8:
	case K_F9:
	case K_F10:
	case K_F11:
	case K_F12:
		return keychar;
	default:
		break;
	}

	switch (KTYP(keychar)) {
	case KT_LETTER:
	case KT_LATIN:
		if (isprint(keychar))
			break;		/* printable characters */
		/* drop through */
	case KT_SPEC:
		if (keychar == K_ENTER)
			break;
		/* drop through */
	case KT_PAD:
		switch (keychar) {
		case K_P0:
		case K_P1:
		case K_P2:
		case K_P4:
		case K_P6:
		case K_P7:
		case K_P8:
		case K_PDOT:
			return keychar;
		}
		return -1;

	case KT_CUR:
		switch (keychar) {
		case K_DOWN:
		case K_LEFT:
		case K_RIGHT:
		case K_UP:
			return keychar;
		}
		return -1;

	default:
		return -1;	/* ignore unprintables */
	}

	if ((scancode & 0x7f) == 0x1c) {
		/* enter key.  All done.  Absorb the release scancode. */
		while ((inb(KBD_STATUS_REG) & KBD_STAT_OBF) == 0)
			;

		/* Fetch the scancode */
		scancode = inb(KBD_DATA_REG);
		scanstatus = inb(KBD_STATUS_REG);

		while (scanstatus & KBD_STAT_MOUSE_OBF) {
			scancode = inb(KBD_DATA_REG);
			scanstatus = inb(KBD_STATUS_REG);
		}

		/* enter-release error */
		/* if (scancode != 0x9c) */

		if (!mdb_suppress_crlf)
			dbg_pr("\n");
		return 13;
	}

	/* echo the character. */
	if (!mdb_suppress_crlf)
		dbg_pr("%c", keychar & 0xff);
	return keychar & 0xff;
}

int mdb_getkey(void)
{
	int key = -1;

	for (;;) {
		key = get_kbd_char();
		if (key != -1)
			break;

		mdb_watchdogs();

		key = get_modem_char();
		if (key != -1)
			break;

		mdb_watchdogs();
	}
	return key;
}

int mdb_copy(void *to, void *from, size_t size)
{
	return probe_kernel_write((char *)to, (char *)from, size);
}

int mdb_verify_rw(void *addr, size_t size)
{
	unsigned char data[size];

	return mdb_copy(data, addr, size);
}

static int mdb_getphys(void *res, unsigned long addr, size_t size)
{
	unsigned long pfn;
	void *vaddr;
	struct page *page;

	pfn = (addr >> PAGE_SHIFT);
	if (!pfn_valid(pfn))
		return 1;
	page = pfn_to_page(pfn);
	vaddr = kmap_atomic(page);
	memcpy(res, vaddr + (addr & (PAGE_SIZE - 1)), size);
	kunmap_atomic(vaddr);

	return 0;
}

int mdb_getphysword(u64 *word, unsigned long addr, size_t size)
{
	int err;
	__u8  w1;
	__u16 w2;
	__u32 w4;
	__u64 w8;
	*word = 0;

	switch (size) {
	case 1:
		err = mdb_getphys(&w1, addr, sizeof(w1));
		if (!err)
			*word = w1;
		break;
	case 2:
		err = mdb_getphys(&w2, addr, sizeof(w2));
		if (!err)
			*word = w2;
		break;
	case 4:
		err = mdb_getphys(&w4, addr, sizeof(w4));
		if (!err)
			*word = w4;
		break;
	case 8:
		if (size <= sizeof(*word)) {
			err = mdb_getphys(&w8, addr, sizeof(w8));
			if (!err)
				*word = w8;
			break;
		}
	default:
		err = -EFAULT;
	}
	return err;
}

int mdb_getlword(u64 *word, unsigned long addr, size_t size)
{
	int err;

	__u8  w1;
	__u16 w2;
	__u32 w4;
	__u64 w8;

	*word = 0;	/* Default value if addr or size is invalid */
	switch (size) {
	case 1:
		err = mdb_copy(&w1, (void *)addr, size);
		if (!err)
			*word = w1;
		break;
	case 2:
		err = mdb_copy(&w2, (void *)addr, size);
		if (!err)
			*word = w2;
		break;
	case 4:
		err = mdb_copy(&w4, (void *)addr, size);
		if (!err)
			*word = w4;
		break;
	case 8:
		err = mdb_copy(&w8, (void *)addr, size);
		if (!err)
			*word = w8;
		break;
	default:
		err = -EFAULT;
	}
	return err;
}

int mdb_putword(unsigned long addr, unsigned long word, size_t size)
{
	int err;
	__u8  w1;
	__u16 w2;
	__u32 w4;
	__u64 w8;

	switch (size) {
	case 1:
		w1 = word;
		err = mdb_copy((void *)addr, &w1, size);
		break;
	case 2:
		w2 = word;
		err = mdb_copy((void *)addr, &w2, size);
		break;
	case 4:
		w4 = word;
		err = mdb_copy((void *)addr, &w4, size);
		break;
	case 8:
		w8 = word;
		err = mdb_copy((void *)addr, &w8, size);
		break;
	default:
		err = -EFAULT;
	}
	return err;
}

int mdb_putqword(u64 *addr, u64 word, size_t size)
{
	int err;
	__u8  w1;
	__u16 w2;
	__u32 w4;
	__u64 w8;

	switch (size) {
	case 1:
		w1 = word;
		err = mdb_copy((void *)addr, &w1, size);
		break;
	case 2:
		w2 = word;
		err = mdb_copy((void *)addr, &w2, size);
		break;
	case 4:
		w4 = word;
		err = mdb_copy((void *)addr, &w4, size);
		break;
	case 8:
		w8 = word;
		err = mdb_copy((void *)addr, &w8, size);
		break;
	default:
		err = -EFAULT;
	}
	return err;
}

u64 mdb_getqword(u64 *addr, size_t size)
{
	u64 data = 0;

	register int ret;

	ret = mdb_getlword(&data, (unsigned long)addr, size);
	if (ret)
		return 0;

	return data;
}

u64 mdb_phys_getqword(u64 *addr, size_t size)
{
	u64 data = 0;
	register int ret;

	ret = mdb_getphysword(&data, (unsigned long)addr, size);
	if (ret)
		return 0;

	return data;
}

unsigned long mdb_phys_getword(unsigned long addr, size_t size)
{
	u64 data = 0;
	register int ret;

	ret = mdb_getphysword((u64 *)&data, addr, size);
	if (ret)
		return 0;

	return (unsigned long)data;
}

unsigned long mdb_getword(unsigned long addr, size_t size)
{
	u64 data = 0;
	register int ret;

	ret = mdb_getlword((u64 *)&data, addr, size);
	if (ret)
		return 0;

	return (unsigned long)data;
}

u64 mdb_segment_getqword(unsigned long sv, u64 *addr, size_t size)
{
	u64 data = 0;
	register int ret;

	ret = mdb_getlword(&data, (unsigned long)addr, size);
	if (ret)
		return 0;

	return data;
}

unsigned long mdb_segment_getword(unsigned long sv, unsigned long addr,
				  size_t size)
{
	u64 data = 0;
	register int ret;

	ret = mdb_getlword((u64 *)&data, addr, size);
	if (ret)
		return 0;

	return (unsigned long)data;
}

int closest_symbol(unsigned long address)
{
	char *modname;
	const char *name;
	unsigned long offset = 0, size;
	char namebuf[KSYM_NAME_LEN + 1];

	name = kallsyms_lookup(address, &size, &offset, &modname, namebuf);
	if (!name)
		return -1;

	if (modname) {
		if (offset)
			dbg_pr("%s|%s+0x%X\n", modname, name, offset);
		else
			dbg_pr("%s|%s\n", modname, name);
	} else {
		if (offset)
			dbg_pr("%s+0x%X\n", name, offset);
		else
			dbg_pr("%s\n", name);
	}
	return 0;
}

void dump_os_symbol_table_match(unsigned char *symbol)
{
	mdb_kallsyms(symbol, dbg_pr);
}

unsigned long get_value_from_symbol(unsigned char *symbol)
{
	return (unsigned long)kallsyms_lookup_name(symbol);
}

unsigned char *get_module_symbol_value(unsigned long value,
				       unsigned char *buf,
				       unsigned long len)
{
	char *modname;
	const char *name;
	unsigned long offset, size;
	char namebuf[KSYM_NAME_LEN + 1];

	name = kallsyms_lookup(value, &size, &offset, &modname, namebuf);
	if (name && modname && buf) {
		mdb_copy(buf, modname, len);
		return (unsigned char *)buf;
	}
	return NULL;
}

unsigned char *get_symbol_value(unsigned long value, unsigned char *buf,
				unsigned long len)
{
	char *modname;
	const char *name;
	unsigned long offset, size;
	char namebuf[KSYM_NAME_LEN + 1];

	name = kallsyms_lookup(value, &size, &offset, &modname, namebuf);
	if (!name)
		return NULL;

	if (!offset && buf) {
		mdb_copy(buf, namebuf, len);
		return (unsigned char *)buf;
	}

	return NULL;
}

unsigned char *get_symbol_value_offset(unsigned long value,
				       unsigned long *sym_offset,
				       unsigned char *buf,
				       unsigned long len)
{
	char *modname;
	const char *name;
	unsigned long offset, size;
	char namebuf[KSYM_NAME_LEN + 1];

	name = kallsyms_lookup(value, &size, &offset, &modname, namebuf);
	if (!name || !buf)
		return NULL;

	if (sym_offset)
		*sym_offset = offset;

	mdb_copy(buf, namebuf, len);
	return (unsigned char *)buf;
}

unsigned char *get_symbol_value_offset_module(unsigned long value,
					      unsigned long *sym_offset,
					      unsigned char **module,
					      unsigned char *buf,
					      unsigned long len)
{
	char *modname;
	const char *name;
	unsigned long offset, size;
	char namebuf[KSYM_NAME_LEN + 1];

	name = kallsyms_lookup(value, &size, &offset, &modname, namebuf);
	if (!name || !buf)
		return NULL;

	if (sym_offset)
		*sym_offset = offset;

	if (module)
		*module = modname;

	mdb_copy(buf, namebuf, len);
	return (unsigned char *)buf;
}

unsigned long get_processor_id(void)
{
#if IS_ENABLED(CONFIG_SMP)
	return raw_smp_processor_id();
#else
	return 0;
#endif
}

unsigned long get_physical_processor(void)
{
#if IS_ENABLED(CONFIG_SMP)
	return raw_smp_processor_id();
#else
	return 0;
#endif
}

unsigned long fpu_present(void)
{
	if (boot_cpu_has(X86_FEATURE_FPU))
		return 1;
	return 0;
}

unsigned long cpu_mttr_on(void)
{
	if (boot_cpu_has(X86_FEATURE_MTRR))
		return 1;
	return 0;
}

unsigned char *upcase_string(unsigned char *s)
{
	register int i;

	for (i = 0; i < strlen(s); i++)
		s[i] = toupper(s[i]);
	return s;
}

void clear_screen(void)
{
	mdb_printf("%c%c", 0x1B, 'c');
}

unsigned long read_ds(void)
{
	unsigned short contents = 0;

	__asm__ ("mov %%ds,%0\n\t" : "=r"(contents));
	return contents;
}

unsigned long read_es(void)
{
	unsigned short contents = 0;

	__asm__ ("mov %%es,%0\n\t" : "=r"(contents));
	return contents;
}

unsigned long read_fs(void)
{
	unsigned short contents = 0;

	__asm__ ("mov %%fs,%0\n\t" : "=r"(contents));
	return contents;
}

unsigned long read_gs(void)
{
	unsigned short contents = 0;

	__asm__ ("mov %%gs,%0\n\t" : "=r"(contents));
	return contents;
}

#if IS_ENABLED(CONFIG_X86_64)
unsigned long dbg_read_dr(unsigned long regnum)
{
	unsigned long contents = 0;

	switch (regnum) {
	case 0:
		__asm__ ("movq %%db0,%0\n\t" : "=r"(contents));
		break;
	case 1:
		__asm__ ("movq %%db1,%0\n\t" : "=r"(contents));
		break;
	case 2:
		__asm__ ("movq %%db2,%0\n\t" : "=r"(contents));
		break;
	case 3:
		__asm__ ("movq %%db3,%0\n\t" : "=r"(contents));
		break;
	case 4:
	case 5:
		break;
	case 6:
		__asm__ ("movq %%db6,%0\n\t" : "=r"(contents));
		break;
	case 7:
		__asm__ ("movq %%db7,%0\n\t" : "=r"(contents));
		break;
	default:
		break;
	}

	return contents;
}

void dbg_write_dr(int regnum, unsigned long contents)
{
	switch (regnum) {
	case 0:
		__asm__ ("movq %0,%%db0\n\t" : : "r"(contents));
		break;
	case 1:
		__asm__ ("movq %0,%%db1\n\t" : : "r"(contents));
		break;
	case 2:
		__asm__ ("movq %0,%%db2\n\t" : : "r"(contents));
		break;
	case 3:
		__asm__ ("movq %0,%%db3\n\t" : : "r"(contents));
		break;
	case 4:
	case 5:
		break;
	case 6:
		__asm__ ("movq %0,%%db6\n\t" : : "r"(contents));
		break;
	case 7:
		__asm__ ("movq %0,%%db7\n\t" : : "r"(contents));
		break;
	default:
		break;
	}
}

unsigned long dbg_read_cr(int regnum)
{
	unsigned long contents = 0;

	switch (regnum) {
	case 0:
		__asm__ ("movq %%cr0,%0\n\t" : "=r"(contents));
		break;
	case 1:
		break;
	case 2:
		__asm__ ("movq %%cr2,%0\n\t" : "=r"(contents));
		break;
	case 3:
		__asm__ ("movq %%cr3,%0\n\t" : "=r"(contents));
		break;
	case 4:
		__asm__ ("movq %%cr4,%0\n\t" : "=r"(contents));
		break;
	default:
		break;
	}
	return contents;
}

void dbg_write_cr(int regnum, unsigned long contents)
{
	switch (regnum) {
	case 0:
		__asm__ ("movq %0,%%cr0\n\t" : : "r"(contents));
		break;
	case 1:
		break;
	case 2:
		__asm__ ("movq %0,%%cr2\n\t" : : "r"(contents));
		break;
	case 3:
		__asm__ ("movq %0,%%cr3\n\t" : : "r"(contents));
		break;
	case 4:
		__asm__ ("movq %0,%%cr4\n\t" : : "r"(contents));
		break;
	default:
		break;
	}
}

#else
unsigned long dbg_read_dr(unsigned long regnum)
{
	unsigned long contents = 0;

	switch (regnum) {
	case 0:
		__asm__ ("movl %%db0,%0\n\t" : "=r"(contents));
		break;
	case 1:
		__asm__ ("movl %%db1,%0\n\t" : "=r"(contents));
		break;
	case 2:
		__asm__ ("movl %%db2,%0\n\t" : "=r"(contents));
		break;
	case 3:
		__asm__ ("movl %%db3,%0\n\t" : "=r"(contents));
		break;
	case 4:
	case 5:
		break;
	case 6:
		__asm__ ("movl %%db6,%0\n\t" : "=r"(contents));
		break;
	case 7:
		__asm__ ("movl %%db7,%0\n\t" : "=r"(contents));
		break;
	default:
		break;
	}

	return contents;
}

void dbg_write_dr(int regnum, unsigned long contents)
{
	switch (regnum) {
	case 0:
		__asm__ ("movl %0,%%db0\n\t" : : "r"(contents));
		break;
	case 1:
		__asm__ ("movl %0,%%db1\n\t" : : "r"(contents));
		break;
	case 2:
		__asm__ ("movl %0,%%db2\n\t" : : "r"(contents));
		break;
	case 3:
		__asm__ ("movl %0,%%db3\n\t" : : "r"(contents));
		break;
	case 4:
	case 5:
		break;
	case 6:
		__asm__ ("movl %0,%%db6\n\t" : : "r"(contents));
		break;
	case 7:
		__asm__ ("movl %0,%%db7\n\t" : : "r"(contents));
		break;
	default:
		break;
	}
}

unsigned long dbg_read_cr(int regnum)
{
	unsigned long contents = 0;

	switch (regnum) {
	case 0:
		__asm__ ("movl %%cr0,%0\n\t" : "=r"(contents));
		break;
	case 1:
		break;
	case 2:
		__asm__ ("movl %%cr2,%0\n\t" : "=r"(contents));
		break;
	case 3:
		__asm__ ("movl %%cr3,%0\n\t" : "=r"(contents));
		break;
	case 4:
		__asm__ ("movl %%cr4,%0\n\t" : "=r"(contents));
		break;
	default:
		break;
	}
	return contents;
}

void dbg_write_cr(int regnum, unsigned long contents)
{
	switch (regnum) {
	case 0:
		__asm__ ("movl %0,%%cr0\n\t" : : "r"(contents));
		break;
	case 1:
		break;
	case 2:
		__asm__ ("movl %0,%%cr2\n\t" : : "r"(contents));
		break;
	case 3:
		__asm__ ("movl %0,%%cr3\n\t" : : "r"(contents));
		break;
	case 4:
		__asm__ ("movl %0,%%cr4\n\t" : : "r"(contents));
		break;
	default:
		break;
	}
}
#endif

unsigned long read_tr(void)
{
	unsigned short tr;

	__asm__ __volatile__("str %0" : "=a"(tr));

	return (unsigned long)tr;
}

unsigned long read_ldtr(void)
{
	unsigned short ldt;

	__asm__ __volatile__("sldt %0" : "=a"(ldt));

	return (unsigned long)ldt;
}

void read_gdtr(unsigned long *v)
{
	__asm__ __volatile__("sgdt %0" : "=m"(*v));
}

void read_idtr(unsigned long *v)
{
	__asm__ __volatile__("sidt %0" : "=m"(*v));
}

void save_npx(NUMERIC_FRAME *v)
{
	__asm__ __volatile__("fsave %0" : "=m"(*v));
}

void load_npx(NUMERIC_FRAME *v)
{
	__asm__ __volatile__("frstor %0" : "=m"(*v));
}

unsigned long dbg_read_dr6(void)  {  return dbg_read_dr(6); }

unsigned long dbg_read_dr0(void)  {  return dbg_read_dr(0); }
unsigned long dbg_read_dr1(void)  {  return dbg_read_dr(1); }
unsigned long dbg_read_dr2(void)  {  return dbg_read_dr(2); }
unsigned long dbg_read_dr3(void)  {  return dbg_read_dr(3); }
unsigned long dbg_read_dr7(void)  {  return dbg_read_dr(7); }

void dbg_write_dr0(unsigned long v) { dbg_write_dr(0, v); }
void dbg_write_dr1(unsigned long v) { dbg_write_dr(1, v); }
void dbg_write_dr2(unsigned long v) { dbg_write_dr(2, v); }
void dbg_write_dr3(unsigned long v) { dbg_write_dr(3, v); }
void dbg_write_dr6(unsigned long v) { dbg_write_dr(6, v); }
void dbg_write_dr7(unsigned long v) { dbg_write_dr(7, v); }

unsigned long dbg_read_cr0(void) {  return dbg_read_cr(0); }
unsigned long dbg_read_cr2(void) {  return dbg_read_cr(2); }
unsigned long dbg_read_cr3(void) {  return dbg_read_cr(3); }
unsigned long dbg_read_cr4(void) {  return dbg_read_cr(4); }

void dbg_write_cr0(unsigned long v) { dbg_write_cr(0, v); }
void dbg_write_cr2(unsigned long v) { dbg_write_cr(2, v); }
void dbg_write_cr3(unsigned long v) { dbg_write_cr(3, v); }
void dbg_write_cr4(unsigned long v) { dbg_write_cr(4, v); }

void read_msr(unsigned long r, unsigned long *v1, unsigned long *v2)
{
	unsigned long vv1, vv2;

	rdmsr(r, vv1, vv2);

	if (v1)
		*v1 = vv1;
	if (v2)
		*v2 = vv2;
}

void write_msr(unsigned long r, unsigned long *v1, unsigned long *v2)
{
	unsigned long vv1 = 0, vv2 = 0;

	if (v1)
		vv1 = *v1;
	if (v2)
		vv2 = *v2;

	wrmsr(r, vv1, vv2);
}
