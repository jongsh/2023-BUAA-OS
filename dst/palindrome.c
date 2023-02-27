#include <stdio.h>
int check(int);
int main() {
	int n;
	scanf("%d", &n);

	if (check(n) == n) {
		printf("Y\n");
	} else {
		printf("N\n");
	}
	return 0;
}

int check(int a) {
	int b = 0;
	while (a != 0) {
		b = b * 10 + a % 10;
		a /= 10;
	}
	return b;
}

