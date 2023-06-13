#ifndef _ENV_H_
#define _ENV_H_

#include <mmu.h>
#include <queue.h>
#include <trap.h>
#include <types.h>

#define LOG2NENV 10
#define NENV (1 << LOG2NENV)
#define ENVX(envid) ((envid) & (NENV - 1))

// lab4 challenge
typedef struct sigset_t{
    int sig[2]; //最多 32*2=64 种信号
}sigset_t;

struct sigaction{
    void (*sa_handler)(int);  // 处理函数
    sigset_t sa_mask;         // 掩码
};

struct signode{
	u_int signum;
	struct signode *next;
};


// Values of env_status in struct Env
#define ENV_FREE 0
#define ENV_RUNNABLE 1
#define ENV_NOT_RUNNABLE 2

struct Env {
	struct Trapframe env_tf;  // Saved registers
	LIST_ENTRY(Env) env_link; // Free list
	u_int env_id;		  // Unique environment identifier
	u_int env_asid;		  // ASID
	u_int env_parent_id;	  // env_id of this env's parent
	u_int env_status;	  // Status of the environment
	Pde *env_pgdir;		  // Kernel virtual address of page dir
	TAILQ_ENTRY(Env) env_sched_link;
	u_int env_pri;
	// Lab 4 IPC
	u_int env_ipc_value;   // data value sent to us
	u_int env_ipc_from;    // envid of the sender
	u_int env_ipc_recving; // env is blocked receiving
	u_int env_ipc_dstva;   // va at which to map received page
	u_int env_ipc_perm;    // perm of page mapping received

	// Lab 4 fault handling
	u_int env_user_tlb_mod_entry; // user tlb mod handler

	// Lab 6 scheduler counts
	u_int env_runs; // number of times been env_run'ed

	// Lab4 challenge
	struct sigaction env_sigaction[64];   // 64 中信号的处理结构体
	u_int env_sigaction_avai[64];         // sigaction 有效标志, 1：有效 0：无效
	int env_waiting_siglist[128];         // 等待被处理的信号
	int env_waiting_top;
	int env_handling_siglist[128];        // 正在处理的信号
	int env_handling_top;
	sigset_t env_sa_mask;                 // 进程的掩码
	u_int env_user_sigaction_entry;       // sigaction caller
};

LIST_HEAD(Env_list, Env);
TAILQ_HEAD(Env_sched_list, Env);
extern struct Env *curenv;		     // the current env
extern struct Env_sched_list env_sched_list; // runnable env list

void env_init(void);
int env_alloc(struct Env **e, u_int parent_id);
void env_free(struct Env *);
struct Env *env_create(const void *binary, size_t size, int priority);
void env_destroy(struct Env *e);

int envid2env(u_int envid, struct Env **penv, int checkperm);
void env_run(struct Env *e) __attribute__((noreturn));
void enable_irq(void);

void env_check(void);
void envid2env_check(void);

// lab4 challenge
void remove_index(int a[], int index);

#define ENV_CREATE_PRIORITY(x, y)                                                                  \
	({                                                                                         \
		extern u_char binary_##x##_start[];                                                \
		extern u_int binary_##x##_size;                                                    \
		env_create(binary_##x##_start, (u_int)binary_##x##_size, y);                       \
	})

#define ENV_CREATE(x)                                                                              \
	({                                                                                         \
		extern u_char binary_##x##_start[];                                                \
		extern u_int binary_##x##_size;                                                    \
		env_create(binary_##x##_start, (u_int)binary_##x##_size, 1);                       \
	})

#endif // !_ENV_H_
