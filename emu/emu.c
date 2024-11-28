#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "./emu.h" 

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

void init_cpu(emulator_t *emulator) {
    memset(&emulator->cpu, 0, sizeof(cpu_t));
    emulator->frequency = CPU_FREQUENCY;
    emulator->cpu.pc = TEXT_SECTION_START;
    
    // Initialize file descriptors
    emulator->file_descriptors[0] = stdin;   // Standard input
    emulator->file_descriptors[1] = stdout;  // Standard output
    emulator->file_descriptors[2] = stderr;  // Standard error
    emulator->num_open_files = 3;
}

uint64_t get_current_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

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
			return -1;
	}

	return 1;
}

int simple_arithmatic(instruction_t current_instruction, uint16_t *registers[], int operation) {
	if (current_instruction.rd > 16) {
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




int fetch_decode_execute(emulator_t *emulator) {
	cpu_t *cpu = &emulator->cpu; // this should be chagned, adding it only because it was already here before

	// so we access cpu registers using the enum values
	uint16_t* registers[] = {
		&cpu->r_t0, &cpu->r_t1, &cpu->r_t2, &cpu->r_t3,
		&cpu->r_a0, &cpu->r_a1, &cpu->r_a2, &cpu->r_a3,
		&cpu->r_s0, &cpu->r_s1, &cpu->r_s2, &cpu->r_s3,
		&cpu->r_gp, &cpu->r_sp, &cpu->r_fp, &cpu->r_ra
	};

	// should be for the whole text segment or till we see NULL

	for(int i = 0x000; i <= 0x3FFF; i+=16) {
		instruction_t current_instruction = cpu->memory[cpu->pc];
		if(!current_instruction.op_code) {
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

			case I_CMP:
			case I_LA:

			case I_INT:
				// the imm passed to the 'int' instruction should specifcy which register should we be reading
				// or a sequence of registers or smth idk
				// but we'll stick with 0x80 and one register for now
				if (current_instruction.src_type == OPERAND_IMMEDIATE && 
					current_instruction.src.imm == 0x80) {

					// this is half-assed but that's ok
					uint16_t syscall_num = cpu->r_s1;
					uint16_t arg0 = cpu->r_a1;
					void *arg1 = &cpu->r_a2;
					uint16_t arg2 = cpu->r_a3;


					// for when doing file operations:
					// arg0 = file descriptor
					// arg1 = buffer address
					// arg2 = count
					// TODO: there's some typecasting shit here that i haven't checked. CHECK IT.
					switch(syscall_num) {
						case SYS_EXIT:
							return arg0; // Exit with provided code

						case SYS_PRINT:
							printf("%u\n", arg0);
						break;

						case SYS_READ: 
							if (arg0 < emulator->num_open_files && emulator->file_descriptors[arg0]) {
								char buffer[1024];  // Temporary buffer
								size_t count = arg2 > 1024 ? 1024 : arg2;
								size_t read = fread(buffer, 1, count, emulator->file_descriptors[arg0]);

								// Copy to emulated memory
								for(size_t i = 0; i < read; i++) {
									cpu->memory[(*(uint16_t *)arg1) + i].op_code = buffer[i];
								}

								cpu->r_a0 = read;  // Return number of bytes read
							}
						break;

						case SYS_WRITE: 
							// Similar to read but for writing
							if (arg0 < emulator->num_open_files && emulator->file_descriptors[arg0]) {
								char buffer[1024];
								size_t count = arg2 > 1024 ? 1024 : arg2;

								// Copy from emulated memory
								for(size_t i = 0; i < count; i++) {
									buffer[i] = cpu->memory[(*(uint16_t *)arg1) + i].op_code;
								}

								size_t written = fwrite(buffer, 1, count, emulator->file_descriptors[arg0]);
								cpu->r_a0 = written;
							}
						break;

						case SYS_OPEN:
							if (arg0 < emulator->num_open_files && !emulator->file_descriptors[arg0]) {
								FILE *f = fopen((const char *)arg1, "r");

								if (f) {
									emulator->file_descriptors[emulator->num_open_files] = f;
									cpu->r_a0 = emulator->num_open_files++;  // Return file descriptor
							} else {
									cpu->r_a0 = -1;  // Return error
								}
							}

						break;

						case SYS_CLOSE: 
							if (arg0 < emulator->num_open_files && emulator->file_descriptors[arg0]) {
								fclose(emulator->file_descriptors[arg0]);
								emulator->file_descriptors[arg0] = NULL;
								cpu->r_a0 = 0;  // Success
							} else {
								cpu->r_a0 = -1;  // Error
							}
						break;

						default:
							printf("invalid syscall number %u\n", syscall_num);
							return -1;
					}
				}
				cpu->pc += 16;
				break;


			case I_MOV:
				if (current_instruction.rd > 16) {
					printf("Invalid destination register\n");
					return -1;
				}

				if (current_instruction.src_type == OPERAND_REGISTER) {
					if (current_instruction.src.rs > 16) {
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
			case I_JMPC:

			default:
				printf("insuffecient instruction code: %u\n", current_instruction.op_code);
				return -1;
		}
	}

	return 1;
}

int emulate(emulator_t *emulator) {
	uint64_t start_time = get_current_time_us();
	uint64_t expected_cycles = 0;

	for(int i = 0x000; i <= 0x3FFF; i+=16) {
		// Calculate expected time based on cycles
		expected_cycles += CYCLES_PER_INSTRUCTION;
		uint64_t expected_time = start_time + 
			(expected_cycles * 1000000 / emulator->frequency);

		// Wait if we're running too fast
		uint64_t current_time = get_current_time_us();
		if (current_time < expected_time) {
			usleep(expected_time - current_time);
		}

		// fetch and decode code goes here
		fetch_decode_execute(emulator);

		emulator->clock_cycles += CYCLES_PER_INSTRUCTION;
	}
	return 1;
}

void test_program_print(int start, cpu_t *cpu) {
	cpu->memory[start] = (instruction_t){ .op_code = I_MOV, .src_type = OPERAND_IMMEDIATE, .src.imm = 1, .rd = R_S1 };
	cpu->memory[start+0x10] = (instruction_t){ .op_code = I_MOV, .src_type = OPERAND_IMMEDIATE, .src.imm = 69, .rd = R_A1 };
	cpu->memory[start+0x20] = (instruction_t){ .op_code = I_INT, .src_type = OPERAND_IMMEDIATE, .src.imm = 0x80 };
}


int main() {
	emulator_t emulator;
	cpu_t cpu = emulator.cpu;

	cpu.pc = TEXT_SECTION_START; // set PC to text start, which is where the start of the instructions is located ( aka .text section )

	// we currently store the parsed instruction so we skip the assembler step for now.
	cpu.memory[TEXT_SECTION_START] = (instruction_t){ .op_code = I_MOV, .src_type = OPERAND_IMMEDIATE, .src.imm = 13, .rd = R_T0 };
	cpu.memory[TEXT_SECTION_START+0x0010] = (instruction_t){ .op_code = I_NOT, .rd = R_T2 };

	test_program_print(TEXT_SECTION_START+0x20, &cpu);

	int exit_code = emulate(&emulator);
	printf("program exited with code: %d\n", exit_code);

	return 1;
}

