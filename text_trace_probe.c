#include "cyc_trace.h"
#include "cyc_core.h"
#include <stdio.h>

#define ROOT "./"
#define HISTORY 192u

typedef struct {
    unsigned pc, addr, value, bank;
    char kind;
    unsigned dma;
} TraceRow;

static TraceRow history[HISTORY];
static unsigned used, head, after, captures, frames;
static FILE *output;
static bool previous_enabled;

static void print_row(const TraceRow *r)
{
    fprintf(output, "%04X %c %04X %02X %u %u\n",
        r->pc, r->kind, r->addr, r->value, r->bank, r->dma);
}

static void observe_cycle(const CycTraceCycle *cycle)
{
    CycCpuState state;
    cyc_cpu_state(&state);

    TraceRow row = {
        state.pc, cycle->addr, cycle->value,
        cycle->addr >= 0x8000 ? hw_prg_bank4(cycle->addr) : 0,
        cycle->kind, cycle->dma ? 1u : 0u
    };

    if (after) {
        print_row(&row);
        --after;
        if (!after) {
            fprintf(output, "END_EVENT\n");
            fflush(output);
        }
    } else if (captures < 8 &&
               cycle->kind == 'W' &&
               !cycle->dma &&
               cycle->addr >= 0x2000 &&
               cycle->addr < 0x4000 &&
               (cycle->addr & 7u) == 7u &&
               cycle->value == 0xD2) {
        ++captures;
        fprintf(output, "\nEVENT %u: PPU DATA WRITE D2\n", captures);
        fprintf(output, "PC_DURING_CYCLE KIND ADDRESS VALUE PRG_BANK4 DMA\n");
        unsigned first = (head + HISTORY - used) % HISTORY;
        for (unsigned i = 0; i < used; ++i) {
            print_row(&history[(first + i) % HISTORY]);
        }
        print_row(&row);
        after = 64;
    }

    history[head] = row;
    head = (head + 1u) % HISTORY;
    if (used < HISTORY) ++used;
}

static void finish_trace(void)
{
    cyc_trace_cycle_hook = NULL;
    cyc_trace_enabled = previous_enabled;
    fprintf(output, "\nTRACE_COMPLETE events=%u\n", captures);
    fclose(output);
    output = NULL;
}

void text_trace_frame_end(void)
{
    if (output) {
        if (++frames >= 1800 || (captures >= 8 && !after)) {
            finish_trace();
        }
        return;
    }

    static unsigned poll;
    if (++poll % 30u) return;

    FILE *request = fopen(ROOT "trace_text.request", "rb");
    if (!request) return;
    fclose(request);

    if (cyc_trace_cycle_hook || cyc_trace_file) {
        FILE *status = fopen(ROOT "text_reader_trace.txt", "w");
        if (status) {
            fprintf(status, "STOP: Another trace is already active.\n");
            fclose(status);
        }
        remove(ROOT "trace_text.request");
        return;
    }

    output = fopen(ROOT "text_reader_trace.txt", "w");
    if (!output) return;

    remove(ROOT "trace_text.request");
    used = head = after = captures = frames = 0;
    previous_enabled = cyc_trace_enabled;

    fprintf(output, "TEXT READER TRACE V1\n");
    fprintf(output, "PC column is CPU PC during the cycle, not necessarily instruction start.\n");
    fprintf(output, "PRG_BANK4 identifies the mapping during that access.\n");
    fflush(output);

    cyc_trace_cycle_hook = observe_cycle;
    cyc_trace_enabled = true;
}
