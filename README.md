# 16-bit CPU Emulator Specs

## Instruction Set Architecture (ISA)

### Instruction Format

This design uses a consistent approach for instructions that involve explicit address operands. Instructions requiring a 16‑bit address (for memory, I/O, and control operations) are 3 bytes (24 bits) long, with 1 byte dedicated to the opcode (and register field if needed) and 2 bytes for the address. Instructions that do not use an address remain 2 bytes (16 bits) long.

#### Address Instructions (I-Type for I/O and Memory Accesses)  
For instructions such as `in`, `out`, `sw`, `lw`, and `mw` that work with a memory address, the format is:  
```
[ op:4 | type:1 (1) | rd:3 | addr:16 ]
```
*Total length: 24 bits (3 bytes)*  
- The opcode (4 bits), combined with a type bit (set to 1) and a 3‑bit register field (`rd`), specifies the operation and the register operand (if applicable).  
- The 16‑bit address field provides a full address for memory-mapped I/O or data movement.

#### Control/Jump Instructions (J-Type)  
For jump instructions (e.g. `jmp`, `jmpc`) that reference a target address, the format is:  
```
[ op:4 | cond:4 | addr:16 ]
```
*Total length: 24 bits (3 bytes)*  
- The opcode (4 bits) along with a 4‑bit condition field specify the jump type and any conditional criteria.  
- The 16‑bit address field allows jumps to any location within the 64KB address space.

#### Arithmetic and Logical Instructions (A-Type)  
For arithmetic, logic, memory (non-address), and compare instructions that operate entirely on registers, the format is:  
```
[ op:4 | type:1 (0) | rd:3 | rs:3 | 0:5 ]
```
*Total length: 16 bits (2 bytes)*

#### Push/Pop Instructions (I-Type)  
For stack operations like `push` and `pop` that use an immediate 8‑bit field, the format is:  
```
[ op:4 | type:1 (1) | rd:3 | 0:8 ]
```
*Total length: 16 bits (2 bytes)*

#### System Instructions (S-Type)  
For system-level instructions such as `int`, the format is:  
```
[ op:4 | 0:4 | imm:8 ]
```
*Total length: 16 bits (2 bytes)*

---

### Instruction Categories

The instruction set is organized by functionality. Instructions that involve an explicit address use the 3‑byte (24‑bit) format.

| **Category**   | **Instructions**                      | **Description**                                                                                                                                           |
|----------------|---------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Arithmetic** | `add`, `sub`                          | Basic integer arithmetic operations on registers (2‑byte format).                                                                    |
| **Logical**    | `and`, `or`                           | Bitwise logic operations on registers (2‑byte format).                                                                                                   |
| **Memory**     | `sw`, `lw`, `mw`, `push`, `pop`        | Data movement and stack operations:  
– `sw` (store word), `lw` (load word), and `mw` (move word) use the 3‑byte format with a 16‑bit address field.  
– `push` and `pop` use a 2‑byte format with an immediate field. |
| **Control**    | `jmp`, `jmpc`                         | Jump and branch operations that specify a target address using the 3‑byte format.               |
| **I/O**        | `in`, `out`                           | I/O operations accessing memory-mapped I/O devices via a 16‑bit address operand (3‑byte format).                                        |
| **System**     | `int`, `cmp`, `reti`                  | Software interrupt generation and flag‑setting/comparison operations, using the S‑type or A‑type formats as appropriate.             |

---

## Registers (16 Total)

| Register | Name    | Purpose                                | Convention       |
|----------|---------|----------------------------------------|------------------|
| r0–r3   | `r0–r3` | Temporary registers                    | Caller-saved     |
| r4       | `gp`    | Global pointer                         | Reserved         |
| r5       | `sp`    | Stack pointer                          | Reserved         |
| r6       | `fp`    | Frame pointer                          | Reserved         |
| r7       | `ra`    | Return address                         | Reserved         |

---

## Memory Map (64KB Address Space)

| Region       | Address Range      | Size  | Description                      |
|--------------|--------------------|-------|----------------------------------|
| **TEXT**     | `0x0000–0x5FFF`    | 24KB  | Executable code section          |
| **DATA**     | `0x6000–0x9FFF`    | 16KB  | Initialized static data          |
| **VRAM**     | `0xA000–0xAFFF`    | 4KB   | Video RAM (framebuffer)          |
| **I/O**      | `0xB000–0xBFFF`    | 4KB   | Memory-mapped I/O devices        |
| **HEAP**     | `0xC000–0xEFFF`    | 12KB  | Dynamic memory (grows upward)    |
| **STACK**    | `0xF000–0xFBFF`    | 3KB   | Call stack (grows downward)      |
| **IVT**      | `0xFC00–0xFFFF`    | 1KB   | Interrupt Vector Table           |

---

## Key Design Notes

1. **Word Size:** 16‑bit words; memory is byte‑addressable.
2. **Interrupts:**  
   – The `int` instruction triggers a software interrupt.  
   – The Interrupt Vector Table (IVT) contains 256 entries of 4 bytes each (spanning 0xFC00 to 0xFFFF).

---

*Implementation details can be found in [emu.h](src/emu.h) and [emu.c](src/emu.c).*
