
global constexpr int c_max_hot_files = 64;

struct s_v2i
{
	int x;
	int y;
};

struct s_platform_data
{
	b8 quit;
	b8 window_resized;
	u64 start_cycles;
	u64 cycle_frequency;
	SDL_Window* window;
	SDL_GLContext gl_context;
	s_v2i window_size;
	s_v2i prev_window_size;
	u8* memory;
	int hot_read_index;
	int hot_write_index;
	char hot_file_arr[c_max_hot_files][32];
};

void init(s_platform_data* platform_data);
void init_after_recompile(s_platform_data* platform_data);
void do_game(s_platform_data* platform_data);

#include "generated/generated_shared.cpp"