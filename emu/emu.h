#ifndef EMU_H
#define EMU_H 

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

typedef struct {
	// Temporary registers (caller-saved)
	uint16_t r_t0;    // r0
	uint16_t r_t1;    // r1
	uint16_t r_t2;    // r2  
	uint16_t r_t3;    // r3

	// Argument/Return registers
	uint16_t r_a0;    // r4 - First argument/return value
	uint16_t r_a1;    // r5 - Second argument
	uint16_t r_a2;    // r6 - Third argument
	uint16_t r_a3;    // r7 - Fourth argument

	// Saved registers (callee-saved)
	uint16_t r_s0;    // r8
	uint16_t r_s1;    // r9
	uint16_t r_s2;    // r10
	uint16_t r_s3;    // r11

	// Special registers
	uint16_t r_gp;    // r12 - Global Pointer
	uint16_t r_sp;    // r13 - Stack Pointer
	uint16_t r_fp;    // r14 - Frame Pointer
	uint16_t r_ra;    // r15 - Return Address

	uint16_t pc;      
	uint16_t memory[65536];  // 64K memory
} cpu_t;

// print all registers
void dump_registers(cpu_t *cpu) {
	printf("=== CPU Registers ===\n");

	// Temporary registers
	printf("\nTemporary Registers:\n");
	printf("t0: 0x%04x (%5u)    t1: 0x%04x (%5u)\n", 
				cpu->r_t0, cpu->r_t0, cpu->r_t1, cpu->r_t1);
	printf("t2: 0x%04x (%5u)    t3: 0x%04x (%5u)\n", 
				cpu->r_t2, cpu->r_t2, cpu->r_t3, cpu->r_t3);

	// Argument registers
	printf("\nArgument Registers:\n");
	printf("a0: 0x%04x (%5u)    a1: 0x%04x (%5u)\n", 
				cpu->r_a0, cpu->r_a0, cpu->r_a1, cpu->r_a1);
	printf("a2: 0x%04x (%5u)    a3: 0x%04x (%5u)\n", 
				cpu->r_a2, cpu->r_a2, cpu->r_a3, cpu->r_a3);

	// Saved registers
	printf("\nSaved Registers:\n");
	printf("s0: 0x%04x (%5u)    s1: 0x%04x (%5u)\n", 
				cpu->r_s0, cpu->r_s0, cpu->r_s1, cpu->r_s1);
	printf("s2: 0x%04x (%5u)    s3: 0x%04x (%5u)\n", 
				cpu->r_s2, cpu->r_s2, cpu->r_s3, cpu->r_s3);

	// Special registers
	printf("\nSpecial Registers:\n");
	printf("pc: 0x%04x (%5u)    sp: 0x%04x (%5u)\n", 
				cpu->pc, cpu->pc, cpu->r_sp, cpu->r_sp);
	printf("fp: 0x%04x (%5u)    ra: 0x%04x (%5u)\n", 
				cpu->r_fp, cpu->r_fp, cpu->r_ra, cpu->r_ra);
	printf("gp: 0x%04x (%5u)\n", 
				cpu->r_gp, cpu->r_gp);

	printf("\n===================\n");
}

// we still have ranges issues
void dump_stack(cpu_t *cpu, char* start, int range) {
	FILE *pager;

	if(!range) {
		// -R allows color codes if you add them later
		pager	= popen("less -R", "w");  

		if (!pager) return;
		range = 32 << 10;
	}

	if(start == NULL) {
		start = "head";
	}

	bool is_head = strcmp(start, "head") == 0;

	int step_multiplier = is_head ? -1 : 1;
	int step = (step_multiplier * 16);

	int i_start = is_head ? 64 << 10 : 32 << 10;
	int i_end = is_head ? (64 << 10) - range : (32 << 10) + range;

	for(int i = i_start; i >= i_end; i+=step) {
		if(pager) {
			fprintf(pager, "0x%04x: 0x%04x\n", i, cpu->memory[i]);
		} else {
			printf("0x%04x: 0x%04x\n", i, cpu->memory[i]);
		}
	}

	if(pager) {
		pclose(pager);
	}
}

void load_word();
void store_word();
void jmp();

#endif
