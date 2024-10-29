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
sll
slr

-- Memory
lw
sw
la
push
pop

-- Branching
jmp
jmpz
jmpc

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

STACK     0xFFFF -> 0xC000 (16KB) // grows downwards
HEAP      0x6000 -> 0xBFFF (24KB) // grows upwards
TEXT      0x0000 -> 0x3FFF (16KB)
DATA      0x4000 -> 0x5FFF (8KB)
