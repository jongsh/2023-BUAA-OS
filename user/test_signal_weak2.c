#include <lib.h> 

struct sigset_t set2;
int global = 0;
void handler(int num){
    debugf("Reach handler, now the signum is %d!\n", num);
    global++;
    if(global == 10){
        kill(0,9);
    }
}

int main(int argc, char **argv) {
    debugf("============= test start ===========\n");
    struct sigset_t set;
    sigemptyset(&set);
    struct sigaction sig;
    sig.sa_mask = set;
    sig.sa_handler = handler;
    sigaction(3, &sig, NULL);

    int ret = fork();
    if (ret != 0) {
        debugf("this is father\n");
        while(kill(ret, 3) == 0);
    } else {
        debugf("this is child\n");
        while(1);
    }
    
    debugf("TEST PASS!\n");
    debugf("============= test end =============\n");
    return 0;
}   