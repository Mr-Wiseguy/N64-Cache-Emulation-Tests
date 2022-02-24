#ifndef __GFX_H__
#define __GFX_H__

#include <gfx.h>
#include <ultra64.h>

constexpr unsigned int output_buff_len = 1024;

constexpr unsigned int display_list_len = 1024;
constexpr unsigned int gui_display_list_len = 4096;

constexpr unsigned int num_frame_buffers = 2;

constexpr int screen_width = 320;
constexpr int screen_height = 240;
constexpr int border_height = 8;

#define BUFFER_SEGMENT 0x01

struct GfxContext {
    // Master displaylist
    Gfx dlist_buffer[display_list_len];
    // Gui displaylist
    Gfx gui_dlist_buffer[gui_display_list_len];
    // Graphics tasks done message
    OSMesg taskDoneMesg;
    // Graphics tasks done message queue
    OSMesgQueue taskDoneQueue;
};

extern struct GfxContext g_gfxContexts[num_frame_buffers];

extern u32 g_curGfxContext;
extern Gfx *g_dlist_head;

#include <array>
extern std::array<std::array<u16, screen_width * screen_height>, num_frame_buffers> g_frameBuffers;

void gfx_init(void);

void start_frame(void);
void end_frame(void);

#endif
