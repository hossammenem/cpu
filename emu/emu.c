#include "stdio.h"
#include <stdint.h>
#include "./emu.h" 

typedef enum {
    OPERAND_REGISTER,
    OPERAND_IMMEDIATE
} operand_type_t;

typedef struct {
    uint8_t op_code;
    uint8_t rd;           // Destination register
    operand_type_t src_type;  // Type of source (register or immediate)
    union {
        uint8_t rs;       // Source register if src_type is OPERAND_REGISTER
        uint16_t imm;     // Immediate value if src_type is OPERAND_IMMEDIATE
    } src;
} instruction_t;

int execute(cpu_t *cpu, instruction_t (*program)[], size_t program_size) {
	uint16_t* registers[] = {
		&cpu->r_t0, &cpu->r_t1, &cpu->r_t2, &cpu->r_t3,
		&cpu->r_a0, &cpu->r_a1, &cpu->r_a2, &cpu->r_a3,
		&cpu->r_s0, &cpu->r_s1, &cpu->r_s2, &cpu->r_s3,
		&cpu->r_gp, &cpu->r_sp, &cpu->r_fp, &cpu->r_ra
	};

	for(int i = 0; i < program_size; i++) {
		instruction_t current_instruction = (*program)[cpu->pc];
		switch(current_instruction.op_code) {
			case I_MOV:
				if (current_instruction.rd >= 16) {
					printf("Invalid destination register\n");
					return -1;
				}

				if (current_instruction.src_type == OPERAND_REGISTER) {
					if (current_instruction.src.rs >= 16) {
						printf("Invalid source register\n");
						return -1;
					}

					*registers[current_instruction.rd] = *registers[current_instruction.src.rs];

				} else if (current_instruction.src_type == OPERAND_IMMEDIATE) {
					puts("we here");
					*registers[current_instruction.rd] = current_instruction.src.imm;

				} else {
					printf("Invalid source type\n");
					return -1;
				}

				cpu->pc++;
				break;

			default:
				printf("insuffecient instruction code: %u\n", (*program)[cpu->pc].op_code);
				return -1;
		}
	}

	return 1;
}

int main() {
	cpu_t CPU;
	CPU.r_t0 = 12;

	// dump_stack(&CPU, "head", 40);

	// so the program should be ran like this
	// ./emu program.asm
	// then we use the assembler and parse it,
	// and this program below is what's the output of the parse would be
	instruction_t program[] = {
		{ .op_code = I_MOV,.src_type = OPERAND_IMMEDIATE, .src.imm = 13, .rd = R_T0 },
	};

	size_t program_size = sizeof(program) / sizeof(program[0]);

	puts("before: ");
	dump_registers(&CPU);

	CPU.pc = 0;
	int exit_code = execute(&CPU, &program, program_size);
	printf("program exited with code: %d\n", exit_code);

	puts("after: ");
	dump_registers(&CPU);

	return 1;
}
