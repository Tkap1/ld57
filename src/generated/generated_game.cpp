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
template <typename t>
func void add_to_render_group(t data, e_shader shader_id, e_texture texture_id, e_mesh mesh_id);
func s_shader load_shader_from_file(char* file, s_linear_arena* arena);
func b8 is_boost_hovered(s_v3 mouse_point, s_v3 boost_pos);
func void set_player_state(e_player_state state);
func void try_to_dash();
func void spawn_particles(int count, s_v3 pos, s_particle_spawn_data data);
func void update_particles();
func void set_state0_next_frame(e_game_state0 state);
func void set_temp_state0_next_frame(e_game_state0 state);
func void pop_game_state();
func b8 do_button(s_len_str text, s_v2 pos, b8 centered);
func b8 do_bool_button(s_len_str text, s_v2 pos, b8 centered, b8* out);
func b8 is_key_pressed(int key, b8 consume);
template <int n>
func void cstr_into_builder(s_str_builder<n>* builder, char* str);
template <int n>
func s_len_str builder_to_len_str(s_str_builder<n>* builder);
template <int n>
func b8 handle_string_input(s_input_str<n>* str, float time);
func void handle_key_event(int key, b8 is_down, b8 is_repeat);
func void do_leaderboard();
func void init_obstacles();
func b8 sphere_out_of_bounds_left(s_v3 pos, float radius);
func b8 sphere_out_of_bounds_right(s_v3 pos, float radius);
func s_v3 get_wanted_cam_pos(s_v3 player_pos);
func s_v2 get_rect_normal_of_closest_edge(s_v2 p, s_v2 center, s_v2 size);
