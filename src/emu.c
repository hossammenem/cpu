#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include "emu.h"

static struct termios original_term;

void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &original_term);
    
    raw = original_term;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_term);
}

// ==================
// Core Execution
// ==================
static void update_flags(cpu_t* cpu, uint16_t result) {
    cpu->status = 0;
    if(result == 0) cpu->status |= FLAG_ZERO;
    if(result & 0x8000) cpu->status |= FLAG_NEGATIVE;
}

static uint16_t read_memory(emulator_t* emu, uint16_t addr) {
    if(addr >= MEMORY_SIZE - 1) {
        emu->cpu.last_error = ERR_MEM_OUT_OF_BOUNDS;
        return 0;
    }
    return emu->cpu.memory[addr] | (emu->cpu.memory[addr+1] << 8);
}

static void write_memory(emulator_t* emu, uint16_t addr, uint16_t value) {
    if(addr >= MEMORY_SIZE - 1) {
        emu->cpu.last_error = ERR_MEM_OUT_OF_BOUNDS;
        return;
    }
    emu->cpu.memory[addr] = value & 0xFF;
    emu->cpu.memory[addr+1] = (value >> 8) & 0xFF;
}

// ==================
// Instruction Handlers
// ==================
static void handle_arithmetic(emulator_t* emu, uint16_t instr) {
    cpu_t* cpu = &emu->cpu;
    uint8_t op = OP16(instr);
    uint8_t rd = RD16(instr);
    uint16_t operand = INSTR_T16(instr) ? cpu->reg[RS16(instr)] : IMM16(instr);

    uint16_t result = 0;
    switch(op) {
        case OP_ADD: result = cpu->reg[rd] + operand; break;
        case OP_SUB: result = cpu->reg[rd] - operand; break;
        case OP_AND: result = cpu->reg[rd] & operand; break;
        case OP_NOT: result = ~operand; break;
        default: emu->cpu.last_error = ERR_INVALID_OPCODE; return;
    }

    cpu->reg[rd] = result;
    update_flags(cpu, result);
    emu->clock_cycles += emu->cycles_per_instr[op];
}

static void handle_memory(emulator_t* emu, uint32_t full_instr) {
    cpu_t* cpu = &emu->cpu;
    uint8_t op = OP8(full_instr);
    uint16_t address = ADDR_FIELD(full_instr);

    switch(op) {
        case OP_LW: {
            uint8_t rd = RD8(full_instr);
            cpu->reg[rd] = read_memory(emu, address);
            break;
        }
        case OP_SW: {
            uint8_t rd = RD8(full_instr);
            write_memory(emu, address, cpu->reg[rd]);
            break;
        }
        case OP_MW: {
            uint8_t rd = RD8(full_instr);
            if(TYPE8(full_instr)) { // Immediate variant
                cpu->reg[rd] = address;
            } else { // Register variant
                uint8_t rs = (full_instr >> 16) & 0x7;
                cpu->reg[rd] = cpu->reg[rs];
            }
            break;
        }
        case OP_PUSH: {
            cpu->reg[R_SP] -= 2;
            write_memory(emu, cpu->reg[R_SP], cpu->reg[RD8(full_instr)]);
            break;
        }
        case OP_POP: {
            cpu->reg[RD8(full_instr)] = read_memory(emu, cpu->reg[R_SP]);
            cpu->reg[R_SP] += 2;
            break;
        }
        default: emu->cpu.last_error = ERR_INVALID_OPCODE; return;
    }
    emu->clock_cycles += emu->cycles_per_instr[op];
}

static void handle_interrupt(emulator_t *emu, uint8_t vector) {
    cpu_t *cpu = &emu->cpu;
    
    cpu->reg[R_SP] -= 2;
    write_memory(emu, cpu->reg[R_SP], cpu->pc);
    
    // Jump to handler
    cpu->pc = read_memory(emu, IVT_START + (vector * 2));
}

static void handle_control(emulator_t* emu, uint32_t full_instr) {
    cpu_t* cpu = &emu->cpu;
    uint8_t op = J_OP(full_instr);

    switch(op) {
        case OP_JMP: {
            cpu->pc = J_ADDR(full_instr);
            break;
        }
        case OP_JMPC: {
            condcode_t cond = COND8(full_instr);
            uint16_t target = J_ADDR(full_instr);
            bool jump = false;
            
            switch(cond) {
                case COND_EQ: jump = (cpu->status & FLAG_ZERO); break;
                case COND_NE: jump = !(cpu->status & FLAG_ZERO); break;
                case COND_LT: jump = (cpu->status & FLAG_NEGATIVE); break;
                case COND_GT: jump = !(cpu->status & FLAG_NEGATIVE) && 
                                    !(cpu->status & FLAG_ZERO); break;
                default: break;
            }
            
            if(jump) cpu->pc = target;
            break;
        }
		case OP_INT: {
			uint8_t vector = IMM16(full_instr); handle_interrupt(emu, vector);
			break;
		}
        case OP_CMP: {
            uint8_t rs = RS16(full_instr >> 16);
            uint8_t rd = RD16(full_instr >> 16);
            int16_t result = cpu->reg[rs] - cpu->reg[rd];
            update_flags(cpu, result);
            break;
        }
        case OP_RETI: {
            cpu->pc = read_memory(emu, cpu->reg[R_SP]);
            cpu->reg[R_SP] += 2;
            break;
        }
        case OP_IN: {
            uint8_t rd = RD8(full_instr);
            cpu->reg[rd] = emu->io_handlers.read(emu, ADDR_FIELD(full_instr));
            break;
        }
        case OP_OUT: {
            uint8_t rd = RD8(full_instr);
            emu->io_handlers.write(emu, ADDR_FIELD(full_instr), cpu->reg[rd]);
            break;
        }
        default: emu->cpu.last_error = ERR_INVALID_OPCODE; return;
    }
    emu->clock_cycles += emu->cycles_per_instr[op];
}

// ==================
// Core Execution Flow
// ==================
void emulator_step(emulator_t* emu) {
    if(!emu->running) return;

    cpu_t* cpu = &emu->cpu;
    
    // Read first byte directly from memory
    uint8_t first_byte = cpu->memory[cpu->pc];
    uint8_t op = (first_byte >> 4) & 0x0F;  // Extract 4-bit opcode
    uint8_t instr_length = (op <= OP_NOT) ? 2 : 3;

    // Check memory boundaries
    if(cpu->pc + instr_length > MEMORY_SIZE) {
        cpu->last_error = ERR_MEM_OUT_OF_BOUNDS;
        emu->running = false;
        return;
    }

	// printf("we are at %04x, and we have this op %04b\n", cpu->pc, op);

    // Build full instruction
    uint32_t full_instr = first_byte << 16;
    if(instr_length == 3) {
        full_instr |= (cpu->memory[cpu->pc+1] << 8) | cpu->memory[cpu->pc+2];
    } else {
        full_instr |= cpu->memory[cpu->pc+1] << 8;
    }

    // Update PC first to support jumps
    uint16_t original_pc = cpu->pc;
    cpu->pc += instr_length;

    // Dispatch instruction
    if(op <= OP_NOT) {
        handle_arithmetic(emu, full_instr >> 8);  // Use 16-bit portion
    }
    else if(op <= OP_POP) {
        handle_memory(emu, full_instr);
    }
    else if(op <= OP_RETI) {
        handle_control(emu, full_instr);
    }
    else {
        emu->running = false;
        cpu->last_error = ERR_INVALID_OPCODE;
        cpu->pc = original_pc;
    }
}

// ==================
// Initialization
// ==================
void cpu_init(cpu_t* cpu) {
    memset(cpu, 0, sizeof(cpu_t));
    cpu->pc = TEXT_START;
    cpu->reg[R_SP] = STACK_START;
    cpu->reg[R_GP] = HEAP_START;
}

uint8_t io_read(emulator_t *emu, uint16_t addr) {
    if(addr == 0xB000) { // Keyboard status
        return (emu->keyboard.buf_head != emu->keyboard.buf_tail) | 
               (emu->keyboard.status & 0x02);
    }
    if(addr == 0xB001) { // Keyboard data
        if(emu->keyboard.buf_head == emu->keyboard.buf_tail) return 0;
        uint8_t c = emu->keyboard.buffer[emu->keyboard.buf_head];
        emu->keyboard.buf_head = (emu->keyboard.buf_head + 1) % 16;
        return c;
    }
    return 0;
}

void io_write(emulator_t *emu, uint16_t addr, uint8_t value) {
    if(addr == 0xB000) {
        emu->keyboard.status = (value & 0x02) | (emu->keyboard.status & 0x01);
    }

    if(addr == SYS_WRITE) {
        printf("R0 Value: %d\n", emu->cpu.reg[R_0]);
    }
}

void emulator_init(emulator_t* emu, uint32_t frequency) {
    memset(emu, 0, sizeof(emulator_t));
    cpu_init(&emu->cpu);
    emu->frequency = frequency;
    emu->running = true;
    
    const uint8_t DEFAULT_CYCLES[OP_COUNT] = {
        [OP_ADD] = 1, [OP_SUB] = 1, [OP_AND] = 1, [OP_NOT] = 1,
        [OP_LW] = 3, [OP_SW] = 3, [OP_MW] = 2,
        [OP_PUSH] = 2, [OP_POP] = 2,
        [OP_JMP] = 1, [OP_JMPC] = 2, [OP_INT] = 5, [OP_CMP] = 1,
        [OP_IN] = 4, [OP_OUT] = 4, [OP_RETI] = 4
    };
    memcpy(emu->cycles_per_instr, DEFAULT_CYCLES, OP_COUNT);

    emu->io_handlers.read = io_read;
    emu->io_handlers.write = io_write;
}

void poll_keyboard(emulator_t *emu) {
    char c;
    while(read(STDIN_FILENO, &c, 1) > 0) {
        if(c == 'q') emu->running = false;
        
        int next_tail = (emu->keyboard.buf_tail + 1) % 16;
        if(next_tail != emu->keyboard.buf_head) {
            emu->keyboard.buffer[emu->keyboard.buf_tail] = c;
            emu->keyboard.buf_tail = next_tail;
            if(emu->keyboard.status & 0x02) emu->irq_line = 1;
        }
    }
}

// ====================
// Main Execution Loop
// ====================
void emulator_run(emulator_t* emu) {
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);
    enable_raw_mode();

    while(emu->running) {
        poll_keyboard(emu);

        // Timing control
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        uint64_t elapsed_ns = (now.tv_sec - last_time.tv_sec) * 1000000000UL +
                             (now.tv_nsec - last_time.tv_nsec);
        uint64_t target_ns = (1000000000UL * emu->clock_cycles) / emu->frequency;

        if(elapsed_ns < target_ns) {
            struct timespec sleep_time = {
                0, (long)(target_ns - elapsed_ns)
            };
            nanosleep(&sleep_time, NULL);
        }

        emulator_step(emu);
        clock_gettime(CLOCK_MONOTONIC, &last_time);
    }

    disable_raw_mode();
}

void cpu_dump(cpu_t* cpu) {
    printf("Registers:\n");
    for(int i = 0; i < REG_COUNT; i++) {
        printf("R%d: 0x%04X (%6d)%s", 
             i, cpu->reg[i], cpu->reg[i], (i+1) % 4 ? "\t" : "\n");
    }
    printf("\nPC: 0x%04X  FLAGS: [%c%c%c%c]\n", cpu->pc,
        (cpu->status & FLAG_ZERO) ? 'Z' : '-',
        (cpu->status & FLAG_CARRY) ? 'C' : '-',
        (cpu->status & FLAG_OVERFLOW) ? 'V' : '-',
        (cpu->status & FLAG_NEGATIVE) ? 'N' : '-');
}

int main() {
    emulator_t emu;
    emulator_init(&emu, DEFAULT_FREQ);

	// Correct program: MW R0, 0xB000
	uint8_t program[] = {
		// mw R_0, 2A
		(OP_MW << 4) | (1 << 3) | R_0,
		0x00,
		0x45,

		// int 0x80
		(OP_INT << 4), 0x00, 0x80,


		(OP_MW << 4) | (1 << 3) | R_0,
		0x00,
		0xF0,

		(OP_ADD << 4) | (1 << 3) | R_0,
		0x0F,

		// int 0x80
		(OP_INT << 4), 0x00, 0x80,
	};

    memcpy(emu.cpu.memory + TEXT_START, program, sizeof(program));

	write_memory(&emu, IVT_START + (0x80 * 2), 0x00);   // Low byte
	write_memory(&emu, IVT_START + (0x80 * 2) + 1, 0xFE); // High byte
	
	// Load ISR code at 0x0100 (24-bit instructions)
	uint8_t isr_code[] = {
		// out [sys_write], R_0
		(OP_OUT << 4) | (1 << 3) | R_0,
		0xB0,
		0x02,

		(OP_RETI << 4), 0x00
	};
	memcpy(emu.cpu.memory + ISR_START, isr_code, sizeof(isr_code));

	emulator_run(&emu);
	cpu_dump(&emu.cpu);
	return 0;
}

// Set keyboard interrupt handler at 0x0100
// emu.cpu.memory[IVT_START] = 0x0100;
// emu.cpu.memory[0x0100] = (OP_IN << 12) | (1 << 11) | (R_0 << 8) | 0xB001;
// emu.cpu.memory[0x0102] = (OP_RETI << 12);
