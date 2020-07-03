#ifndef f_machine_h
#define f_machine_h

#include "../common.h"

#include "../cpu/65xx.h"
#include "apu.h"
#include "cartridge.h"
#include "memory_maps.h"
#include "ppu.h"

// Size of various memory structures
#define SIZE_WRAM 0x800
#define SIZE_PRG_ROM 0x8000
#define SIZE_CHR_ROM 0x2000
#define SIZE_NAMETABLE 0x400

// Timing constants
#define T_CPU_MULTIPLIER 3
#define T_APU_MULTIPLIER 6

// Screen dimensions
#define WIDTH 256
#define HEIGHT 240

// Forward decalarations
typedef struct Driver Driver;
typedef struct FCartInfo FCartInfo;
typedef struct InputState InputState;

// IRQ bits
typedef enum {
    IRQ_APU_FRAME = 0,
    IRQ_APU_DMC,
    IRQ_MAPPER,
} IRQFlag;

typedef struct {
    uint16_t addr;
    char label[256];
} DebugMap;

typedef struct Machine {
    CPU65xx cpu;
    PPU ppu;
    APU apu;
    MemoryMap cpu_mm;
    MemoryMap ppu_mm;
    Cartridge cart;
    
    const DebugMap *dbg_map;
    
    // System RAM
    uint8_t wram[SIZE_WRAM];
    uint8_t nametables[4][SIZE_NAMETABLE];
    uint8_t *nt_layout[4];
    
    // Controller I/O
    uint8_t ctrl_latch[2];
    InputState *input;
} Machine;

typedef enum {
    NT_SINGLE_A = 0,
    NT_SINGLE_B = 1,
    NT_VERTICAL = 2,
    NT_HORIZONTAL = 3,
    NT_FOUR = 4,
} NametableMirroring;

void machine_init(Machine *vm, FCartInfo *carti, Driver *driver);
void machine_teardown(Machine *vm);

bool machine_advance_frame(Machine *vm, bool verbose);

void machine_set_nt_mirroring(Machine *vm, NametableMirroring m);

#endif /* f_machine_h */
