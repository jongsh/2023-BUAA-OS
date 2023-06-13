#include <lib.h>

int global = 0; 

void handler1(int num){
    kill(0,2);
    debugf("handler1 arrive!\n");
    if (global != 2) {
    	user_panic("Wrong in handler2");
	}
	global++;
}
void handler2(int num){
    kill(0,3);
    debugf("handler2 arrive!\n");
    if (global != 0) {
    	user_panic("Wrong in handler2");
	}
	global++;
}
void handler3(int num){
    debugf("handler3 arrive!\n");
    if (global != 1) {
    	user_panic("Wrong in handler3");
	}
	global++;
}
int main(){
    struct sigset_t a;
    a.sig[1]=0;
    a.sig[0]=0;
    struct sigaction act;
    act.sa_mask=a;
    act.sa_handler=handler1;
    sigaction(1,&act,NULL);
    act.sa_handler=handler2;
    a.sig[0]|=(1<<2);  //block signal 3
    act.sa_mask=a;
    sigaction(2,&act,NULL);
    act.sa_mask.sig[0]=0;
    act.sa_handler=handler3;
    sigaction(3,&act,NULL);
    kill(0,1);
   
    int ans = 0;
    for (int i = 0; i < 10000000; ++i) {
   		ans += i;
    }
    if (global == 3) {
    	debugf("!! PASS !!\n");
	}
    return 0;
}