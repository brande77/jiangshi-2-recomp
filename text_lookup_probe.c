#include "cyc_core.h"
#include "cyc_mod.h"
#include "cpu6502.h"
#include <stdio.h>
#define LOOKUP_ROOT "./"
void english_lookup_reset(void);
void english_lookup_observe(uint32_t id, uint16_t pointer, int append);
static FILE *lookup_log;
static unsigned lookup_events;
static int lookup_code_matches(uint16_t addr, const uint8_t *bytes, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) {
        uint8_t value;
        if (!cyc_debug_peek((uint16_t)(addr+i), &value) || value != bytes[i]) return 0;
    }
    return 1;
}
static void lookup_return_observer(void)
{
    CycCpuState state;
    cyc_cpu_state(&state);
    if ((state.pc != 0xD3D3 && state.pc != 0xD3E5) || hw_prg_bank4(state.pc) != 23) return;
    static const uint8_t first[] = {0x86,0x1A,0x84,0x1B,0x20,0x64,0xDE};
    static const uint8_t second[] = {0x86,0x1A,0x84,0x1B,0x20,0x84,0xDE};
    if (!lookup_code_matches(state.pc, state.pc == 0xD3D3 ? first : second, 7)) return;
    uint16_t pointer = (uint16_t)(state.x | ((uint16_t)state.y << 8));
    unsigned bank = pointer >= 0x8000 ? hw_prg_bank4(pointer) : 32;
    uint32_t id = bank < 32 ? bank * 4096u + (pointer & 0xFFFu) : UINT32_C(0xFFFFFFFF);
    english_lookup_observe(id, pointer, state.pc == 0xD3E5);
    if (!lookup_log || lookup_events >= 512) return;
    const uint8_t *ram = cyc_cpu_ram();
    unsigned script = ram ? ram[0x84] | ((unsigned)ram[0x85] << 8) : 0;
    fprintf(lookup_log, "EVENT %u return_pc=%04X script_cursor=%04X text_cpu=%04X bank4=%u id=prg_%05X raw=",
            ++lookup_events, state.pc, script, pointer, bank, (unsigned)id);
    unsigned ended = 0;
    for (unsigned i = 0; i < 192 && (unsigned)pointer + i < 0x10000u; ++i) {
        uint16_t addr = (uint16_t)(pointer+i); uint8_t value;
        if ((addr >= 0x2000 && addr < 0x6000) || !cyc_debug_peek(addr, &value)) break;
        fprintf(lookup_log, "%02X ", value);
        if (value == 0xFF) { ended = 1; break; }
    }
    fprintf(lookup_log, "terminated=%u\n", ended); fflush(lookup_log);
}
void text_lookup_probe_power_on(void)
{
    english_lookup_reset();
    if (lookup_log) { fclose(lookup_log); lookup_log = NULL; }
    lookup_events = 0;
    lookup_log = fopen(LOOKUP_ROOT "text_lookup_probe.log", "w");
    if (cyc_cpu_rts_observer && cyc_cpu_rts_observer != lookup_return_observer) {
        if (lookup_log) { fprintf(lookup_log, "STOP: Another return observer is installed.\n"); fclose(lookup_log); lookup_log = NULL; }
        return;
    }
    cyc_mod_set_return_hook(lookup_return_observer);
    if (lookup_log) {
        fprintf(lookup_log, "TEXT LOOKUP OBSERVER V2\nREADY: catalog-backed presentation; no guest state writes.\n");
        fflush(lookup_log);
    }
}
