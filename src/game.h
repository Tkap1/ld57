
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

enum e_mesh
{
	e_mesh_quad,
	e_mesh_cube,
	e_mesh_monkey,
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
	e_shader_text,
	e_shader_circle,
	e_shader_portal,
	e_shader_count,
};

global constexpr char* c_shader_path_arr[e_shader_count] = {
	"shaders/mesh.shader",
	"shaders/depth_only.shader",
	"shaders/flat.shader",
	"shaders/fresnel.shader",
	"shaders/post.shader",
	"shaders/text.shader",
	"shaders/circle.shader",
	"shaders/portal.shader",
};

struct s_len_str
{
	char* str;
	int len;

	char operator[](int index)
	{
		assert(index < len);
		return str[index];
	}
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
	e_texture_shadow_map,
	e_texture_noise,
	e_texture_font,
	e_texture_count
};

global constexpr char* c_texture_path_arr[e_texture_count] = {
	"",
	"",
	"assets/noise.png",
	"",
};

enum e_game_state
{
	e_game_state_default,
	e_game_state_defeat,
};

enum e_sound
{
	e_sound_pop,
	e_sound_defeat,
	e_sound_clap,
	e_sound_dash,
	e_sound_knock,
	e_sound_count,
};

struct s_sound_data
{
	char* path;
	u8 volume;
};

global constexpr s_sound_data c_sound_data_arr[e_sound_count] = {
	{"assets/pop.wav", 128},
	{"assets/defeat.wav", 128},
	{"assets/clap.wav", 128},
	{"assets/dash.wav", 48},
	{"assets/knock.wav", 128},
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

enum e_view_state
{
	e_view_state_default,
	e_view_state_shadow_map,
	e_view_state_curr_depth,
	e_view_state_count,
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
};

struct s_projectile
{
	e_projectile_type type;
	b8 going_right;
	float radius;
	s_v3 prev_pos;
	s_v3 pos;
};

struct s_fct
{
	float spawn_timestamp;
	s_len_str text;
	s_v3 pos;
	s_v3 start_pos;
	s_v3 target_pos;
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

struct s_soft_game_data
{
	s_player player;
	s_list<s_speed_boost, 1024> speed_boost_arr;
	s_list<s_projectile, 1024> projectile_arr;
	e_game_state state;
	float want_dash_timestamp;
	int hovered_boost;
	float defeat_timestamp;
	s_list<s_particle, 4096> particle_arr;
};

struct s_hard_game_data
{
	b8 display_checkpoint;
	float highest_z;
	s_soft_game_data soft_data;
	s_list<s_fct, 16> fct_arr;
};

struct s_game
{
	b8 reload_shaders;
	s_linear_arena update_frame_arena;
	s_linear_arena render_frame_arena;
	u32 ubo;
	s_texture texture_arr[e_texture_count];
	s_mesh mesh_arr[e_mesh_count];
	s_shader shader_arr[e_shader_count];
	float render_time;
	float update_time;
	f64 accumulator;
	f64 time_before;
	s_fbo shadow_map_fbo;
	u32 curr_fbo;
	e_view_state view_state;
	s_down_input down_input;
	Mix_Chunk* sound_arr[e_sound_count];
	int speed_index;
	s_font font;
	s_rng rng;

	b8 do_soft_reset;
	b8 do_hard_reset;

	s_hard_game_data hard_data;

	int render_instance_count[e_shader_count][e_texture_count][e_mesh_count];
	int render_instance_max_elements[e_shader_count][e_texture_count][e_mesh_count];
	int render_group_index_arr[e_shader_count][e_texture_count][e_mesh_count];
	s_instance_data* render_instance_arr[e_shader_count][e_texture_count][e_mesh_count];
	s_list<s_render_group, 128> render_group_arr;
};


template <size_t T>
func constexpr s_len_str S(const char (&str)[T])
{
	s_len_str result;
	result.len = T - 1;
	result.str = (char*)str;
	return result;
}

func constexpr s_len_str S(char* str)
{
	s_len_str result;
	result.str = str;
	result.len = 0;
	while(str[result.len] != '\0') {
		result.len += 1;
	}
	return result;
}


#include "generated/generated_game.cpp"