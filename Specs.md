### My 16-bit CPU

### instructions set
-- Arithmetic
add
sub
mult
div

-- Logic
and
or
not
sll
slr

-- Memory
la
mov
push
pop

-- Branching
jmp
jmpc

-- Interrupting
int

opCode[4] rs[4] rt[4] rd[4]

### registers
r0-r3: t0-t3 -- tmp regs
r4-r11: s0-s7 -- idk
r12: gp -- global pointer
r13: sp -- stack pointer
r14: fp -- frame pointer
r15: ra -- return address

_As you can see here im inspired by the MIPS registers, but just got rid of the $ cuz it reminds me of php, and i hate php._

### memroy layout
MEMORY_SIZE 0xFFFF (64KB)

IVT       0xFC00 -> 0xFFFF (1KB)    // Interrupt Vector Table
STACK     0xF000 -> 0xFBFF (3KB)    // Stack (grows downwards)
HEAP      0xC000 -> 0xEFFF (12KB)   // Heap (grows upwards)
I/O       0xB000 -> 0xBFFF (4KB)    // Memory-mapped I/O
VRAM      0xA000 -> 0xAFFF (4KB)    // Video RAM
DATA      0x6000 -> 0x9FFF (16KB)   // Data section
TEXT      0x0000 -> 0x5FFF (24KB)   // Text section
