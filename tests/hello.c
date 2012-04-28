#include <stdio.h>

void func2();

void func1() {
	int i = 0;
	int j;
	
	while (i < 20) {
		i+=1;
	}
	
}

void func2() {
	int j = 12;
	j = ++j;
	printf("hello world\n");
}

void check(int i) {

	if (i < 10) printf("dslkjf");
}

int main() {

int a[100];
//int *b = a;
//int integer = b[1];
	
	int j = 0;
	int i = 0;
	while (i < 20) {
	if (i < 10) {
		i+=1;
		i = i + j;
		check(i);
 		//printf("hello world\n");
		/*
		while (j != 100) {
			j++;	
		}
		*/
	}
	else {
		i -=1;
		i = i-j;
	}
	i = i+1;
	}

 // printf("hello world\n");
	//printf("Whoops, something went wrong");
//	func1();
//	func2();
  return 0;
}


