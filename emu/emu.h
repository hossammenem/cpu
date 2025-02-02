#ifndef EMU_H
#define EMU_H

#include <stdint.h>
#include <time.h>

#define TEXT_START   0x0000
#define TEXT_END     0x5FFF    // 24KB
#define DATA_START   0x6000
#define DATA_END     0x9FFF    // 16KB
#define VRAM_START   0xA000
#define VRAM_END     0xAFFF    // 4KB
#define IO_START     0xB000
#define IO_END       0xBFFF    // 4KB
#define HEAP_START   0xC000
#define HEAP_END     0xEFFF    // 12KB
#define STACK_START  0xF000
#define STACK_END    0xFBFF    // 3KB
#define IVT_START    0xFC00
#define IVT_END      0xFFFF    // 1KB

#define MEMORY_SIZE 0xFFFF  // 64KB
#define PC_START 0x0000 // should be start of text section, as programs are stored there
#define DEFAULT_FREQ 1000000  // 1 MHz

typedef enum {
    FLAG_ZERO      = 1 << 0,
    FLAG_CARRY     = 1 << 1,
    FLAG_OVERFLOW  = 1 << 2,
    FLAG_NEGATIVE  = 1 << 3
} CPUFLAGS;

typedef enum {
    ERR_NONE,
    ERR_INVALID_OPCODE,
    ERR_MEM_OUT_OF_BOUNDS,
    ERR_STACK_OVERFLOW
} CPUERR;

typedef enum {
    SYS_EXIT,
    SYS_PRINT,
    SYS_READ,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_CALL_COUNT
} SYSCALL;

// Register indices with meaningful names
typedef enum {
    // Temporary registers
    R_T0, R_T1, R_T2, R_T3,    
    
    // Argument/return registers
    R_A0, R_A1, R_A2, R_A3,  
    
    // Saved registers
    R_S0, R_S1, R_S2, R_S3,

    // Special registers
    R_GP, R_SP, R_FP, R_RA,
    
    REG_COUNT
} REGISTERS;

typedef struct {
	uint16_t reg[REG_COUNT];  // All registers including special ones
	uint16_t pc;              // Program Counter
	uint16_t memory[MEMORY_SIZE];
    CPUFLAGS status;
	CPUERR last_error;
} cpu_t;

typedef enum {
    OP_ADD, OP_SUB, OP_MULT, OP_DIV,
    OP_AND, OP_OR, OP_NOT, OP_SLL, OP_SLR,
    OP_LA, OP_MOV, OP_PUSH, OP_POP,
    OP_JMP, OP_JMPC, OP_INT,
    OP_COUNT
} opcode_t;


typedef struct {
    cpu_t cpu;
    uint8_t running;
    uint64_t clock_cycles;
    uint32_t frequency;
    struct timespec last_tick;
	uint8_t cycles_per_instr[OP_COUNT]; 
	struct {
		uint8_t (*read)(uint16_t addr);
		void (*write)(uint16_t addr, uint16_t value);
	} io_handlers;
} emulator_t;

// Helper macros for clearer register access
#define REG_T0(cpu) ((cpu)->reg[R_T0])
#define REG_T1(cpu) ((cpu)->reg[R_T1])
#define REG_T2(cpu) ((cpu)->reg[R_T2])
#define REG_T3(cpu) ((cpu)->reg[R_T3])

#define REG_A0(cpu) ((cpu)->reg[R_A0])
#define REG_A1(cpu) ((cpu)->reg[R_A1])
#define REG_A2(cpu) ((cpu)->reg[R_A2])
#define REG_A3(cpu) ((cpu)->reg[R_A3])

#define REG_S0(cpu) ((cpu)->reg[R_S0])
#define REG_S1(cpu) ((cpu)->reg[R_S1])
#define REG_S2(cpu) ((cpu)->reg[R_S2])
#define REG_S3(cpu) ((cpu)->reg[R_S3])

#define REG_GP(cpu) ((cpu)->reg[R_GP])
#define REG_SP(cpu) ((cpu)->reg[R_SP])
#define REG_FP(cpu) ((cpu)->reg[R_FP])
#define REG_RA(cpu) ((cpu)->reg[R_RA])
#define REG_PC(cpu) ((cpu)->pc)

// Helper macros for instruction decoding
#define OP(i) ((i >> 12) & 0xF)      // 4-bit opcode
#define RD(i) ((i >> 8) & 0xF)       // 4-bit destination
#define RS(i) ((i >> 4) & 0xF)       // 4-bit source
#define RT(i) (i & 0xF)              // 4-bit target
#define IMM(i) (i & 0xFF)            // 8-bit immediate
#define ADDR(i) (i & 0xFFFF)         // 16-bit address


void cpu_init(cpu_t* cpu);
void emulator_init(emulator_t* emu, uint32_t frequency);
void emulator_step(emulator_t* emu);
void emulator_run(emulator_t* emu);
void cpu_dump(cpu_t* cpu);

#endif
