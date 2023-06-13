#include <lib.h>
#include <env.h>

// 清空信号集，将所有位都设置为 0
void sigemptyset(sigset_t *set) {
    set->sig[0] = 0;
    set->sig[1] = 0;
}

// 设置信号集，即将所有位都设置为 1
void sigfillset(sigset_t *set) {
    set->sig[0] = -1;
    set->sig[1] = -1;
}

// 向信号集中添加一个信号，即将指定信号的位设置为 1
void sigaddset(sigset_t *set, int signum) {
    if (signum > 64 || signum <= 0) {
        return;
    }
    int temp = signum - 1;
    set->sig[temp/32] |= (1 << (temp % 32)); 
}

// 从信号集中删除一个信号，即将指定信号的位设置为 0
void sigdelset(sigset_t *set, int signum) {
    if (signum > 64 || signum <= 0) {
        return;
    }
    int temp = signum - 1;
    set->sig[temp/32] &= ~(1 << (temp % 32)); 
}

// 检查一个信号是否在信号集中，如果在则返回 1，否则返回 0
int sigismember(const sigset_t *set, int signum) {
    if (signum > 64 || signum <= 0) {
        return 0;
    }
    int temp = signum - 1;
    return (set->sig[temp/32] & (1 << (temp % 32))) != 0; 
}

// 信号注册函数
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    if (signum > 64 || signum <= 0) {
        return -1;
    }
    if (oldact) {
		memcpy(oldact, oldact, sizeof(struct sigaction));
	}
    return syscall_sigaction(signum, act, oldact);
}

// 信号发送函数
int kill(u_int envid, int sig) {
    if (sig > 64 || sig <= 0) {
        return -1;
    }
    return syscall_sig_kill(envid, sig);
}

// 修改进程掩码
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    if (how != 0 && how != 1 && how != 2) {
        return -1;    // 不符合规定的掩码修改类型
    }
    if (oldset) {
		oldset->sig[1] = 0;
        oldset->sig[0] = 0;
	}
    return syscall_sigprocmask(how, set, oldset);
}

void __attribute__((noreturn)) sigaction_entry(struct Trapframe *tf, void (*sa_handler)(int), u_int signum) {
    
    if (sa_handler) {
        sa_handler(signum);
    } else {
        exit();
    }

    syscall_sigaction_back(tf);

    user_panic("Wrong in sigaction_entry\n");
}