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

#define IVT_ENTRIES 256        // 256 interrupt vectors
#define IVT_SIZE    (IVT_ENTRIES * 2)  // 512 bytes
#define ISR_START   (IVT_START + IVT_SIZE)  // 0xFE00


#define SYS_WRITE    0xB002  // I/O port for writing to console

#define MEMORY_SIZE 0xFFFF  // 64KB
#define DEFAULT_FREQ 1000000  // 1 MHz

typedef enum {
    FLAG_ZERO      = 1 << 0,
    FLAG_CARRY     = 1 << 1,
    FLAG_OVERFLOW  = 1 << 2,
    FLAG_NEGATIVE  = 1 << 3
} CPU_FLAGS;

typedef enum {
	ERR_NONE,
	ERR_INVALID_OPCODE,
	ERR_MEM_OUT_OF_BOUNDS,
	ERR_STACK_OVERFLOW,
	ERR_STACK_UNDERFLOW,
	ERR_INVALID_INSTR,

	ERRS_COUNT
} CPU_ERR;

typedef enum {
    SYS_EXIT,
    SYS_PRINT,
    SYS_READ,
    // SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,

    SYS_CALL_COUNT
} SYSCALL;

typedef enum {
    COND_EQ,   // Equal (Z=1)
    COND_NE,   // Not Equal (Z=0)
    COND_LT,   // Less Than (N≠V)
    COND_GT,   // Greater Than
	
    COND_COUNT
} condcode_t;

// Register indices with meaningful names
typedef enum {
    // Temporary registers
    R_0, R_1, R_2, R_3,    

    // Special registers
    R_GP, R_SP, R_FP, R_RA,
    
    REG_COUNT
} REGISTERS;

typedef enum {
	OP_ADD, OP_SUB,
	OP_AND, OP_NOT, 

	OP_LW, OP_SW, OP_MW,
	OP_PUSH, OP_POP,

	OP_CMP, OP_JMP, OP_JMPC,
	OP_IN, OP_OUT,
	OP_INT, OP_RETI,

	OP_COUNT
} opcode_t;

typedef struct {
	uint16_t reg[REG_COUNT];  // All registers including special ones
	uint16_t pc;              // Program Counter
	uint8_t memory[MEMORY_SIZE];
    CPU_FLAGS status;
	CPU_ERR last_error;
} cpu_t;

typedef struct emulator_t {
    cpu_t cpu;
    uint8_t running;
    uint64_t clock_cycles;
    uint32_t frequency;
    struct timespec last_tick;
	uint8_t cycles_per_instr[OP_COUNT]; 

    struct {
        uint8_t status;
        uint8_t data;
        uint8_t buffer[16];
        int buf_head;
        int buf_tail;
    } keyboard;

    uint8_t irq_line;  

	struct {
		uint8_t (*read)(struct emulator_t *emu, uint16_t addr);
		void (*write)(struct emulator_t *emu, uint16_t addr, uint8_t value);
	} io_handlers;
} emulator_t;

// Helper macros for clearer register access
#define REG_R0(cpu) ((cpu)->reg[R_R0])
#define REG_R1(cpu) ((cpu)->reg[R_R1])
#define REG_R2(cpu) ((cpu)->reg[R_R2])
#define REG_R3(cpu) ((cpu)->reg[R_R3])

#define REG_GP(cpu) ((cpu)->reg[R_GP])
#define REG_SP(cpu) ((cpu)->reg[R_SP])
#define REG_FP(cpu) ((cpu)->reg[R_FP])
#define REG_RA(cpu) ((cpu)->reg[R_RA])
#define REG_PC(cpu) ((cpu)->pc)

/*
 * Instruction Decoding Macros:
 *
 * There are two groups:
 *
 * 1. For 16-bit instructions (e.g. A-type, push/pop, system instructions).
 *    These macros operate on a 16-bit instruction word.
 */
#define OP16(i)       (((i) >> 12) & 0xF)
#define INSTR_T16(i)  (((i) >> 11) & 0x1)
#define RD16(i)       (((i) >> 8)  & 0x7)
#define RS16(i)       (((i) >> 5)  & 0x7)
#define IMM16(i)      ((i) & 0xFF)

/*
 * 2. For 24-bit address-handling instructions (I-type for I/O/memory and J-type for control).
 *    These instructions are 3 bytes (24 bits) long. We assume that the 24-bit instruction is
 *    stored in a 32-bit unsigned integer where only the lower 24 bits are used.
 *
 * For I-type (address instructions such as `in`, `out`, `lw`, `sw`, `mw`):
 * Format: [ op:4 | type:1 | rd:3 ] in the upper 8 bits, and a 16-bit address field.
 */
#define OP8(i)        (((i) >> 20) & 0xF)    // Bits 23-20: opcode (4 bits)
#define TYPE8(i)      (((i) >> 19) & 0x1)    // Bit 19: type flag (should be 1 for address instructions)
#define RD8(i)        (((i) >> 16) & 0x7)    // Bits 18-16: register destination/source (if applicable)
#define ADDR_FIELD(i) ((i) & 0xFFFF)         // Lower 16 bits: address

/*
 * For J-type (control instructions such as `jmp`, `jmpc`):
 * Format: [ op:4 | cond:4 ] in the upper 8 bits, and a 16-bit address field.
 */
#define J_OP(i)       (((i) >> 20) & 0xF)    // Bits 23-20: opcode (4 bits)
#define COND8(i)      (((i) >> 16) & 0xF)    // Bits 19-16: condition code (4 bits)
#define J_ADDR(i)     ((i) & 0xFFFF)         // Lower 16 bits: jump target address


void cpu_init(cpu_t* cpu);
void emulator_init(emulator_t* emu, uint32_t frequency);
void emulator_step(emulator_t* emu);
void emulator_run(emulator_t* emu);
void cpu_dump(cpu_t* cpu);

#endif
