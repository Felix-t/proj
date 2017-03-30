#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IDN_EXPECTED_ANSWER "Opsens Sol., WLX-2-P2-N-62LCA-V2, SN:32R0078"
#define NB_CH 1

void func(char *str)
{
	char *a = malloc(100);	
	a = "bjikl";
	memcpy(str, a, 50);
}
int main(void)
{
	char str[100];
	func(str);
	printf("%s\n", str);
}
