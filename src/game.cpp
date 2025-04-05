
#define m_cpu_side 1

#pragma comment(lib, "opengl32.lib")

#pragma warning(push, 0)
// #define SDL_MAIN_HANDLED
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

#include <stdlib.h>
#include <stdio.h>

#include <gl/GL.h>
#if !defined(__EMSCRIPTEN__)
#include "external/glcorearb.h"
#endif

#include "tk_types.h"

#if defined(m_debug)
#define gl(...) __VA_ARGS__; {int error = glGetError(); if(error != 0) { on_gl_error(#__VA_ARGS__, __FILE__, __LINE__, error); }}
#define assert(condition) if(!(condition)) { on_failed_assert(#condition, __FILE__, __LINE__); }
#else // m_debug
#define gl(...) __VA_ARGS__
#define assert(condition)
#endif // m_debug

func void on_failed_assert(char* condition, char* file, int line);

#include "shared.h"
#include "tk_arena.h"
#include "tk_array.h"

#define m_tk_random_impl
#include "tk_random.h"

#include "tk_math.h"
#include "config.h"
#include "game.h"
#include "tk_color.h"
#include "shader_shared.h"

global s_platform_data* g_platform_data;
global s_game* game;
global s_v2 g_mouse;

#include "shared.cpp"

void init(s_platform_data* platform_data)
{
	g_platform_data = platform_data;
	game = (s_game*)platform_data->memory;
	game->speed_index = 5;

	{
		u8* memory = platform_data->memory + sizeof(s_game);
		game->update_frame_arena = make_arena_from_memory(memory, 10 * c_mb);
	}
	{
		u8* memory = platform_data->memory + sizeof(s_game) + 10 * c_mb;
		game->render_frame_arena = make_arena_from_memory(memory, 10 * c_mb);
	}

	platform_data->cycle_frequency = SDL_GetPerformanceFrequency();
	platform_data->start_cycles = SDL_GetPerformanceCounter();

	platform_data->window_size.x = (int)c_world_size.x;
	platform_data->window_size.y = (int)c_world_size.y;

	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	Mix_OpenAudioDevice(44100, MIX_DEFAULT_FORMAT, 2, 512, NULL, 0);
	Mix_Volume(-1, floorfi(MIX_MAX_VOLUME * 0.1f));

	g_platform_data->window = SDL_CreateWindow(
		"LD57", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		(int)c_world_size.x, (int)c_world_size.y, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	SDL_SetWindowPosition(g_platform_data->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(g_platform_data->window);

	#if defined(__EMSCRIPTEN__)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	#endif

	g_platform_data->gl_context = SDL_GL_CreateContext(g_platform_data->window);
	SDL_GL_SetSwapInterval(1);

	#define X(type, name) name = (type)SDL_GL_GetProcAddress(#name); assert(name);
		m_gl_funcs
	#undef X

	{
		gl(glGenBuffers(1, &game->ubo));
		gl(glBindBuffer(GL_UNIFORM_BUFFER, game->ubo));
		gl(glBufferData(GL_UNIFORM_BUFFER, sizeof(s_uniform_data), NULL, GL_DYNAMIC_DRAW));
		gl(glBindBufferBase(GL_UNIFORM_BUFFER, 0, game->ubo));
	}

	{
		constexpr float c_size = 0.5f;
		s_vertex vertex_arr[] = {
			{v3(-c_size, -c_size, 0), v3(0, -1, 0), make_color(1), v2(0, 1)},
			{v3(c_size, -c_size, 0), v3(0, -1, 0), make_color(1), v2(1, 1)},
			{v3(c_size, c_size, 0), v3(0, -1, 0), make_color(1), v2(1, 0)},
			{v3(-c_size, -c_size, 0), v3(0, -1, 0), make_color(1), v2(0, 1)},
			{v3(c_size, c_size, 0), v3(0, -1, 0), make_color(1), v2(1, 0)},
			{v3(-c_size, c_size, 0), v3(0, -1, 0), make_color(1), v2(0, 0)},
		};
		game->mesh_arr[e_mesh_quad] = make_mesh_from_vertices(vertex_arr, array_count(vertex_arr));
	}

	{
		game->mesh_arr[e_mesh_cube] = make_mesh_from_ply_file("assets/cube.ply", &game->render_frame_arena);
		game->mesh_arr[e_mesh_monkey] = make_mesh_from_ply_file("assets/monkey.ply", &game->render_frame_arena);
		game->mesh_arr[e_mesh_sphere] = make_mesh_from_ply_file("assets/sphere.ply", &game->render_frame_arena);
	}

	{
		game->shadow_map_fbo.size.x = c_shadow_map_res;
		game->shadow_map_fbo.size.y = c_shadow_map_res;
		gl(glGenFramebuffers(1, &game->shadow_map_fbo.id));
		gl(glGenTextures(1, &game->texture_arr[e_texture_shadow_map].id));
		gl(glBindTexture(GL_TEXTURE_2D, game->texture_arr[e_texture_shadow_map].id));
		gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, c_shadow_map_res, c_shadow_map_res, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, null));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

		bind_framebuffer(game->shadow_map_fbo.id);
		gl(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, game->texture_arr[e_texture_shadow_map].id, 0));
		// gl(glDrawBuffer(GL_NONE));
		gl(glReadBuffer(GL_NONE));
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			assert(false);
		}

		bind_framebuffer(0);
	}

	for(int i = 0; i < e_shader_count; i += 1) {
		game->shader_arr[i] = load_shader_from_file(c_shader_path_arr[i], &game->render_frame_arena);
	}

	for(int i = 0; i < e_sound_count; i += 1) {
		game->sound_arr[i] = load_sound_from_file(c_sound_path_arr[i]);
	}
}

void init_after_recompile(s_platform_data* platform_data)
{
	g_platform_data = platform_data;
	game = (s_game*)platform_data->memory;

	#define X(type, name) name = (type)SDL_GL_GetProcAddress(#name); assert(name);
		m_gl_funcs
	#undef X
}

#if defined(__EMSCRIPTEN__)
EM_JS(int, browser_get_width, (), {
	const { width, height } = canvas.getBoundingClientRect();
	return width;
});

EM_JS(int, browser_get_height, (), {
	const { width, height } = canvas.getBoundingClientRect();
	return height;
});
#endif // __EMSCRIPTEN__

void do_game(s_platform_data* platform_data)
{
	g_platform_data = platform_data;
	game = (s_game*)platform_data->memory;

	f64 delta64 = get_seconds() - game->time_before;
	game->time_before = get_seconds();

	{
		#if defined(__EMSCRIPTEN__)
		int width = browser_get_width();
		int height = browser_get_height();
		g_platform_data->window_size.x = width;
		g_platform_data->window_size.y = height;
		if(g_platform_data->prev_window_size.x != width || g_platform_data->prev_window_size.y != height) {
			set_window_size(width, height);
			g_platform_data->window_resized = true;
		}
		g_platform_data->prev_window_size.x = width;
		g_platform_data->prev_window_size.y = height;
		#endif // __EMSCRIPTEN__
	}

	input();
	float game_speed = c_game_speed_arr[game->speed_index];
	game->accumulator += delta64 * game_speed;
	while(game->accumulator >= c_update_delay) {
		game->accumulator -= c_update_delay;
		update();
	}
	float interp_dt = (float)(game->accumulator / c_update_delay);
	render(interp_dt, (float)delta64);
}

func void input()
{
	{
		int x;
		int y;
		u32 state = SDL_GetMouseState(&x, &y);
		game->down_input.speed_boost = state & SDL_BUTTON(1);
		g_mouse = v2(x, y);
	}

	s_v3 dir = zero;
	u8* key_state = (u8*)SDL_GetKeyboardState(null);
	// if(key_state[SDL_SCANCODE_A]) {
	// 	dir.x -= 1;
	// }
	// if(key_state[SDL_SCANCODE_D]) {
	// 	dir.x += 1;
	// }
	// if(key_state[SDL_SCANCODE_W]) {
	// 	dir.y += 1;
	// }
	// if(key_state[SDL_SCANCODE_S]) {
	// 	dir.y -= 1;
	// }
	// dir = v3_normalized(dir);
	// game->player.pos += dir * 0.5f;

	SDL_Event event;
	while(SDL_PollEvent(&event) != 0) {
		switch(event.type) {
			case SDL_QUIT: {
				g_platform_data->quit = true;
			} break;

			case SDL_WINDOWEVENT: {
				if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					int width = event.window.data1;
					int height = event.window.data2;
					g_platform_data->window_size.x = width;
					g_platform_data->window_size.y = height;
					g_platform_data->window_resized = true;
				}
			} break;

			case SDL_KEYDOWN:
			case SDL_KEYUP: {
				if(event.type == SDL_KEYDOWN) {
					if(event.key.keysym.sym == SDLK_h) {
						game->view_state = (e_view_state)circular_index(game->view_state + 1, e_view_state_count);
					}
					else if(event.key.keysym.sym == SDLK_g) {
						game->view_state = (e_view_state)circular_index(game->view_state - 1, e_view_state_count);
					}
					else if(event.key.keysym.sym == SDLK_f && event.key.repeat == 0) {
						game->want_dash_timestamp = game->update_time;
					}
					else if(event.key.keysym.sym == SDLK_r && event.key.repeat == 0) {
						game->dont_reset_game = false;
					}
					#if defined(m_debug)
					else if(event.key.keysym.sym == SDLK_KP_PLUS) {
						game->speed_index = circular_index(game->speed_index + 1, array_count(c_game_speed_arr));
						printf("Game speed: %f\n", c_game_speed_arr[game->speed_index]);
					}
					else if(event.key.keysym.sym == SDLK_KP_MINUS) {
						game->speed_index = circular_index(game->speed_index - 1, array_count(c_game_speed_arr));
						printf("Game speed: %f\n", c_game_speed_arr[game->speed_index]);
					}
					#endif // m_debug
				}
				// int key = sdl_key_to_windows_key(event.key.keysym.sym);
				// if(key == -1) { break; }
				// b8 is_repeat = event.key.repeat ? true : false;
				// b8 is_down = event.type == SDL_KEYDOWN;

				// handle_key_event(key, is_down, is_repeat);

				// // @Note(tkap, 11/11/2023): SDL does not give us a text input event for backspace, so let's hack it
				// if(key == c_key_backspace && is_down) {
				// 	g_platform_data->input.char_events.add('\b');
				// }

			} break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				// int key = sdl_key_to_windows_key(event.button.button);
				// b8 is_down = event.type == SDL_MOUSEBUTTONDOWN;
				// handle_key_event(key, is_down, false);
			} break;

			case SDL_TEXTINPUT: {
				// char c = event.text.text[0];
				// g_platform_data->input.char_events.add((char)c);
			} break;

			case SDL_MOUSEWHEEL: {
				// float movement = (float)event.wheel.y;
				// g_platform_data->input.wheel_movement = movement / 120;
			} break;
		}
	}
}

func void update()
{
	game->update_frame_arena.used = 0;
	game->player.prev_pos = game->player.pos;

	if(!game->dont_reset_game) {
		game->player = zero;
		game->state = e_game_state_default;
		game->dont_reset_game = true;
		game->speed_boost_arr.count = 0;
		game->projectile_arr.count = 0;
		game->player.dash_target = -1;

		s_rng rng = make_rng(0);
		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn speed boosts start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		{
			int z = 10;
			while(z < 1000) {
				if(chance100(&rng, 50)) {
					int count = rand_range_ii(&rng, 1, 5);
					for(int i = 0; i < count; i += 1) {
						s_speed_boost boost = zero;
						boost.pos = v3(
							-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, -z
						);
						z += 10;
						game->speed_boost_arr.add(boost);
					}
				}
				z += 20;
			}
		}
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		spawn speed boosts end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn projectiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		{
			int z = c_default_proj_z_start;
			while(z < c_default_proj_z_end) {
				if(chance100(&rng, c_default_proj_chance)) {
					s_projectile proj = zero;
					proj.pos = v3(
						-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, -z
					);
					proj.type = e_projectile_type_default;
					proj.prev_pos = proj.pos;
					proj.radius = randf_range(&rng, 0.5f, 1.5f);
					proj.going_right = rand_bool(&rng);
					game->projectile_arr.add(proj);
					z += c_default_proj_z_step;
				}
				z += c_default_proj_z_step * 2;
			}
		}

		{
			int z = c_bounce_proj_z_start;
			while(z < c_bounce_proj_z_end) {
				if(chance100(&rng, c_bounce_proj_chance)) {
					s_projectile proj = zero;
					proj.pos = v3(
						-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, -z
					);
					proj.type = e_projectile_type_bounce;
					proj.prev_pos = proj.pos;
					proj.radius = randf_range(&rng, 0.5f, 1.5f);
					proj.going_right = rand_bool(&rng);
					game->projectile_arr.add(proj);
					z += c_bounce_proj_z_step;
				}
				z += c_bounce_proj_z_step * 2;
			}
		}
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		spawn projectiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	}

	b8 handle_input = game->state == e_game_state_default;
	b8 can_dash = false;
	b8 update_entities = game->state != e_game_state_defeat;
	if(handle_input) {
		can_dash = true;
		if(game->player.state == e_player_state_dashing) {
			s_speed_boost* boost = &game->speed_boost_arr[game->player.dash_target];
			if(v3_distance(game->player.pos, boost->pos) >= 0.01f) {
				can_dash = false;
			}
		}
	}

	{
		float passed = game->update_time - game->want_dash_timestamp;
		if(
			game->hovered_boost >= 0 && passed <= 0.1f && game->want_dash_timestamp > 0 &&
			game->hovered_boost != game->player.dash_target && can_dash
		) {
			game->want_dash_timestamp = 0;
			set_player_state(e_player_state_dashing);
			game->player.dash_target = game->hovered_boost;
		}
	}

	if(game->state == e_game_state_defeat) {
		float passed = game->update_time - game->defeat_timestamp;
		if(passed >= 0.5f) {
			game->dont_reset_game = false;
		}
	}

	switch(game->player.state) {
		case e_player_state_default: {
			if(handle_input) {
				if(v3_length(game->player.vel) > 0.001f) {
					game->player.pos += game->player.vel;
					game->player.last_dir = game->player.vel;
					game->player.vel *= 0.9f;
				}
				else {
					s_v3 temp0 = game->player.wanted_pos;
					s_v3 temp1 = game->player.pos;
					temp0.z = 0;
					temp1.z = 0;
					s_v3 v = temp0 - temp1;
					float dist = v3_length(v);
					float speed = smoothstep(0.0f, 5.0f, dist) * 0.3f;
					v = v3_set_mag(v, min(dist, speed));
					constexpr float z_speed = 0.1f;
					game->player.z_speed.target = z_speed;
					if(game->down_input.speed_boost) {
						game->player.z_speed.target = z_speed * 3;
					}
					game->player.z_speed.curr = go_towards(game->player.z_speed.curr, game->player.z_speed.target, 0.01f);
					v.z = -game->player.z_speed.curr;
					game->player.pos += v;
					game->player.last_dir = v;
				}
				at_least_ptr(&game->player.pos.x, -c_wall_x + 2);
				at_most_ptr(&game->player.pos.x, c_wall_x - 2);
			}
		} break;

		case e_player_state_dashing: {
			assert(game->player.dash_target >= 0);
			s_speed_boost* boost = &game->speed_boost_arr[game->player.dash_target];
			game->player.pos = go_towards(game->player.pos, boost->pos, 1.0f);
			game->player.last_dir = v3_normalized(boost->pos - game->player.pos);
			if(v3_distance(game->player.pos, boost->pos) < 0.01f) {
				game->speed_boost_arr.remove_and_swap(game->player.dash_target);
				set_player_state(e_player_state_post_dash);
				game->player.dash_target = -1;
				game->player.post_dash_timestamp = game->update_time;
				play_sound(e_sound_pop);
			}
		} break;

		case e_player_state_post_dash: {
			float passed = game->update_time - game->player.post_dash_timestamp;
			if(passed >= 0.5f) {
				set_player_state(e_player_state_default);
			}
		} break;
	}

	if(game->state == e_game_state_default) {
		foreach_val(proj_i, proj, game->projectile_arr) {
			if(sphere_vs_sphere(game->player.pos, c_player_radius, proj.pos, proj.radius)) {
				if(proj.type == e_projectile_type_default) {
					game->state = e_game_state_defeat;
					game->defeat_timestamp = game->update_time;
					play_sound(e_sound_defeat);
					break;
				}
				else if(proj.type == e_projectile_type_bounce) {
					game->player.vel = game->player.last_dir * -5;
					game->projectile_arr.remove_and_swap(proj_i);
					if(game->player.state == e_player_state_dashing || game->player.state == e_player_state_post_dash) {
						set_player_state(e_player_state_default);
					}
					proj_i -= 1;
					play_sound(e_sound_pop);
				}
			}
		}
	}

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update projectiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		foreach_ptr(proj_i, proj, game->projectile_arr) {
			proj->prev_pos = proj->pos;
			if(update_entities) {
				if(proj->going_right) {
					proj->pos.x += 0.1f;
				}
				else {
					proj->pos.x -= 0.1f;
				}
				if(proj->pos.x <= -c_wall_x + 2) {
					proj->going_right = !proj->going_right;
				}
				else if(proj->pos.x >= c_wall_x - 2) {
					proj->going_right = !proj->going_right;
				}
			}
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update projectiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	game->update_time += (float)c_update_delay;
}

func void render(float interp_dt, float delta)
{
	game->render_frame_arena.used = 0;

	for(int i = 0; i < e_shader_count; i += 1) {
		for(int j = 0; j < e_texture_count; j += 1) {
			for(int k = 0; k < e_mesh_count; k += 1) {
				game->render_group_index_arr[i][j][k] = -1;
			}
		}
	}
	game->render_group_arr.count = 0;
	memset(game->render_instance_count, 0, sizeof(game->render_instance_count));

	s_m4 light_projection = make_orthographic(-50, 50, -50, 50, -50, 50);
	s_v3 sun_pos = v3(0, -10, 10);
	s_v3 sun_dir = v3_normalized(v3(1, 1, -1));
	s_m4 light_view = look_at(sun_pos, sun_pos + sun_dir, v3(0, 0, 1));
	s_m4 ortho = make_orthographic(0, (float)g_platform_data->window_size.x, (float)g_platform_data->window_size.y, 0, -1, 1);
	s_m4 perspective = make_perspective(60.0f, c_world_size.x / c_world_size.y, 0.01f, 100.0f);

	s_v3 player_pos = lerp_v3(game->player.prev_pos, game->player.pos, interp_dt);

	s_v3 cam_pos = v3(
		0, -20, player_pos.z - 5
	);
	s_v3 cam_forward = v3_normalized(v3(0, 1, -0.1f));
	s_m4 view = look_at(cam_pos, cam_pos + cam_forward, c_up_axis);

	s_ray ray = get_camera_ray(cam_pos, view, perspective, g_mouse, c_world_size);
	// printf("%f, %f, %f\n", ray.dir.x, ray.dir.y, ray.dir.z);

	{
		s_v3 p = ray_at_y(ray, 0.0f);
		game->player.wanted_pos = p;
		// p.z = 0;
		// s_v3 temp = game->player.pos;
		// temp.z = 0;
		// game->player_dir = v3_normalized(p - temp);
		// float z_speed = range_lerp(g_mouse.y / c_world_size.y, 0, 1, 0.1f, 2.0f);
		// game->player_dir.z = -z_speed;
	}

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		render into depth start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		// draw_game(e_shader_depth_only);
		// s_render_flush_data data = make_render_flush_data();
		// data.projection = light_projection;
		// data.view = light_view;
		// data.fbo = game->shadow_map_fbo;
		// clear_framebuffer_depth(game->shadow_map_fbo.id);
		// render_flush(data, true);
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		render into depth end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	if(game->view_state == e_view_state_shadow_map) {
		draw_texture_screen(c_world_center, c_world_size, make_color(1), e_texture_shadow_map);
		s_render_flush_data data = make_render_flush_data(cam_pos);
		data.projection = ortho;
		clear_framebuffer_depth(0);
		clear_framebuffer_color(0, v4(0.0f, 0, 0, 0));
		render_flush(data, true);
	}

	else if(game->view_state == e_view_state_curr_depth) {
		draw_game(ray, interp_dt);
		s_render_flush_data data = make_render_flush_data(cam_pos);
		data.projection = light_projection;
		data.view = light_view;
		data.light_projection = light_projection;
		data.light_view = light_view;
		clear_framebuffer_depth(0);
		clear_framebuffer_color(0, v4(0.0f, 0, 0, 0));
		render_flush(data, true);
	}
	else {
		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		render scene start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		{
			draw_game(ray, interp_dt);
			s_render_flush_data data = make_render_flush_data(cam_pos);
			data.projection = perspective;
			data.view = view;
			data.light_projection = light_projection;
			data.light_view = light_view;
			clear_framebuffer_depth(0);
			clear_framebuffer_color(0, v4(0.0f, 0, 0, 0));
			render_flush(data, true);
		}
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		render scene end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	}

	SDL_GL_SwapWindow(g_platform_data->window);

	game->render_time += delta;
}

func f64 get_seconds()
{
	u64 now =	SDL_GetPerformanceCounter();
	return (now - g_platform_data->start_cycles) / (f64)g_platform_data->cycle_frequency;
}

func void on_gl_error(const char* expr, char* file, int line, int error)
{
	#define m_gl_errors \
	X(GL_INVALID_ENUM, "GL_INVALID_ENUM") \
	X(GL_INVALID_VALUE, "GL_INVALID_VALUE") \
	X(GL_INVALID_OPERATION, "GL_INVALID_OPERATION") \
	X(GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW") \
	X(GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW") \
	X(GL_OUT_OF_MEMORY, "GL_STACK_OUT_OF_MEMORY") \
	X(GL_INVALID_FRAMEBUFFER_OPERATION, "GL_STACK_INVALID_FRAME_BUFFER_OPERATION")

	const char* error_str;
	#define X(a, b) case a: { error_str = b; } break;
	switch(error) {
		m_gl_errors
		default: {
			error_str = "unknown error";
		} break;
	}
	#undef X
	#undef m_gl_errors

	printf("GL ERROR: %s - %i (%s)\n", expr, error, error_str);
	printf("  %s(%i)\n", file, line);
	printf("--------\n");
}

func void on_failed_assert(char* condition, char* file, int line)
{
	printf("Failed assert at %s(%i)\n", file, line);
	printf("\t%s\n", condition);
	printf("-----------------------------\n");

	#if defined(_WIN32)
	__debugbreak();
	#else
	__builtin_trap();
	#endif
}

func void draw_rect(s_v2 pos, s_v2 size, s_v4 color)
{
	s_instance_data data = zero;
	data.model = m4_translate(v3(pos, 0));
	data.model = m4_multiply(data.model, m4_scale(v3(size, 1)));
	data.color = color;

	add_to_render_group(data, e_shader_mesh, e_texture_white, e_mesh_quad);
}

func void draw_texture_screen(s_v2 pos, s_v2 size, s_v4 color, e_texture texture_id)
{
	s_instance_data data = zero;
	data.model = m4_translate(v3(pos, 0));
	data.model = m4_multiply(data.model, m4_scale(v3(size, 1)));
	data.color = color;

	add_to_render_group(data, e_shader_flat, texture_id, e_mesh_quad);
}

func void draw_mesh(e_mesh mesh_id, s_m4 model, s_v4 color, e_shader shader_id)
{
	s_instance_data data = zero;
	data.model = model;
	data.color = color;
	add_to_render_group(data, shader_id, e_texture_white, mesh_id);
}

func void draw_mesh(e_mesh mesh_id, s_v3 pos, s_v3 size, s_v4 color, e_shader shader_id)
{
	s_m4 model = m4_translate(pos);
	model = m4_multiply(model, m4_scale(size));
	draw_mesh(mesh_id, model, color, shader_id);
}

func void bind_framebuffer(u32 fbo)
{
	if(game->curr_fbo != fbo) {
		gl(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
		game->curr_fbo = fbo;
	}
}

func void clear_framebuffer_color(u32 fbo, s_v4 color)
{
	bind_framebuffer(fbo);
	glClearColor(color.x, color.y, color.z, color.w);
	glClear(GL_COLOR_BUFFER_BIT);
}

func void clear_framebuffer_depth(u32 fbo)
{
	bind_framebuffer(fbo);
	set_depth_mode(e_depth_mode_read_and_write);
	glClear(GL_DEPTH_BUFFER_BIT);
}

func void render_flush(s_render_flush_data data, b8 reset_render_count)
{
	bind_framebuffer(data.fbo.id);

	if(data.fbo.id == 0) {
		s_rect letterbox = do_letterbox(v2(g_platform_data->window_size), c_world_size);
		glViewport((int)letterbox.x, (int)letterbox.y, (int)letterbox.w, (int)letterbox.h);
	}
	else {
		glViewport(0, 0, data.fbo.size.x, data.fbo.size.y);
	}

	set_cull_mode(data.cull_mode);
	set_depth_mode(data.depth_mode);
	set_blend_mode(data.blend_mode);

	{
		gl(glBindBuffer(GL_UNIFORM_BUFFER, game->ubo));
		s_uniform_data uniform_data = zero;
		uniform_data.view = data.view;
		uniform_data.projection = data.projection;
		uniform_data.light_view = data.light_view;
		uniform_data.light_projection = data.light_projection;
		uniform_data.render_time = game->render_time;
		uniform_data.cam_pos = data.cam_pos;
		// data.view = render_pass_data.view;
		// data.projection = render_pass_data.projection;
		// data.base_res = g_base_res;
		// data.window_size.x = (float)g_platform_data.window_width;
		// data.window_size.y = (float)g_platform_data.window_height;
		// data.time = (float)gr->total_time;
		// data.mouse = g_platform_data.mouse;
		// data.cam_pos = render_pass_data.cam_pos;
		gl(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(s_uniform_data), &uniform_data));
	}

	foreach_val(group_i, group, game->render_group_arr) {
		s_mesh* mesh = &game->mesh_arr[group.mesh_id];
		int* instance_count = &game->render_instance_count[group.shader_id][group.texture_id][group.mesh_id];
		assert(*instance_count > 0);
		s_instance_data* instance_data = game->render_instance_arr[group.shader_id][group.texture_id][group.mesh_id];

		gl(glUseProgram(game->shader_arr[group.shader_id].id));

		int in_texture_loc = glGetUniformLocation(game->shader_arr[group.shader_id].id, "in_texture");
		int shadow_map_loc = glGetUniformLocation(game->shader_arr[group.shader_id].id, "shadow_map");
		if(in_texture_loc >= 0) {
			glUniform1i(in_texture_loc, 0);
			gl(glActiveTexture(GL_TEXTURE0));
			gl(glBindTexture(GL_TEXTURE_2D, game->texture_arr[group.texture_id].id));
		}
		if(shadow_map_loc >= 0) {
			glUniform1i(shadow_map_loc, 1);
			glActiveTexture(GL_TEXTURE1);
			gl(glBindTexture(GL_TEXTURE_2D, game->texture_arr[e_texture_shadow_map].id));
		}

		gl(glBindVertexArray(mesh->vao));
		gl(glBindBuffer(GL_ARRAY_BUFFER, mesh->instance_vbo.id));
		gl(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(s_instance_data) * *instance_count, instance_data));
		gl(glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertex_count, *instance_count));
		if(reset_render_count) {
			game->render_group_arr.remove_and_swap(group_i);
			group_i -= 1;
			*instance_count = 0;
		}
	}
}

func void add_to_render_group(s_instance_data data, e_shader shader_id, e_texture texture_id, e_mesh mesh_id)
{
	s_render_group render_group = zero;
	render_group.shader_id = shader_id;
	render_group.texture_id = texture_id;
	render_group.mesh_id = mesh_id;

	s_mesh* mesh = &game->mesh_arr[render_group.mesh_id];

	int* render_group_index = &game->render_group_index_arr[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	if(*render_group_index < 0) {
		game->render_group_arr.add(render_group);
		*render_group_index = game->render_group_arr.count - 1;
	}
	int* count = &game->render_instance_count[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	int* max_elements = &game->render_instance_max_elements[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	s_instance_data* ptr = game->render_instance_arr[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	b8 expand = *max_elements <= *count;
	b8 get_new_ptr = *count <= 0 || expand;
	int new_max_elements = *max_elements;
	if(expand) {
		if(new_max_elements <= 0) {
			new_max_elements = 64;
		}
		else {
			new_max_elements *= 2;
		}
		if(new_max_elements > mesh->instance_vbo.max_elements) {
			gl(glBindBuffer(GL_ARRAY_BUFFER, mesh->instance_vbo.id));
			gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_instance_data) * new_max_elements, null, GL_DYNAMIC_DRAW));
			mesh->instance_vbo.max_elements = new_max_elements;
		}
	}
	if(get_new_ptr) {
		s_instance_data* temp = (s_instance_data*)arena_alloc(&game->render_frame_arena, sizeof(s_instance_data) * new_max_elements);
		if(*count > 0) {
			memcpy(temp, ptr, *count * sizeof(s_instance_data));
		}
		game->render_instance_arr[render_group.shader_id][render_group.texture_id][render_group.mesh_id] = temp;
		ptr = temp;
	}
	*max_elements = new_max_elements;
	*(ptr + *count) = data;
	*count += 1;
}

func s_shader load_shader_from_file(char* file, s_linear_arena* arena)
{
	b8 delete_shaders[2] = {true, true};
	char* src = (char*)read_file(file, arena);
	assert(src);

	u32 shader_arr[] = {glCreateShader(GL_VERTEX_SHADER), glCreateShader(GL_FRAGMENT_SHADER)};

	#if defined(__EMSCRIPTEN__)
	const char* header = "#version 300 es\nprecision highp float;\n";
	#else
	const char* header = "#version 330 core\n";
	#endif

	char* shared_src = (char*)read_file("src/shader_shared.h", arena);
	assert(shared_src);

	for(int i = 0; i < 2; i += 1) {
		const char* src_arr[] = {header, "", "", shared_src, src};
		if(i == 0) {
			src_arr[1] = "#define m_vertex 1\n";
			src_arr[2] = "#define shared_var out\n";
		}
		else {
			src_arr[1] = "#define m_fragment 1\n";
			src_arr[2] = "#define shared_var in\n";
		}
		gl(glShaderSource(shader_arr[i], array_count(src_arr), (const GLchar * const *)src_arr, null));
		gl(glCompileShader(shader_arr[i]));

		int compile_success;
		char info_log[1024];
		gl(glGetShaderiv(shader_arr[i], GL_COMPILE_STATUS, &compile_success));

		if(!compile_success) {
			gl(glGetShaderInfoLog(shader_arr[i], sizeof(info_log), null, info_log));
			printf("Failed to compile shader: %s\n%s", file, info_log);
			delete_shaders[i] = false;
		}
	}

	b8 detach_shaders = delete_shaders[0] && delete_shaders[1];

	u32 program = 0;
	if(delete_shaders[0] && delete_shaders[1]) {
		program = gl(glCreateProgram());
		gl(glAttachShader(program, shader_arr[0]));
		gl(glAttachShader(program, shader_arr[1]));
		gl(glLinkProgram(program));

		int linked = 0;
		gl(glGetProgramiv(program, GL_LINK_STATUS, &linked));
		if(!linked) {
			char info_log[1024] = zero;
			gl(glGetProgramInfoLog(program, sizeof(info_log), null, info_log));
			printf("FAILED TO LINK: %s\n", info_log);
		}
	}

	if(detach_shaders) {
		gl(glDetachShader(program, shader_arr[0]));
		gl(glDetachShader(program, shader_arr[1]));
	}

	if(delete_shaders[0]) {
		gl(glDeleteShader(shader_arr[0]));
	}
	if(delete_shaders[1]) {
		gl(glDeleteShader(shader_arr[1]));
	}

	s_shader result = zero;
	result.id = program;
	return result;
}

func u8* read_file(char* path, s_linear_arena* arena)
{
	FILE* file = fopen(path, "rb");
	b8 read_file = false;
	u8* result = null;
	if(file) {
		read_file = true;
	}
	if(read_file) {
		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		fseek(file, 0, SEEK_SET);
		result = arena_alloc(arena, size + 1);
		fread(result, 1, size, file);
		fclose(file);
		result[size] = '\0';
	}
	return result;
}

func void set_window_size(int width, int height)
{
	SDL_SetWindowSize(g_platform_data->window, width, height);
}

func s_rect do_letterbox(s_v2 curr_size, s_v2 base_size)
{
	s_rect rect_result = zero;
	rect_result.size = curr_size;
	float curr_ar = curr_size.x / curr_size.y;
	float base_ar = base_size.x / base_size.y;

	// @Note(tkap, 30/12/2024): We are too wide
	if(curr_ar > base_ar) {
		rect_result.w = base_ar / curr_ar * curr_size.x;
		rect_result.x = (curr_size.x - rect_result.w) * 0.5f;
	}
	// @Note(tkap, 30/12/2024): We are too tall
	else if(base_ar > curr_ar) {
		rect_result.h = curr_ar / base_ar * curr_size.y;
		rect_result.y = (curr_size.y - rect_result.h) * 0.5f;
	}
	return rect_result;
}

func s_render_flush_data make_render_flush_data(s_v3 cam_pos)
{
	s_render_flush_data result = zero;
	result.view = m4_identity();
	result.projection = m4_identity();
	result.light_view = m4_identity();
	result.light_projection = m4_identity();
	result.cam_pos = cam_pos;
	result.cull_mode = e_cull_mode_disabled;
	result.blend_mode = e_blend_mode_normal;
	result.depth_mode = e_depth_mode_read_and_write;
	return result;
}

func s_mesh make_mesh_from_vertices(s_vertex* vertex_arr, int vertex_count)
{
	s_mesh result = zero;
	gl(glGenVertexArrays(1, &result.vao));
	gl(glBindVertexArray(result.vao));

	int attrib_index = 0;
	{
		gl(glGenBuffers(1, &result.vertex_vbo));
		gl(glBindBuffer(GL_ARRAY_BUFFER, result.vertex_vbo));

		u8* offset = 0;
		constexpr int stride = sizeof(float) * 12;

		gl(glVertexAttribPointer(attrib_index, 3, GL_FLOAT, GL_FALSE, stride, offset)); // pos
		gl(glEnableVertexAttribArray(attrib_index));
		attrib_index += 1;
		offset += sizeof(float) * 3;

		gl(glVertexAttribPointer(attrib_index, 3, GL_FLOAT, GL_FALSE, stride, offset)); // normal
		gl(glEnableVertexAttribArray(attrib_index));
		attrib_index += 1;
		offset += sizeof(float) * 3;

		gl(glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, stride, offset)); // rgba
		gl(glEnableVertexAttribArray(attrib_index));
		attrib_index += 1;
		offset += sizeof(float) * 4;

		gl(glVertexAttribPointer(attrib_index, 2, GL_FLOAT, GL_FALSE, stride, offset)); // uv
		gl(glEnableVertexAttribArray(attrib_index));
		attrib_index += 1;
		offset += sizeof(float) * 2;

		result.vertex_count = vertex_count;
		gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertex) * vertex_count, vertex_arr, GL_STATIC_DRAW));
	}

	{
		gl(glGenBuffers(1, &result.instance_vbo.id));
		gl(glBindBuffer(GL_ARRAY_BUFFER, result.instance_vbo.id));

		u8* offset = 0;
		constexpr int stride = sizeof(float) * 20;

		gl(glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, stride, offset)); // instance color
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 4;

		gl(glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, stride, offset)); // mat4_0
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 4;

		gl(glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, stride, offset)); // mat4_1
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 4;

		gl(glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, stride, offset)); // mat4_2
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 4;

		gl(glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, stride, offset)); // mat4_3
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 4;
	}
	return result;
}

func s_ply_mesh parse_ply_mesh(char* path, s_linear_arena* arena)
{
	char* data = (char*)read_file(path, arena);
	char* cursor = data;
	s_ply_mesh result = zero;
	b8 has_color = strstr(cursor, "property uchar red") != null;

	{
		char* temp = strstr(cursor, "element vertex ") + 15;
		result.vertex_count = atoi(temp);
		assert(result.vertex_count <= c_max_vertices);
	}
	{
		char* temp = strstr(cursor, "element face ") + 13;
		result.face_count = atoi(temp);
		assert(result.face_count <= c_max_faces);
	}
	cursor = strstr(data, "end_header") + 11;

	{
		u8* temp = (u8*)cursor;
		for(int i = 0; i < result.vertex_count; i += 1) {
			memcpy(&result.vertex_arr[i].pos, temp, sizeof(s_v3));
			temp += sizeof(s_v3);
			memcpy(&result.vertex_arr[i].normal, temp, sizeof(s_v3));
			temp += sizeof(s_v3);
			if(has_color) {
				result.vertex_arr[i].color.x = (*temp) / 255.0f;
				temp += 1;
				result.vertex_arr[i].color.y = (*temp) / 255.0f;
				temp += 1;
				result.vertex_arr[i].color.z = (*temp) / 255.0f;
				temp += 1;
				result.vertex_arr[i].color.w = (*temp) / 255.0f;
				temp += 1;
			}
			else {
				result.vertex_arr[i].color = make_color(1);
			}
			memcpy(&result.vertex_arr[i].uv, temp, sizeof(s_v2));
			temp += sizeof(s_v2);
		}
		cursor = (char*)temp;
	}
	{
		s_ply_face* temp = (s_ply_face*)cursor;
		for(int i = 0; i < result.face_count; i += 1) {
			result.face_arr[i] = *temp;
			assert(result.face_arr[i].index_count == 3);
			temp += 1;
		}
	}
	return result;
}

func s_mesh make_mesh_from_ply_file(char* file, s_linear_arena* arena)
{
	s_ply_mesh ply_mesh = parse_ply_mesh(file, arena);
	int vertex_count = ply_mesh.face_count * 3;
	assert(vertex_count < c_max_vertices);
	s_vertex vertex_arr[c_max_vertices] = zero;
	for(int i = 0; i < ply_mesh.face_count; i += 1) {
		vertex_arr[i * 3 + 0] = ply_mesh.vertex_arr[ply_mesh.face_arr[i].index_arr[0]];
		vertex_arr[i * 3 + 1] = ply_mesh.vertex_arr[ply_mesh.face_arr[i].index_arr[1]];
		vertex_arr[i * 3 + 2] = ply_mesh.vertex_arr[ply_mesh.face_arr[i].index_arr[2]];
	}
	s_mesh result = make_mesh_from_vertices(vertex_arr, vertex_count);
	return result;
}

func void set_cull_mode(e_cull_mode mode)
{
	switch(mode) {
		case e_cull_mode_disabled: {
			glDisable(GL_CULL_FACE);
		} break;

		case e_cull_mode_back_ccw: {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);
		} break;

		case e_cull_mode_front_ccw: {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glFrontFace(GL_CCW);
		} break;

		case e_cull_mode_back_cw: {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);
		} break;

		case e_cull_mode_front_cw: {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glFrontFace(GL_CW);
		} break;
		invalid_default_case;
	}
}

func void set_depth_mode(e_depth_mode mode)
{
	switch(mode) {
		case e_depth_mode_no_read_no_write: {
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
		} break;
		case e_depth_mode_read_and_write: {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glDepthMask(GL_TRUE);
		} break;
		case e_depth_mode_read_no_write: {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glDepthMask(GL_FALSE);
		} break;
		case e_depth_mode_no_read_yes_write: {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			glDepthMask(GL_TRUE);
		} break;
		invalid_default_case;
	}
}

func void set_blend_mode(e_blend_mode mode)
{
	switch(mode) {
		case e_blend_mode_disabled: {
			glDisable(GL_BLEND);
		} break;
		case e_blend_mode_additive: {
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
		} break;
		case e_blend_mode_premultiply_alpha: {
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		} break;
		case e_blend_mode_multiply: {
			glEnable(GL_BLEND);
			glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		} break;
		case e_blend_mode_multiply_inv: {
			glEnable(GL_BLEND);
			// glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
			glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE);
		} break;
		case e_blend_mode_normal: {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} break;
		case e_blend_mode_additive_no_alpha: {
			glEnable(GL_BLEND);
			glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
		} break;
		invalid_default_case;
	}
}

func void draw_game(s_ray ray, float interp_dt)
{
	s_v3 ray_p = ray_at_y(ray, 0.0f);
	s_v3 player_pos = lerp_v3(game->player.prev_pos, game->player.pos, interp_dt);

	for(int i = 0; i < 20; i += 1) {
		int vertical_index = ceilfi((player_pos.z + 10) / 2);
		float z = vertical_index * 2.0f;
		z -= i * 2.0f;
		s_v4 color = make_color((vertical_index + i) & 1 ? 0.5f : 1.0f);
		{
			s_m4 model = m4_translate(v3(-c_wall_x, 0.0f, z));
			model = m4_multiply(model, m4_scale(v3(2.0f)));
			draw_mesh(e_mesh_cube, model, color, e_shader_mesh);
		}
		{
			s_m4 model = m4_translate(v3(c_wall_x, 0.0f, z));
			model = m4_multiply(model, m4_scale(v3(2.0f)));
			draw_mesh(e_mesh_cube, model, color, e_shader_mesh);
		}
	}

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw projectiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		foreach_val(proj_i, proj, game->projectile_arr) {
			s_v3 proj_pos = lerp_v3(proj.prev_pos, proj.pos, interp_dt);
			s_m4 model = m4_translate(proj_pos);
			scale_m4_by_radius(&model, proj.radius);
			s_v4 color = make_color(1.0f, 0.1f, 0.1f);
			if(proj.type == e_projectile_type_bounce) {
				color = make_color(0.1f, 0.1f, 1.0f);
			}
			draw_mesh(e_mesh_sphere, model, color, e_shader_fresnel);
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw projectiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	game->hovered_boost = -1;
	foreach_val(boost_i, boost, game->speed_boost_arr) {
		s_m4 model = m4_translate(boost.pos);
		scale_m4_by_radius(&model, c_boost_radius);
		s_v4 color = make_color(0.1f, 1, 0.1f);
		if(boost_is_hovered(ray_p, boost.pos)) {
			color = make_color(0.5f, 1, 0.5f);
			game->hovered_boost = boost_i;
		}
		draw_mesh(e_mesh_sphere, model, color, e_shader_mesh);
	}

	{
		s_m4 model = m4_translate(player_pos);
		scale_m4_by_radius(&model, c_player_radius);
		draw_mesh(e_mesh_sphere, model, make_color(1), e_shader_mesh);
	}


	// {
	// 	s_m4 model = m4_translate(game->player.pos);
	// 	model = m4_multiply(model, quaternion_to_m4(game->player_rot));
	// 	model = m4_multiply(model, m4_scale(v3(5)));
	// 	draw_mesh(e_mesh_monkey, model, make_color(1), shader_id);
	// }

	// {
	// 	s_rng rng = make_rng(0);
	// 	for(int y = 0; y < 100; y += 1) {
	// 		for(int x = 0; x < 100; x += 1) {
	// 			s_m4 model = m4_translate(v3(x - 50, y - 50, 0.0f));
	// 			model = m4_multiply(model, m4_scale(v3(0.5f, 0.5f, 0.5f)));
	// 			draw_mesh(e_mesh_cube, model, make_color((x+y)&1?0.5f:1.0f), shader_id);
	// 		}
	// 	}
	// 	for(int y = 0; y < 100; y += 1) {
	// 		for(int x = 0; x < 100; x += 1) {
	// 			if(chance100(&rng, 10)) {
	// 				s_m4 model = m4_translate(v3(x - 50, y - 50, 5));
	// 				model = m4_multiply(model, m4_scale(v3(0.5f, 0.5f, 0.5f)));
	// 				draw_mesh(e_mesh_cube, model, make_color((x+y)&1?0.5f:1.0f), shader_id);
	// 			}
	// 		}
	// 	}
	// }
}

template <typename t>
func void toggle(t* out, t a, t b)
{
	if(*out == a) {
		*out = b;
	}
	else if(*out == b) {
		*out = a;
	}
	invalid_else;
}

func b8 boost_is_hovered(s_v3 mouse_point, s_v3 boost_pos)
{
	float dist = v3_distance(mouse_point, boost_pos);
	b8 result = dist <= c_boost_radius * 2;
	return result;
}

func Mix_Chunk* load_sound_from_file(char* path)
{
	Mix_Chunk* chunk = Mix_LoadWAV(path);
	assert(chunk);
	return chunk;
}

func void play_sound(e_sound sound_id)
{
	Mix_Chunk* chunk = game->sound_arr[sound_id];
	Mix_PlayChannel(-1, chunk, 0);
}

func void set_player_state(e_player_state state)
{
	if(state != e_player_state_dashing) {
		game->player.dash_target = -1;
	}
	game->player.state = state;
}