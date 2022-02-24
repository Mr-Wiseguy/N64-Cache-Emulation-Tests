#include <text.h>
#include <gfx.h>

#include <ultra64.h>

// Font modified from https://opengameart.org/content/8x8-font
__asm__(
 ".section \".rodata\", \"a\", @progbits\n"
 ".align 4\n"
 ".global font_img\n"
 "font_img:\n"
 ".incbin \"./src/gfx/font.bin\"\n"
 ".previous\n"
);

extern u8 font_img[];

constexpr int font_char_offset = ' ';
constexpr int font_img_width = 608;
constexpr int font_img_height = 8;

u32 cur_color = 0x0;

void set_text_color(int r, int g, int b, int a)
{
    cur_color = (r & 0xFF) << 24 |
                (g & 0xFF) << 16 |
                (b & 0xFF) << 8 |
                (a & 0xFF) << 0;
}

extern Gfx* g_gui_dlist_head;

void print_text(int x, int y, char const* text, int length)
{
    Gfx* dlist = g_gui_dlist_head;
    gDPPipeSync(dlist++);
    gDPSetCycleType(dlist++, G_CYC_1CYCLE);
    gDPSetTexturePersp(dlist++, G_TP_NONE);
    gDPSetCombineLERP(dlist++, ENVIRONMENT, 0, TEXEL0, 0, 0, 0, 0, TEXEL0, ENVIRONMENT, 0, TEXEL0, 0, 0, 0, 0, TEXEL0);
    gDPSetRenderMode(dlist++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPLoadTextureBlock_4b(dlist++, font_img, G_IM_FMT_IA, font_img_width, font_img_height, 0,  0, 0, 0, 0, 0, 0);
    gDPSetTexturePersp(dlist++, G_TP_NONE);
    gDPSetTextureFilter(dlist++, G_TF_POINT);
    
    gDPPipeSync(dlist++);
    gDPSetColor(dlist++, G_SETENVCOLOR, cur_color);
    for (int i = 0; i < length; i++)
    {
        unsigned int character = *text;
        gSPTextureRectangle(dlist++, (x) << 2, (y) << 2, (x + 6) << 2, (y + 8) << 2, 0,
            ((character - font_char_offset) * 6) << 5, 0 << 5,
            1 << 10, 1 << 10);
        ++text;
        x += 6;
    }
    g_gui_dlist_head = dlist;
}

void print_text_centered(int x, int y, char const* text, int length)
{
    int width = length * 6;
    print_text(x - width / 2, y, text, length);
}
