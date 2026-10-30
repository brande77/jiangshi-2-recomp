void text_lookup_probe_power_on(void);
#include "cyc_host_extras.h"
#include "cyc_mod.h"
#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint32_t offset;
    uint8_t original;
    uint8_t replacement;
} TextBytePatch;

static const TextBytePatch patches[] = {
    {0x04D9Fu, 0xCF, 0x45},
    {0x04DA0u, 0xF1, 0x52},
    {0x04DA1u, 0xD0, 0x41},
    {0x04DA2u, 0xC6, 0x53},
    {0x04DA3u, 0xD1, 0x45},
    {0x04DA4u, 0xD5, 0x20},
    {0x04DCDu, 0xE1, 0x45},
    {0x04DCEu, 0xCF, 0x52},
    {0x04DCFu, 0xF1, 0x41},
    {0x04DD0u, 0xD0, 0x53},
    {0x04DD1u, 0xC6, 0x45},
    {0x04DD2u, 0xD1, 0x20},
    {0x04DD3u, 0xD4, 0x53},
    {0x04DD4u, 0xDB, 0x41},
    {0x04DD5u, 0xEA, 0x56},
    {0x04DD6u, 0xCA, 0x45},
    {0x04DD7u, 0xCA, 0x3F},
    {0x04DD8u, 0xE1, 0x20},
    {0x04DD9u, 0xF5, 0x20},
    {0x04DDCu, 0xCA, 0x4E},
    {0x04DDDu, 0xCA, 0x4F},
    {0x04DDEu, 0xCC, 0x20},
    {0x04DE0u, 0xE2, 0x4F},
    {0x04DE1u, 0xCA, 0x4B},
    {0x04DE6u, 0xCF, 0x45},
    {0x04DE7u, 0xF1, 0x52},
    {0x04DE8u, 0xD0, 0x41},
    {0x04DE9u, 0xC6, 0x53},
    {0x04DEAu, 0xD1, 0x45},
    {0x04DEBu, 0xD4, 0x20},
    {0x04DECu, 0xDB, 0x53},
    {0x04DEDu, 0xEA, 0x41},
    {0x04DEEu, 0xCA, 0x56},
    {0x04DEFu, 0xCA, 0x45},
    {0x04DF0u, 0xE1, 0x3F},
    {0x04DF1u, 0xF5, 0x20},
    {0x04DF4u, 0xCA, 0x4E},
    {0x04DF5u, 0xCA, 0x4F},
    {0x04DF6u, 0xCC, 0x20},
    {0x04DF8u, 0xE2, 0x4F},
    {0x04DF9u, 0xCA, 0x4B},
};

static void translation_power_on(void *ctx)
{
    text_lookup_probe_power_on();

    (void)ctx;
    FILE *log = fopen(
        "./menu_translation_test.log",
        "w"
    );

    unsigned applied = 0;
    unsigned count = (unsigned)(sizeof(patches) / sizeof(patches[0]));

    for (unsigned i = 0; i < count; ++i) {
        bool ok = cyc_mod_set_prg_read_override(
            patches[i].offset,
            patches[i].original,
            patches[i].replacement
        );
        if (ok) ++applied;
        if (log) {
            fprintf(log, "PRG %05X: %s\n",
                (unsigned)patches[i].offset,
                ok ? "registered" : "FAILED");
        }
    }

    if (log) {
        fprintf(log, "Registered %u of %u test bytes.\n", applied, count);
        fclose(log);
    }
}


#include "cyc_render.h"

#define CAPTURE_ROOT "./"

void text_trace_frame_end(void);

static void translation_capture_frame(void *ctx)
{
    text_trace_frame_end();
    (void)ctx;
    static unsigned ticks = 0;
    if (++ticks % 30u != 0) return;

    FILE *request = fopen(CAPTURE_ROOT "capture_screen.request", "rb");
    if (!request) return;
    fclose(request);

    FILE *out = fopen(CAPTURE_ROOT "live_screen_capture.txt", "w");
    if (!out) return;

    remove(CAPTURE_ROOT "capture_screen.request");

    fprintf(out, "LIVE SCREEN CAPTURE V1\n");

    fprintf(out, "\n[CHR]\n");
    for (unsigned a = 0; a < 8192; ++a) {
        if ((a & 15u) == 0) fprintf(out, "%04X:", a);
        fprintf(out, " %02X", (unsigned)cyc_render_chr((uint16_t)a));
        if ((a & 15u) == 15u) fputc('\n', out);
    }

    fprintf(out, "\n[NAMETABLE]\n");
    for (unsigned a = 0x2000; a < 0x3000; ++a) {
        if ((a & 15u) == 0) fprintf(out, "%04X:", a);
        fprintf(out, " %02X",
            (unsigned)cyc_render_nametable((uint16_t)a));
        if ((a & 15u) == 15u) fputc('\n', out);
    }

    fprintf(out, "\n[CPU_RAM]\n");
    const uint8_t *ram = cyc_cpu_ram();
    if (ram) {
        for (unsigned a = 0; a < 2048; ++a) {
            if ((a & 15u) == 0) fprintf(out, "%04X:", a);
            fprintf(out, " %02X", (unsigned)ram[a]);
            if ((a & 15u) == 15u) fputc('\n', out);
        }
    }

    fprintf(out, "\n[LINES]\n");
    fprintf(out, "Y SCROLL_X SCROLL_Y BG_TABLE MASK\n");
    for (int y = 0; y < 240; ++y) {
        fprintf(out, "%d %d %d %04X %02X\n",
            y,
            cyc_render_line_scroll_x(y),
            cyc_render_line_scroll_y(y),
            (unsigned)cyc_render_line_bg_table(y),
            (unsigned)cyc_render_line_mask(y));
    }

    fprintf(out, "\n[PRG_MAP]\n");
    for (unsigned a = 0x8000; a < 0x10000; a += 4096) {
        fprintf(out, "%04X: %u\n",
            a, hw_prg_bank4((uint16_t)a));
    }

    fprintf(out, "\nEND_CAPTURE\n");
    fclose(out);
}


const uint32_t *english_display_present(void *ctx, int *width, int *height);

static const CycHostExtras translation_extras = {
    .present = english_display_present,
    .power_on = translation_power_on,
    .frame_end = translation_capture_frame
};

const CycHostExtras *cyc_host_extras(void)
{
    return &translation_extras;
}
