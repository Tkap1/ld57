func void input();
func void update();
func void render(float interp_dt, float delta);
func f64 get_seconds();
func void on_gl_error(const char* expr, char* file, int line, int error);
func void draw_rect(s_v2 pos, s_v2 size, s_v4 color);
func void draw_rect_topleft(s_v2 pos, s_v2 size, s_v4 color);
func void draw_texture_screen(s_v2 pos, s_v2 size, s_v4 color, e_texture texture_id, e_shader shader_id, s_v2 uv_min, s_v2 uv_max);
func void draw_mesh(e_mesh mesh_id, s_m4 model, s_v4 color, e_shader shader_id);
func void draw_mesh(e_mesh mesh_id, s_v3 pos, s_v3 size, s_v4 color, e_shader shader_id);
func void bind_framebuffer(u32 fbo);
func void clear_framebuffer_color(u32 fbo, s_v4 color);
func void clear_framebuffer_depth(u32 fbo);
func void render_flush(s_render_flush_data data, b8 reset_render_count);
func void add_to_render_group(s_instance_data data, e_shader shader_id, e_texture texture_id, e_mesh mesh_id);
func s_shader load_shader_from_file(char* file, s_linear_arena* arena);
func u8* read_file(char* path, s_linear_arena* arena);
func void set_window_size(int width, int height);
func s_rect do_letterbox(s_v2 curr_size, s_v2 base_size);
func s_render_flush_data make_render_flush_data(s_v3 cam_pos, s_v3 player_pos);
func s_mesh make_mesh_from_vertices(s_vertex* vertex_arr, int vertex_count);
func s_ply_mesh parse_ply_mesh(char* path, s_linear_arena* arena);
func s_mesh make_mesh_from_ply_file(char* file, s_linear_arena* arena);
func s_mesh make_mesh_from_obj_file(char* file, s_linear_arena* arena);
func void set_cull_mode(e_cull_mode mode);
func void set_depth_mode(e_depth_mode mode);
func void set_blend_mode(e_blend_mode mode);
template <typename t>
func void toggle(t* out, t a, t b);
func b8 is_boost_hovered(s_v3 mouse_point, s_v3 boost_pos);
func Mix_Chunk* load_sound_from_file(char* path, u8 volume);
func void play_sound(e_sound sound_id);
func void set_player_state(e_player_state state);
func void try_to_dash();
func s_texture load_texture_from_file(char* path);
func s_texture load_texture_from_data(void* data, int width, int height, int format);
template <typename t>
func s_maybe<t> maybe();
template <typename t>
func s_maybe<t> maybe(t value);
func s_font load_font_from_file(char* file, int font_size, s_linear_arena* arena);
func s_v2 draw_text(s_len_str text, s_v2 in_pos, float font_size, s_v4 color, b8 centered, s_font* font);
func s_v2 get_text_size_with_count(s_len_str in_text, s_font* font, float font_size, int count, int in_column);
func s_v2 get_text_size(s_len_str text, s_font* font, float font_size);
func b8 iterate_text(s_text_iterator* it, s_len_str text, s_v4 color);
func s_len_str substr_from_to_exclusive(s_len_str x, int start, int end);
func int get_spaces_for_column(int column);
func int hex_str_to_int(s_len_str str);
func b8 is_number(char c);
func b8 is_alpha(char c);
func b8 is_alpha_numeric(char c);
func b8 can_start_identifier(char c);
func b8 can_continue_identifier(char c);
func s_len_str format_text(const char* text, ...);
func void spawn_particles(int count, s_v3 pos, s_particle_spawn_data data);
func void update_particles();
func s_v4 rand_color(s_rng* rng);
func s_v4 rand_color_normalized(s_rng* rng);
func s_v3 random_point_in_sphere(s_rng* rng, float radius);
func u8* try_really_hard_to_read_file(char* file, s_linear_arena* arena);
func float update_time_to_render_time(float time, float interp_dt);
func s_m4 fullscreen_m4();
func s_time_data get_time_data(float curr, float timestamp, float duration);
func s_time_format update_count_to_time_format(int update_count);
func s_obj_mesh parse_obj_mesh(char* path, s_linear_arena* arena);
func char* skip_whitespace(char* str);
func void set_state0_next_frame(e_game_state0 state);
func void set_temp_state0_next_frame(e_game_state0 state);
func void pop_game_state();
func s_v2 wxy(float x, float y);
func s_v2 wcxy(float x, float y);
func b8 do_button(s_len_str text, s_v2 pos, b8 centered);
func b8 do_bool_button(s_len_str text, s_v2 pos, b8 centered, b8* out);
func b8 rect_vs_rect_topleft(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1);
func b8 rect_vs_rect_center(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1);
func b8 mouse_vs_rect_topleft(s_v2 pos, s_v2 size);
func b8 mouse_vs_rect_center(s_v2 pos, s_v2 size);
func b8 is_key_pressed(int key, b8 consume);
template <int n>
func void cstr_into_builder(s_str_builder<n>* builder, char* str);
template <int n>
func s_len_str builder_to_len_str(s_str_builder<n>* builder);
template <int n>
func char* builder_to_cstr(s_str_builder<n>* builder, s_circular_arena* arena);
template <int n0, int n1>
func b8 builder_equals(s_str_builder<n0>* a, s_str_builder<n1>* b);
template <int n>
func b8 handle_string_input(s_input_str<n>* str, float time);
func void handle_key_event(int key, b8 is_down, b8 is_repeat);
template <int n>
func b8 is_builder_full(s_str_builder<n>* builder);
template <int n>
func void builder_insert(s_str_builder<n>* builder, int index, char c);
template <int n>
func void builder_remove_char_at(s_str_builder<n>* builder, int index);
func void do_leaderboard();
func void init_obstacles();
func b8 sphere_out_of_bounds_left(s_v3 pos, float radius);
func b8 sphere_out_of_bounds_right(s_v3 pos, float radius);
func s_v3 get_wanted_cam_pos(s_v3 player_pos);
