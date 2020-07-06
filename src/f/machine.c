#include "machine.h"

#include "../driver.h"
#include "loader.h"

void machine_init(Machine *vm, FCartInfo *carti, Driver *driver) {
    memset(vm, 0, sizeof(Machine));
    
    vm->input = &driver->input;
    
    vm->cart.prg_rom = carti->prg_rom;
    vm->cart.chr_memory = carti->chr_rom;
    vm->cart.has_battery_backup = carti->has_battery_backup;

    memory_map_cpu_init(&vm->cpu_mm, vm);
    memory_map_ppu_init(&vm->ppu_mm, vm);
    cpu_65xx_init(&vm->cpu, &vm->cpu_mm,
                  (CPU65xxReadFunc)mm_read, (CPU65xxWriteFunc)mm_write);
    ppu_init(&vm->ppu, &vm->ppu_mm, &vm->cpu,
             driver->screen, &driver->input.lightgun_pos);
    apu_init(&vm->apu, &vm->cpu, driver->audio_frame);
    
    if (!vm->cart.chr_memory.size) {
        vm->cart.chr_memory.size = SIZE_CHR_ROM;
        vm->cart.chr_is_ram = true;
        vm->cart.chr_memory.data = malloc(SIZE_CHR_ROM);
        memset(vm->cart.chr_memory.data, 0, SIZE_CHR_ROM);
    }
    
    machine_set_nt_mirroring(vm, carti->default_mirroring);
    mapper_init(vm, carti->mapper_id);
    
    cpu_65xx_reset(&vm->cpu, false);
}

void machine_teardown(Machine *vm) {
    // TODO: Save SRAM
    if (vm->cart.sram.data) {
        free(vm->cart.sram.data);
    }
    
    if (vm->cart.chr_is_ram) {
        free(vm->cart.chr_memory.data);
    }
}

bool machine_advance_frame(Machine *vm, bool verbose) {
    bool done;
    int pos = 0;
    do {
        if (vm->ppu.time > vm->cpu.time * T_CPU_MULTIPLIER) {
            // Check for debug label
            bool is_endless_loop = false;
            if (verbose && vm->dbg_map) {
                int i = 0;
                while (vm->dbg_map[i].label[0]) {
                    if (vm->dbg_map[i].addr == vm->cpu.pc) {
                        const char *label = vm->dbg_map[i].label;
                        if (strcmp(label, "EndlessLoop")) {
                            printf(":%s\n", vm->dbg_map[i].label);
                        } else {
                            is_endless_loop = true;
                        }
                        break;
                    }
                    i++;
                }
            }
            
            // Run next CPU instruction
            cpu_65xx_step(&vm->cpu, verbose && !is_endless_loop);
        }
        
        if (vm->ppu.time > vm->apu.time * T_APU_MULTIPLIER) {
            apu_step(&vm->apu);
        }
        // Yeah this needs to be done better
        if (!(pos % 121)) {
            apu_sample(&vm->apu, pos / 121);
        }
        
        done = ppu_step(&vm->ppu, verbose);
        
        ++pos;
    } while (!done);
    return true;
}

void machine_set_nt_mirroring(Machine *vm, NametableMirroring nm) {
    const int layouts[] = {
        0, 0, 0, 0, // SINGLE_A
        1, 1, 1, 1, // SINGLE_B
        0, 1, 0, 1, // VERTICAL
        0, 0, 1, 1, // HORIZONTAL
        0, 1, 2, 3, // FOUR
    };
    const int *layout = layouts + nm * 4;
    for (int i = 0; i < 4; i++) {
        vm->nt_layout[i] = vm->nametables[layout[i]];
    }
}
