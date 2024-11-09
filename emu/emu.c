#include "stdio.h"
#include <stdint.h>
#include "./emu.h" 

int apply_arithmatic(int operation, uint16_t **registers, instruction_t current_instruction) {
	switch (operation) {
		case I_ADD:
			*registers[current_instruction.rd] += *registers[current_instruction.src.rs];
			break;

		case I_SUB:
			*registers[current_instruction.rd] -= *registers[current_instruction.src.rs];
			break;

		case I_MULT:
			*registers[current_instruction.rd] *= *registers[current_instruction.src.rs];
			break;

		case I_DIV:
			*registers[current_instruction.rd] /= *registers[current_instruction.src.rs];
			break;

		case I_AND:
			*registers[current_instruction.rd] &= *registers[current_instruction.src.rs];
			break;

		case I_OR:
			*registers[current_instruction.rd] |= *registers[current_instruction.src.rs];
			break;

		case I_SLL:
			*registers[current_instruction.rd] <<= *registers[current_instruction.src.rs];
			break;

		case I_SLR:
			*registers[current_instruction.rd] >>= *registers[current_instruction.src.rs];
			break;

		default:
			printf("not arithemtic tho: %u\n", operation);
			return -1;
	}

	return 1;
}

int simple_arithmatic(instruction_t current_instruction, uint16_t *registers[], int operation) {
	if (current_instruction.rd >= 16) {
		printf("Invalid destination register\n");
		return -1;
	}

	if (current_instruction.src_type == OPERAND_REGISTER) {
		if (current_instruction.src.rs >= 16) {
			printf("Invalid source register\n");
			return -1;
		}

		return apply_arithmatic(operation, registers, current_instruction);

	} else if (current_instruction.src_type == OPERAND_IMMEDIATE) {
		return apply_arithmatic(operation, registers, current_instruction);

	} else {
		printf("Invalid source type\n");
		return -1;
	}

	return -1;
}

int execute(cpu_t *cpu) {
	// so we access cpu registers using the enum values
	uint16_t* registers[] = {
		&cpu->r_t0, &cpu->r_t1, &cpu->r_t2, &cpu->r_t3,
		&cpu->r_a0, &cpu->r_a1, &cpu->r_a2, &cpu->r_a3,
		&cpu->r_s0, &cpu->r_s1, &cpu->r_s2, &cpu->r_s3,
		&cpu->r_gp, &cpu->r_sp, &cpu->r_fp, &cpu->r_ra
	};

	// should be for the whole text segment or till we see NULL

	for(int i = 0x0000; i <= 0x3FFF; i+=16) {
		instruction_t current_instruction = cpu->memory[cpu->pc];
		if(current_instruction.op_code) {
			printf("we reached the end at: 0x%04x\n", i);
			return 0;
		}

		if(simple_arithmatic(current_instruction, registers, current_instruction.op_code) == 1) {
			cpu->pc+=16;
		}

		switch(current_instruction.op_code) {
			case I_NOT:
				// so we are only given 'not r0' for example, not source here
				*registers[current_instruction.rd] = !*registers[current_instruction.rd];
				cpu->pc+=16;
				break;

			case I_LW:
			case I_SW:
			case I_LA:

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
					*registers[current_instruction.rd] = current_instruction.src.imm;

				} else {
					printf("Invalid source type\n");
					return -1;
				}

				cpu->pc+=16;
				break;

			case I_JMP:
			case I_JMPZ:
			case I_JMPC:

			default:
				printf("insuffecient instruction code: %u\n", current_instruction.op_code);
				return -1;
		}
	}

	return 1;
}

int main() {
	cpu_t CPU;
	CPU.r_t0 = 12;

	// dump_stack(&CPU, "head", 40);


	CPU.pc = 0x000; // set PC to text start, which is where the start of the instructions is located
	
	// we currently store the parsed instruction so we skip the assembler step for now.
	CPU.memory[0x000] = (instruction_t){ .op_code = I_MOV, .src_type = OPERAND_IMMEDIATE, .src.imm = 13, .rd = R_T0 };
	CPU.memory[0x0010] = (instruction_t){ .op_code = I_NOT, .rd = R_T2 };

	int exit_code = execute(&CPU);
	printf("program exited with code: %d\n", exit_code);

	return 1;
}
