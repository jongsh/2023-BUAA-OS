#include <env.h>
#include <mmu.h>
#include <printk.h>

extern struct Env *curenv;

void remove_index(int a[], int index) {
    for (int i = index; a[i] != 0; ++i) {
        a[i] = a[i+1];
    }
}
/*
 * handling_signum 表示进程正在处理的信号
 * 如果 handling_signum = 0, 则表示进程当前没有处理信号
 * 返回 0 表示不用处理该信号; 返回 非0 表示要处理该信号
 */
int check_sig(u_int signum, u_int handling_signum) {
    sigset_t mask;
    // 得到正确的掩码
    if (handling_signum == 0) {
        mask = curenv->env_sa_mask;
    } else {
        mask = curenv->env_sigaction[handling_signum - 1].sa_mask;
    }

    u_int temp = signum - 1;
    if (signum == 9) {
        return 1;
    } else {
        return (~mask.sig[temp/32]) & (1 << (temp % 32));
    }
}

// 返回的数字表示要处理的信号，若为 0，表示不调用 sigaction_entry
int pre_handle_signal() {
    // 检查当前进程有没有等待处理的信号，如果没有则返回 0
    if (curenv->env_waiting_top < 0) {
        return 0;
    }
    u_int handling_signum = (curenv->env_handling_top >= 0) ? 
                    curenv->env_handling_siglist[curenv->env_handling_top] : 0;

    for (int i = curenv->env_waiting_top; i >= 0; --i) {
        u_int signum = curenv->env_waiting_siglist[i];
        if (handling_signum != 0 && signum == handling_signum) {
            return 0;
        }
        if (check_sig(signum, handling_signum)) {
            if (curenv->env_sigaction_avai[signum - 1] == 1) {
                return signum;
            } else {
                if (signum != 9 && signum != 11 && signum != 15) {
                    remove_index(curenv->env_waiting_siglist, i);
                    continue;
                } else {
                    return signum;
                }
            }
        }
    }
    //printk("kern return ...\n");
    return 0;
}

// 处理当前进程接收的所有信号
void handle_signal(struct Trapframe *tf) {
    struct Trapframe tmp_tf = *tf;

    //printk("%d : ----start--\n", curenv->env_id);
    
    u_int signum = pre_handle_signal();
    if (signum != 0) {
        curenv->env_handling_siglist[++curenv->env_handling_top] = signum;

        if (tf->regs[29] < USTACKTOP || tf->regs[29] >= UXSTACKTOP) {
		    tf->regs[29] = UXSTACKTOP;
	    }

        tf->regs[29] -= sizeof(struct Trapframe);
	    *(struct Trapframe *)tf->regs[29] = tmp_tf;

        if (curenv->env_user_sigaction_entry) {
		    tf->regs[4] = tf->regs[29];
            tf->regs[5] = (curenv->env_sigaction_avai[signum - 1] == 0) ? 
                            0 : (u_int)curenv->env_sigaction[signum - 1].sa_handler;
            tf->regs[6] = signum;
		    tf->regs[29] -= sizeof(tf->regs[4]);
            tf->regs[29] -= sizeof(tf->regs[5]);
            tf->regs[29] -= sizeof(tf->regs[6]);
            tf->cp0_epc = curenv->env_user_sigaction_entry;
	    } else {
            printk("!! lack of sigaction_entry !!\n");
        }
    }

    //printk("%d : ----end--\n", curenv->env_id);
    return;
}