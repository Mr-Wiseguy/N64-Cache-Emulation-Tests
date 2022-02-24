#ifndef __TEXT_H__
#define __TEXT_H__

#include <cstring>

void set_text_color(int r, int g, int b, int a);
void print_text(int x, int y, char const* text, int length);
void print_text_centered(int x, int y, char const* text, int length);

static inline void print_text(int x, int y, char const* text)
{
    print_text(x, y, text, std::strlen(text));
}

static inline void print_text_centered(int x, int y, char const* text)
{
    print_text_centered(x, y, text, std::strlen(text));
}

#endif