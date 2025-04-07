
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

enum e_mesh
{
	e_mesh_quad,
	e_mesh_cube,
	e_mesh_sphere,
	e_mesh_count,
};

enum e_shader
{
	e_shader_mesh,
	e_shader_depth_only,
	e_shader_flat,
	e_shader_fresnel,
	e_shader_post,
	e_shader_circle,
	e_shader_portal,
	e_shader_button,
	e_shader_text,
	e_shader_background,
	e_shader_count,
};

global constexpr char* c_shader_path_arr[e_shader_count] = {
	"shaders/mesh.shader",
	"shaders/depth_only.shader",
	"shaders/flat.shader",
	"shaders/fresnel.shader",
	"shaders/post.shader",
	"shaders/circle.shader",
	"shaders/portal.shader",
	"shaders/button.shader",
	"shaders/text.shader",
	"shaders/background.shader",
};

struct s_text_iterator
{
	int index;
	s_len_str text;
	s_list<s_v4, 4> color_stack;
	s_v4 color;
};


enum e_texture
{
	e_texture_white,
	e_texture_noise,
	e_texture_font,
	e_texture_checkpoint,
	e_texture_count
};

global constexpr char* c_texture_path_arr[e_texture_count] = {
	"assets/white.png",
	"assets/noise.png",
	"",
	"assets/checkpoint.png",
};

enum e_game_state1
{
	e_game_state1_default,
	e_game_state1_defeat,
	e_game_state1_pre_victory,
};

struct s_sound_data
{
	char* path;
	u8 volume;
};

enum e_sound
{
	e_sound_pop,
	e_sound_defeat,
	e_sound_clap,
	e_sound_dash,
	e_sound_knock,
	e_sound_victory,
	e_sound_click,
	e_sound_key,
	e_sound_checkpoint,
	e_sound_count,
};

global constexpr s_sound_data c_sound_data_arr[e_sound_count] = {
	{"assets/pop.wav", 128},
	{"assets/defeat.wav", 128},
	{"assets/clap.wav", 128},
	{"assets/dash.wav", 48},
	{"assets/knock.wav", 255},
	{"assets/victory.wav", 128},
	{"assets/click.wav", 128},
	{"assets/key.wav", 128},
	{"assets/checkpoint.wav", 128},
};

enum e_depth_mode
{
	e_depth_mode_no_read_no_write,
	e_depth_mode_read_and_write,
	e_depth_mode_read_no_write,
	e_depth_mode_no_read_yes_write,
};

enum e_cull_mode
{
	e_cull_mode_disabled,
	e_cull_mode_back_ccw,
	e_cull_mode_front_ccw,
	e_cull_mode_back_cw,
	e_cull_mode_front_cw,
};

enum e_blend_mode
{
	e_blend_mode_disabled,
	e_blend_mode_additive,
	e_blend_mode_premultiply_alpha,
	e_blend_mode_multiply,
	e_blend_mode_multiply_inv,
	e_blend_mode_normal,
	e_blend_mode_additive_no_alpha,
};

#pragma pack(push, 1)
struct s_ply_face
{
	s8 index_count;
	int index_arr[3];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct s_vertex
{
	s_v3 pos;
	s_v3 normal;
	s_v4 color;
	s_v2 uv;
};
#pragma pack(pop)


struct s_ply_mesh
{
	int vertex_count;
	int face_count;
	s_vertex vertex_arr[c_max_vertices];
	s_ply_face face_arr[c_max_faces];
};

struct s_obj_face
{
	int vertex_index[3];
	int normal_index[3];
	int uv_index[3];
};

struct s_obj_mesh
{
	int vertex_count;
	int normal_count;
	int uv_count;
	int face_count;
	s_v3 pos_arr[c_max_vertices];
	s_v3 normal_arr[c_max_vertices];
	s_v2 uv_arr[c_max_vertices];
	s_obj_face face_arr[c_max_faces];
};

#pragma pack(push, 1)
struct s_instance_data
{
	s_v4 color;
	s_v2 uv_min;
	s_v2 uv_max;
	s_m4 model;
};
#pragma pack(pop)

struct s_fbo
{
	s_v2i size;
	u32 id;
};

struct s_render_flush_data
{
	s_m4 view;
	s_m4 projection;
	s_m4 light_view;
	s_m4 light_projection;
	e_blend_mode blend_mode;
	e_depth_mode depth_mode;
	e_cull_mode cull_mode;
	s_v3 cam_pos;
	s_fbo fbo;
	s_v3 player_pos;
};

struct s_render_group
{
	e_shader shader_id;
	e_texture texture_id;
	e_mesh mesh_id;
};

struct s_glyph
{
	int advance_width;
	int width;
	int height;
	int x0;
	int y0;
	int x1;
	int y1;
	s_v2 uv_min;
	s_v2 uv_max;
};

struct s_texture
{
	u32 id;
};

struct s_font
{
	float size;
	float scale;
	int ascent;
	int descent;
	int line_gap;
	s_glyph glyph_arr[1024];
};

struct s_vbo
{
	u32 id;
	int max_elements;
};

struct s_mesh
{
	int vertex_count;
	u32 vao;
	u32 vertex_vbo;
	s_vbo instance_vbo;
};

struct s_shader
{
	u32 id;
};

struct s_speed_boost
{
	s_v3 pos;
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

template <typename t>
struct s_maybe
{
	b8 valid;
	t value;
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
};

struct s_projectile
{
	e_projectile_type type;
	b8 going_right;
	float radius;
	s_v3 prev_pos;
	s_v3 pos;
};

struct s_time_data
{
	float passed;
	float percent;
	float inv_percent;
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

struct s_time_format
{
	int hours;
	int minutes;
	int seconds;
	int milliseconds;
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


struct s_soft_game_data
{
	s_player player;
	s_list<s_speed_boost, 1024> speed_boost_arr;
	s_list<s_projectile, 1024> projectile_arr;
	e_game_state1 state;
	float want_dash_timestamp;
	int hovered_boost;
	float defeat_timestamp;
	s_list<s_particle, 4096> particle_arr;
	float pre_victory_timestamp;
};

struct s_hard_game_data
{
	s_soft_game_data soft_data;
	int update_count;
	s_maybe<int> curr_checkpoint;
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
	s_instance_data* render_instance_arr[e_shader_count][e_texture_count][e_mesh_count];
	s_list<s_render_group, 128> render_group_arr;
};


#include "generated/generated_game.cpp"