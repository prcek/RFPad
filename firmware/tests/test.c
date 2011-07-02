#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

#include"../prompt.h"
#include"../print.h"
#include"../scan.h"
#include"../rf.h"

int main(int nargs, char **args) {
	rf_config_init();
	print("TEST MODE\r\n");
	while(do_prompt()) {
	}
	return 0;
}
