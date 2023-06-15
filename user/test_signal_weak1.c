#include<lib.h>

sigset_t set;
int global;

void handler(int num) {
    debugf("%x Reach handler, now the signum is %d!\n",syscall_getenvid(), num);
    global++;
}

int compare_set(sigset_t t1, sigset_t t2) {
    return (t1.sig[0] == t2.sig[0]) 
            && (t1.sig[1] == t2.sig[1]);
}

int compare_sig(struct sigaction sig1, struct sigaction sig2) {
    return (sig1.sa_handler == sig2.sa_handler) &&
           compare_set(sig1.sa_mask, sig2.sa_mask); 
}

void test1() {
    sigset_t temp;
    sigemptyset(&temp);
    if(compare_set(set, temp)) {
        debugf("sigemptyset is correct!\n");
    } else {
        user_panic("sigemptyset is wrong!\n");
    }

    set.sig[0] = 0xFFFFFFFF;
    set.sig[1] = 0xFFFFFFFF;
    sigfillset(&temp);
    if(compare_set(set, temp)) {
        debugf("sigfillset is correct!\n");
    } else {
        user_panic("sigfillyset is wrong!\n");
    }

    set.sig[0] = 9;
    set.sig[1] = 4;
    sigemptyset(&temp);
    sigaddset(&temp, 1);
    sigaddset(&temp, 4);
    sigaddset(&temp, 4);
    sigaddset(&temp, 35);
    if(compare_set(set, temp)) {
        debugf("sigaddset is correct!\n");
    } else {
        user_panic("sigaddset is wrong!\n");
    }

    set.sig[0] = 8;
    set.sig[1] = 0;
    sigdelset(&temp, 1);
    sigdelset(&temp, 35);
    sigdelset(&temp, 8);
    if(compare_set(set, temp)) {
        debugf("sigdeletset is correct!\n");
    } else {
        user_panic("sigdeletset is wrong!\n");
    }

    for (int i = 1; i <= 64; ++i) {
        if (i != 4 && sigismember(&set, i) == 1) {
            user_panic("sigismember is wrong\n");
        }
        if (i == 4 && sigismember(&set, i) == 0) {
            user_panic("sigismember is wrong\n");
        }
    }
    debugf("sigismember is correct\n");
    sigemptyset(&set);
}

void test2() {
    int r;
    struct sigaction sig;
    struct sigaction temp;
    sigaddset(&set, 33);
    sigaddset(&set, 99);
    sig.sa_handler = handler;
    sig.sa_mask = set;
    if ((r = sigaction(1, &sig, NULL)) != 0) {
        user_panic("sigaction is wrong\n");
    }
    if ((r = sigaction(1, &sig, &temp)) != 0) {
        user_panic("sigaction is wrong\n");
    }
    if (!compare_sig(sig, temp)) {
        user_panic("sigaction is wrong\n");
    }
///////////////////////////////////////////////////////
    sigaddset(&set, 64);
    if ((r = sigaction(64, &sig, NULL)) != 0) {
        user_panic("sigaction is wrong\n");
    }
    if ((r = sigaction(64, &sig, &temp)) != 0) {
        user_panic("sigaction is wrong\n");
    }
    if (!compare_sig(sig, temp)) {
        user_panic("sigaction is wrong\n");
    }
///////////////////////////////////////////////////////
    if ((r = sigaction(70, &sig, NULL)) == 0) {
        user_panic("sigaction is wrong\n");
    }
    debugf("sigaction is correct!\n");
    sigemptyset(&set);
}

void test3() {
    // 测试简单的信号传送，不涉及其他进程
    global = 0;
    sigaddset(&set, 1);
    struct sigaction sig;
    sig.sa_handler = handler;
    sig.sa_mask = set;
    sigaction(1, &sig, NULL);
    sigaction(2, &sig, NULL);
    sigprocmask(2, &set, NULL);
    kill(0, 1);
    int ans = 0;
    for (int i = 0; i < 10000000; i++) {
        ans += i;
    }
    if (global != 0) {
        user_panic("signal handle is wrong!\n");
    }
    kill(0, 2);
    for (int i = 0; i < 10000000; i++) {
        ans += i;
    }
    if (global != 1) {
        user_panic("signal handle is wrong!\n");
    }
    sigprocmask(1, &set, NULL);
    for (int i = 0; i < 10000000; i++) {
        ans += i;
    }
    if (global != 2) {
        user_panic("signal handle is wrong!\n");
    }
    debugf("signal handle is correct!\n");
}

int main() {
    debugf("============= test start ===========\n");
    test1();
    test2();
    test3();
    debugf("TEST PASS!\n");
    debugf("============= test end =============\n");
    return 0;
}