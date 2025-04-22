
#if defined(__EMSCRIPTEN__)

#define m_gl_funcs \
X(PFNGLBUFFERDATAPROC, glBufferData) \
X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
X(PFNGLGENBUFFERSPROC, glGenBuffers) \
X(PFNGLBINDBUFFERPROC, glBindBuffer) \
X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
X(PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer) \
X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
X(PFNGLCREATESHADERPROC, glCreateShader) \
X(PFNGLSHADERSOURCEPROC, glShaderSource) \
X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
X(PFNGLATTACHSHADERPROC, glAttachShader) \
X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
X(PFNGLCOMPILESHADERPROC, glCompileShader) \
X(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor) \
X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced) \
X(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced) \
X(PFNGLUNIFORM1FVPROC, glUniform1fv) \
X(PFNGLUNIFORM2FVPROC, glUniform2fv) \
X(PFNGLUNIFORM3FVPROC, glUniform3fv) \
X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
X(PFNGLUSEPROGRAMPROC, glUseProgram) \
X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
X(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) \
X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) \
X(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) \
X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) \
X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
X(PFNGLDELETESHADERPROC, glDeleteShader) \
X(PFNGLUNIFORM1IPROC, glUniform1i) \
X(PFNGLUNIFORM1FPROC, glUniform1f) \
X(PFNGLDETACHSHADERPROC, glDetachShader) \
X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) \
X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv) \
X(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate) \
X(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap) \
X(PFNGLBINDBUFFERBASEPROC, glBindBufferBase) \

#else
#define m_gl_funcs \
X(PFNGLBUFFERDATAPROC, glBufferData) \
X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
X(PFNGLGENBUFFERSPROC, glGenBuffers) \
X(PFNGLBINDBUFFERPROC, glBindBuffer) \
X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
X(PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer) \
X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
X(PFNGLCREATESHADERPROC, glCreateShader) \
X(PFNGLSHADERSOURCEPROC, glShaderSource) \
X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
X(PFNGLATTACHSHADERPROC, glAttachShader) \
X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
X(PFNGLCOMPILESHADERPROC, glCompileShader) \
X(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor) \
X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced) \
X(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced) \
X(PFNGLUNIFORM1FVPROC, glUniform1fv) \
X(PFNGLUNIFORM2FVPROC, glUniform2fv) \
X(PFNGLUNIFORM3FVPROC, glUniform3fv) \
X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
X(PFNGLUSEPROGRAMPROC, glUseProgram) \
X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
X(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) \
X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) \
X(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) \
X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) \
X(PFNGLACTIVETEXTUREPROC, glActiveTexture) \
X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
X(PFNGLDELETESHADERPROC, glDeleteShader) \
X(PFNGLUNIFORM1IPROC, glUniform1i) \
X(PFNGLUNIFORM1FPROC, glUniform1f) \
X(PFNGLDETACHSHADERPROC, glDetachShader) \
X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) \
X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv) \
X(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate) \
X(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap) \
X(PFNGLBINDBUFFERBASEPROC, glBindBufferBase) \

#endif

#define X(type, name) static type name = NULL;
m_gl_funcs
#undef X

#define invalid_default_case default: { assert(!"Invalid default case"); }
#define invalid_else else { assert(!"Invalid else"); }

#if defined(_WIN32)
#define m_dll_export extern "C" __declspec(dllexport)
#else // _WIN32
#define m_dll_export
#endif

enum e_game_state1
{
	e_game_state1_default,
	e_game_state1_defeat,
	e_game_state1_pre_victory,
};

struct s_down_input
{
	b8 speed_boost;
};

struct s_lerpable
{
	float curr;
	float target;
};

enum e_player_state
{
	e_player_state_default,
	e_player_state_dashing,
	e_player_state_post_dash,
};

struct s_player
{
	e_player_state state;
	s_maybe<int> dash_target;
	s_v3 pos;
	s_v3 vel;
	s_v3 prev_pos;
	s_v3 wanted_pos;
	float post_dash_timestamp;
	s_lerpable z_speed;
};

enum e_projectile_type
{
	e_projectile_type_default,
	e_projectile_type_bounce,
	e_projectile_type_static,
	e_projectile_type_follow,
	e_projectile_type_diagonal,
};

struct s_teleporter
{
	s_v3 pos;
	s_v3 destination;
};

struct s_projectile
{
	float start_follow_timestamp;
	e_projectile_type type;
	b8 going_right;
	float radius;
	s_v3 prev_pos;
	s_v3 pos;
	s_v3 dir;
	s_v4 color;
};

struct s_particle_spawn_data
{
	float shrink;
	b8 color_rand_per_channel;
	float duration;
	float duration_rand;
	float radius;
	float radius_rand;
	s_v4 color;
	float color_rand;
	s_v3 dir;
	s_v3 dir_rand;
	float speed;
	float speed_rand;
};

struct s_particle
{
	s_v3 pos;
	s_v3 dir;
	float speed;
	float shrink;
	float duration;
	float spawn_timestamp;
	float radius;
	s_v4 color;
};

enum e_game_state0
{
	e_game_state0_main_menu,
	e_game_state0_leaderboard,
	e_game_state0_win_leaderboard,
	e_game_state0_options,
	e_game_state0_play,
	e_game_state0_input_name,
};

struct s_key_event
{
	b8 went_down;
	int modifiers;
	int key;
};

struct s_state
{
	b8 temporary;
	int value;
};

struct s_key_state
{
	b8 is_down;
	int half_transition_count;
};

template <int n>
struct s_input_str
{
	b8 visual_pos_initialized;
	s_v2 cursor_visual_pos;
	float last_edit_time;
	float last_action_time;
	s_maybe<int> cursor;
	s_str_builder<n> str;
};

struct s_input_name_state
{
	s_input_str<256> name;
	s_str_builder<64> error_str;
};

struct s_speed_boost
{
	s_v3 pos;
};

struct s_soft_game_data
{
	s_player player;
	s_list<s_speed_boost, 1024> speed_boost_arr;
	s_list<s_projectile, 4096> projectile_arr;
	s_list<s_teleporter, 64> teleporter_arr;
	e_game_state1 state;
	float want_dash_timestamp;
	int hovered_boost;
	float defeat_timestamp;
	s_list<s_particle, 16384> particle_arr;
	float pre_victory_timestamp;
};

struct s_hard_game_data
{
	s_soft_game_data soft_data;
	int update_count;
	s_maybe<int> curr_checkpoint;
	float checkpoint_hit_timestamp_arr[32];
};

struct s_game
{
	b8 reload_shaders;
	b8 any_key_pressed;
	s_linear_arena update_frame_arena;
	s_linear_arena render_frame_arena;
	s_circular_arena circular_arena;
	u32 ubo;
	s_texture texture_arr[e_texture_count];
	s_mesh mesh_arr[e_mesh_count];
	s_shader shader_arr[e_shader_count];
	float render_time;
	float update_time;
	f64 accumulator;
	f64 time_before;
	u32 curr_fbo;
	s_down_input down_input;
	Mix_Chunk* sound_arr[e_sound_count];
	int speed_index;
	s_font font;
	s_rng rng;
	float speed;
	s_input_name_state input_name_state;

	s_fbo light_fbo;

	b8 turn_off_death_sound;
	b8 hide_timer;
	b8 hide_depth;
	b8 turn_off_all_sounds;

	s_v3 cam_pos;

	int update_count_at_win_time;

	b8 pop_state;
	b8 clear_state;
	s_maybe<s_state> next_state;
	s_list<s_state, 16> state0;

	s_list<s_leaderboard_entry, c_max_leaderboard_entries> leaderboard_arr;
	b8 leaderboard_received;

	s_str_builder<256> leaderboard_session_token;
	s_str_builder<256> leaderboard_public_uid;
	s_str_builder<256> leaderboard_nice_name;
	s_str_builder<256> leaderboard_player_identifier;
	int leaderboard_player_id;

	s_key_state input_arr[c_max_keys];
	s_list<s_key_event, 128> key_events;
	s_list<char, 128> char_events;

	b8 do_soft_reset;
	b8 do_hard_reset;

	s_hard_game_data hard_data;

	int render_instance_count[e_shader_count][e_texture_count][e_mesh_count];
	int render_instance_max_elements[e_shader_count][e_texture_count][e_mesh_count];
	int render_group_index_arr[e_shader_count][e_texture_count][e_mesh_count];
	void* render_instance_arr[e_shader_count][e_texture_count][e_mesh_count];
	s_list<s_render_group, 128> render_group_arr;
};


#include "generated/generated_game.cpp"