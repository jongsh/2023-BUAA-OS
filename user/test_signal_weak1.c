#include <lib.h>


void handler(int num) {
    debugf("Reach handler, now the signum is %d!\n", num);
    int ans;
    for (int i = 0; i < 10000000; i++) {
        ans += i;
    }
    debugf("Reach handler, the signum %d end....\n", num);
}

sigset_t set2;

int main(int argc, char **argv) {
    sigset_t set;
    sigemptyset(&set);
    struct sigaction sig;
    sig.sa_handler = handler;
    sig.sa_mask = set;
    //sigaddset(&set, 1);
    //sigaddset(&set, 2);
    //panic_on(sigprocmask(0, &set, NULL));
    //sigdelset(&set, 2);
    panic_on(sigaction(1, &sig, NULL));
    panic_on(sigaction(2, &sig, NULL));
    int ret = fork();
    if (ret != 0) {
        kill(ret, 1);
        syscall_yield();
        kill(ret, 2);
        debugf("Father: %d.\n", sigismember(&set, 2));
    } else {
        debugf("Child: %d.\n", sigismember(&set, 2));
    }
    return 0;
}