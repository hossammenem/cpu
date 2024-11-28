#ifndef EMU_H
#define EMU_H 

#define CPU_FREQUENCY 1000000  // 1 MHz
#define CYCLES_PER_INSTRUCTION 4  // Simple approximation
#define TEXT_SECTION_START 0x000


enum SYS_CALL {
	SYS_PRINT,
	SYS_EXIT,
	SYS_WRITE,
	SYS_READ,
	SYS_CLOSE,
	SYS_OPEN
};

enum OP_CODE {
	I_ADD,
	I_SUB,
	I_MULT,
	I_DIV,
	I_AND,
	I_OR,
	I_NOT,
	I_SLL,
	I_SLR,
	I_CMP,
	I_LA,
	I_PUSH,
	I_POP,
	I_MOV,
	I_JMP,
	I_JMPC,
	I_INT
};

enum REG {
	R_T0,
	R_T1,
	R_T2,
	R_T3,
	R_A0,
	/**
 * @brief used in syscalls as arg 0
 */
	R_A1,

	/**
 * @brief used in syscalls as arg 1
 */
	R_A2,
	/**
 * @brief used in syscalls as arg 2
 */
	R_A3,
	R_S0,

	/**
 * @brief used in syscalls as sys call number
 */
	R_S1,
	R_S2,
	R_S3,
	R_GP,
	R_SP,
	R_FP,
	R_RA
};

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

typedef struct {
	// Temporary registers (caller-saved)
	uint16_t r_t0;    // r0
	uint16_t r_t1;    // r1
	uint16_t r_t2;    // r2  
	uint16_t r_t3;    // r3

	// Argument/Return registers

	uint16_t r_a0;    // r4 - First argument/return value
	
	/**
 * @brief used in syscalls as arg 0
 */
	uint16_t r_a1;    // r5 - Second argument
	
	/**
 * @brief used in syscalls as arg 1
 */
	uint16_t r_a2;    // r6 - Third argument
	
	/**
 * @brief used in syscalls as arg 2
 */
	uint16_t r_a3;    // r7 - Fourth argument

	// Saved registers (callee-saved)
	uint16_t r_s0;    // r8

	/**
 * @brief used in syscalls syscall number
 */
	uint16_t r_s1;    // r9
	uint16_t r_s2;    // r10
	uint16_t r_s3;    // r11

	// Special registers
	uint16_t r_gp;    // r12 - Global Pointer
	uint16_t r_sp;    // r13 - Stack Pointer
	uint16_t r_fp;    // r14 - Frame Pointer
	uint16_t r_ra;    // r15 - Return Address

	//
	uint16_t pc;      
	instruction_t memory[65536];  // 64K memory
} cpu_t;

typedef struct {
	uint64_t clock_cycles;
	uint32_t frequency;  // in Hz

	// Add file descriptors for IO
	FILE* file_descriptors[16];
	int num_open_files;
	cpu_t cpu;
} emulator_t;

// utils
void dump_registers(cpu_t *cpu);
void dump_stack(cpu_t *cpu, char* start, int range); // we still have ranges issues

void init_emulator(emulator_t *emulator);
uint64_t get_current_time_us(void);
int emulate(emulator_t *eumlator);
int fetch_decode_execute(emulator_t *eumlator);

#endif
