int english_lookup_lines(const char **line1, const char **line2);
#include "cyc_render.h"
#include <stdint.h>
#include <string.h>

static uint32_t english_picture[256 * 240];


static const uint8_t *letter_columns(char c)
{
    /* DIALOGUE_SAVE_BRACKETS_V1 */
    static const uint8_t save_left_bracket[5]={0,0x7F,0x41,0x41,0};
    static const uint8_t save_right_bracket[5]={0,0x41,0x41,0x7F,0};
    if(c=='[')return save_left_bracket;
    if(c==']')return save_right_bracket;

    /* DIALOGUE_DIGITS_V1 */
    static const uint8_t dialogue_digits[10][5]={
        {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},
        {0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},
        {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},
        {0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
        {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E}
    };
    if(c>='0'&&c<='9')return dialogue_digits[c-'0'];

    static const uint8_t popup_apostrophe[5] = {0,0,0x07,0,0};
    if ((unsigned char)c == 39u) return popup_apostrophe;

    static const uint8_t letters[26][5] = {
        {0x7E,0x11,0x11,0x11,0x7E},
        {0x7F,0x49,0x49,0x49,0x36},
        {0x3E,0x41,0x41,0x41,0x22},
        {0x7F,0x41,0x41,0x22,0x1C},
        {0x7F,0x49,0x49,0x49,0x41},
        {0x7F,0x09,0x09,0x09,0x01},
        {0x3E,0x41,0x49,0x49,0x7A},
        {0x7F,0x08,0x08,0x08,0x7F},
        {0x00,0x41,0x7F,0x41,0x00},
        {0x20,0x40,0x41,0x3F,0x01},
        {0x7F,0x08,0x14,0x22,0x41},
        {0x7F,0x40,0x40,0x40,0x40},
        {0x7F,0x02,0x0C,0x02,0x7F},
        {0x7F,0x04,0x08,0x10,0x7F},
        {0x3E,0x41,0x41,0x41,0x3E},
        {0x7F,0x09,0x09,0x09,0x06},
        {0x3E,0x41,0x51,0x21,0x5E},
        {0x7F,0x09,0x19,0x29,0x46},
        {0x46,0x49,0x49,0x49,0x31},
        {0x01,0x01,0x7F,0x01,0x01},
        {0x3F,0x40,0x40,0x40,0x3F},
        {0x1F,0x20,0x40,0x20,0x1F},
        {0x3F,0x40,0x38,0x40,0x3F},
        {0x63,0x14,0x08,0x14,0x63},
        {0x07,0x08,0x70,0x08,0x07},
        {0x61,0x51,0x49,0x45,0x43}
    };
    static const uint8_t blank[5] = {0,0,0,0,0};
    static const uint8_t dot[5] = {0,0x60,0x60,0,0};
    static const uint8_t comma[5] = {0,0x40,0x30,0,0};
    static const uint8_t question[5] = {0x02,0x01,0x51,0x09,0x06};

    if (c >= 'A' && c <= 'Z') return letters[c - 'A'];
    static const uint8_t warehouse_exclamation[5]={0,0,0x5F,0,0};
    if(c=='!')return warehouse_exclamation;
    if (c == '.') return dot;
    if (c == ',') return comma;
    if (c == '?') return question;
    return blank;
}

static void draw_words(int x, int y, const char *text)
{
    while (*text) {
        const uint8_t *columns = letter_columns(*text++);
        for (int dx = 0; dx < 5; ++dx) {
            for (int dy = 0; dy < 7; ++dy) {
                if (columns[dx] & (1u << dy)) {
                    int px = x + dx;
                    int py = y + dy;
                    if (px >= 0 && px < 256 && py >= 0 && py < 240) {
                        english_picture[py * 256 + px] = UINT32_C(0xFFFFFFFF);
                    }
                }
            }
        }
        x += 6;
    }
}

static int temple_sentence_is_present(void)
{
    static const uint8_t first[] = {
        0xD2,0xD2,0xE2,0x8A
    };
    static const uint8_t second[] = {
        0x8D,0xCD,0xDB,0xED,0x8E,0xDB,0xD5,0x8B
    };

    for (unsigned i = 0; i < sizeof(first); ++i) {
        if (cyc_render_nametable((uint16_t)(0x2084 + i)) != first[i])
            return 0;
    }
    for (unsigned i = 0; i < sizeof(second); ++i) {
        if (cyc_render_nametable((uint16_t)(0x20C4 + i)) != second[i])
            return 0;
    }

    return cyc_render_line_bg_table(40) == 0x1000;
}

static void clear_text_area(int left, int top, int right, int bottom)
{
    for (int y = top; y < bottom; ++y)
        for (int x = left; x < right; ++x)
            english_picture[y * 256 + x] = UINT32_C(0xFF000000);
}


static int character_menu_v2_is_present(void)
{
    static const uint8_t player[] = {0x50,0x4C,0x41,0x59,0x45,0x52,0x20};
    static const uint8_t names[4][4] = {
        {0x08,0x09,0x0A,0x0B}, {0x0C,0x0D,0x0E,0x0F},
        {0x00,0x01,0x02,0x03}, {0x04,0x05,0x06,0x07}
    };
    static const uint8_t ascii_a[16] = {
        0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00,
        0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00
    };
    for (unsigned i = 0; i < sizeof(player); ++i)
        if (cyc_render_nametable((uint16_t)(0x208C + i)) != player[i]) return 0;
    uint8_t player_number = cyc_render_nametable(0x2093);
    if (player_number != 0x31 && player_number != 0x32) return 0;
    for (unsigned row = 0; row < 4; ++row) {
        int y = 65 + (int)row*16;
        if (cyc_render_line_bg_table(y) != 0x1000 ||
            cyc_render_line_scroll_x(y) != 0 ||
            cyc_render_line_scroll_y(y) != 239+y ||
            cyc_render_line_sprite16(y)) return 0;
        for (unsigned i = 0; i < 4; ++i)
            if (cyc_render_nametable((uint16_t)(0x210E + row*0x40 + i)) != names[row][i]) return 0;
    }
    for (unsigned i = 0; i < 16; ++i)
        if (cyc_render_chr((uint16_t)(0x1410 + i)) != ascii_a[i] ||
            cyc_render_chr((uint16_t)(0x1200 + i)) != 0) return 0;
    return 1;
}

static int character_menu_v2_highlight(unsigned row)
{
    const uint8_t *oam = cyc_render_oam();
    if (!oam) return -1;
    unsigned top_minus_one = 64 + row*16;
    for (unsigned slot = 0; slot < 64; ++slot) {
        const uint8_t *p = oam + slot*4;
        if (p[0] != top_minus_one || p[1] != 0xE1 ||
            p[3] != 112 || !(p[2] & 0x20)) continue;
        unsigned mask = 0;
        for (unsigned j = 0; j < 64; ++j) {
            const uint8_t *q = oam + j*4;
            if (q[0] != p[0] || q[1] != p[1] || q[2] != p[2]) continue;
            if (q[3] >= 112 && q[3] <= 136 && ((q[3]-112)%8) == 0)
                mask |= 1u << ((q[3]-112)/8);
        }
        if (mask == 15) return (int)slot;
    }
    return -1;
}

/* CHARACTER_ERASE_NAMES_V1: display-only, no save or input changes. */
static void character_erase_names_draw(void)
{
    static const uint8_t names[4][4] = {
        {8,9,10,11},{12,13,14,15},{0,1,2,3},{4,5,6,7}
    };
    static const char *const prompts[4] = {
        "DEPPA ERASE SAVE?","TORA ERASE SAVE?",
        "CHIBIKURO ERASE SAVE?","SUIKA ERASE SAVE?"
    };
    static const char question[]="ERASE SAVE?";
    int who=-1;
    for(unsigned n=0;n<4;n++){
        int match=1;
        for(unsigned i=0;i<4;i++)
            if(cyc_render_nametable((uint16_t)(0x2343+i))!=names[n][i])match=0;
        if(match){who=(int)n;break;}
    }
    if(who<0||cyc_render_nametable(0x2347)!=0x20||
       cyc_render_nametable(0x2357)!='N'||cyc_render_nametable(0x2358)!='O'||
       cyc_render_nametable(0x235B)!='O'||cyc_render_nametable(0x235C)!='K')return;
    for(unsigned i=0;i<sizeof(question)-1;i++)
        if(cyc_render_nametable((uint16_t)(0x2348+i))!=(uint8_t)question[i])return;
    for(int y=209;y<217;y++)
        if(cyc_render_line_bg_table(y)!=0x1000||cyc_render_line_scroll_x(y)!=0||
           cyc_render_line_scroll_y(y)!=239+y||!(cyc_render_line_mask(y)&8))return;
    /* The longest prompt is 21 glyphs: 125 pixels in the original 128-pixel text region. */
    for(int y=209;y<217;y++){
        uint32_t background=english_picture[y*256+16];
        for(int x=24;x<152;x++)english_picture[y*256+x]=background;
    }
    draw_words(24,209,prompts[who]);
}

static void character_menu_v2_draw(void)
{
    static const char *const labels[4] = {"DEPPA","TORA","CHIBIKURO","SUIKA"};
    const uint8_t *oam = cyc_render_oam();
    for (unsigned row = 0; row < 4; ++row) {
        int top = 65 + (int)row*16;
        unsigned tile_y = 8 + row*2;
        unsigned address = 0x23C0 + (tile_y/4)*8 + 14/4;
        unsigned shift = ((tile_y & 2u) << 1) | (14u & 2u);
        int palette = (cyc_render_nametable((uint16_t)address) >> shift) & 3;
        for (int y = top; y < top+8; ++y) {
            uint32_t background = english_picture[y*256 + 111];
            for (int x = 112; x < 184; ++x)
                english_picture[y*256+x] = background;
        }
        int slot = character_menu_v2_highlight(row);
        if (slot >= 0 && (cyc_render_line_mask(top) & 0x10)) {
            const uint8_t *p = oam + slot*4;
            unsigned flags = ((p[2] & 0x40) ? CYC_TILE_HFLIP : 0) |
                             ((p[2] & 0x80) ? CYC_TILE_VFLIP : 0);
            uint16_t pattern = (uint16_t)(cyc_render_line_sprite_table(top) + p[1]*16u);
            for (unsigned i = 0; labels[row][i]; ++i)
                cyc_render_tile(english_picture,256,240,112+(int)i*8,top,
                                pattern,4+(p[2]&3),flags,NULL,NULL);
        }
        for (unsigned i = 0; labels[row][i]; ++i)
            cyc_render_tile(english_picture,256,240,112+(int)i*8,top,
                            (uint16_t)(0x1000+(unsigned char)labels[row][i]*16u),
                            palette,0,NULL,NULL);
    }
}


static int title_romaji_is_present(void)
{
    static const uint8_t logo[4][24] = {
        {0x80,0x82,0x88,0x8A,0x90,0x92,0x98,0x9A,0xA0,0xA2,0xA8,0xAA,0xB0,0xB2,0xB8,0xBA,0xC0,0xC2,0xC8,0xCA,0xD0,0xD2,0xD8,0xDA},
        {0x81,0x83,0x89,0x8B,0x91,0x93,0x99,0x9B,0xA1,0xA3,0xA9,0xAB,0xB1,0xB3,0xB9,0xBB,0xC1,0xC3,0xC9,0xCB,0xD1,0xD3,0xD9,0xDB},
        {0x84,0x86,0x8C,0x8E,0x94,0x96,0x9C,0x9E,0xA4,0xA6,0xAC,0xAE,0xB4,0xB6,0xBC,0xBE,0xC4,0xC6,0xCC,0xCE,0xD4,0xD6,0xDC,0xDE},
        {0x20,0x87,0x8D,0x8F,0x95,0x97,0x9D,0x9F,0xA5,0xA7,0xAD,0xAF,0xB5,0xB7,0xBD,0xBF,0xC5,0xC7,0xCD,0xCF,0xD5,0xD7,0xDD,0xDF}
    };
    static const char player1[] = "PLAYER 1", player2[] = "PLAYER 2";
    static const char copyright_text[] = "TAITO CORP. 1987";
    static const uint8_t letter_k[8] = {0xC6,0xCC,0xD8,0xF0,0xF8,0xDC,0xCE,0x00};
    for (unsigned row = 0; row < 4; ++row)
        for (unsigned col = 0; col < 24; ++col)
            if (cyc_render_nametable((uint16_t)(0x24C4 + row*32 + col)) != logo[row][col]) return 0;
    if (cyc_render_nametable(0x24A5) != 0x85 ||
        cyc_render_nametable(0x2442) != 0x60 ||
        cyc_render_nametable(0x277D) != 0x6F) return 0;
    for (unsigned i = 0; i < 8; ++i)
        if (cyc_render_nametable((uint16_t)(0x25CC + i)) != (uint8_t)player1[i] ||
            cyc_render_nametable((uint16_t)(0x262C + i)) != (uint8_t)player2[i] ||
            cyc_render_chr((uint16_t)(0x14B0 + i)) != letter_k[i]) return 0;
    for (unsigned i = 0; i < sizeof(copyright_text)-1; ++i)
        if (cyc_render_nametable((uint16_t)(0x26A9 + i)) != (uint8_t)copyright_text[i]) return 0;
    int scroll = cyc_render_line_scroll_x(40);
    if (scroll < 0 || scroll > 511) return 0;
    int offset = 255-scroll;
    if (32+offset >= 256 || 228+offset <= 0) return 0;
    for (int y = 40; y < 96; ++y)
        if (cyc_render_line_bg_table(y) != 0x1000 ||
            cyc_render_line_scroll_x(y) != scroll ||
            cyc_render_line_scroll_y(y) != 239+y) return 0;
    return 1;
}

static int title_romaji_colors(const uint32_t *original, uint32_t *face, uint32_t *shadow)
{
    (void)original;
    unsigned found = 0;
    for (unsigned row = 6; row < 10; ++row) {
        for (unsigned col = 4; col < 28; ++col) {
            uint8_t tile = cyc_render_nametable((uint16_t)(0x2400 + row*32 + col));
            unsigned attribute_address = 0x27C0 + (row/4)*8 + col/4;
            unsigned shift = ((row & 2u) << 1) | (col & 2u);
            unsigned palette = (cyc_render_nametable((uint16_t)attribute_address) >> shift) & 3u;
            for (unsigned dy = 0; dy < 8; ++dy) {
                uint16_t pattern = (uint16_t)(0x1000 + tile*16u + dy);
                uint8_t a = cyc_render_chr(pattern), b = cyc_render_chr((uint16_t)(pattern+8));
                for (unsigned dx = 0; dx < 8; ++dx) {
                    unsigned bit = 0x80u >> dx;
                    unsigned value = ((a & bit) ? 1u : 0u) | ((b & bit) ? 2u : 0u);
                    if (value == 2 && !(found & 1)) { *face = cyc_render_color((int)(palette*4+value)); found |= 1; }
                    if (value == 1 && !(found & 2)) { *shadow = cyc_render_color((int)(palette*4+value)); found |= 2; }
                    if (found == 3) return 1;
                }
            }
        }
    }
    return 0;
}

/* JIANGSHI_TITLE_V1: localized title with the requested original-name credit. */
static const uint8_t *title_original_small_glyph(char c)
{
    static const char alphabet[]="kyonshizu2";
    static const uint8_t letters[10][5]={
        {0x7F,0x10,0x28,0x44,0x00}, /* k */
        {0x0C,0x50,0x50,0x50,0x3C}, /* y */
        {0x38,0x44,0x44,0x44,0x38}, /* o */
        {0x7C,0x08,0x04,0x04,0x78}, /* n */
        {0x48,0x54,0x54,0x54,0x20}, /* s */
        {0x7F,0x08,0x04,0x04,0x78}, /* h */
        {0x00,0x44,0x7D,0x40,0x00}, /* i */
        {0x44,0x64,0x54,0x4C,0x44}, /* z */
        {0x3C,0x40,0x40,0x20,0x7C}, /* u */
        {0x42,0x61,0x51,0x49,0x46}  /* 2 */
    };
    static const uint8_t blank[5]={0,0,0,0,0};
    for(unsigned i=0;alphabet[i];i++)if(c==alphabet[i])return letters[i];
    return blank;
}
/* SAVE_SLOT_HIGHLIGHT_V2: follow the game's rendered pink selection. */
static int title_slot_highlight(const uint32_t *original,int label_x,int y,uint32_t *color)
{
    unsigned count=0;
    for(int py=y-1;py<y+8;py++)for(int px=label_x-2;px<label_x+80;px++){
        if(px<0||px>=256||py<0||py>=240)continue;
        uint32_t v=original[py*256+px];
        int r=(int)((v>>16)&255),g=(int)((v>>8)&255),b=(int)(v&255);
        if(r>96&&b>96&&r>g+48&&b>g+48){*color=v;count++;}
    }
    return count>=4;
}

static int title_romaji_draw(const uint32_t *original)
{
    uint32_t face=0,shadow=0;
    if(!title_romaji_colors(original,&face,&shadow))return 0;
    int offset_x=255-cyc_render_line_scroll_x(40);
    int left=32+offset_x,right=228+offset_x;
    if(left<0)left=0;
    if(right>256)right=256;
    if(left>=right)return 0;
    memcpy(english_picture,original,sizeof english_picture);
    uint32_t background=cyc_render_color(0);
    for(int y=40;y<84;y++)for(int x=left;x<right;x++)english_picture[y*256+x]=background;
    int subtitle_clip_left=88+offset_x,subtitle_clip_right=168+offset_x;
    if(subtitle_clip_left<0)subtitle_clip_left=0;
    if(subtitle_clip_right>256)subtitle_clip_right=256;
    for(int y=84;y<96;y++)for(int x=subtitle_clip_left;x<subtitle_clip_right;x++)english_picture[y*256+x]=background;
    static const char title[]="JIANGSHI 2";
    int title_left=(256-(int)(sizeof(title)-1)*16)/2+offset_x;
    for(int pass=0;pass<2;pass++){
        int delta=pass==0?2:0;uint32_t color=pass==0?shadow:face;
        for(unsigned i=0;i<sizeof(title)-1;i++)for(unsigned dy=0;dy<8;dy++){
            uint16_t pattern=(uint16_t)(0x1000+(unsigned char)title[i]*16u+dy);
            uint8_t a=cyc_render_chr(pattern),b=cyc_render_chr((uint16_t)(pattern+8));
            for(unsigned dx=0;dx<8;dx++)if((a|b)&(0x80u>>dx)){
                int gx=title_left+(int)i*16+(int)dx*2+delta,gy=46+(int)dy*4+delta;
                for(int sy=0;sy<4;sy++)for(int sx=0;sx<2;sx++){
                    int x=gx+sx,y=gy+sy;
                    if(x>=left&&x<right&&y>=40&&y<96)english_picture[y*256+x]=color;
                }
            }
        }
    }
    static const char subtitle[]="kyonshiizu 2";
    int subtitle_left=(256-((int)(sizeof(subtitle)-1)*6-1))/2+offset_x;
    for(unsigned i=0;i<sizeof(subtitle)-1;i++){
        const uint8_t *glyph=title_original_small_glyph(subtitle[i]);
        for(int dx=0;dx<5;dx++)for(int dy=0;dy<7;dy++)if(glyph[dx]&(1u<<dy)){
            int x=subtitle_left+(int)i*6+dx,y=86+dy;
            if(x>=subtitle_clip_left&&x<subtitle_clip_right&&y>=84&&y<96)english_picture[y*256+x]=face;
        }
    }
    /* SAVE_SLOT_LABELS_V2: full English-label selection background. */
    static const char *const save_slot_labels[2]={"SAVE SLOT ONE","SAVE SLOT TWO"};
    for(unsigned slot=0;slot<2;slot++){
        int y=113+(int)slot*24;
        int slot_scroll=cyc_render_line_scroll_x(y);
        if(slot_scroll<0||slot_scroll>511||cyc_render_line_bg_table(y)!=0x1000||
           cyc_render_line_scroll_y(y)!=239+y)continue;
        int label_x=97+255-slot_scroll;
        uint32_t selection_color=background;
        int selected=title_slot_highlight(original,label_x,y,&selection_color);
        int erase_left=label_x-2,erase_right=label_x+80;
        if(erase_left<0)erase_left=0;
        if(erase_right>256)erase_right=256;
        if(erase_left>=erase_right)continue;
        for(int py=y-1;py<y+8;py++)for(int px=erase_left;px<erase_right;px++)
            english_picture[py*256+px]=selected?selection_color:background;
        draw_words(label_x,y,save_slot_labels[slot]);
    }
    return 1;
}



static int gameplay_ui_layout(int y)
{
    return cyc_render_line_bg_table(y) == 0x1000 &&
           cyc_render_line_scroll_x(y) == 16 &&
           cyc_render_line_scroll_y(y) == 239+y;
}
static unsigned gameplay_ui_flags(void)
{
    unsigned flags = 0;
    if (gameplay_ui_layout(50) &&
        cyc_render_nametable(0x20D4) == 0xC5 &&
        cyc_render_nametable(0x20D5) == 0xCD &&
        cyc_render_nametable(0x20D6) == 0xCE &&
        cyc_render_nametable(0x20D7) == 0xE0 &&
        cyc_render_nametable(0x20DF) == 0xC5)
        flags |= 1u;
    static const uint8_t heading[4] = {0xFB,0xFC,0xFD,0xFE};
    int header = gameplay_ui_layout(90) &&
        cyc_render_nametable(0x217A) == 0xC5 &&
        cyc_render_nametable(0x217F) == 0xC5;
    for (unsigned i = 0; i < 4; ++i)
        if (cyc_render_nametable((uint16_t)(0x217B + i)) != heading[i]) header = 0;
    if (!header) return flags;
    flags |= 2u;
    static const uint8_t words[5][3] = {
        {0xE2,0xDD,0xD5}, {0xE8,0xFF,0xEF}, {0xDC,0xFF,0xEF},
        {0xF2,0xD8,0xD5}, {0xD5,0xDB,0xEF}
    };
    for (unsigned row = 0; row < 5; ++row) {
        uint16_t address = (uint16_t)(0x21BC + row*0x40);
        int ok = gameplay_ui_layout(106+(int)row*16) &&
                 cyc_render_nametable((uint16_t)(address-2)) == 0xC5 &&
                 cyc_render_nametable((uint16_t)(address+3)) == 0xC5;
        for (unsigned i = 0; i < 3; ++i)
            if (cyc_render_nametable((uint16_t)(address+i)) != words[row][i]) ok = 0;
        if (ok) flags |= 4u << row;
    }
    return flags;
}
static void gameplay_ui_draw(unsigned flags)
{
    if (flags & 1u) {
        clear_text_area(152,48,184,58);
        draw_words(152,50,"MONEY");
    }
    if (flags & 2u) {
        clear_text_area(200,88,232,98);
        draw_words(207,90,"CMD");
    }
    static const char *const labels[5] = {"TALK","LOOK","TAKE","GIVE","DROP"};
    for (unsigned row = 0; row < 5; ++row) {
        if (!(flags & (4u << row))) continue;
        int top = 104+(int)row*16;
        clear_text_area(208,top,232,top+10);
        draw_words(208,top+2,labels[row]);
    }
}


typedef struct { int x, y; } GameplayCannotPopup;
/* ACTION_POPUP_SCROLL_V2: exact frame/text checks, live scanline positioning. */
static unsigned gameplay_cannot_popups(GameplayCannotPopup out[4])
{
    static const uint8_t tiles[36]={
        0xC0,0xC4,0xC4,0xC4,0xC4,0xC1,
        0xC5,0xF4,0xFF,0xFF,0xFF,0xC5,
        0xC5,0xDB,0xCF,0xDD,0xCA,0xC5,
        0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,
        0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,
        0xC2,0xC4,0xC4,0xC4,0xC4,0xC3
    };
    unsigned count=0;
    for(unsigned table=0;table<4&&count<4;table++)
    for(unsigned row=0;row<=24&&count<4;row++)
    for(unsigned col=0;col<=26&&count<4;col++){
        uint16_t address=(uint16_t)(0x2000+table*0x400+row*32+col);
        if(cyc_render_nametable(address)!=0xC0)continue;
        int ok=1;
        for(unsigned r=0;r<6&&ok;r++)for(unsigned c=0;c<6;c++)
            if(cyc_render_nametable((uint16_t)(address+r*32+c))!=tiles[r*6+c]){ok=0;break;}
        if(!ok)continue;
        int world_x=(int)((table&1u)*256+col*8);
        int world_y=(int)((table>>1)*240+row*8);
        for(int top=0;top+48<=240&&count<4;top++){
            if(cyc_render_line_scroll_y(top)!=world_y)continue;
            int scroll=cyc_render_line_scroll_x(top);
            if(scroll<0||scroll>511)continue;
            int left=(world_x-scroll+512)%512;
            if(left+48>256)continue;
            ok=1;
            for(int y=top;y<top+48;y++)
                if(cyc_render_line_bg_table(y)!=0x1000||
                   !(cyc_render_line_mask(y)&8)||
                   cyc_render_line_scroll_x(y)!=scroll||
                   cyc_render_line_scroll_y(y)!=world_y+y-top){ok=0;break;}
            if(!ok)continue;
            out[count].x=left+8;out[count].y=top;count++;
        }
    }
    return count;
}
static void gameplay_cannot_draw(const GameplayCannotPopup *popups, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) {
        int x = popups[i].x, y = popups[i].y;
        clear_text_area(x,y+8,x+32,y+30);
        draw_words(x+1,y+9,"CAN'T");
        draw_words(x+1,y+21,"DO IT");
    }
}


/* COMMAND_PANEL_FAMILY_V2: full frame/text match; scanline-derived positioning. */
typedef struct { const uint8_t *tiles; unsigned height,count; const char *labels[7]; } CommandPanelDefinition;
typedef struct { const CommandPanelDefinition *definition; int x,y; } CommandPanel;
static const uint8_t command_panel_tiles_0[] = {0xC0,0xFF,0xFF,0xFF,0xFF,0xC1,0xC5,0xFB,0xFC,0xFD,0xFE,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xDA,0xCE,0xCB,0xC5,0xC5,0xFF,0xFF,0xFF,0xF4,0xC5,0xC5,0xFF,0xD7,0xCB,0xE3,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xD5,0xDB,0xEF,0xC5,0xC5,0xFF,0xF4,0xFF,0xF4,0xC5,0xC5,0xFF,0xDC,0xCB,0xD0,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xDA,0xEC,0xD3,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xE8,0xFF,0xEF,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xE3,0xDE,0xD9,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC2,0xC4,0xC4,0xC4,0xC4,0xC3};
static const uint8_t command_panel_tiles_1[] = {0xC0,0xFF,0xFF,0xFF,0xFF,0xC1,0xC5,0xFB,0xFC,0xFD,0xFE,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xE2,0xDD,0xD5,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xF2,0xD8,0xD5,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC2,0xC4,0xC4,0xC4,0xC4,0xC3};
static const uint8_t command_panel_tiles_2[] = {0xC0,0xFF,0xFF,0xFF,0xFF,0xC1,0xC5,0xFB,0xFC,0xFD,0xFE,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xE2,0xDD,0xD5,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xE8,0xFF,0xEF,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xDC,0xFF,0xEF,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xF2,0xD8,0xD5,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC5,0xFF,0xD5,0xDB,0xEF,0xC5,0xC5,0xFF,0xFF,0xFF,0xFF,0xC5,0xC2,0xC4,0xC4,0xC4,0xC4,0xC3};
static const CommandPanelDefinition command_panel_definitions[] = {
    {command_panel_tiles_0,18,7,{"USE","EQP","DROP","ITEM","STAT","LOOK","DATE"}},
    {command_panel_tiles_1,8,2,{"TALK","GIVE",NULL,NULL,NULL,NULL,NULL}},
    {command_panel_tiles_2,14,5,{"TALK","LOOK","TAKE","GIVE","DROP",NULL,NULL}}
};
/* COMMAND_PANEL_WRAP_V3: match across the complete 64x60 logical tile grid. */
#include <stdio.h>
static uint8_t command_world_tile(unsigned x,unsigned y)
{
    x%=64;y%=60;
    unsigned table=(y/30)*2+x/32;
    return cyc_render_nametable((uint16_t)(0x2000+table*0x400+(y%30)*32+x%32));
}
static void command_wrap_report(unsigned headings,unsigned exact,unsigned visible)
{
    static unsigned previous=0xFFFFFFFFu,events=0;
    unsigned state=(headings&255u)|((exact&255u)<<8)|((visible&255u)<<16);
    if(state==previous||events>=64)return;
    previous=state;events++;
    FILE *f=fopen("./command_panel_wrap_status.log","a");
    if(!f)return;
    fprintf(f,"WRAP_V3 headings=%u exact=%u visible=%u sx80=%d sy80=%d bg80=%04X mask80=%02X\n",
        headings,exact,visible,cyc_render_line_scroll_x(80),cyc_render_line_scroll_y(80),
        (unsigned)cyc_render_line_bg_table(80),(unsigned)cyc_render_line_mask(80));
    fclose(f);
}
static unsigned command_panel_find_full(CommandPanel out[4])
{
    unsigned count=0,exact=0,headings=0;
    for(unsigned row=0;row<60;row++)for(unsigned col=0;col<64;col++){
        if(command_world_tile(col+1,row+1)==0xFB&&command_world_tile(col+2,row+1)==0xFC&&
           command_world_tile(col+3,row+1)==0xFD&&command_world_tile(col+4,row+1)==0xFE)headings++;
        if(command_world_tile(col,row)!=0xC0)continue;
        for(unsigned d=0;d<sizeof(command_panel_definitions)/sizeof(command_panel_definitions[0]);d++){
            const CommandPanelDefinition *def=&command_panel_definitions[d];int ok=1;
            for(unsigned r=0;r<def->height&&ok;r++)for(unsigned c=0;c<6;c++)
                if(command_world_tile(col+c,row+r)!=def->tiles[r*6+c]){ok=0;break;}
            if(!ok)continue;
            exact++;
            int world_x=(int)col*8,world_y=(int)row*8,height=(int)def->height*8;
            for(int top=0;top+height<=240&&count<4;top++){
                int sy=cyc_render_line_scroll_y(top);
                if(sy<0||sy>=480||sy!=world_y)continue;
                int sx=cyc_render_line_scroll_x(top);
                if(sx<0||sx>=512)continue;
                int left=(world_x-sx+512)%512;if(left+48>256)continue;
                ok=1;
                for(int y=top;y<top+height;y++){
                    int expected_y=(world_y+y-top)%480;
                    if(cyc_render_line_bg_table(y)!=0x1000||!(cyc_render_line_mask(y)&8)||
                       cyc_render_line_scroll_x(y)!=sx||cyc_render_line_scroll_y(y)!=expected_y){ok=0;break;}
                }
                if(!ok)continue;
                out[count].definition=def;out[count].x=left;out[count].y=top;count++;
            }
        }
    }
    command_wrap_report(headings,exact,count);
    return count;
}

static void command_panel_draw_base(const CommandPanel *panels,unsigned count)
{
    for(unsigned i=0;i<count;i++){
        int left=panels[i].x,top=panels[i].y;
        const CommandPanelDefinition *def=panels[i].definition;
        clear_text_area(left+8,top+8,left+40,top+16);
        draw_words(left+15,top+9,"CMD");
        for(unsigned r=0;r<def->count;r++){
            int baseline=top+24+(int)r*16;
            /* Erase the old word and its separate voicing marks, not the cursor column. */
            clear_text_area(left+16,baseline-8,left+40,baseline+8);
            draw_words(left+16,baseline+1,def->labels[r]);
        }
    }
}


/* GROUPED_SUBMENUS_V1: ROM-derived labels with scanline-aware positioning. */
typedef struct { int l,t,r,b,x,y; const char *text; } SubmenuPatch;
typedef struct { const int16_t *tiles; unsigned width,height; const SubmenuPatch *patches; unsigned patch_count,date_suffix; } SubmenuDefinition;
typedef struct { const SubmenuDefinition *definition; int x,y; } SubmenuPanel;
static const int16_t submenu_tiles_0[] = {192,255,255,255,255,255,255,255,255,255,255,193,197,255,255,218,255,236,255,211,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,216,202,238,200,208,255,255,255,255,255,197,197,255,244,255,255,255,255,255,255,255,255,197,197,213,226,235,211,255,255,255,255,255,255,197,197,255,255,244,255,255,255,255,255,255,255,197,197,210,203,209,207,238,200,208,255,255,255,197,197,255,255,244,255,255,255,255,255,255,255,197,197,212,199,227,238,200,208,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,245,245,245,245,245,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_0[] = {{24,0,64,16,24,9,"STATUS"},{8,16,48,32,8,25,"HP"},{8,32,40,48,8,41,"SPEED"},{8,48,64,64,8,57,"ATTACK"},{8,64,56,80,8,73,"DEFENSE"}};
static const int16_t submenu_tiles_1[] = {192,255,255,255,255,255,255,255,255,255,255,193,197,255,255,218,255,236,255,211,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,216,202,238,200,208,255,255,255,255,255,197,197,255,244,255,255,255,255,255,255,255,255,197,197,213,226,235,211,255,255,255,255,255,255,197,197,255,255,244,255,255,255,255,255,255,255,197,197,210,203,209,207,238,200,208,255,255,255,197,197,255,255,244,255,255,255,255,255,255,255,197,197,212,199,227,238,200,208,255,255,255,255,197,197,255,255,255,255,244,255,255,255,255,255,197,197,210,203,203,243,220,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_1[] = {{24,0,64,16,24,9,"STATUS"},{8,16,48,32,8,25,"HP"},{8,32,40,48,8,41,"SPEED"},{8,48,64,64,8,57,"ATTACK"},{8,64,56,80,8,73,"DEFENSE"},{8,80,48,96,8,89,"LUCK"}};
static const int16_t submenu_tiles_2[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,255,255,255,255,255,255,255,255,255,197,197,205,206,224,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_2[] = {{8,8,40,24,8,17,"MONEY"}};
static const int16_t submenu_tiles_3[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,255,255,255,255,255,255,255,255,255,197,197,227,222,217,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_3[] = {{8,8,32,24,8,17,"DAY"}};
static const int16_t submenu_tiles_4[] = {192,255,255,244,255,255,255,244,255,193,197,255,255,220,255,203,255,208,255,197,197,255,255,255,244,255,255,255,255,197,197,255,222,217,212,200,203,227,243,197,197,255,244,255,255,255,244,255,244,197,197,255,228,207,220,255,230,203,208,197,197,255,255,255,255,255,244,255,244,197,197,255,215,225,216,225,220,203,208,197,197,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_4[] = {{24,0,64,16,24,9,"ITEMS"},{16,16,72,32,16,25,"EVERYDAY"},{16,32,72,48,16,41,"GEAR"},{16,48,72,64,16,57,"OTHER"}};
static const int16_t submenu_tiles_5[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,255,255,255,255,255,255,255,255,255,197,197,221,222,234,232,218,206,237,221,202,236,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_5[] = {{8,8,88,32,27,17,"NOTHING"},{0,0,0,0,30,25,"FOUND."}};
static const int16_t submenu_tiles_6[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,255,244,255,255,255,255,255,255,255,197,197,210,210,219,226,218,206,204,221,202,236,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_6[] = {{8,8,88,32,18,17,"CANNOT USE"},{0,0,0,0,33,25,"HERE."}};
static const int16_t submenu_tiles_7[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,255,255,255,255,255,255,255,255,255,197,197,210,243,221,234,225,202,237,221,202,236,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_7[] = {{8,8,88,32,15,17,"NOT WANTED."}};
static const int16_t submenu_tiles_8[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,255,255,244,255,255,255,255,255,255,197,197,210,240,202,212,200,203,234,219,221,202,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_8[] = {{8,8,88,32,12,17,"CANNOT CARRY"},{0,0,0,0,21,25,"ANY MORE."}};
static const int16_t submenu_tiles_9[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,255,255,255,255,255,255,255,255,255,197,197,221,222,206,255,216,238,221,202,236,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_9[] = {{8,8,88,32,21,17,"SOMETHING"},{0,0,0,0,15,25,"IS MISSING."}};
static const int16_t submenu_tiles_10[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,255,244,255,255,255,255,255,255,255,197,197,215,203,227,212,219,239,234,225,226,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,213,219,237,240,221,202,236,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_10[] = {{8,8,88,48,15,17,"CANNOT DROP"},{0,0,0,0,24,25,"EQUIPPED"},{0,0,0,0,30,33,"ITEMS."}};
static const int16_t submenu_tiles_11[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,244,255,255,244,255,255,255,244,255,197,197,206,209,222,243,207,200,203,255,206,255,197,197,255,255,255,255,255,255,255,255,255,244,197,197,210,242,211,240,219,212,231,201,216,215,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_11[] = {{8,8,88,48,15,17,"SHADOW DOLL"},{0,0,0,0,15,25,"WAS BROKEN."}};
static const int16_t submenu_tiles_12[] = {192,196,196,196,196,196,196,196,196,196,196,193,197,255,255,255,255,255,255,244,255,255,255,197,197,255,216,202,238,200,208,206,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,206,202,228,208,212,216,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_12[] = {{8,8,88,48,30,17,"HEALTH"},{0,0,0,0,18,25,"RECOVERED."}};
static const int16_t submenu_tiles_13[] = {192,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,193,197,129,214,243,216,208,255,255,255,255,255,255,255,255,226,255,202,197,197,255,255,255,255,255,255,255,202,202,204,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_13[] = {{8,0,48,16,8,9,"PICK"},{112,0,136,16,112,9,"YES"},{64,8,88,24,64,17,"NO"}};
static const int16_t submenu_tiles_14[] = {192,255,255,255,255,255,255,255,255,255,255,193,197,255,221,222,198,255,218,206,203,245,255,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_14[] = {{16,0,88,16,16,9,"USE WHAT?"}};
static const int16_t submenu_tiles_15[] = {192,255,255,255,255,255,255,255,255,255,255,193,197,255,221,222,198,255,242,216,213,245,255,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_15[] = {{16,0,88,16,16,9,"GIVE WHAT?"}};
static const int16_t submenu_tiles_16[] = {192,255,255,255,255,255,255,255,244,255,255,193,197,255,221,222,198,255,215,203,227,245,255,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_16[] = {{16,0,88,16,16,9,"EQUIP WHAT?"}};
static const int16_t submenu_tiles_17[] = {192,255,255,255,255,255,255,255,255,255,255,193,197,255,221,222,198,255,213,219,239,245,255,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_17[] = {{16,0,88,16,16,9,"DROP WHAT?"}};
static const int16_t submenu_tiles_18[] = {192,255,255,255,255,255,255,255,255,255,255,193,197,255,221,222,198,255,255,203,239,245,255,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_18[] = {{16,0,88,16,16,9,"SELL WHAT?"}};
/* ACQUISITION_POPUP_V1: ROM template 16EE0; retain the item-name row. */
static const int16_t submenu_tiles_acquired[] = {192,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,193,197,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,198,219,222,202,240,216,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,197,255,255,255,255,255,255,255,255,255,255,197,194,196,196,196,196,196,196,196,196,196,196,195};
static const SubmenuPatch submenu_patches_acquired[] = {{8,16,88,32,15,25,"YOU GOT IT."}};

static const SubmenuDefinition submenu_definitions[] = {
{submenu_tiles_acquired,12,8,submenu_patches_acquired,1,0},
{submenu_tiles_0,12,14,submenu_patches_0,5,0},
{submenu_tiles_1,12,14,submenu_patches_1,6,0},
{submenu_tiles_2,12,6,submenu_patches_2,1,0},
{submenu_tiles_3,12,6,submenu_patches_3,1,1},
{submenu_tiles_4,10,10,submenu_patches_4,4,0},
{submenu_tiles_5,12,6,submenu_patches_5,2,0},
{submenu_tiles_6,12,6,submenu_patches_6,2,0},
{submenu_tiles_7,12,6,submenu_patches_7,1,0},
{submenu_tiles_8,12,6,submenu_patches_8,2,0},
{submenu_tiles_9,12,6,submenu_patches_9,2,0},
{submenu_tiles_10,12,8,submenu_patches_10,3,0},
{submenu_tiles_11,12,8,submenu_patches_11,2,0},
{submenu_tiles_12,12,8,submenu_patches_12,2,0},
{submenu_tiles_13,18,6,submenu_patches_13,3,0},
{submenu_tiles_14,12,20,submenu_patches_14,1,0},
{submenu_tiles_15,12,20,submenu_patches_15,1,0},
{submenu_tiles_16,12,20,submenu_patches_16,1,0},
{submenu_tiles_17,12,20,submenu_patches_17,1,0},
{submenu_tiles_18,12,20,submenu_patches_18,1,0}
};
/* SUBMENU_WRAP_V2: logical-grid matching across nametable boundaries. */
static unsigned grouped_submenu_find_full(SubmenuPanel out[8])
{
    unsigned count=0;
    for(unsigned row=0;row<60&&count<8;row++)
    for(unsigned col=0;col<64&&count<8;col++){
        if(command_world_tile(col,row)!=0xC0)continue;
        for(unsigned d=0;d<sizeof(submenu_definitions)/sizeof(submenu_definitions[0])&&count<8;d++){
            const SubmenuDefinition *def=&submenu_definitions[d];
            int ok=1;
            for(unsigned r=0;r<def->height&&ok;r++)for(unsigned c=0;c<def->width;c++){
                int tile=def->tiles[r*def->width+c];
                if(tile>=0&&command_world_tile(col+c,row+r)!=(uint8_t)tile){ok=0;break;}
            }
            if(!ok)continue;
            int world_x=(int)col*8;
            int world_y=(int)row*8;
            int height=(int)def->height*8,width=(int)def->width*8;
            for(int top=0;top+height<=240&&count<8;top++){
                if(cyc_render_line_scroll_y(top)!=world_y)continue;
                int scroll=cyc_render_line_scroll_x(top);
                if(scroll<0||scroll>511)continue;
                int left=(world_x-scroll+512)%512;
                if(left+width>256)continue;
                ok=1;
                for(int y=top;y<top+height;y++)
                    if(cyc_render_line_bg_table(y)!=0x1000||!(cyc_render_line_mask(y)&8)||
                       cyc_render_line_scroll_x(y)!=scroll||
                       cyc_render_line_scroll_y(y)!=(world_y+y-top)%480){ok=0;break;}
                if(!ok)continue;
                out[count].definition=def;out[count].x=left;out[count].y=top;count++;
            }
        }
    }
    return count;
}
static void grouped_submenu_draw_base(const SubmenuPanel *panels,unsigned count)
{
    for(unsigned i=0;i<count;i++){
        const SubmenuDefinition *def=panels[i].definition;
        int x=panels[i].x,y=panels[i].y;
        for(unsigned j=0;j<def->patch_count;j++){
            const SubmenuPatch *p=&def->patches[j];
            if(p->r>p->l&&p->b>p->t)clear_text_area(x+p->l,y+p->t,x+p->r,y+p->b);
            draw_words(x+p->x,y+p->y,p->text);
        }
        /* Date routine ABEC places a native FC day-suffix sprite in the sixth slot.
           Keep all digits; suppress that suffix only when its OAM record agrees. */
        if(def->date_suffix){
            int px=x+80,py=y+16;
            const uint8_t *oam=cyc_render_oam();
            if(oam&&(cyc_render_line_mask(py)&0x10)&&!cyc_render_line_sprite16(py)&&
               cyc_render_line_sprite_table(py)==0){
                for(unsigned slot=0;slot<64;slot++){
                    const uint8_t *p=oam+slot*4;
                    if(p[0]+1==py&&p[1]==0xFC&&p[2]==0&&p[3]==px){
                        clear_text_area(px,py,px+8,py+8);break;
                    }
                }
            }
        }
    }
}


/* SUBMENU_OCCLUSION_V2: validate visible parent tiles; preserve confirmed child windows. */
typedef struct { int l,t,r,b; } SubmenuOccluder;
static SubmenuOccluder submenu_occluders[20];
static unsigned submenu_occluder_count;
static uint32_t submenu_preserved[256*240];
static void submenu_occluder_add(int x,int y,int width,int height)
{
    for(unsigned i=0;i<submenu_occluder_count;i++){
        const SubmenuOccluder *o=&submenu_occluders[i];
        if(o->l==x&&o->t==y&&o->r==x+width&&o->b==y+height)return;
    }
    if(submenu_occluder_count>=20)return;
    SubmenuOccluder *o=&submenu_occluders[submenu_occluder_count++];
    o->l=x;o->t=y;o->r=x+width;o->b=y+height;
}
static int submenu_front_rect(unsigned index,int x,int y,int width,int height)
{
    const SubmenuOccluder *o=&submenu_occluders[index];
    if(o->l==x&&o->t==y&&o->r==x+width&&o->b==y+height)return 0;
    /* These modal UI families open smaller child windows in front of parents. */
    return (o->r-o->l)*(o->b-o->t)<width*height;
}
static int submenu_tile_covered(int px,int py,int x,int y,int width,int height)
{
    for(unsigned i=0;i<submenu_occluder_count;i++){
        const SubmenuOccluder *o=&submenu_occluders[i];
        if(submenu_front_rect(i,x,y,width,height)&&px>=o->l&&py>=o->t&&px+8<=o->r&&py+8<=o->b)return 1;
    }
    return 0;
}
static unsigned grouped_submenu_find(SubmenuPanel out[8])
{
    unsigned count=grouped_submenu_find_full(out);
    submenu_occluder_count=0;
    for(unsigned i=0;i<count;i++)submenu_occluder_add(out[i].x,out[i].y,
        (int)out[i].definition->width*8,(int)out[i].definition->height*8);
    GameplayCannotPopup popups[4];unsigned popup_count=gameplay_cannot_popups(popups);
    for(unsigned i=0;i<popup_count;i++)submenu_occluder_add(popups[i].x-8,popups[i].y,48,48);
    if(submenu_occluder_count){
        for(unsigned pass=0;pass<8&&count<8;pass++){
            unsigned before=count;
            for(unsigned row=0;row<60&&count<8;row++)
            for(unsigned col=0;col<64&&count<8;col++){
                for(unsigned d=0;d<sizeof(submenu_definitions)/sizeof(submenu_definitions[0])&&count<8;d++){
                    const SubmenuDefinition *def=&submenu_definitions[d];
                    if(command_world_tile(col,row)!=0xC0&&
                       command_world_tile(col+def->width-1,row)!=0xC1&&
                       command_world_tile(col+def->width-1,row+def->height-1)!=0xC3)continue;
                    int width=(int)def->width*8,height=(int)def->height*8;
                    int world_x=(int)col*8,world_y=(int)row*8;
                    for(int top=0;top+height<=240&&count<8;top++){
                        if(cyc_render_line_scroll_y(top)!=world_y)continue;
                        int scroll=cyc_render_line_scroll_x(top);
                        if(scroll<0||scroll>511)continue;
                        int left=(world_x-scroll+512)%512;
                        if(left+width>256)continue;
                        int duplicate=0;
                        for(unsigned i=0;i<count;i++)if(out[i].x==left&&out[i].y==top&&out[i].definition==def)duplicate=1;
                        if(duplicate)continue;
                        int ok=1;
                        for(int y=top;y<top+height;y++)if(cyc_render_line_bg_table(y)!=0x1000||
                           !(cyc_render_line_mask(y)&8)||cyc_render_line_scroll_x(y)!=scroll||
                           cyc_render_line_scroll_y(y)!=(world_y+y-top)%480){ok=0;break;}
                        if(!ok)continue;
                        unsigned covered=0,glyphs=0,borders=0;
                        for(unsigned r=0;r<def->height&&ok;r++)for(unsigned c=0;c<def->width;c++){
                            if(submenu_tile_covered(left+(int)c*8,top+(int)r*8,left,top,width,height)){covered++;continue;}
                            int tile=def->tiles[r*def->width+c];
                            if(tile<0)continue;
                            if(command_world_tile(col+c,row+r)!=(uint8_t)tile){ok=0;break;}
                            if(tile>=0xC0&&tile<=0xC5)borders++;
                            else if(tile!=0xFF&&tile!=0xF4)glyphs++;
                        }
                        if(!ok||!covered||glyphs<3||borders<4)continue;
                        out[count].definition=def;out[count].x=left;out[count].y=top;count++;
                        submenu_occluder_add(left,top,width,height);
                    }
                }
            }
            if(count==before)break;
        }
    }
    /* Parent overlays first; smaller modal overlays last. */
    for(unsigned i=0;i<count;i++)for(unsigned j=i+1;j<count;j++)
        if(out[j].definition->width*out[j].definition->height>out[i].definition->width*out[i].definition->height){
            SubmenuPanel p=out[i];out[i]=out[j];out[j]=p;
        }
    return count;
}
static void grouped_submenu_draw(const SubmenuPanel *panels,unsigned count)
{
    for(unsigned i=0;i<count;i++){
        int x=panels[i].x,y=panels[i].y;
        int width=(int)panels[i].definition->width*8,height=(int)panels[i].definition->height*8;
        memcpy(submenu_preserved,english_picture,sizeof submenu_preserved);
        grouped_submenu_draw_base(&panels[i],1);
        for(unsigned j=0;j<submenu_occluder_count;j++){
            if(!submenu_front_rect(j,x,y,width,height))continue;
            const SubmenuOccluder *o=&submenu_occluders[j];
            int l=o->l>x?o->l:x,t=o->t>y?o->t:y;
            int r=o->r<x+width?o->r:x+width,b=o->b<y+height?o->b:y+height;
            for(int py=t;py<b;py++)for(int px=l;px<r;px++)
                english_picture[py*256+px]=submenu_preserved[py*256+px];
        }
    }
}


/* LOCATION_CAPTIONS_V1: complete 90-entry table, source-validated catalog display. */
#include "cyc_core.h"
#include <stdio.h>
#include <stdlib.h>
#define LOCATION_ROOT "./"
typedef struct { unsigned length; uint8_t bytes[11]; } LocationSource;
typedef struct { int present; char line1[21],line2[21]; } LocationTranslation;
typedef struct { unsigned room; int x,y,width; } LocationCaption;
static const LocationSource location_sources[90]={
    {7,{0x24,0x12,0x79,0x4E,0x53,0x55,0x6D}},
    {8,{0x16,0x2D,0x12,0x79,0x4E,0x53,0x55,0x6D}},
    {6,{0x13,0x79,0x4E,0x53,0x55,0x6D}},
    {8,{0x16,0x2E,0x12,0x79,0x4E,0x53,0x55,0x6D}},
    {7,{0x6A,0x44,0x79,0x4A,0x54,0x55,0x6D}},
    {7,{0x66,0x45,0x79,0x4A,0x54,0x55,0x6D}},
    {7,{0x52,0x44,0x79,0x4A,0x54,0x55,0x6D}},
    {7,{0x46,0x6D,0x79,0x4A,0x54,0x55,0x6D}},
    {6,{0x76,0x79,0x5E,0x4A,0x55,0x6D}},
    {10,{0x76,0x79,0x5E,0x4A,0x55,0x6D,0x45,0x67,0xCA,0x53}},
    {7,{0x26,0x12,0x79,0x5E,0x4A,0x55,0x6D}},
    {11,{0x26,0x12,0x79,0x5E,0x4A,0x55,0x6D,0x45,0x67,0xCA,0x53}},
    {8,{0x48,0x52,0x57,0x65,0x79,0x13,0x77,0x12}},
    {9,{0x48,0x52,0x57,0x65,0x79,0x19,0x2E,0x12,0x1F}},
    {7,{0x73,0x11,0x79,0x0F,0xDE,0x67,0x65}},
    {7,{0x13,0x77,0x79,0x0F,0xDE,0x67,0x65}},
    {9,{0x20,0x2B,0x73,0x77,0x79,0xC9,0x6D,0x4C,0x45}},
    {10,{0x75,0x12,0x16,0x2D,0x12,0x79,0xC9,0x6D,0x4C,0x45}},
    {9,{0x16,0x2D,0x14,0x79,0x4A,0x4F,0x68,0x55,0x6D}},
    {8,{0x13,0x11,0x79,0x4A,0x4F,0x68,0x55,0x6D}},
    {5,{0xDD,0x42,0x45,0x44,0x6D}},
    {10,{0x23,0x12,0x79,0x4E,0x42,0x4A,0x5D,0x6D,0x55,0x6D}},
    {10,{0x1A,0x11,0x79,0x4E,0x42,0x4A,0x5D,0x6D,0x55,0x6D}},
    {11,{0x4E,0x42,0x45,0x4B,0x6D,0x79,0xD6,0x45,0xCE,0x42,0x45}},
    {10,{0x4D,0x44,0x4B,0x6D,0x79,0xD6,0x45,0xCE,0x42,0x45}},
    {11,{0x53,0x42,0x45,0x4B,0x6D,0x79,0xD6,0x45,0xCE,0x42,0x45}},
    {10,{0x5E,0x43,0x4B,0x6D,0x79,0xD6,0x45,0xCE,0x42,0x45}},
    {9,{0x98,0x11,0x78,0x12,0x79,0x4D,0x4B,0x55,0x6D}},
    {7,{0x75,0x12,0x79,0x4D,0x4B,0x55,0x6D}},
    {9,{0xA0,0x16,0x2E,0x12,0x79,0x4D,0x4B,0x55,0x6D}},
    {7,{0x25,0x77,0x79,0x4D,0x4B,0x55,0x6D}},
    {7,{0x73,0x11,0x28,0x77,0x79,0x55,0x44}},
    {7,{0x12,0x77,0x1C,0x77,0x79,0x55,0x44}},
    {7,{0x1B,0x77,0x75,0x12,0x79,0x55,0x44}},
    {7,{0x29,0x12,0x27,0x77,0x79,0x55,0x44}},
    {8,{0x16,0x17,0x77,0x79,0x5C,0x6D,0x55,0x6D}},
    {9,{0x13,0x77,0x1B,0x77,0x79,0x5C,0x6D,0x55,0x6D}},
    {8,{0x22,0x72,0x77,0x79,0x5C,0x6D,0x55,0x6D}},
    {10,{0x16,0x2C,0x77,0x1D,0x11,0x79,0x5C,0x6D,0x55,0x6D}},
    {7,{0x16,0x2D,0x77,0x79,0x5C,0x57,0x65}},
    {5,{0x13,0x79,0x5C,0x57,0x65}},
    {7,{0x16,0x2D,0x12,0x79,0x5C,0x57,0x65}},
    {6,{0x13,0x77,0x79,0x48,0x4E,0x65}},
    {7,{0x96,0x2D,0x77,0x79,0x48,0x4E,0x65}},
    {6,{0x15,0x11,0x79,0x48,0x4E,0x65}},
    {10,{0x5C,0x69,0x5B,0x65,0x79,0x01,0xCC,0x45,0x55,0x6D}},
    {10,{0x5C,0x69,0x5B,0x65,0x79,0x02,0xCC,0x45,0x55,0x6D}},
    {10,{0x5C,0x69,0x5B,0x65,0x79,0x03,0xCC,0x45,0x55,0x6D}},
    {10,{0x5C,0x69,0x5B,0x65,0x79,0x04,0xCC,0x45,0x55,0x6D}},
    {8,{0x45,0x67,0x57,0x44,0x5B,0x79,0x62,0x50}},
    {7,{0x66,0x45,0x63,0x44,0x6D,0x4E,0x54}},
    {7,{0x94,0x27,0x77,0x78,0x23,0x1A,0x74}},
    {6,{0x21,0x27,0x95,0x23,0x1A,0x74}},
    {5,{0x61,0x53,0x65,0x4A,0xDC}},
    {10,{0x4C,0x45,0x4E,0x42,0x45,0x58,0x6D,0x65,0x4A,0xDC}},
    {9,{0x9D,0x14,0x19,0xDC,0x79,0x19,0x77,0x1E,0x77}},
    {9,{0x9D,0x14,0x19,0xDC,0x79,0x23,0x77,0x1E,0x77}},
    {8,{0x4B,0x44,0x48,0x6D,0x54,0x10,0x4E,0x42}},
    {8,{0x4B,0x44,0x48,0x6D,0x54,0x10,0x4E,0x42}},
    {8,{0x4B,0x44,0x48,0x6D,0x54,0x10,0x4E,0x42}},
    {9,{0x75,0x12,0x29,0x79,0xCB,0x49,0xCE,0x42,0x45}},
    {10,{0x17,0x11,0x28,0x77,0x79,0x46,0x44,0xC8,0x48,0x6D}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x62,0x6D,0x48}},
    {3,{0x47,0x55,0x67}},
    {4,{0x48,0xDC,0x6D,0x65}},
    {9,{0x5C,0x57,0x6A,0xCE,0x61,0x5B,0x47,0xD6,0x45}},
    {3,{0x44,0x53,0xDC}},
    {3,{0x44,0x53,0xDC}},
    {3,{0x65,0x47,0x65}},
    {3,{0x00,0x00,0x00}},
    {3,{0x51,0x45,0x4C}},
    {3,{0x51,0x45,0x4C}},
    {5,{0xDE,0x6D,0xCA,0x55,0x6D}},
    {6,{0x4B,0x44,0x4D,0x54,0x4E,0x42}},
    {6,{0x4B,0x44,0x4D,0x54,0x4E,0x42}},
    {4,{0x52,0xDC,0x4C,0x65}},
    {4,{0x52,0xDC,0x4C,0x65}},
    {7,{0x56,0x45,0x5B,0x44,0x68,0xCA,0x53}},
    {7,{0x56,0x45,0x5B,0x44,0x68,0xCA,0x53}},
    {7,{0x56,0x45,0x5B,0x44,0x68,0xCA,0x53}},
};
static LocationTranslation location_catalog[90];
static unsigned location_catalog_initialized;
static uint8_t location_reported[90][3];
static int location_supported_line(const char *s)
{
    if(strlen(s)>20)return 0;
    for(;*s;s++)if(!(*s>='A'&&*s<='Z')&&!strchr(" .,?",*s))return 0;
    return 1;
}
static void location_load_catalog(void)
{
    if(location_catalog_initialized)return;
    location_catalog_initialized=1;
    FILE *log=fopen(LOCATION_ROOT "location_catalog_status.log","w");
    FILE *f=fopen(LOCATION_ROOT "location_translations.tsv","r");
    if(!f){if(log){fprintf(log,"STOP: location_translations.tsv could not be opened.\n");fclose(log);}return;}
    unsigned loaded=0,rejected=0,number=0;char line[256];
    while(fgets(line,sizeof line,f)){
        number++;
        if(number==1&&(unsigned char)line[0]==0xEF&&(unsigned char)line[1]==0xBB&&(unsigned char)line[2]==0xBF)
            memmove(line,line+3,strlen(line+3)+1);
        if(!strchr(line,'\n')&&!feof(f)){int ch;while((ch=fgetc(f))!='\n'&&ch!=EOF){}rejected++;continue;}
        line[strcspn(line,"\r\n")]=0;
        if(!line[0]||line[0]=='#')continue;
        char *first=strchr(line,'\t');if(!first){rejected++;continue;}*first++=0;
        char *second=strchr(first,'\t');if(!second){rejected++;continue;}*second++=0;
        size_t idlen=strlen(line);
        if(!idlen||strspn(line,"0123456789")!=idlen||strchr(second,'\t')){rejected++;continue;}
        unsigned long room=strtoul(line,NULL,10);
        if(room<1||room>90||room==80||location_catalog[room-1].present||!first[0]||
           !location_supported_line(first)||!location_supported_line(second)){rejected++;continue;}
        LocationTranslation *t=&location_catalog[room-1];t->present=1;
        strcpy(t->line1,first);strcpy(t->line2,second);loaded++;
    }
    fclose(f);
    if(log){fprintf(log,"LOADED %u entries; REJECTED %u entries.\nIndex 80 is the reserved 000 placeholder.\n",loaded,rejected);fclose(log);}
}
static void location_report(unsigned room,unsigned reason)
{
    if(room<1||room>90||reason>=3||location_reported[room-1][reason])return;
    location_reported[room-1][reason]=1;
    static const char *const reasons[]={"missing_catalog_entry","reserved_000_placeholder","native_caption_mismatch"};
    FILE *f=fopen(LOCATION_ROOT "location_caption_coverage.tsv","a");
    if(f){fprintf(f,"%u\t%s\n",room,reasons[reason]);fclose(f);}
}
static int location_match_word(uint16_t base,unsigned row,unsigned col,const uint8_t *raw,unsigned length)
{
    for(unsigned i=0;i<length;i++){
        unsigned tile=(raw[i]&0x7Fu)+0x80u;if(tile>=0xC0u)tile+=6;
        if(tile>255||cyc_render_nametable((uint16_t)(base+row*32+col+i))!=(uint8_t)tile||
           cyc_render_nametable((uint16_t)(base+(row-1)*32+col+i))!=((raw[i]&0x80u)?0xF4:0xFF))return 0;
    }
    return 1;
}
static int location_caption_find(LocationCaption *out)
{
    location_load_catalog();
    const uint8_t *ram=cyc_cpu_ram();if(!ram)return 0;
    unsigned room=ram[0x6A];if(room<1||room>90)return 0;
    const LocationSource *source=&location_sources[room-1];
    static const uint8_t here[4]={0x4C,0x4C,0x5C,0x0A};
    uint8_t text[16];unsigned length=source->length+5;
    if(length>sizeof text)return 0;
    text[0]=0x0D;memcpy(text+1,source->bytes,source->length);
    text[source->length+1]=0x0E;text[source->length+2]=0xD5;
    text[source->length+3]=0x4F;text[source->length+4]=0x0B;
    for(unsigned table=0;table<4;table++){
        uint16_t base=(uint16_t)(0x2000+table*0x400);
        if(!location_match_word(base,4,4,here,4))continue;
        int world_y=(int)((table>>1)*240+32);
        int world_x=(int)((table&1u)*256+32);
        for(int top=8;top+24<=240;top++){
            if(cyc_render_line_scroll_y(top)!=world_y)continue;
            int scroll=cyc_render_line_scroll_x(top);
            if(scroll<0||scroll>511)continue;
            int left=(world_x-scroll+512)%512;
            int width=(int)length*8;if(width<120)width=120;
            if(left+width>256)continue;
            int ok=1;
            for(int y=top-8;y<top+24;y++)if(cyc_render_line_bg_table(y)!=0x1000||
               !(cyc_render_line_mask(y)&8)||cyc_render_line_scroll_x(y)!=scroll||
               cyc_render_line_scroll_y(y)!=world_y+y-top){ok=0;break;}
            if(!ok)continue;
            if(!location_match_word(base,6,4,text,length)){location_report(room,2);continue;}
            if(room==80){location_report(room,1);return 0;}
            if(!location_catalog[room-1].present){location_report(room,0);return 0;}
            out->room=room;out->x=left;out->y=top;out->width=width;return 1;
        }
    }
    return 0;
}
static void location_caption_draw(const LocationCaption *caption)
{
    const LocationTranslation *text=&location_catalog[caption->room-1];
    clear_text_area(caption->x,caption->y-8,caption->x+caption->width,caption->y+24);
    draw_words(caption->x,caption->y+1,text->line1);
    if(text->line2[0])draw_words(caption->x,caption->y+17,text->line2);
}


/* ITEM_TABLE_DISPLAY_V1: item names, prices and covered command panels. */
#include <stdio.h>
#include <stdlib.h>
#define ITEM_ROOT "./"
typedef struct { unsigned length; uint8_t tiles[9],marks[9]; } NativeItemName;
typedef struct { unsigned table,row,col,width,height; int x,y; } ItemWindow;
typedef struct { unsigned id,width; int x,y; } ItemNameMatch;
typedef struct { int x,y; } ItemCoinMatch;
static const NativeItemName native_item_names[62]={
 {8,{210,221,208,213,238,255,255,255},{255,255,244,255,255,255,255,255}},
 {8,{205,228,216,255,255,255,255,255},{255,255,244,255,255,255,255,255}},
 {8,{216,231,210,216,243,255,255,255},{255,255,244,244,255,255,255,255}},
 {8,{206,204,243,227,243,255,255,255},{255,255,255,244,255,255,255,255}},
 {8,{208,213,238,255,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{212,200,208,238,200,203,255,255},{255,255,255,255,255,255,255,255}},
 {8,{227,225,210,228,255,255,255,255},{255,255,244,255,255,255,255,255}},
 {8,{218,217,225,210,228,255,255,255},{255,255,255,244,255,255,255,255}},
 {8,{232,213,225,210,228,255,255,255},{255,244,255,244,255,255,255,255}},
 {8,{206,214,225,210,228,255,255,255},{255,244,255,244,255,255,255,255}},
 {8,{209,243,228,225,210,228,255,255},{244,255,244,255,244,255,255,255}},
 {8,{227,235,201,210,225,210,228,255},{244,255,255,255,255,244,255,255}},
 {8,{214,202,238,199,203,225,210,228},{255,255,255,255,255,255,244,255}},
 {8,{213,211,208,225,210,228,255,255},{255,244,255,255,244,255,255,255}},
 {8,{231,220,203,212,225,228,208,255},{255,244,255,255,255,255,255,255}},
 {9,{220,208,229,218,240,202,225,228,208},{255,255,244,255,255,255,255,255,255}},
 {8,{240,202,206,202,234,243,255,255},{255,255,255,255,255,255,255,255}},
 {8,{216,202,240,202,206,202,234,243},{244,255,255,255,255,255,255,255}},
 {8,{209,212,200,203,220,203,208,255},{255,255,255,255,244,255,244,255}},
 {8,{208,218,255,255,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{212,200,203,206,226,243,255,255},{255,255,255,255,244,255,255,255}},
 {8,{217,199,203,206,226,243,255,255},{255,255,255,255,244,255,255,255}},
 {8,{216,202,206,226,243,255,255,255},{244,255,255,244,255,255,255,255}},
 {8,{206,209,222,243,207,200,203,255},{255,244,255,255,244,255,255,255}},
 {8,{213,202,212,200,203,216,231,255},{255,255,255,255,255,244,255,255}},
 {8,{213,202,241,225,217,213,255,255},{255,255,255,255,255,244,255,255}},
 {8,{217,230,212,243,225,210,228,255},{255,244,255,255,255,244,255,255}},
 {8,{218,203,210,203,219,206,216,255},{255,255,255,255,255,244,255,255}},
 {8,{247,246,250,255,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{236,241,202,255,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{231,230,203,225,216,219,255,255},{255,255,255,255,255,255,255,255}},
 {8,{231,230,203,225,218,239,207,255},{255,255,255,255,255,255,244,255}},
 {8,{212,238,216,216,207,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{214,202,238,199,203,220,203,255},{255,255,255,255,255,255,255,255}},
 {8,{206,216,221,255,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{212,200,203,227,200,203,255,255},{244,255,255,244,255,255,255,255}},
 {8,{211,243,214,218,210,243,255,255},{255,255,255,255,255,255,255,255}},
 {8,{235,238,255,255,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{216,243,209,243,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{230,203,255,255,255,255,255,255},{244,255,255,255,255,255,255,255}},
 {8,{241,203,215,208,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{212,200,208,216,202,255,255,255},{255,255,255,244,255,255,255,255}},
 {8,{219,217,200,203,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{249,246,248,255,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{226,221,216,226,255,255,255,255},{255,255,255,244,255,255,255,255}},
 {9,{207,200,243,212,246,231,211,233,224},{255,255,255,255,255,255,255,255,255}},
 {8,{210,203,206,255,255,255,255,255},{255,255,244,255,255,255,255,255}},
 {8,{207,216,226,206,226,255,255,255},{255,255,255,255,244,255,255,255}},
 {8,{236,203,213,210,203,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{230,212,206,212,255,255,255,255},{255,255,244,255,255,255,255,255}},
 {8,{226,221,242,255,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{205,206,212,255,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{222,243,212,243,255,255,255,255},{255,255,244,255,255,255,255,255}},
 {8,{235,211,202,208,213,255,255,255},{255,255,255,255,244,255,255,255}},
 {8,{208,213,238,255,255,129,210,255},{255,255,255,255,255,255,255,255}},
 {8,{208,213,238,255,129,128,210,255},{255,255,255,255,255,255,255,255}},
 {8,{212,199,203,231,202,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{235,207,215,226,255,255,255,255},{255,255,255,244,255,255,255,255}},
 {8,{153,172,246,157,253,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{220,238,222,208,255,255,255,255},{255,255,255,255,255,255,255,255}},
 {8,{210,202,225,231,239,143,209,255},{255,255,255,255,255,255,244,255}},
 {8,{228,206,227,240,255,255,255,255},{255,255,255,255,255,255,255,255}},
};
static char item_labels[62][18];
static unsigned item_catalog_initialized;
static ItemWindow item_windows[16];static unsigned item_window_count;
static ItemNameMatch item_matches[48];static unsigned item_match_count;
static ItemCoinMatch item_coins[24];static unsigned item_coin_count;
static uint32_t item_preserved[256*240];
static void item_catalog_load(void)
{
    if(item_catalog_initialized)return;
    item_catalog_initialized=1;
    FILE *f=fopen(ITEM_ROOT "item_translations.tsv","r");
    FILE *log=fopen(ITEM_ROOT "item_catalog_status.log","w");
    if(!f){if(log){fprintf(log,"STOP: item_translations.tsv could not be opened.\n");fclose(log);}return;}
    char line[128];unsigned loaded=0,rejected=0,number=0;
    while(fgets(line,sizeof line,f)){
        number++;
        if(number==1&&(unsigned char)line[0]==0xEF&&(unsigned char)line[1]==0xBB&&(unsigned char)line[2]==0xBF)memmove(line,line+3,strlen(line+3)+1);
        if(!strchr(line,'\n')&&!feof(f)){int ch;while((ch=fgetc(f))!='\n'&&ch!=EOF){}rejected++;continue;}
        line[strcspn(line,"\r\n")]=0;if(!line[0]||line[0]=='#')continue;
        char *name=strchr(line,'\t');if(!name){rejected++;continue;}*name++=0;
        size_t n=strlen(line);int ok=n&&n<=2&&strspn(line,"0123456789ABCDEFabcdef")==n;
        unsigned long id=ok?strtoul(line,NULL,16):62;
        if(id>=62||item_labels[id][0]||!name[0]||(strlen(name)>17||strlen(name)*4-1>native_item_names[id].length*8)){rejected++;continue;}
        for(const char *p=name;*p;p++)if(!(*p>='A'&&*p<='Z')&&!(*p>='0'&&*p<='9')&&*p!=' ')ok=0;
        if(!ok){rejected++;continue;}strcpy(item_labels[id],name);loaded++;
    }
    fclose(f);if(log){fprintf(log,"LOADED %u entries; REJECTED %u entries.\n",loaded,rejected);fclose(log);}
}
/* ITEM_WINDOW_WRAP_V2: shared 64x60 grid access for item windows. */
static uint8_t item_window_tile(const ItemWindow *w,int r,int c)
{
    unsigned x=(w->table&1u)*32+w->col;
    unsigned y=(w->table>>1)*30+w->row;
    return command_world_tile((unsigned)((int)x+c+64)%64,(unsigned)((int)y+r+60)%60);
}

static int item_window_position(unsigned table,unsigned row,unsigned col,unsigned width,unsigned height,ItemWindow *out)
{
    int wx=(int)((table&1u)*256+col*8),wy=(int)((table>>1)*240+row*8);
    for(int top=0;top+(int)height*8<=240;top++){
        if(cyc_render_line_scroll_y(top)!=wy)continue;
        int sx=cyc_render_line_scroll_x(top);
        if(sx<0||sx>=512)continue;
        int left=(wx-sx+512)%512;
        if(left+(int)width*8>256)continue;
        int ok=1;
        for(int y=top;y<top+(int)height*8;y++)
            if(cyc_render_line_bg_table(y)!=0x1000||!(cyc_render_line_mask(y)&8)||
               cyc_render_line_scroll_x(y)!=sx||cyc_render_line_scroll_y(y)!=(wy+y-top)%480){ok=0;break;}
        if(ok){*out=(ItemWindow){table,row,col,width,height,left,top};return 1;}
    }
    return 0;
}
static void item_collect_windows_full(void)
{
    item_window_count=0;
    for(unsigned y=0;y<60&&item_window_count<16;y++)for(unsigned x=0;x<64&&item_window_count<16;x++){
        if(command_world_tile(x,y)!=0xC0)continue;
        for(unsigned width=4;width<=32&&item_window_count<16;width++){
            if(command_world_tile(x+width-1,y)!=0xC1)continue;
            int ok=1;
            for(unsigned c=1;c<width-1;c++){
                unsigned t=command_world_tile(x+c,y);
                if(t!=0xFF&&t!=0xC4&&t!=0xF4){ok=0;break;}
            }
            if(!ok)continue;
            for(unsigned height=4;height<=30&&item_window_count<16;height++){
                if(command_world_tile(x,y+height-1)!=0xC2||command_world_tile(x+width-1,y+height-1)!=0xC3)continue;
                ok=1;
                for(unsigned c=1;c<width-1;c++)if(command_world_tile(x+c,y+height-1)!=0xC4){ok=0;break;}
                for(unsigned r=1;r<height-1&&ok;r++)if(command_world_tile(x,y+r)!=0xC5||command_world_tile(x+width-1,y+r)!=0xC5)ok=0;
                if(ok&&item_window_position((y/30)*2+x/32,y%30,x%32,width,height,&item_windows[item_window_count]))item_window_count++;
            }
        }
    }
}

/* ITEM_MENU_OCCLUSION_V1: validate parent borders around confirmed children. */
static int item_child_covers(int px,int py,const ItemWindow *parent)
{
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];
        if(w->width*w->height>=parent->width*parent->height)continue;
        if(px>=w->x&&py>=w->y&&px+8<=w->x+(int)w->width*8&&py+8<=w->y+(int)w->height*8)return 1;
    }
    return 0;
}
static void item_collect_windows(void)
{
    item_collect_windows_full();
    for(unsigned pass=0;pass<4&&item_window_count<16;pass++){
        unsigned before=item_window_count;
        for(unsigned y=0;y<60&&item_window_count<16;y++)for(unsigned x=0;x<64&&item_window_count<16;x++){
            if(command_world_tile(x,y)!=0xC0)continue;
            for(unsigned width=4;width<=32&&item_window_count<16;width++){
                if(command_world_tile(x+width-1,y)!=0xC1)continue;
                int top_ok=1;
                for(unsigned c=1;c<width-1;c++){
                    unsigned t=command_world_tile(x+c,y);
                    if(t!=0xFF&&t!=0xF4&&t!=0xC4){top_ok=0;break;}
                }
                if(!top_ok)continue;
                for(unsigned height=4;height<=30&&item_window_count<16;height++){
                    ItemWindow p;
                    if(!item_window_position((y/30)*2+x/32,y%30,x%32,width,height,&p))continue;
                    int duplicate=0;
                    for(unsigned i=0;i<item_window_count;i++)if(item_windows[i].x==p.x&&item_windows[i].y==p.y&&item_windows[i].width==width&&item_windows[i].height==height)duplicate=1;
                    if(duplicate)continue;
                    int ok=1;unsigned covered=0,visible=0,left=0,right=0,bottom=0;
                    for(unsigned r=1;r<height&&ok;r++)for(unsigned c=0;c<width;c++){
                        if(r!=height-1&&c!=0&&c!=width-1)continue;
                        if(item_child_covers(p.x+(int)c*8,p.y+(int)r*8,&p)){covered++;continue;}
                        unsigned want=r==height-1?(c==0?0xC2:c==width-1?0xC3:0xC4):0xC5;
                        if(command_world_tile(x+c,y+r)!=want){ok=0;break;}
                        visible++;if(c==0)left++;if(c==width-1)right++;if(r==height-1)bottom++;
                    }
                    if(ok&&covered&&visible>=8&&left>=3&&right>=3&&bottom>=2)item_windows[item_window_count++]=p;
                }
            }
        }
        if(item_window_count==before)break;
    }
}
static void item_restore_child_pixels(int l,int t,int r,int b)
{
    unsigned area=0xFFFFFFFFu;
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];
        if(l>=w->x&&t>=w->y&&r<=w->x+(int)w->width*8&&b<=w->y+(int)w->height*8&&w->width*w->height<area)area=w->width*w->height;
    }
    if(area==0xFFFFFFFFu)return;
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];if(w->width*w->height>=area)continue;
        int left=l>w->x?l:w->x,top=t>w->y?t:w->y;
        int right=r<w->x+(int)w->width*8?r:w->x+(int)w->width*8;
        int bottom=b<w->y+(int)w->height*8?b:w->y+(int)w->height*8;
        for(int y=top;y<bottom;y++)for(int x=left;x<right;x++)english_picture[y*256+x]=item_preserved[y*256+x];
    }
}

static void item_collect_names(void)
{
    item_catalog_load();item_match_count=item_coin_count=0;
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];
        uint16_t a=(uint16_t)(0x2000+w->table*1024+w->row*32+w->col);
        for(unsigned r=1;r<w->height-1;r++)for(unsigned c=1;c<w->width-1;c++){
            uint16_t pos=(uint16_t)(a+r*32+c);
            for(unsigned id=0;id<62&&item_match_count<48;id++){
                const NativeItemName *name=&native_item_names[id];
                if(!item_labels[id][0]||c+name->length>w->width-1)continue;
                int ok=1;
                for(unsigned j=0;j<name->length;j++)if(item_window_tile(w,(int)r,(int)(c+j))!=name->tiles[j]||
                    item_window_tile(w,(int)r-1,(int)(c+j))!=name->marks[j]){ok=0;break;}
                if(!ok)continue;
                int x=w->x+(int)c*8,y=w->y+(int)r*8;int duplicate=0;
                for(unsigned j=0;j<item_match_count;j++)if(item_matches[j].x==x&&item_matches[j].y==y)duplicate=1;
                if(!duplicate){ItemNameMatch *m=&item_matches[item_match_count++];m->id=id;m->x=x;m->y=y;m->width=name->length*8;}
            }
            if(item_coin_count<24&&c+2<=w->width-1&&item_window_tile(w,(int)r,(int)c)==0xB8&&item_window_tile(w,(int)r,(int)c+1)==0xB9){
                unsigned digits=0;
                for(unsigned j=1;j<=6&&j<c;j++){
                    unsigned v=item_window_tile(w,(int)r,(int)c-(int)j);
                    if(v>=0x80&&v<=0x89)digits++;else if(v!=0xFF)break;
                }
                if(digits){ItemCoinMatch *m=&item_coins[item_coin_count++];m->x=w->x+(int)c*8;m->y=w->y+(int)r*8;}
            }
        }
    }
}
static int item_cell_covered(int px,int py,int left,int top,int width,int height)
{
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];int r=w->x+(int)w->width*8,b=w->y+(int)w->height*8;
        if(w->x==left&&w->y==top&&r==left+width&&b==top+height)continue;
        if(px>=w->x&&py>=w->y&&px+8<=r&&py+8<=b)return 1;
    }
    return 0;
}
static unsigned command_panel_find(CommandPanel out[4])
{
    item_collect_windows();item_collect_names();
    unsigned count=command_panel_find_full(out);
    if(!item_window_count)return count;
    for(unsigned table=0;table<4&&count<4;table++)for(unsigned row=0;row<30&&count<4;row++)
    for(unsigned col=0;col<=26&&count<4;col++)for(unsigned d=0;d<3&&count<4;d++){
        const CommandPanelDefinition *def=&command_panel_definitions[d];if(row+def->height>30)continue;
        uint16_t a=(uint16_t)(0x2000+table*1024+row*32+col);
        if(cyc_render_nametable(a)!=0xC0&&cyc_render_nametable((uint16_t)(a+5))!=0xC1&&
           cyc_render_nametable((uint16_t)(a+(def->height-1)*32+5))!=0xC3)continue;
        ItemWindow position;if(!item_window_position(table,row,col,6,def->height,&position))continue;
        int duplicate=0;for(unsigned j=0;j<count;j++)if(out[j].definition==def&&out[j].x==position.x&&out[j].y==position.y)duplicate=1;
        if(duplicate)continue;
        int ok=1;unsigned covered=0,glyphs=0,borders=0;
        for(unsigned r=0;r<def->height&&ok;r++)for(unsigned c=0;c<6;c++){
            if(item_cell_covered(position.x+(int)c*8,position.y+(int)r*8,position.x,position.y,48,(int)def->height*8)){covered++;continue;}
            unsigned tile=def->tiles[r*6+c];if(cyc_render_nametable((uint16_t)(a+r*32+c))!=tile){ok=0;break;}
            if(tile>=0xC0&&tile<=0xC5)borders++;else if(tile!=0xFF&&tile!=0xF4)glyphs++;
        }
        if(ok&&covered&&glyphs>=3&&borders>=4){out[count].definition=def;out[count].x=position.x;out[count].y=position.y;count++;}
    }
    return count;
}
static void command_panel_draw(const CommandPanel *panels,unsigned count)
{
    for(unsigned i=0;i<count;i++){
        memcpy(item_preserved,english_picture,sizeof item_preserved);
        command_panel_draw_base(&panels[i],1);
        int left=panels[i].x,top=panels[i].y,right=left+48,bottom=top+(int)panels[i].definition->height*8;
        for(unsigned j=0;j<item_window_count;j++){
            const ItemWindow *w=&item_windows[j];int wr=w->x+(int)w->width*8,wb=w->y+(int)w->height*8;
            if(w->x==left&&w->y==top&&wr==right&&wb==bottom)continue;
            int l=w->x>left?w->x:left,t=w->y>top?w->y:top,r=wr<right?wr:right,b=wb<bottom?wb:bottom;
            for(int y=t;y<b;y++)for(int x=l;x<r;x++)english_picture[y*256+x]=item_preserved[y*256+x];
        }
    }
}
static void item_compact_words(int x,int y,const char *text)
{
    static const uint16_t letters[26]={11245,27566,14627,27502,31143,31140,14699,23533,29847,4714,23469,18727,24557,24573,11114,27556,11131,27565,14478,29842,23407,23402,23549,23213,23186,29351};
    /* ITEM_COMPACT_DIGITS_V1: 3x5 numerals, existing four-pixel advance. */
    static const uint16_t digits[10]={31599,11415,29671,29647,23497,31183,31215,29257,31727,31695};
    for(;*text;text++,x+=4){
        unsigned bits;
        if(*text>='A'&&*text<='Z')bits=letters[*text-'A'];
        else if(*text>='0'&&*text<='9')bits=digits[*text-'0'];
        else continue;
        for(unsigned r=0;r<5;r++)for(unsigned c=0;c<3;c++)
            if(bits&(1u<<(14-r*3-c)))english_picture[(y+(int)r)*256+x+(int)c]=UINT32_C(0xFFFFFFFF);
    }
}
/* ITEM_OBTAINED_V1: shared acquisition suffix, display only. */
static int item_obtained(int draw)
{
    static const uint8_t text[6]={0xC6,0xDB,0xDE,0xCA,0xF0,0xD8};
    int found=0;
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];
        for(unsigned r=2;r+1<w->height;r++)for(unsigned c=1;c+7<w->width;c++){
            uint16_t a=(uint16_t)(0x2000+w->table*1024+(w->row+r)*32+w->col+c);
            int ok=1;
            for(unsigned j=0;j<6;j++)
                if(item_window_tile(w,(int)r,(int)(c+j))!=text[j]||
                   item_window_tile(w,(int)r-1,(int)(c+j))!=0xFF){ok=0;break;}
            if(!ok||item_window_tile(w,(int)r,(int)c+6)!=0xFF||
               item_window_tile(w,(int)r-1,(int)c+6)!=0xFF)continue;
            int x=w->x+(int)c*8,y=w->y+(int)r*8;
            if(x<0||x+56>256||y<8||y+8>240)continue;
            found++;
            if(draw){clear_text_area(x,y-8,x+56,y+8);draw_words(x,y+1,"OBTAINED.");}
        }
    }
    return found;
}

/* GIVE_HEADING_HEIGHT_V1: variable-height item windows, heading only. */
static int item_give_heading(int draw)
{
    static const uint8_t heading[2][12]={
        {192,255,255,255,255,255,255,255,255,255,255,193},
        {197,255,221,222,198,255,242,216,213,245,255,197}
    };
    int found=0;
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];
        if(w->width!=12||w->height<4)continue;
        uint16_t a=(uint16_t)(0x2000+w->table*1024+w->row*32+w->col);
        int ok=1;
        for(unsigned r=0;r<2&&ok;r++)for(unsigned c=0;c<12;c++)
            if(item_window_tile(w,(int)r,(int)c)!=heading[r][c]){ok=0;break;}
        if(!ok||w->x<0||w->x+96>256||w->y<0||w->y+16>240)continue;
        /* Do not paint across a smaller modal window covering this heading. */
        for(int r=0;r<2&&ok;r++)for(int c=2;c<11;c++)
            if(submenu_tile_covered(w->x+c*8,w->y+r*8,w->x,w->y,96,(int)w->height*8)){ok=0;break;}
        if(!ok)continue;
        found++;
        if(draw){
            clear_text_area(w->x+16,w->y,w->x+88,w->y+16);
            draw_words(w->x+16,w->y+9,"GIVE WHAT?");
        }
    }
    return found;
}

/* ITEM_EVERYDAY_HEADING_V1: captured category heading, display only. */
static int item_category_heading(int draw)
{
    static const uint8_t heading[2][12]={{192,255,255,255,255,244,255,255,255,255,255,193},{197,255,255,222,217,212,200,203,227,243,255,197}};
    int found=0;
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];
        if(w->width!=12||w->height<4)continue;
            int ok=1;
        for(unsigned r=0;r<2&&ok;r++)for(unsigned c=0;c<12;c++)
            if(item_window_tile(w,(int)r,(int)c)!=heading[r][c]){ok=0;break;}
        if(!ok||w->x<0||w->x+96>256||w->y<0||w->y+16>240)continue;
        /* Do not paint across a smaller modal window covering this heading. */
        for(int r=0;r<2&&ok;r++)for(int c=2;c<11;c++)
            if(submenu_tile_covered(w->x+c*8,w->y+r*8,w->x,w->y,96,(int)w->height*8)){ok=0;break;}
        if(!ok)continue;
        found++;
        if(draw){
            clear_text_area(w->x+16,w->y,w->x+88,w->y+16);
            draw_words(w->x+16,w->y+9,"EVERYDAY");
        }
    }
    return found;
}

/* DROP_HEADING_HEIGHT_V1: shared drop-selection heading, display only. */
static int item_drop_heading(int draw)
{
    static const uint8_t heading[2][12]={
        {192,255,255,255,255,255,255,255,255,255,255,193},
        {197,255,221,222,198,255,213,219,239,245,255,197}
    };
    int found=0;
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];
        if(w->width!=12||w->height<4)continue;
            int ok=1;
        for(unsigned r=0;r<2&&ok;r++)for(unsigned c=0;c<12;c++)
            if(item_window_tile(w,(int)r,(int)c)!=heading[r][c]){ok=0;break;}
        if(!ok||w->x<0||w->x+96>256||w->y<0||w->y+16>240)continue;
        /* Do not paint across a smaller modal window covering this heading. */
        for(int r=0;r<2&&ok;r++)for(int c=2;c<11;c++)
            if(submenu_tile_covered(w->x+c*8,w->y+r*8,w->x,w->y,96,(int)w->height*8)){ok=0;break;}
        if(!ok)continue;
        found++;
        if(draw){
            clear_text_area(w->x+16,w->y,w->x+88,w->y+16);
            draw_words(w->x+16,w->y+9,"DROP WHAT?");
        }
    }
    return found;
}

/* ITEM_NOT_WANTED_V1: shared item-discard popup, presentation only. */
static int item_not_wanted(int draw)
{
    static const uint8_t suffix[12]={197,255,255,226,255,202,237,221,202,235,255,197};
    int found=0;
    for(unsigned i=0;i<item_window_count;i++){
        const ItemWindow *w=&item_windows[i];
        if(w->width!=12||w->height!=8)continue;
        int ok=1;
        for(unsigned c=0;c<12;c++){
            unsigned top=c==0?192:c==11?193:196;
            if(item_window_tile(w,0,(int)c)!=top||item_window_tile(w,4,(int)c)!=suffix[c]){ok=0;break;}
            for(int r=3;r<=6;r++)if(r!=4){
                unsigned want=c==0||c==11?197:255;
                if(item_window_tile(w,r,(int)c)!=want){ok=0;break;}
            }
            if(!ok)break;
        }
        if(!ok)continue;
        /* Match any native item name, not a particular catalog translation. */
        int item=0;
        for(unsigned id=0;id<62&&!item;id++){
            const NativeItemName *n=&native_item_names[id];
            int same=1;
            for(unsigned c=1;c<11;c++){
                unsigned glyph=255,mark=255;
                if(c>=2&&c<2+n->length){glyph=n->tiles[c-2];mark=n->marks[c-2];}
                if(item_window_tile(w,2,(int)c)!=glyph||item_window_tile(w,1,(int)c)!=mark){same=0;break;}
            }
            if(same)item=1;
        }
        if(!item)continue;
        for(int r=3;r<=4&&ok;r++)for(int c=1;c<11;c++)
            if(submenu_tile_covered(w->x+c*8,w->y+r*8,w->x,w->y,96,64)){ok=0;break;}
        if(!ok)continue;
        found++;
        if(draw){
            clear_text_area(w->x+8,w->y+24,w->x+88,w->y+40);
            /* ITEM_DROPPED_WORDING_V1: discard-result wording only. */
            draw_words(w->x+24,w->y+33,"DROPPED.");
        }
    }
    return found;
}

static void item_table_draw(void)
{
    for(unsigned i=0;i<item_match_count;i++){
        const ItemNameMatch *m=&item_matches[i];const char *name=item_labels[m->id];
        memcpy(item_preserved,english_picture,sizeof item_preserved);
        clear_text_area(m->x,m->y-8,m->x+(int)m->width,m->y+8);
        if((int)strlen(name)*6-1<=(int)m->width)draw_words(m->x,m->y+1,name);
        else item_compact_words(m->x,m->y+2,name);
        item_restore_child_pixels(m->x,m->y-8,m->x+(int)m->width,m->y+8);
    }
    for(unsigned i=0;i<item_coin_count;i++){
        const ItemCoinMatch *m=&item_coins[i];memcpy(item_preserved,english_picture,sizeof item_preserved);clear_text_area(m->x,m->y,m->x+16,m->y+8);draw_words(m->x+2,m->y+1,"C");
        item_restore_child_pixels(m->x,m->y,m->x+16,m->y+8);
    }
}

/* NPC_TEMPLE_INTEGRATED_V2: exact captured frame/text, presentation only. */
static int npc_temple_box_find(int *left_out, int *top_out)
{
    static const uint8_t expected[8][12] = {
        {0xC0,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC1},
        {0xC5,0xFF,0xFF,0xF4,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC5},
        {0xC5,0xDC,0xCF,0xDC,0xCF,0xCD,0xDB,0xED,0xDE,0xFF,0xFF,0xC5},
        {0xC5,0xFF,0xFF,0xFF,0xFF,0xFF,0xF4,0xFF,0xFF,0xFF,0xFF,0xC5},
        {0xC5,0xCA,0xC9,0xD8,0xE6,0xCB,0xCE,0xCA,0xCA,0xEC,0xFF,0xC5},
        {0xC5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC5},
        {0xC5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC5},
        {0xC2,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC3}
    };
    for(unsigned row=0;row<60;row++)for(unsigned col=0;col<64;col++){
        if(command_world_tile(col,row)!=0xC0)continue;
        int match=1;
        for(unsigned r=0;r<8&&match;r++)for(unsigned c=0;c<12;c++)
            if(command_world_tile(col+c,row+r)!=expected[r][c]){match=0;break;}
        if(!match)continue;
        int world_x=(int)col*8,world_y=(int)row*8;
        for(int top=0;top+64<=240;top++){
            if(cyc_render_line_scroll_y(top)!=world_y)continue;
            int sx=cyc_render_line_scroll_x(top);
            if(sx<0||sx>=512)continue;
            int left=(world_x-sx+512)%512;
            if(left+96>256)continue;
            int visible=1;
            for(int y=top;y<top+64;y++)
                if(cyc_render_line_bg_table(y)!=0x1000||!(cyc_render_line_mask(y)&8)||
                   cyc_render_line_scroll_x(y)!=sx||
                   cyc_render_line_scroll_y(y)!=(world_y+y-top)%480){visible=0;break;}
            if(visible){*left_out=left;*top_out=top;return 1;}
        }
    }
    return 0;
}

/* GRANDPA_ALCOHOL_V1: PRG 1ED74, exact three-line text and enclosing frame. */
static int grandpa_alcohol_find(int *left_out,int *top_out,int *width_out)
{
    static const uint8_t raw[3][6]={
        {0x47,0xCE,0x44,0x4D,0x6D,0x5C},
        {0x47,0x4D,0x4B,0xC8,0,0},
        {0x4F,0x49,0xD2,0x66,0,0}
    };
    static const unsigned lengths[3]={6,4,4};
    for(unsigned row=0;row<60;row++)for(unsigned col=0;col<64;col++){
        if(command_world_tile(col,row)!=0xC0)continue;
        int text_ok=1;
        for(unsigned line=0;line<3&&text_ok;line++)for(unsigned i=0;i<lengths[line];i++){
            unsigned tile=(raw[line][i]&0x7Fu)+0x80u;
            if(tile>=0xC0u)tile+=6;
            if(command_world_tile(col+1+i,row+2+line*2)!=tile||
               command_world_tile(col+1+i,row+1+line*2)!=((raw[line][i]&0x80u)?0xF4:0xFF)){
                text_ok=0;break;
            }
        }
        if(!text_ok)continue;
        for(unsigned w=8;w<=32;w++){
            if(command_world_tile(col+w-1,row)!=0xC1)continue;
            for(unsigned h=8;h<=24;h++){
                if(command_world_tile(col,row+h-1)!=0xC2||
                   command_world_tile(col+w-1,row+h-1)!=0xC3)continue;
                int ok=1;
                for(unsigned r=0;r<h&&ok;r++)for(unsigned c=0;c<w;c++){
                    unsigned wanted;
                    if(r==0)wanted=c==0?0xC0:c==w-1?0xC1:0xC4;
                    else if(r==h-1)wanted=c==0?0xC2:c==w-1?0xC3:0xC4;
                    else if(c==0||c==w-1)wanted=0xC5;
                    else{
                        wanted=0xFF;
                        if(r<=6){
                            unsigned line=(r-1)/2,i=c-1;
                            if(i<lengths[line]){
                                if(r&1)wanted=(raw[line][i]&0x80u)?0xF4:0xFF;
                                else{wanted=(raw[line][i]&0x7Fu)+0x80u;if(wanted>=0xC0u)wanted+=6;}
                            }
                        }
                    }
                    if(command_world_tile(col+c,row+r)!=wanted){ok=0;break;}
                }
                if(!ok)continue;
                int world_x=(int)col*8,world_y=(int)row*8;
                for(int top=0;top+(int)h*8<=240;top++){
                    if(cyc_render_line_scroll_y(top)!=world_y)continue;
                    int sx=cyc_render_line_scroll_x(top);
                    if(sx<0||sx>=512)continue;
                    int left=(world_x-sx+512)%512;
                    if(left+(int)w*8>256)continue;
                    ok=1;
                    for(int y=top;y<top+(int)h*8;y++)
                        if(cyc_render_line_bg_table(y)!=0x1000||!(cyc_render_line_mask(y)&8)||
                           cyc_render_line_scroll_x(y)!=sx||
                           cyc_render_line_scroll_y(y)!=(world_y+y-top)%480){ok=0;break;}
                    if(ok){*left_out=left;*top_out=top;*width_out=(int)w*8;return 1;}
                }
            }
        }
    }
    return 0;
}

/* SWING_TREE_HINT_V1: PRG 1EDA7, exact two-line hint and frame. */
static int swing_tree_hint_find(int *left_out,int *top_out)
{
    static const uint8_t expected[8][12] = {
        {0xC0,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC1},
        {0xC5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC5},
        {0xC5,0xE9,0xD2,0xCB,0xE1,0xFF,0xCF,0xE1,0xD4,0xD8,0xC6,0xC5},
        {0xC5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF4,0xFF,0xFF,0xC5},
        {0xC5,0xE7,0xCA,0xDE,0xD9,0xFF,0xE8,0xDB,0xD2,0xED,0xF3,0xC5},
        {0xC5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC5},
        {0xC5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC5},
        {0xC2,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC4,0xC3}
    };
    for(unsigned row=0;row<60;row++)for(unsigned col=0;col<64;col++){
        if(command_world_tile(col,row)!=0xC0)continue;
        int match=1;
        for(unsigned r=0;r<8&&match;r++)for(unsigned c=0;c<12;c++)
            if(command_world_tile(col+c,row+r)!=expected[r][c]){match=0;break;}
        if(!match)continue;
        int world_x=(int)col*8,world_y=(int)row*8;
        for(int top=0;top+64<=240;top++){
            if(cyc_render_line_scroll_y(top)!=world_y)continue;
            int sx=cyc_render_line_scroll_x(top);
            if(sx<0||sx>=512)continue;
            int left=(world_x-sx+512)%512;
            if(left+96>256)continue;
            int visible=1;
            for(int y=top;y<top+64;y++)
                if(cyc_render_line_bg_table(y)!=0x1000||!(cyc_render_line_mask(y)&8)||
                   cyc_render_line_scroll_x(y)!=sx||
                   cyc_render_line_scroll_y(y)!=(world_y+y-top)%480){visible=0;break;}
            if(visible){*left_out=left;*top_out=top;return 1;}
        }
    }
    return 0;
}

/* NPC_GREEDY_V1: exact source message at PRG 1ED3C. */
static int npc_greedy_find(int *left,int *top,int *width)
{
    static const uint8_t glyph[2][6]={{0xCD,0xF3,0xDD,0xE1,0xD2,0xE2},{0xEC,0xD0,0xE2,0xEE,0xD8,0xEC}};
    static const uint8_t marks[2][6]={{255,255,255,255,255,255},{255,255,0xF4,255,0xF4,255}};
    for(unsigned row=0;row<60;row++)for(unsigned col=0;col<64;col++){
        if(command_world_tile(col,row)!=0xC0)continue;
        for(unsigned w=8;w<=12;w++){
            int ok=1;
            for(unsigned r=0;r<8&&ok;r++)for(unsigned c=0;c<w;c++){
                unsigned want=255;
                if(r==0)want=c==0?0xC0:c==w-1?0xC1:0xC4;
                else if(r==7)want=c==0?0xC2:c==w-1?0xC3:0xC4;
                else if(c==0||c==w-1)want=0xC5;
                else if(c<=6){
                    if(r==1||r==3)want=marks[(r-1)/2][c-1];
                    if(r==2||r==4)want=glyph[(r-2)/2][c-1];
                }
                if(command_world_tile(col+c,row+r)!=want){ok=0;break;}
            }
            if(!ok)continue;
            for(int y=0;y+64<=240;y++){
                if(cyc_render_line_scroll_y(y)!=(int)row*8)continue;
                int sx=cyc_render_line_scroll_x(y);
                if(sx<0||sx>=512)continue;
                int x=((int)col*8-sx+512)%512;
                if(x+(int)w*8>256)continue;
                int visible=1;
                for(int yy=y;yy<y+64;yy++)
                    if(cyc_render_line_bg_table(yy)!=0x1000||!(cyc_render_line_mask(yy)&8)||
                       cyc_render_line_scroll_x(yy)!=sx||
                       cyc_render_line_scroll_y(yy)!=((int)row*8+yy-y)%480){visible=0;break;}
                if(visible){*left=x;*top=y;*width=(int)w;return 1;}
            }
        }
    }
    return 0;
}

/* NPC_SWEETS_V1: source PRG 1ED4A, display only. */
static int npc_sweets_find(int *left,int *top)
{
    static const uint8_t expected[10][12]={{192,196,196,196,196,196,196,196,196,196,196,193},{197,255,255,255,255,255,255,255,255,255,244,197},{197,205,243,221,225,210,226,255,236,203,206,197},{197,255,255,255,255,255,255,255,255,255,244,197},{197,212,220,255,207,240,202,221,234,225,206,197},{197,255,255,244,255,255,255,255,255,255,255,197},{197,213,207,216,236,255,255,255,255,255,255,197},{197,255,255,255,255,255,255,255,255,255,255,197},{197,255,255,255,255,255,255,255,255,255,255,197},{194,196,196,196,196,196,196,196,196,196,196,195}};
    for(unsigned row=0;row<60;row++)for(unsigned col=0;col<64;col++){
        if(command_world_tile(col,row)!=0xC0)continue;
        int match=1;
        for(unsigned r=0;r<10&&match;r++)for(unsigned c=0;c<12;c++){
            if(r==8&&c>0&&c<11)continue; /* Preserve native continuation indicator. */
            if(command_world_tile(col+c,row+r)!=expected[r][c]){match=0;break;}
        }
        if(!match)continue;
        for(int y=0;y+80<=240;y++){
            if(cyc_render_line_scroll_y(y)!=(int)row*8)continue;
            int sx=cyc_render_line_scroll_x(y);
            if(sx<0||sx>=512)continue;
            int x=((int)col*8-sx+512)%512;
            if(x+96>256)continue;
            int visible=1;
            for(int yy=y;yy<y+80;yy++)
                if(cyc_render_line_bg_table(yy)!=0x1000||!(cyc_render_line_mask(yy)&8)||
                   cyc_render_line_scroll_x(yy)!=sx||
                   cyc_render_line_scroll_y(yy)!=((int)row*8+yy-y)%480){visible=0;break;}
            if(visible){*left=x;*top=y;return 1;}
        }
    }
    return 0;
}

/* FLOWER_PURCHASE_V1: exact captured 1000-coin bouquet prompt. */
static int flower_purchase_present(void)
{
    static const uint8_t tiles[32]={255,255,255,255,226,221,216,226,226,138,129,128,128,128,184,185,219,213,139,206,202,231,213,206,245,255,255,255,255,255,255,255};
    static const uint8_t marks[32]={255,255,255,255,255,255,255,244,255,255,255,255,255,255,255,255,244,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
    for(unsigned i=0;i<32;i++)
        if(cyc_render_nametable((uint16_t)(0x2320+i))!=tiles[i]||
           cyc_render_nametable((uint16_t)(0x2300+i))!=marks[i])return 0;
    for(int y=193;y<233;y++)
        if(cyc_render_line_bg_table(y)!=0x1000||cyc_render_line_scroll_x(y)!=16||
           cyc_render_line_scroll_y(y)!=239+y||!(cyc_render_line_mask(y)&8))return 0;
    return 1;
}

/* NPC_TENTEN_V1: exact captured frame; display only. */
static int npc_tenten_find(int *left_out, int *top_out)
{
    static const uint8_t expected[8][12] = {
        {192,196,196,196,196,196,196,196,196,196,196,193},
        {197,255,255,255,255,255,255,255,255,255,255,197},
        {197,219,243,219,243,225,220,210,241,222,255,197},
        {197,255,255,255,255,255,255,255,255,255,255,197},
        {197,202,201,216,245,255,255,255,255,255,255,197},
        {197,255,255,255,255,255,255,255,255,255,255,197},
        {197,255,255,255,255,255,255,255,255,255,255,197},
        {194,196,196,196,196,196,196,196,196,196,196,195}
    };
    for(unsigned row=0;row<60;row++)for(unsigned col=0;col<64;col++){
        if(command_world_tile(col,row)!=0xC0)continue;
        int match=1;
        for(unsigned r=0;r<8&&match;r++)for(unsigned c=0;c<12;c++)
            if(command_world_tile(col+c,row+r)!=expected[r][c]){match=0;break;}
        if(!match)continue;
        int world_x=(int)col*8,world_y=(int)row*8;
        for(int top=0;top+64<=240;top++){
            if(cyc_render_line_scroll_y(top)!=world_y)continue;
            int sx=cyc_render_line_scroll_x(top);
            if(sx<0||sx>=512)continue;
            int left=(world_x-sx+512)%512;
            if(left+96>256)continue;
            int visible=1;
            for(int y=top;y<top+64;y++)
                if(cyc_render_line_bg_table(y)!=0x1000||!(cyc_render_line_mask(y)&8)||
                   cyc_render_line_scroll_x(y)!=sx||
                   cyc_render_line_scroll_y(y)!=(world_y+y-top)%480){visible=0;break;}
            if(visible){*left_out=left;*top_out=top;return 1;}
        }
    }
    return 0;
}
/* NPC_SHOPPING_BAG_V1: exact captured text/frame, display only. */
static int npc_shopping_bag_find(int *left_out, int *top_out)
{
    static const uint8_t expected[8][12] = {
        {192,196,196,196,196,196,196,196,196,196,196,193},
        {197,255,255,255,255,255,255,255,255,255,255,197},
        {197,206,202,234,225,222,226,255,255,255,255,197},
        {197,255,244,255,244,255,255,255,255,255,255,197},
        {197,206,226,243,206,202,239,236,255,255,255,197},
        {197,255,255,255,255,255,255,255,255,255,255,197},
        {197,255,255,255,255,255,255,255,255,255,255,197},
        {194,196,196,196,196,196,196,196,196,196,196,195}
    };
    for(unsigned row=0;row<60;row++)for(unsigned col=0;col<64;col++){
        if(command_world_tile(col,row)!=0xC0)continue;
        int match=1;
        for(unsigned r=0;r<8&&match;r++)for(unsigned c=0;c<12;c++)
            if(command_world_tile(col+c,row+r)!=expected[r][c]){match=0;break;}
        if(!match)continue;
        int world_x=(int)col*8,world_y=(int)row*8;
        for(int top=0;top+64<=240;top++){
            if(cyc_render_line_scroll_y(top)!=world_y)continue;
            int sx=cyc_render_line_scroll_x(top);
            if(sx<0||sx>=512)continue;
            int left=(world_x-sx+512)%512;
            if(left+96>256)continue;
            int visible=1;
            for(int y=top;y<top+64;y++)
                if(cyc_render_line_bg_table(y)!=0x1000||!(cyc_render_line_mask(y)&8)||
                   cyc_render_line_scroll_x(y)!=sx||
                   cyc_render_line_scroll_y(y)!=(world_y+y-top)%480){visible=0;break;}
            if(visible){*left_out=left;*top_out=top;return 1;}
        }
    }
    return 0;
}

/* WAREHOUSE_THIEF_TEXT_V3: display-only exact dialogue rows. */
static int warehouse_thief_page(void){
static const uint8_t pages[3][2][32]={{{255,255,255,255,255,255,255,255,255,255,244,255,244,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},{255,255,255,255,235,201,218,209,216,255,220,241,230,203,225,228,220,210,241,222,255,255,255,255,255,255,255,255,255,255,255,255}},{{255,255,255,255,255,255,255,255,255,244,255,244,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},{255,255,255,255,218,203,210,203,219,206,216,206,255,226,202,201,219,202,216,139,255,255,255,255,255,255,255,255,255,255,255,255}},{{255,255,255,255,244,255,244,255,244,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},{255,255,255,255,220,241,230,203,206,202,216,139,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255}}};
for(int y=193;y<233;y++)if(cyc_render_line_bg_table(y)!=0x1000||cyc_render_line_scroll_x(y)!=16||cyc_render_line_scroll_y(y)!=239+y||!(cyc_render_line_mask(y)&8))return 0;
for(unsigned p=0;p<3;p++){int ok=1;for(unsigned r=0;r<2;r++)for(unsigned c=0;c<32;c++)if(cyc_render_nametable((uint16_t)(0x2300+r*32+c))!=pages[p][r][c])ok=0;if(ok)return (int)p+1;}return 0;
}

const uint32_t *english_display_present(void *ctx, int *width, int *height)
{
    (void)ctx;
    const uint32_t *original = cyc_render_present(width, height);
    if (!original || *width != 256 || *height != 240)
        return original;
    if (title_romaji_is_present() && title_romaji_draw(original))
        return english_picture;

    if (character_menu_v2_is_present()) {
        memcpy(english_picture, original, sizeof(english_picture));
        character_menu_v2_draw();
        character_erase_names_draw();
        return english_picture;
    }


    int npc_left=0,npc_top=0;
    int npc_temple=npc_temple_box_find(&npc_left,&npc_top);

    int grandpa_left=0,grandpa_top=0,grandpa_width=0;
    int grandpa=grandpa_alcohol_find(&grandpa_left,&grandpa_top,&grandpa_width);

    int swing_left=0,swing_top=0;
    int swing_tree=swing_tree_hint_find(&swing_left,&swing_top);

    int greedy_left=0,greedy_top=0,greedy_width=0;
    int greedy=npc_greedy_find(&greedy_left,&greedy_top,&greedy_width);
    int sweets_left=0,sweets_top=0;
    int sweets=npc_sweets_find(&sweets_left,&sweets_top);
    int flower_purchase=flower_purchase_present();
    int tenten_left=0,tenten_top=0;
    int tenten=npc_tenten_find(&tenten_left,&tenten_top);
    int shopping_bag_left=0,shopping_bag_top=0;
    int shopping_bag=npc_shopping_bag_find(&shopping_bag_left,&shopping_bag_top);
    int warehouse_page=warehouse_thief_page();
    LocationCaption location_caption;
    int location = location_caption_find(&location_caption);
    int temple = temple_sentence_is_present();
    unsigned gameplay_ui = gameplay_ui_flags();
    CommandPanel command_panels[4];
    unsigned command_count = command_panel_find(command_panels);
    int obtained = item_obtained(0);
    SubmenuPanel submenus[8];
    unsigned submenu_count = grouped_submenu_find(submenus);
    int give_heading=item_give_heading(0);
    int not_wanted=item_not_wanted(0);
    int drop_heading=item_drop_heading(0);
    int category_heading=item_category_heading(0);
    GameplayCannotPopup popups[4];
    unsigned popup_count = gameplay_cannot_popups(popups);

    const char *lookup_line1 = NULL, *lookup_line2 = NULL;
    int lookup = english_lookup_lines(&lookup_line1, &lookup_line2);

    static const uint16_t choice_addresses[] = {
        0x2239,0x223A,0x223B,0x223C,
        0x227A,0x227B,0x227C,
        0x22BA,0x22BB,0x22BC
    };
    static const uint8_t choice_tiles[] = {
        0xD6,0xF3,0xD8,0xD0,
        0xE2,0xFF,0xCA,
        0xCA,0xCA,0xCC
    };

    int choices =
        cyc_render_line_bg_table(138) == 0x1000 &&
        cyc_render_line_bg_table(154) == 0x1000 &&
        cyc_render_line_bg_table(170) == 0x1000 &&
        cyc_render_nametable(0x2238) == 0xC5 &&
        cyc_render_nametable(0x223D) == 0xC5 &&
        cyc_render_nametable(0x2278) == 0xC5 &&
        cyc_render_nametable(0x227D) == 0xC5 &&
        cyc_render_nametable(0x22B8) == 0xC5 &&
        cyc_render_nametable(0x22BD) == 0xC5 &&
        cyc_render_nametable(0x22F8) == 0xC2 &&
        cyc_render_nametable(0x22FD) == 0xC3;
    for (unsigned i = 0; i < sizeof(choice_tiles); ++i) {
        if (cyc_render_nametable(choice_addresses[i]) != choice_tiles[i]) {
            choices = 0;
            break;
        }
    }

    if (!grandpa && !npc_temple && !swing_tree && !greedy && !sweets && !flower_purchase && !tenten && !category_heading && !drop_heading && !not_wanted && !give_heading && !warehouse_page && !shopping_bag && !temple && !lookup && !choices && !gameplay_ui && !popup_count && !command_count && !submenu_count && !location && !item_match_count && !item_coin_count && !obtained)
        return original;

    memcpy(english_picture, original, sizeof(english_picture));
    gameplay_ui_draw(gameplay_ui);
    command_panel_draw(command_panels, command_count);
    grouped_submenu_draw(submenus, submenu_count);
    gameplay_cannot_draw(popups, popup_count);


    if (temple) {
        clear_text_area(8, 30, 144, 64);
        draw_words(16, 34, "THIS IS THE");
        draw_words(16, 50, "TEMPLE.");
    }









    if (lookup) {
        clear_text_area(8, 193, 248, 233);
        draw_words(16, 202, lookup_line1);
        if (lookup_line2[0]) draw_words(16, 214, lookup_line2);
    }

    if (choices) {
        clear_text_area(184, 136, 216, 145);
        clear_text_area(192, 152, 216, 161);
        clear_text_area(192, 168, 216, 177);
        draw_words(184, 138, "PICK");
        draw_words(192, 154, "YES");
        draw_words(192, 170, "NO");
    }

    if (location) location_caption_draw(&location_caption);

    item_table_draw();
    if(warehouse_page){
        clear_text_area(8,193,248,233);
        if(warehouse_page==1){
            draw_words(16,202,"IN THE POCKETS OF THE THIEF");
            draw_words(16,214,"YOU DEFEATED...");
        }else if(warehouse_page==2)draw_words(16,202,"THERE WAS A TRAVEL PASS.");
        else draw_words(16,202,"YOU HAVE FOUND A THIEF!");
    }
    if(shopping_bag){
        clear_text_area(shopping_bag_left+8,shopping_bag_top+8,shopping_bag_left+88,shopping_bag_top+48);
        draw_words(shopping_bag_left+8,shopping_bag_top+9,"YOU NEED");
        draw_words(shopping_bag_left+8,shopping_bag_top+21,"A BAG FOR");
        draw_words(shopping_bag_left+8,shopping_bag_top+33,"SHOPPING.");
    }
    item_give_heading(1);
    item_drop_heading(1);
    item_category_heading(1);
    if(tenten){
        clear_text_area(tenten_left+8,tenten_top+8,tenten_left+88,tenten_top+48);
        draw_words(tenten_left+8,tenten_top+9,"HAVE YOU");
        draw_words(tenten_left+8,tenten_top+21,"BEEN TO");
        draw_words(tenten_left+8,tenten_top+33,"TENTEN'S?");
    }
    if(flower_purchase){
        clear_text_area(8,193,248,233);
        draw_words(16,202,"A BOUQUET COSTS 1000 COINS.");
        draw_words(16,214,"WILL YOU BUY ONE?");
    }
    if(sweets){
        clear_text_area(sweets_left+8,sweets_top+8,sweets_left+88,sweets_top+64);
        draw_words(sweets_left+8,sweets_top+17,"GIRLS LIKE");
        draw_words(sweets_left+8,sweets_top+29,"SWEETS AND");
        draw_words(sweets_left+8,sweets_top+41,"PRETTY");
        draw_words(sweets_left+8,sweets_top+53,"THINGS.");
    }
    item_obtained(1);
    item_not_wanted(1);
    if(greedy){
        int inner=(greedy_width-2)*8;
        clear_text_area(greedy_left+8,greedy_top+8,greedy_left+(greedy_width-1)*8,greedy_top+48);
        if(inner>=53)draw_words(greedy_left+8+(inner-53)/2,greedy_top+17,"GIRLS ARE");
        else item_compact_words(greedy_left+8+(inner-35)/2,greedy_top+18,"GIRLS ARE");
        draw_words(greedy_left+8+(inner-41)/2,greedy_top+33,"GREEDY.");
    }

    if(swing_tree){
        clear_text_area(swing_left+8,swing_top+8,swing_left+88,swing_top+48);
        draw_words(swing_left+15,swing_top+9,"CHECK UNDER");
        draw_words(swing_left+21,swing_top+19,"THAT TREE");
        draw_words(swing_left+18,swing_top+29,"OVER THERE");
        draw_words(swing_left+18,swing_top+39,"EVERY DAY.");
    }

    if(grandpa){
        clear_text_area(grandpa_left+8,grandpa_top+8,
                        grandpa_left+grandpa_width-8,grandpa_top+56);
        if(grandpa_width>=88){
            draw_words(grandpa_left+8,grandpa_top+17,"GRANDPA LIKES");
            draw_words(grandpa_left+8,grandpa_top+33,"ALCOHOL.");
        }else{
            draw_words(grandpa_left+8,grandpa_top+17,"GRANDPA");
            draw_words(grandpa_left+8,grandpa_top+33,"LIKES");
            draw_words(grandpa_left+8,grandpa_top+49,"ALCOHOL.");
        }
    }

    if(npc_temple){
        clear_text_area(npc_left+8,npc_top+8,npc_left+88,npc_top+56);
        draw_words(npc_left+12,npc_top+13,"VISIT THE");
        draw_words(npc_left+12,npc_top+29,"TEMPLE NOW");
        draw_words(npc_left+12,npc_top+45,"AND THEN.");
    }

    return english_picture;
}
