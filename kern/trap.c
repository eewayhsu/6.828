#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>

#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/env.h>
#include <kern/syscall.h>

static struct Taskstate ts;

/* For debugging, so print_trapframe can distinguish between printing
 * a saved trapframe and printing the current trapframe and print some
 * additional information in the latter case.
 */
static struct Trapframe *last_tf;

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};


static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Fault",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	return "(unknown trap)";
}


void
trap_init(void)
{
	extern struct Segdesc gdt[];
	extern uint32_t handlers[];	
	
	void handler0();
        void handler1();
        void handler2();
        void handler3();
        void handler4();
        void handler5();
        void handler6();
        void handler7();
        void handler8();
        void handler9();
        void handler10();
        void handler11();
        void handler12();
        void handler13();
        void handler14();
        void handler15();
        void handler16();
        void handler17();
        void handler18();
        void handler19();
        void handler20();
        void handler21();
        void handler22();
        void handler23();
        void handler24();
        void handler25();
        void handler26();
        void handler27();
        void handler28();
        void handler29();
        void handler30();
        void handler31();
	void handler48();
	// LAB 3: Your code here.
	
	int sel = GD_KT;

	SETGATE(idt[0], 0, sel, &handler0, 0);
        SETGATE(idt[1], 0, sel, &handler1, 0);
        SETGATE(idt[2], 0, sel, &handler2, 0);
        SETGATE(idt[3], 1, sel, &handler3, 3);
        SETGATE(idt[4], 1, sel, &handler4, 0);
        SETGATE(idt[5], 0, sel, &handler5, 0);
        SETGATE(idt[6], 0, sel, &handler6, 0);
        SETGATE(idt[7], 0, sel, &handler7, 0);
        SETGATE(idt[8], 0, sel, &handler8, 0);
        SETGATE(idt[9], 0, sel, &handler9, 0);

        SETGATE(idt[10], 0, sel, &handler10, 0);
        SETGATE(idt[11], 0, sel, &handler11, 0);
        SETGATE(idt[12], 0, sel, &handler12, 0);
        SETGATE(idt[13], 0, sel, &handler13, 0);
        SETGATE(idt[14], 0, sel, &handler14, 0);
        SETGATE(idt[15], 0, sel, &handler15, 0);

        SETGATE(idt[16], 0, sel, &handler16, 0);
        SETGATE(idt[17], 0, sel, &handler17, 0);
        SETGATE(idt[18], 0, sel, &handler18, 0);
        SETGATE(idt[19], 0, sel, &handler19, 0);
	
	SETGATE(idt[20], 0, sel, &handler20, 0);
        SETGATE(idt[21], 0, sel, &handler21, 0);
        SETGATE(idt[22], 0, sel, &handler22, 0);
        SETGATE(idt[23], 0, sel, &handler23, 0);
        SETGATE(idt[24], 0, sel, &handler24, 0);
        SETGATE(idt[25], 0, sel, &handler25, 0);
        SETGATE(idt[26], 0, sel, &handler26, 0);
        SETGATE(idt[27], 0, sel, &handler27, 0);
        SETGATE(idt[28], 0, sel, &handler28, 0);
        SETGATE(idt[29], 0, sel, &handler29, 0);
        SETGATE(idt[30], 0, sel, &handler30, 0);
        SETGATE(idt[31], 0, sel, &handler31, 0);
	SETGATE(idt[48], 0, sel, &handler48, 3);

	/*cprintf("0 %d \n", &handler0);
	cprintf("1 %d \n", &handler1);
	cprintf("2 %d \n", &handler2);
	cprintf("3 %d \n", &handler3);
	cprintf("4 %d \n", &handler4);
	cprintf("5 %d \n", &handler5);
	cprintf("6 %d \n", &handler6);
	cprintf("7 %d \n", &handler7);
	cprintf("8 %d \n", &handler8);
	cprintf("9 %d \n", &handler9);
	cprintf("10 %d \n", &handler10);
	cprintf("11 %d \n", &handler11);
	cprintf("12 %d \n", &handler12);
	cprintf("13 %d \n", &handler13);
	cprintf("14 %d \n", &handler14);
	cprintf("15 %d \n", &handler15);
	cprintf("16 %d \n", &handler16);
	cprintf("17 %d \n", &handler17);
	cprintf("18 %d \n", &handler18);
	cprintf("19 %d \n", &handler19);
	cprintf("20 %d \n", &handler20);
	cprintf("21 %d \n", &handler21);
	cprintf("22 %d \n", &handler22);*/



/*
	int i = 0;
	long count = 0;
	
	for (; i < 8; i++) {
		if (i == 3){
			SETGATE(idt[i], 1, GD_KT, &handlers[count], 3);}
		if (i == 4){
			SETGATE(idt[i], 1, GD_KT, &handlers[count], 0);}
		else{
			SETGATE(idt[i], 0, GD_KT, &handlers[count], 0);}
		cprintf("%d %d\n", i, &handlers[count]);
		count += 2;
	}

	for (; 8 <= i && i < 15; i++) {
                SETGATE(idt[i], 0, GD_KT, &handlers[count], 0);
		count += 2;
                cprintf("%d %d\n", i,  &handlers[count]);
	}

	for (; 15 <= i && i < 17; i++) {
                SETGATE(idt[i], 0, GD_KT, &handlers[count], 0);
        	count += 2;
                cprintf("%d %d\n", i, &handlers[count]);


	}

	SETGATE(idt[i], 0, GD_KT, &handlers[count], 0);
		i += 1;
		count += 2;
                cprintf("%d %d\n", i, &handlers[count]);
		

        for (; 17 <= i && i < 32; i++) {
                SETGATE(idt[i], 0, GD_KT, &handlers[count], 0);
		count += 2;
                cprintf("%d %d\n", i, &handlers[count]);

        }
*/

/*	int i = 0;
	long addr = (long)&handlers[0];
	
	for (; i < 8; i ++){

		if (i == 3){
                        SETGATE(idt[i], 1, GD_KT, addr, 3);}
                if (i == 4){
                        SETGATE(idt[i], 1, GD_KT, addr, 0);}
                else{
                        SETGATE(idt[i], 0, GD_KT, addr, 0);}

		addr += 10;}			
	
	for (; 8 <= i && i < 15; i++) {
	        SETGATE(idt[i], 0, GD_KT, addr, 0);
                addr += 8;
	}


	for (; 15 <= i && i < 17; i++) {
                SETGATE(idt[i], 0, GD_KT, addr, 0);
                addr += 10;
        }

        SETGATE(idt[i], 0, GD_KT, addr, 0);
                i += 1;
                addr += 8;           

        for (; 18 <= i && i < 32; i++) {
                SETGATE(idt[i], 0, GD_KT, addr, 0);
                addr += 10;
        }
*/

	// Per-CPU setup 
	trap_init_percpu();
}

// Initialize and load the per-CPU TSS and IDT
void
trap_init_percpu(void)
{
	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	ts.ts_esp0 = KSTACKTOP;
	ts.ts_ss0 = GD_KD;

	// Initialize the TSS slot of the gdt.
	gdt[GD_TSS0 >> 3] = SEG16(STS_T32A, (uint32_t) (&ts),
					sizeof(struct Taskstate) - 1, 0);
	gdt[GD_TSS0 >> 3].sd_s = 0;

	// Load the TSS selector (like other segment selectors, the
	// bottom three bits are special; we leave them 0)
	ltr(GD_TSS0);

	// Load the IDT
	lidt(&idt_pd);
}

void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p\n", tf);
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	// If this trap was a page fault that just happened
	// (so %cr2 is meaningful), print the faulting linear address.
	if (tf == last_tf && tf->tf_trapno == T_PGFLT)
		cprintf("  cr2  0x%08x\n", rcr2());
	cprintf("  err  0x%08x", tf->tf_err);
	// For page faults, print decoded fault error code:
	// U/K=fault occurred in user/kernel mode
	// W/R=a write/read caused the fault
	// PR=a protection violation caused the fault (NP=page not present).
	if (tf->tf_trapno == T_PGFLT)
		cprintf(" [%s, %s, %s]\n",
			tf->tf_err & 4 ? "user" : "kernel",
			tf->tf_err & 2 ? "write" : "read",
			tf->tf_err & 1 ? "protection" : "not-present");
	else
		cprintf("\n");
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	if ((tf->tf_cs & 3) != 0) {
		cprintf("  esp  0x%08x\n", tf->tf_esp);
		cprintf("  ss   0x----%04x\n", tf->tf_ss);
	}
}

void
print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void
trap_dispatch(struct Trapframe *tf)
{
	// Handle processor exceptions.
	// LAB 3: Your code here   

	struct PushRegs reg;
	reg = tf->tf_regs;

	switch (tf->tf_trapno) {
	
	case T_PGFLT:
		page_fault_handler(tf);
		return;
	
	case T_BRKPT:
		monitor(tf);
		return;

	case T_SYSCALL:
		tf->tf_regs.reg_eax = syscall(reg.reg_eax, reg.reg_edx, 
			reg.reg_ecx, reg.reg_ebx, 
			reg.reg_edi, reg.reg_esi);
		return;
		
	}

	// Unexpected trap: The user process or the kernel has a bug.
	
	   print_trapframe(tf);
	   if (tf->tf_cs == GD_KT)
		panic("unhandled trap in kernel");
	   else {
		env_destroy(curenv);
		return;
	}
}



void
trap(struct Trapframe *tf)
{
	// The environment may have set DF and some versions
	// of GCC rely on DF being clear
	asm volatile("cld" ::: "cc");

	// Check that interrupts are disabled.  If this assertion
	// fails, DO NOT be tempted to fix it by inserting a "cli" in
	// the interrupt path.
	assert(!(read_eflags() & FL_IF));

	cprintf("Incoming TRAP frame at %p\n", tf);

	if ((tf->tf_cs & 3) == 3) {
		// Trapped from user mode.
		assert(curenv);

		// Copy trap frame (which is currently on the stack)
		// into 'curenv->env_tf', so that running the environment
		// will restart at the trap point.
		curenv->env_tf = *tf;
		// The trapframe on the stack should be ignored from here on.
		tf = &curenv->env_tf;
	}

	// Record that tf is the last real trapframe so
	// print_trapframe can print some additional information.
	last_tf = tf;

	// Dispatch based on what type of trap occurred
	trap_dispatch(tf);

	// Return to the current environment, which should be running.
	assert(curenv && curenv->env_status == ENV_RUNNING);
	env_run(curenv);
}


void
page_fault_handler(struct Trapframe *tf)
{
	uint32_t fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	// Handle kernel-mode page faults.

	// LAB 3: Your code here.

 	//TODO: FIX
 
	if ((tf->tf_cs & 3) == 0)
	   panic("page fault occur in kernel.");

	// We've already handled kernel-mode exceptions, so if we get here,
	// the page fault happened in user mode.

	// Destroy the environment that caused the fault.
	cprintf("[%08x] user fault va %08x ip %08x\n",
		curenv->env_id, fault_va, tf->tf_eip);
	print_trapframe(tf);
	env_destroy(curenv);
}

