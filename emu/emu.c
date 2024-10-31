#include "stdio.h"
#include "./emu.h" 

int main() {
	cpu_t CPU;
	CPU.r_t0 = 40;

	// dump_registers(&CPU);
	// dump_stack(&CPU, "head", 40);

	return 0;
}
