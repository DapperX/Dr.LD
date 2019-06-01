#include "lib.h"

int func_internal()
{
	const char str[] = "Called\n";
	const int fd = 1; // stdout
	int ret;
	asm volatile(
		"syscall"
	:
		"=eax"(ret)
	:
		"rax"(1), "rdi"(fd), "rsi"(str), "rdx"(sizeof(str))
	);
	return ret;
}

int func_external()
{
	return func_internal();
}
