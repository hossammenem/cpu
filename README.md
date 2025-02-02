# 16-bit CPU Emulator Specs

## Instruction Set Architecture (ISA)

### Instruction Format
`opCode[4] rd[4] rs[4] rt[4]`  
*(16-bit instructions: 4-bit opcode + three 4-bit register fields)*

### Instruction Categories

| Category       | Instructions                | Description                              |
|----------------|-----------------------------|------------------------------------------|
| **Arithmetic** | `add`, `sub`, `mult`, `div` | Basic integer arithmetic operations      |
| **Logical**    | `and`, `or`, `not`          | Bitwise logic operations                 |
| **Shift**      | `sll`, `slr`                | Shift Left Logical / Shift Right Logical|
| **Memory**     | `la`, `mov`, `push`, `pop`  | Address loading, data movement, stack ops|
| **Control**    | `jmp`, `jmpc`               | Unconditional/Conditional jumps          |
| **System**     | `int`                       | Software interrupt generation            |

---

## Registers (16 Total)

| Register | Name    | Purpose                                | Convention       |
|----------|---------|----------------------------------------|------------------|
| r0-r3    | `t0-t3` | Temporary registers                    | Caller-saved     |
| r4-r7    | `a0-a3` | Function arguments/return values       | Caller-saved     |
| r8-r11   | `s0-s3` | Saved registers                        | Callee-saved     |
| r12      | `gp`    | Global pointer                         | Reserved         |
| r13      | `sp`    | Stack pointer                          | Reserved         |
| r14      | `fp`    | Frame pointer                          | Reserved         |
| r15      | `ra`    | Return address                         | Reserved         |

*Register conventions inspired by MIPS architecture*

---

## Memory Map (64KB Address Space)

| Region       | Address Range      | Size  | Description                      |
|--------------|--------------------|-------|----------------------------------|
| **TEXT**     | `0x0000-0x5FFF`    | 24KB  | Executable code section          |
| **DATA**     | `0x6000-0x9FFF`    | 16KB  | Initialized static data          |
| **VRAM**     | `0xA000-0xAFFF`    | 4KB   | Video RAM (framebuffer)          |
| **I/O**      | `0xB000-0xBFFF`    | 4KB   | Memory-mapped I/O devices        |
| **HEAP**     | `0xC000-0xEFFF`    | 12KB  | Dynamic memory (grows upward)    |
| **STACK**    | `0xF000-0xFBFF`    | 3KB   | Call stack (grows downward)      |
| **IVT**      | `0xFC00-0xFFFF`    | 1KB   | Interrupt Vector Table           |

---

## Key Design Notes
1. **Word Size**: 16-bit words, byte-addressable memory
2. **Endianness**: Little-endian format
3. **Interrupts**: 
   - `int` instruction triggers software interrupt
   - IVT contains 256 4-byte entries (0xFC00-0xFFFF)
4. **Special Regions**:
   - Stack grows downward from 0xFBFF
   - Heap grows upward from 0xC000
   - Video RAM mapped to 0xA000-0xAFFF

---

*Implementation details available in [emu.h](emu.h) and [emu.c](emu.c)*
