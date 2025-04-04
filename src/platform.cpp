
#pragma warning(push, 0)
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL_mixer.h"
#pragma warning(pop)

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#endif // __EMSCRIPTEN__

#include "tk_types.h"
#include "platform.h"
#include "shared.h"
global s_platform_data g_platform_data;

#include "shared.cpp"

int main()
{
	#if defined(__EMSCRIPTEN__)
	emscripten_set_main_loop(do_one_frame, -1, 0);
	#endif // __EMSCRIPTEN__

	g_platform_data.memory = (u8*)calloc(1, 512 * 1024 * 1024);
	init(&g_platform_data);
	init_after_recompile(&g_platform_data);
	#if !defined(__EMSCRIPTEN__)
	while(!g_platform_data.quit) {
		do_one_frame();
	}
	#endif

	return 0;
}

func void do_one_frame()
{
	do_game(&g_platform_data);
}

