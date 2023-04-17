#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);
extern void handle_ov(void);

void (*exception_handlers[32])(void) = {
    [0 ... 31] = handle_reserved,
    [0] = handle_int,
    [2 ... 3] = handle_tlb,
    [12] = handle_ov,
#if !defined(LAB) || LAB >= 4
    [1] = handle_mod,
    [8] = handle_sys,
#endif
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
void do_reserved(struct Trapframe *tf) {
	print_tf(tf);
	panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
}

void do_ov(struct Trapframe *tf) {
	unsigned long uepc = tf->cp0_epc;
	//Pte *pte;
	//page_lookup(curenv->env_pgdir , uepc, &pte);
	unsigned long pepc = va2pa(curenv->env_pgdir, uepc);
	unsigned long *kepc = (unsigned long *)(KADDR(pepc) + (uepc & 0xffff));
	unsigned long s = *kepc & (((1 << 5) - 1) << 21);
        unsigned long t = *kepc & (((1 << 5) - 1) << 16);

	printk("%x\n", *kepc);
	if (((*kepc & 0x20000000) == 0) && ((*kepc & ((1 << 11) - 1)) == 32)) {
		*kepc += 1;
		//unsigned long d = content & (((1 << 5) - 1) << 12);
		//tf->cp0_epc += 4;
		//tf->regs[d] = tf->regs[s] + tf->regs[t];
		printk("add ov handled\n");
	} else if (((*kepc & 0x20000000) == 0) && ((*kepc & ((1 << 11) - 1)) == 34)) {
		*kepc += 1;
		//unsigned long d = content & (((1 << 5) - 1) << 12);
                //tf->cp0_epc += 4;
                //tf->regs[d] = tf->regs[s] - tf->regs[t];
		printk("sub ov handled\n");
	} else if ((*kepc & 0x20000000) != 0) {
		tf->cp0_epc += 4;
		//unsigned long s = content & (((1 << 5) - 1) << 21);
		//unsigned long t = content & (((1 << 5) - 1) << 16);
		tf->regs[t] = tf->regs[s] / 2 + (*kepc & ((1 << 16) - 1)) / 2;
		printk("addi ov handled\n");

	}
	curenv->env_ov_cnt++;
}
