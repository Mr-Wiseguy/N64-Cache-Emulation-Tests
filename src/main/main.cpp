#include <cstring>
#include <memory>

#include <gfx.h>
#include <platform.h>
#include <text.h>

void do_cache_tests();
void print_cache_results();

int main(__attribute__((unused)) int argc, __attribute__((unused)) char** argv)
{
    do_cache_tests();
    gfx_init();

    while (1)
    {
        start_frame();
        print_cache_results();
        end_frame();
    }
}
