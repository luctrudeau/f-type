#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cpu.h"

int main(int argc, const char * argv[]) {
    bool verbose = getenv("VERBOSE");
    
    if (argc != 2) {
        printf("Usage: %s rom.nes\n", argv[0]);
        return 1;
    }
    
    FILE *rom_file = fopen(argv[1], "r");
    if (!rom_file) {
        printf("Error opening file %s\n", argv[1]);
        return 1;
    }
    
    uint8_t header[0x10];
    if (fread(header, sizeof(header), 1, rom_file) < 1) {
        printf("Error reading iNES header\n");
        return 1;
    }
    if (strncmp((const char *)header, "NES\x1a", 4)) {
        printf("Not a iNES file\n");
        return 1;
    }
    
    int prg_size = header[4] * 16;
    printf("PRG ROM: %dKB\n", prg_size);
    if (prg_size <= 0) {
        printf("Unexpected size for PRG ROM\n");
        return 1;
    }
    prg_size *= 1024;
    uint8_t prg_rom[(prg_size < 0x8000 ? 0x8000 : prg_size)];
    if (fread(prg_rom, prg_size, 1, rom_file) < 1) {
        printf("Error reading PRG ROM\n");
        return 1;
    }
    if (prg_size < 0x8000) {
        for (int i = 0; i < 0x4000; i++) {
            prg_rom[i + 0x4000] = prg_rom[i];
        }
    }
    
    int chr_size = header[5] * 8;
    printf("CHR ROM: %dKB\n", chr_size);
    chr_size *= 1024;
    uint8_t chr_rom[chr_size];
    if (chr_size >= 0) {
        if (fread(chr_rom, chr_size, 1, rom_file) < 1) {
            printf("Error reading CHR ROM\n");
            return 1;
        }
    }
    int mapper = ((header[6] & 0b11110000) >> 4) + (header[7] & 0b11110000);
    printf("Mapper: %d\n", mapper);
    if (mapper) {
        printf("Only mapper 0 is supported so far\n");
        return 1;
    }
    bool mirroring = header[6] & 1;
    printf("Mirroring: %s\n", (mirroring ? "Vertical" : "Horizontal"));
    fclose(rom_file);
    
    memory_map mm;
    memory_map_cpu_init(&mm, prg_rom);
    
    cpu_state st;
    cpu_init(&st, &mm);
    
    int total_t = cpu_reset(&st);
    do {
        if (verbose) {
            cpu_debug_print_state(&st);
        }
        total_t += cpu_step(&st, verbose);
    } while(st.pc != 0x8057);
    
    if (verbose) {
        cpu_debug_print_state(&st);
    }
    printf("Ended in %d cycles\n", total_t);
    return 0;
}
