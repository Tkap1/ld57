
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

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef near
#undef far
#endif // _WIN32


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

	#if defined(_WIN32) && defined(m_debug)
	CreateThread(null, 0, watch_for_file_changes, null, 0, null);
	#endif

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

#if defined(_WIN32) && defined(m_debug)
func DWORD WINAPI watch_for_file_changes(void* arg)
{
	(void)arg;
	HANDLE dir = CreateFile(".", GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, null);
	while(true) {
		FILE_NOTIFY_INFORMATION buffer[32] = zero;
		DWORD bytes_returned = 0;
		ReadDirectoryChangesW(dir, buffer, sizeof(buffer), true, FILE_NOTIFY_CHANGE_LAST_WRITE, &bytes_returned, null, null);
		FILE_NOTIFY_INFORMATION* curr = &buffer[0];
		while(true) {
			WCHAR* name = curr->FileName;
			char* builder = g_platform_data.hot_file_arr[g_platform_data.hot_write_index % c_max_hot_files];
			int len = curr->FileNameLength / 2;
			for(int i = 0; i < len; i += 1) {
				builder[i] = (char)name[i];
				builder[i + 1] = '\0';
			}
			g_platform_data.hot_write_index += 1;
			if(curr->NextEntryOffset) {
				curr = (FILE_NOTIFY_INFORMATION*)((u8*)curr + curr->NextEntryOffset);
			}
			else { break; }
		}
	}

	return 0;
}
#endif