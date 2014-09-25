// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Traces stack", mon_backtrace }, 
	{ "showmappings", "Display information about the physical page mappings", mon_showmappings },
	{ "set", "Set, clear or change Permissions", mon_set },
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{

        int *my_ebp =(int *)read_ebp();
        int eip = my_ebp[1];
        struct Eipdebuginfo info;
        cprintf("Stack backtrace: \n");

        while (my_ebp != NULL) {
                debuginfo_eip(eip, &info);

                cprintf("  ebp %08x eip %08x args %08x %08x %08x %08x %08x \n",my_ebp,  eip, my_ebp[2], my_ebp[3], my_ebp[4], my_ebp[5], my_ebp[6] );
                cprintf("        %s:%d: %.*s+%d\n", info.eip_file, info.eip_line, info.eip_fn_namelen, info.eip_fn_name, eip-info.eip_fn_addr);

                my_ebp = (int *)*my_ebp;
		eip = my_ebp[1];

        }

        return 0;
}

int
mon_showmappings(int argc, char **argv, struct Trapframe *tf) 
{
	int i;
	extern pde_t *kern_pgdir;
	cprintf("Virtual Address     Physical Page Mappings     Permissions\n");
	uintptr_t start = strtol(argv[1], NULL, 16), end = strtol(argv[2], NULL, 16);
	
	
	for (i = 0; i <= end - start; i+=0x1000) {
		
		uintptr_t va = start + i;
		pte_t * pteP = pgdir_walk(kern_pgdir,(void*) va, false);
		

		if(pteP == NULL)
			cprintf("0x%08x		None			NULL\n", va);		
			
		else {
			char perm[9];
		
			if(*pteP & PTE_P)
				perm[0] = 'P';
			else{perm[0] = '_';};

	                if(*pteP & PTE_W)
				 perm[1] = 'W';
                        else{perm[1] = '_';};
                        
                        if(*pteP & PTE_U)
                                perm[2] = 'U';
                        else{perm[2] = '_';};

                        if(*pteP & PTE_PWT)
                                perm[3] = 'T';
                        else{perm[3] = '_';};

                        if(*pteP & PTE_PCD)
                                 perm[4] = 'C';
                        else{perm[4] = '_';};

                        if(*pteP & PTE_A)
                                perm[5] = 'A';
                        else{perm[5] = '_';};

                        if(*pteP & PTE_D)
                                 perm[6] = 'D';
                        else{perm[6] = '_';};

                        if(*pteP & PTE_PS)
                                perm[7] = 'S';
                        else{perm[7] = '_';};

                        if(*pteP & PTE_G)
                                 perm[8] = 'G';
                        else{perm[8] = '_';};
			
			cprintf("0x%08x 	0x%08x	              %c%c%c%c%c%c%c%c%c\n", va, *pteP, perm[8], perm[7], perm[6], perm[5], perm[4], perm[3], perm[2], perm[1], perm[0]);	
		}
	}	

	return 0;
}

int 
mon_set(int argc, char **argv, struct Trapframe *tf)
{
	// set address new 

        extern pde_t *kern_pgdir;
     
        uintptr_t va = strtol(argv[1], NULL, 16);

	int perm = *argv[2];	

       	pte_t * pteP = pgdir_walk(kern_pgdir,(void*) va, false);
	
	if (perm < 0x00000FFF){
		*pteP |= perm;
		cprintf("The page table entry pointer is now 0x%08x\n", *pteP);
	}
	else{cprintf("Invalid permission bit\n");};

	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}

