
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

#if defined(__EMSCRIPTEN__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#endif

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT
#include "external/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_assert
#include "external/stb_truetype.h"
#pragma warning(pop)

#if defined(__EMSCRIPTEN__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#endif

#if defined(__EMSCRIPTEN__)
#define m_gl_single_channel GL_LUMINANCE
#else // __EMSCRIPTEN__
#define m_gl_single_channel GL_RED
#endif // __EMSCRIPTEN__


#include "tk_types.h"


#if defined(__EMSCRIPTEN__)
global constexpr b8 c_on_web = true;
#else
global constexpr b8 c_on_web = false;
#endif

#if defined(m_debug)
#define gl(...) __VA_ARGS__; {int error = glGetError(); if(error != 0) { on_gl_error(#__VA_ARGS__, __FILE__, __LINE__, error); }}
#else // m_debug
#define gl(...) __VA_ARGS__
#endif // m_debug

#include "shared.h"
#include "tk_arena.h"
#include "tk_array.h"

#define m_tk_random_impl
#include "tk_random.h"

#include "tk_math.h"
#include "config.h"
#include "leaderboard.h"
#include "game.h"
#include "tk_color.h"
#include "shader_shared.h"


global s_platform_data* g_platform_data;
global s_game* game;
global s_v2 g_mouse;
global b8 g_click;

#include "shared.cpp"

#if defined(__EMSCRIPTEN__)
#include "leaderboard.cpp"
#endif

m_dll_export void init(s_platform_data* platform_data)
{
	g_platform_data = platform_data;
	game = (s_game*)platform_data->memory;
	game->speed_index = 5;
	game->rng = make_rng(1234);
	game->reload_shaders = true;
	game->speed = 1;

	SDL_StartTextInput();

	set_state0_next_frame(e_game_state0_main_menu);

	u8* cursor = platform_data->memory + sizeof(s_game);

	{
		game->update_frame_arena = make_arena_from_memory(cursor, 10 * c_mb);
		cursor += 10 * c_mb;
	}
	{
		game->render_frame_arena = make_arena_from_memory(cursor, 10 * c_mb);
		cursor += 10 * c_mb;
	}
	{
		game->circular_arena = make_circular_arena_from_memory(cursor, 10 * c_mb);
		cursor += 10 * c_mb;
	}

	platform_data->cycle_frequency = SDL_GetPerformanceFrequency();
	platform_data->start_cycles = SDL_GetPerformanceCounter();

	platform_data->window_size.x = (int)c_world_size.x;
	platform_data->window_size.y = (int)c_world_size.y;

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
		game->mesh_arr[e_mesh_cube] = make_mesh_from_obj_file("assets/cube.obj", &game->render_frame_arena);
		game->mesh_arr[e_mesh_sphere] = make_mesh_from_obj_file("assets/sphere.obj", &game->render_frame_arena);
	}

	for(int i = 0; i < e_sound_count; i += 1) {
		game->sound_arr[i] = load_sound_from_file(c_sound_data_arr[i].path, c_sound_data_arr[i].volume);
	}

	for(int i = 0; i < e_texture_count; i += 1) {
		char* path = c_texture_path_arr[i];
		if(strlen(path) > 0) {
			game->texture_arr[i] = load_texture_from_file(path);
		}
	}

	game->font = load_font_from_file("assets/Inconsolata-Regular.ttf", 128, &game->render_frame_arena);

	#if defined(__EMSCRIPTEN__)
	load_or_create_leaderboard_id();
	#endif
}

m_dll_export void init_after_recompile(s_platform_data* platform_data)
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

m_dll_export void do_game(s_platform_data* platform_data)
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

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		handle state start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if(game->clear_state) {
		game->clear_state = false;
		game->state0.count = 0;
	}
	if(game->pop_state) {
		game->pop_state = false;
		while(true) {
			game->state0.pop_last();
			s_state temp = game->state0.get_last();
			if(!temp.temporary) { break; }
		}
		game->accumulator += c_update_delay;
	}

	if(game->next_state.valid) {
		game->state0.add(game->next_state.value);
		game->next_state.valid = zero;
		game->accumulator += c_update_delay;
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		handle state end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	input();
	float game_speed = c_game_speed_arr[game->speed_index] * game->speed;
	game->accumulator += delta64 * game_speed;
	#if defined(__EMSCRIPTEN__)
	game->accumulator = at_most(game->accumulator, 1.0);
	#else
	#endif
	while(game->accumulator >= c_update_delay) {
		game->accumulator -= c_update_delay;
		update();
	}
	float interp_dt = (float)(game->accumulator / c_update_delay);
	render(interp_dt, (float)delta64);
}

func void input()
{

	game->char_events.count = 0;
	game->key_events.count = 0;

	for(int i = 0; i < c_max_keys; i += 1) {
		game->input_arr[i].half_transition_count = 0;
	}

	g_click = false;
	{
		int x;
		int y;
		u32 state = SDL_GetMouseState(&x, &y);
		game->down_input.speed_boost = state & SDL_BUTTON(1);
		g_mouse = v2(x, y);
		s_rect letterbox = do_letterbox(v2(g_platform_data->window_size), c_world_size);
		g_mouse.x = range_lerp(g_mouse.x, letterbox.x, letterbox.x + letterbox.size.x, 0, c_world_size.x);
		g_mouse.y = range_lerp(g_mouse.y, letterbox.y, letterbox.y + letterbox.size.y, 0, c_world_size.y);
	}

	s_hard_game_data* hard_data = &game->hard_data;

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
				int key = event.key.keysym.sym;
				b8 is_down = event.type == SDL_KEYDOWN;
				handle_key_event(key, is_down, event.key.repeat != 0);

				if(key < c_max_keys) {
					game->input_arr[key].is_down = is_down;
					game->input_arr[key].half_transition_count += 1;
				}
				if(event.type == SDL_KEYDOWN) {
					if(key == SDLK_f && event.key.repeat == 0) {
						game->hard_data.soft_data.want_dash_timestamp = game->update_time;
					}
					else if(key == SDLK_r && event.key.repeat == 0) {
						if(event.key.keysym.mod & KMOD_LCTRL) {
							game->do_hard_reset = true;
						}
						else {
							game->do_soft_reset = true;
						}
					}
					#if defined(m_debug)
					else if(key == SDLK_KP_PLUS) {
						game->speed_index = circular_index(game->speed_index + 1, array_count(c_game_speed_arr));
						printf("Game speed: %f\n", c_game_speed_arr[game->speed_index]);
					}
					else if(key == SDLK_KP_MINUS) {
						game->speed_index = circular_index(game->speed_index - 1, array_count(c_game_speed_arr));
						printf("Game speed: %f\n", c_game_speed_arr[game->speed_index]);
					}
					else if(key == SDLK_t) {
						if(!hard_data->curr_checkpoint.valid) {
							hard_data->curr_checkpoint = maybe(0);
						}
						else {
							hard_data->curr_checkpoint = maybe(circular_index(hard_data->curr_checkpoint.value - 1, 10));
						}
						game->do_soft_reset = true;
					}
					else if(key == SDLK_y) {
						if(!hard_data->curr_checkpoint.valid) {
							hard_data->curr_checkpoint = maybe(0);
						}
						else {
							hard_data->curr_checkpoint = maybe(circular_index(hard_data->curr_checkpoint.value + 1, 10));
						}
						game->do_soft_reset = true;
					}
					#endif // m_debug
				}
			} break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1) {
					g_click = true;
				}
				if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 3) {
					game->hard_data.soft_data.want_dash_timestamp = game->update_time;
				}
				// int key = sdl_key_to_windows_key(event.button.button);
				// b8 is_down = event.type == SDL_MOUSEBUTTONDOWN;
				// handle_key_event(key, is_down, false);
			} break;

			case SDL_TEXTINPUT: {
				char c = event.text.text[0];
				game->char_events.add((char)c);
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

	e_game_state0 state0 = (e_game_state0)game->state0.get_last().value;
	s_hard_game_data* hard_data = &game->hard_data;
	s_soft_game_data* soft_data = &hard_data->soft_data;
	s_player* player = &soft_data->player;

	player->prev_pos = player->pos;

	switch(state0) {
		case e_game_state0_play: {

			if(game->do_hard_reset) {
				game->do_hard_reset = false;
				game->do_soft_reset = true;
				memset(hard_data, 0, sizeof(s_hard_game_data));
			}
			if(game->do_soft_reset) {
				game->do_soft_reset = false;
				memset(soft_data, 0, sizeof(s_soft_game_data));

				// @Note(tkap, 07/04/2025): If restarting without a checkpoint, reset the timer so that the player doesn't have to
				if(!hard_data->curr_checkpoint.valid) {
					hard_data->update_count = 0;
				}

				if(hard_data->curr_checkpoint.valid) {
					int index = hard_data->curr_checkpoint.value;
					player->pos.z = (index + 1) * (float)-c_checkpoint_step;
					player->prev_pos = player->pos;
				}

				init_obstacles();
			}

			b8 want_to_dash = false;
			{
				float passed = game->update_time - soft_data->want_dash_timestamp;
				if(passed <= 0.1f && soft_data->want_dash_timestamp > 0) {
					want_to_dash = true;
				}
			}
			b8 handle_input = soft_data->state == e_game_state1_default || soft_data->state == e_game_state1_pre_victory;
			b8 update_entities = soft_data->state != e_game_state1_defeat;

			if(handle_input && want_to_dash) {
				try_to_dash();
			}

			if(soft_data->state == e_game_state1_defeat) {
				float passed = game->update_time - soft_data->defeat_timestamp;
				if(passed >= 0.5f) {
					game->do_soft_reset = true;
				}
			}
			b8 do_move = handle_input;

			switch(player->state) {
				case e_player_state_default: {
					if(handle_input) {
						s_v3 temp0 = player->wanted_pos;
						s_v3 temp1 = player->pos;
						temp0.z = 0;
						temp1.z = 0;
						s_v3 v = temp0 - temp1;
						float dist = v3_length(v);
						float speed = smoothstep(0.0f, 5.0f, dist);
						v = v3_set_mag(v, min(dist, speed));
						constexpr float z_speed = 0.1f;
						player->z_speed.target = z_speed;
						if(game->down_input.speed_boost) {
							player->z_speed.target = z_speed * 3;
						}
						player->z_speed.curr = go_towards(player->z_speed.curr, player->z_speed.target, 0.01f);
						v.z = -player->z_speed.curr;
						v.x *= 0.035f;
						v.z *= 0.1f;
						player->vel += v;
					}
				} break;

				case e_player_state_dashing: {
					do_move = false;
					assert(soft_data->player.dash_target.valid);
					s_speed_boost boost = soft_data->speed_boost_arr[player->dash_target.value];
					s_v3 dir = v3_normalized(boost.pos - player->pos);
					player->vel = dir;
					player->pos = go_towards(player->pos, boost.pos, 1.0f);
					if(v3_distance(player->pos, boost.pos) < 0.01f) {
						soft_data->speed_boost_arr.remove_and_swap(soft_data->player.dash_target.value);
						set_player_state(e_player_state_post_dash);
						player->dash_target = maybe<int>();
						player->post_dash_timestamp = game->update_time;
						player->vel = zero;
						play_sound(e_sound_pop);

						{
							s_particle_spawn_data data = zero;
							data.shrink = 0.5f;
							data.duration = 0.5f;
							data.duration_rand = 0.5f;
							data.radius = 1.5f;
							data.radius_rand = 0.25f;
							data.color = make_color(0.5f, 1.0f, 0.5f);
							data.color_rand = 1.0f;
							data.dir = v3(1);
							data.dir_rand = v3(1);
							data.speed = 0.1f;
							data.speed_rand = 0.5f;
							spawn_particles(64, boost.pos, data);
						}
					}
				} break;

				case e_player_state_post_dash: {
					float passed = game->update_time - player->post_dash_timestamp;
					if(passed >= 0.5f) {
						set_player_state(e_player_state_default);
					}
				} break;
			}

			if(do_move) {
				player->pos += player->vel;

				float x_vel_len = fabsf(player->vel.x);

				if(player->pos.x < -c_wall_x + 2) {
					player->vel.x *= -0.8f;
					player->pos.x = -c_wall_x + 2;
					if(x_vel_len >= 0.25f) {
						play_sound(e_sound_knock);
					}
				}
				if(player->pos.x > c_wall_x - 2) {
					player->vel.x *= -0.8f;
					player->pos.x = c_wall_x - 2;
					if(x_vel_len >= 0.25f) {
						play_sound(e_sound_knock);
					}
				}
				player->vel *= 0.9f;
			}
			player->pos.z = clamp(player->pos.z, -c_bottom, 0.0f);

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		hit goal start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(soft_data->state == e_game_state1_default) {
				float d = v3_distance(c_goal_pos, player->pos);
				if(d < 2) {
					soft_data->pre_victory_timestamp = game->render_time;
					soft_data->state = e_game_state1_pre_victory;
					play_sound(e_sound_victory);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		hit goal end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		teleporter collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(teleporter_i, teleporter, soft_data->teleporter_arr) {
				if(sphere_vs_sphere(player->pos, c_player_radius, teleporter.pos, c_teleporter_radius * 0.25f)) {
					player->pos = teleporter.destination;
					player->prev_pos = player->pos;
					play_sound(e_sound_portal);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		teleporter collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		hit checkpoints start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				float z = c_checkpoint_step;
				int index = 0;
				while(z < c_bottom) {
					s_v3 pos = v3(0.0f, 0.0f, -z);
					if(!hard_data->curr_checkpoint.valid || hard_data->curr_checkpoint.value != index) {
						float d = v3_distance(player->pos, pos);
						if(d <= 5) {
							hard_data->curr_checkpoint = maybe(index);
							play_sound(e_sound_checkpoint);

							{
								s_particle_spawn_data data = zero;
								data.shrink = 0.5f;
								data.duration = 3.5f;
								data.duration_rand = 0.5f;
								data.radius = 1.5f;
								data.radius_rand = 0.25f;
								data.color = hex_to_rgb(0x3B83BD);
								data.color_rand = 1.0f;
								data.dir = v3(1);
								data.dir_rand = v3(1);
								data.speed = 0.1f;
								data.speed_rand = 0.5f;

								spawn_particles(128, pos, data);
							}
						}
					}
					index += 1;
					z += c_checkpoint_step;
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		hit checkpoints end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			if(soft_data->state == e_game_state1_default) {
				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		projectile collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				foreach_val(proj_i, proj, soft_data->projectile_arr) {
					float generous_radius = proj.radius - 0.35f;
					if(sphere_vs_sphere(player->pos, c_player_radius, proj.pos, generous_radius)) {
						if(
							proj.type == e_projectile_type_default || proj.type == e_projectile_type_static || proj.type == e_projectile_type_follow ||
							proj.type == e_projectile_type_diagonal
						) {
							soft_data->state = e_game_state1_defeat;
							soft_data->defeat_timestamp = game->update_time;
							play_sound(e_sound_defeat);
							break;
						}
						else if(proj.type == e_projectile_type_bounce) {
							player->vel = player->vel * -5;
							soft_data->projectile_arr.remove_and_swap(proj_i);
							if(player->state == e_player_state_dashing || player->state == e_player_state_post_dash) {
								set_player_state(e_player_state_default);
							}
							proj_i -= 1;
							play_sound(e_sound_clap);
						}
					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		projectile collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update projectiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_ptr(proj_i, proj, soft_data->projectile_arr) {
					proj->prev_pos = proj->pos;
					if(update_entities) {
						b8 should_move = proj->type != e_projectile_type_static && proj->type != e_projectile_type_follow &&
							proj->type != e_projectile_type_diagonal;
						if(should_move) {
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
						if(proj->type == e_projectile_type_follow) {
							if(proj->start_follow_timestamp > 0) {
								s_v3 dir = v3_normalized(player->pos - proj->pos);
								proj->pos += dir * 0.05f;
								s_time_data time_data = get_time_data(game->update_time, proj->start_follow_timestamp, 3.0f);
								if(time_data.percent >= 1) {
									soft_data->projectile_arr.remove_and_swap(proj_i);
									proj_i -= 1;
								}
							}
							else {
								float d = v3_distance(player->pos, proj->pos);
								if(d <= 15) {
									proj->start_follow_timestamp = game->update_time;
								}
							}
						}
						if(proj->type == e_projectile_type_diagonal) {
							proj->pos += proj->dir * 0.2f;
							if(sphere_out_of_bounds_right(proj->pos, proj->radius)) {
								proj->pos.x = c_wall_x - proj->radius * 2;
								proj->dir *= -1;
							}
							else if(sphere_out_of_bounds_left(proj->pos, proj->radius)) {
								proj->pos.x = -c_wall_x + proj->radius * 2;
								proj->dir *= -1;
							}
						}
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update projectiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			hard_data->update_count += 1;

		} break;
	}

	game->update_time += (float)c_update_delay;
}

func void render(float interp_dt, float delta)
{
	game->render_frame_arena.used = 0;

	#if defined(_WIN32)
	while(g_platform_data->hot_read_index[1] < g_platform_data->hot_write_index) {
		char* path = g_platform_data->hot_file_arr[g_platform_data->hot_read_index[1] % c_max_hot_files];
		if(strstr(path, ".shader")) {
			game->reload_shaders = true;
		}
		g_platform_data->hot_read_index[1] += 1;
	}
	#endif // _WIN32

	if(game->reload_shaders) {
		game->reload_shaders = false;

		for(int i = 0; i < e_shader_count; i += 1) {
			s_shader shader = load_shader_from_file(c_shader_path_arr[i], &game->render_frame_arena);
			if(shader.id > 0) {
				if(game->shader_arr[i].id > 0) {
					gl(glDeleteProgram(game->shader_arr[i].id));
				}
				game->shader_arr[i] = shader;

				#if defined(m_debug)
				printf("Loaded %s\n", c_shader_path_arr[i]);
				#endif // m_debug
			}
		}
	}

	s_hard_game_data* hard_data = &game->hard_data;
	s_soft_game_data* soft_data = &hard_data->soft_data;
	s_player* player = &soft_data->player;

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
	s_m4 ortho = make_orthographic(0, c_world_size.x, c_world_size.y, 0, -1, 1);
	s_m4 perspective = make_perspective(60.0f, c_world_size.x / c_world_size.y, 0.01f, 100.0f);

	s_v3 player_pos = lerp_v3(player->prev_pos, player->pos, interp_dt);

	s_v3 cam_pos = v3(
		0, -25, player_pos.z - 5
	);
	s_v3 cam_forward = v3_normalized(v3(0, 1, -0.1f));
	s_m4 view = look_at(cam_pos, cam_pos + cam_forward, c_up_axis);

	s_ray ray = get_camera_ray(cam_pos, view, perspective, g_mouse, c_world_size);

	{
		s_v3 p = ray_at_y(ray, 0.0f);
		player->wanted_pos = p;
	}

	game->speed = 1;
	if(soft_data->state == e_game_state1_pre_victory || soft_data->state == e_game_state1_default) {
		float d = v3_distance(c_goal_pos, player_pos);
		float slow = smoothstep(10.0, 0.0, d);
		game->speed = range_lerp(slow, 0, 1, 1, 0.1f);
	}

	clear_framebuffer_depth(0);
	clear_framebuffer_color(0, v4(0.0f, 0, 0, 0));

	e_game_state0 state0 = (e_game_state0)game->state0.get_last().value;

	switch(state0) {

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		main menu start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_main_menu: {

			if(do_button(S("Play"), wxy(0.5f, 0.5f), true) || is_key_pressed(SDLK_RETURN, true)) {
				set_state0_next_frame(e_game_state0_play);
				game->do_hard_reset = true;
			}

			if(do_button(S("Leaderboard"), wxy(0.5f, 0.6f), true)) {
				#if defined(__EMSCRIPTEN__)
				get_leaderboard(c_leaderboard_id);
				#endif
				set_state0_next_frame(e_game_state0_leaderboard);
			}

			if(do_button(S("Options"), wxy(0.5f, 0.7f), true)) {
				set_state0_next_frame(e_game_state0_options);
			}

			draw_text(c_game_name, wxy(0.5f, 0.2f), 128, make_color(1), true, &game->font);
			draw_text(S("www.twitch.tv/Tkap1"), wxy(0.5f, 0.3f), 32, make_color(0.6f), true, &game->font);

			{
				s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_no_read_no_write;
				render_flush(data, true);
			}

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		main menu end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		leaderboard start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_leaderboard: {
			do_leaderboard();

			{
				s_render_flush_data data = make_render_flush_data(zero, player_pos);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_no_read_no_write;
				render_flush(data, true);
			}

		} break;
		case e_game_state0_win_leaderboard: {
			do_leaderboard();

			{
				s_time_format data = update_count_to_time_format(game->update_count_at_win_time);
				s_len_str text = format_text("%02i:%02i.%i", data.minutes, data.seconds, data.milliseconds);
				draw_text(text, c_world_center * v2(1.0f, 0.2f), 64, make_color(1), true, &game->font);

				draw_text(S("Press R to restart..."), c_world_center * v2(1.0f, 0.4f), sin_range(48, 60, game->render_time * 8.0f), make_color(0.66f), true, &game->font);
			}

			b8 want_to_reset = is_key_pressed(SDLK_r, true);
			if(
				do_button(S("Restart"), c_world_size * v2(0.87f, 0.82f), true)
				|| is_key_pressed(SDLK_ESCAPE, true) || want_to_reset
			) {
				pop_game_state();
				game->do_hard_reset = true;
			}

			{
				s_render_flush_data data = make_render_flush_data(zero, player_pos);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_no_read_no_write;
				render_flush(data, true);
			}

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		leaderboard end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		options start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_options: {
			b8 escape = is_key_pressed(SDLK_ESCAPE, true);
			if(do_button(S("Back"), wxy(0.87f, 0.92f), true) || escape) {
				pop_game_state();
			}

			{
				s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_no_read_no_write;
				render_flush(data, true);
			}
		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		options end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		play start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_play: {
			switch(soft_data->state) {
				case e_game_state1_default:
				case e_game_state1_pre_victory:
				case e_game_state1_defeat: {

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		background start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					{
						draw_texture_screen(c_world_center, c_world_size, make_color(1), e_texture_white, e_shader_background, zero, zero);
						s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
						data.projection = ortho;
						data.depth_mode = e_depth_mode_no_read_no_write;
						render_flush(data, true);
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		background end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		render scene start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					{
						s_v3 ray_p = ray_at_y(ray, 0.0f);

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
							foreach_val(proj_i, proj, soft_data->projectile_arr) {
								s_v3 proj_pos = lerp_v3(proj.prev_pos, proj.pos, interp_dt);
								s_m4 model = m4_translate(proj_pos);
								scale_m4_by_radius(&model, proj.radius);
								draw_mesh(e_mesh_sphere, model, proj.color, e_shader_fresnel);
							}
						}
						// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw projectiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

						soft_data->hovered_boost = -1;
						foreach_val(boost_i, boost, soft_data->speed_boost_arr) {
							s_m4 model = m4_translate(boost.pos);
							scale_m4_by_radius(&model, c_boost_radius);
							s_v4 color = make_color(0.1f, 1, 0.1f);
							if(is_boost_hovered(ray_p, boost.pos)) {
								color = make_color(0.5f, 1, 0.5f);
								soft_data->hovered_boost = boost_i;
							}
							draw_mesh(e_mesh_sphere, model, color, e_shader_fresnel);
						}

						// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw checkpoints start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
						{
							float z = c_checkpoint_step;
							while(z < c_bottom) {
								s_instance_data data = zero;
								data.model = m4_translate(v3(0.0f, 0.0f, -z));
								data.model = m4_multiply(data.model, m4_rotate(game->render_time, v3(0, 0, 1)));
								data.model = m4_multiply(data.model, m4_scale(v3(2)));
								data.color = make_color(1);
								add_to_render_group(data, e_shader_mesh, e_texture_checkpoint, e_mesh_cube);
								z += c_checkpoint_step;
							}
						}
						// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw checkpoints end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

						// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
						{
							s_m4 model = m4_translate(player_pos);
							scale_m4_by_radius(&model, c_player_radius);
							draw_mesh(e_mesh_sphere, model, make_color(1), e_shader_fresnel);
						}
						// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

						{
							s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
							data.projection = perspective;
							data.view = view;
							data.light_projection = light_projection;
							data.light_view = light_view;
							render_flush(data, true);
						}

					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		render scene end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw teleporters start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					foreach_val(teleporter_i, teleporter, soft_data->teleporter_arr) {
						{
							s_instance_data data = zero;
							data.model = m4_translate(teleporter.pos);
							scale_m4_by_radius(&data.model, c_teleporter_radius);
							data.color = make_color(1);
							add_to_render_group(data, e_shader_teleporter, e_texture_white, e_mesh_quad);
						}
						{
							s_instance_data data = zero;
							data.model = m4_translate(teleporter.destination);
							scale_m4_by_radius(&data.model, c_teleporter_radius);
							data.color = make_color(1);
							add_to_render_group(data, e_shader_portal, e_texture_white, e_mesh_quad);
						}
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw teleporters end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw goal start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					if(fabsf(cam_pos.z - c_goal_pos.z) < 100) {
						{
							s_instance_data data = zero;
							data.model = m4_translate(v3(0.0f, 0.0f, c_goal_pos.z));
							scale_m4_by_radius(&data.model, 20);
							data.color = make_color(1);
							add_to_render_group(data, e_shader_portal, e_texture_white, e_mesh_quad);
						}
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw goal end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					{
						s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
						data.projection = perspective;
						data.view = view;
						data.light_projection = light_projection;
						data.light_view = light_view;
						data.blend_mode = e_blend_mode_normal;
						data.depth_mode = e_depth_mode_no_read_no_write;
						render_flush(data, true);
					}

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		particles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					{
						s_particle_spawn_data data = zero;
						data.shrink = 0.5f;
						data.duration = 0.5f;
						data.duration_rand = 0.5f;
						data.radius = 1.5f;
						data.radius_rand = 0.25f;
						data.color = hex_to_rgb(0xFAD201);
						data.color_rand = 1.0f;
						data.dir = v3(0.5f, 1, 1.0f);
						data.dir_rand = v3(1);
						data.speed = 0.01f;
						data.speed_rand = 0.5f;

						s_v3 pos = random_point_in_sphere(&game->rng, c_player_radius);
						spawn_particles(1, player_pos + pos, data);
					}

					{
						s_particle_spawn_data data = zero;
						data.shrink = 0.5f;
						data.duration = 0.5f;
						data.duration_rand = 0.5f;
						data.radius = 1.5f;
						data.radius_rand = 0.25f;
						data.color = hex_to_rgb(0x0);
						data.dir = v3(0.5f, 1, 1.0f);
						data.dir_rand = v3(1);
						data.speed = 0.01f;
						data.speed_rand = 0.5f;

						spawn_particles(4, ray_at_y(ray, 0.0f), data);
					}

					update_particles();
					{
						s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
						data.projection = perspective;
						data.view = view;
						data.blend_mode = e_blend_mode_additive;
						data.depth_mode = e_depth_mode_read_no_write;
						render_flush(data, true);
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		particles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		post start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					{
						draw_texture_screen(c_world_center, c_world_size, make_color(1), e_texture_white, e_shader_post, zero, zero);
						s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
						data.projection = ortho;
						data.blend_mode = e_blend_mode_normal;
						data.depth_mode = e_depth_mode_no_read_no_write;
						render_flush(data, true);
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		post end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					{
						s_v2 pos = v2(4);
						constexpr float font_size = 48;
						{
							s_time_format format = update_count_to_time_format(hard_data->update_count);
							s_len_str text = format_text("%02d:%02d.%i", format.minutes, format.seconds, format.milliseconds);
							draw_text(text, pos, font_size, make_color(1), false, &game->font);
							pos.y += font_size;
						}
						{
							s_len_str text = format_text("Depth: %i", floorfi(fabsf(player->pos.z)));
							draw_text(text, pos, font_size, make_color(1), false, &game->font);
							pos.y += font_size;
						}
						#if defined(m_debug)
						{
							s_len_str text = format_text("FPS: %i", roundfi(1.0f / delta));
							draw_text(text, pos, font_size, make_color(1), false, &game->font);
							pos.y += font_size;
						}
						#endif // m_debug
					}

					{
						s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
						data.projection = ortho;
						data.blend_mode = e_blend_mode_normal;
						data.depth_mode = e_depth_mode_no_read_no_write;
						render_flush(data, true);
					}

					if(soft_data->state == e_game_state1_pre_victory) {
						{
							s_time_data time_data = get_time_data(
								game->render_time, soft_data->pre_victory_timestamp, c_pre_victory_duration
							);
							if(time_data.percent >= 1) {
								if(game->leaderboard_nice_name.count <= 0 && c_on_web) {
									set_temp_state0_next_frame(e_game_state0_input_name);
								}
								else {
									set_state0_next_frame(e_game_state0_win_leaderboard);
									game->update_count_at_win_time = hard_data->update_count;
									#if defined(__EMSCRIPTEN__)
									submit_leaderboard_score(hard_data->update_count, c_leaderboard_id);
									#endif
								}
							}
							s_instance_data data = zero;
							data.model = fullscreen_m4();
							data.color = make_color(0, time_data.percent);
							add_to_render_group(data, e_shader_flat, e_texture_white, e_mesh_quad);
						}
						{
							s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
							data.projection = ortho;
							data.blend_mode = e_blend_mode_normal;
							data.depth_mode = e_depth_mode_no_read_no_write;
							render_flush(data, true);
						}
					}
				} break;

			}


		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		play end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		case e_game_state0_input_name: {
			s_input_name_state* state = &game->input_name_state;
			float font_size = 36;
			s_v2 pos = c_world_size * v2(0.5f, 0.4f);

			int count_before = state->name.str.count;
			b8 submitted = handle_string_input(&state->name, game->render_time);
			int count_after = state->name.str.count;
			if(count_before != count_after) {
				play_sound(e_sound_key);
			}
			if(submitted) {
				b8 can_submit = true;
				if(state->name.str.count < 2) {
					can_submit = false;
					cstr_into_builder(&state->error_str, "Name must have at least 2 characters!");
				}
				if(can_submit) {
					state->error_str.count = 0;
					#if defined(__EMSCRIPTEN__)
					set_leaderboard_name(builder_to_len_str(&state->name.str));
					#endif
					game->leaderboard_nice_name = state->name.str;
				}
			}

			draw_text(S("Victory!"), c_world_size * v2(0.5f, 0.1f), font_size, make_color(1), true, &game->font);
			draw_text(S("Enter your name"), c_world_size * v2(0.5f, 0.2f), font_size, make_color(1), true, &game->font);
			if(state->error_str.count > 0) {
				draw_text(builder_to_len_str(&state->error_str), c_world_size * v2(0.5f, 0.3f), font_size, hex_to_rgb(0xD77870), true, &game->font);
			}

			if(state->name.str.count > 0) {
				draw_text(builder_to_len_str(&state->name.str), pos, font_size, make_color(1), true, &game->font);
			}

			s_v2 full_text_size = get_text_size(builder_to_len_str(&state->name.str), &game->font, font_size);
			s_v2 partial_text_size = get_text_size_with_count(builder_to_len_str(&state->name.str), &game->font, font_size, state->name.cursor.value, 0);
			s_v2 cursor_pos = v2(
				-full_text_size.x * 0.5f + pos.x + partial_text_size.x,
				pos.y - font_size * 0.5f
			);

			s_v2 cursor_size = v2(15.0f, font_size);
			float t = game->render_time - max(state->name.last_action_time, state->name.last_edit_time);
			b8 blink = false;
			constexpr float c_blink_rate = 0.75f;
			if(t > 0.75f && fmodf(t, c_blink_rate) >= c_blink_rate / 2) {
				blink = true;
			}
			float t2 = clamp(game->render_time - state->name.last_edit_time, 0.0f, 1.0f);
			s_v4 color = lerp_color(hex_to_rgb(0xffdddd), brighter(hex_to_rgb(0xABC28F), 0.8f), 1 - powf(1 - t2, 3));
			float extra_height = ease_out_elastic2_advanced(t2, 0, 0.75f, 20, 0);
			cursor_size.y += extra_height;

			if(!state->name.visual_pos_initialized) {
				state->name.visual_pos_initialized = true;
				state->name.cursor_visual_pos = cursor_pos;
			}
			else {
				state->name.cursor_visual_pos = lerp_snap(state->name.cursor_visual_pos, cursor_pos, delta * 20);
			}

			if(!blink) {
				draw_rect_topleft(state->name.cursor_visual_pos - v2(0.0f, extra_height / 2), cursor_size, color);
			}

			s_render_flush_data data = make_render_flush_data(cam_pos, player_pos);
			data.projection = ortho;
			data.blend_mode = e_blend_mode_normal;
			data.depth_mode = e_depth_mode_no_read_no_write;
			render_flush(data, true);

		} break;

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

func void draw_rect_topleft(s_v2 pos, s_v2 size, s_v4 color)
{
	pos += size * 0.5f;
	draw_rect(pos, size, color);
}

func void draw_texture_screen(s_v2 pos, s_v2 size, s_v4 color, e_texture texture_id, e_shader shader_id, s_v2 uv_min, s_v2 uv_max)
{
	s_instance_data data = zero;
	data.model = m4_translate(v3(pos, 0));
	data.model = m4_multiply(data.model, m4_scale(v3(size, 1)));
	data.color = color;
	data.uv_min = uv_min;
	data.uv_max = uv_max;

	add_to_render_group(data, shader_id, texture_id, e_mesh_quad);
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
		uniform_data.mouse = g_mouse;
		uniform_data.player_pos = data.player_pos;
		gl(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(s_uniform_data), &uniform_data));
	}

	foreach_val(group_i, group, game->render_group_arr) {
		s_mesh* mesh = &game->mesh_arr[group.mesh_id];
		int* instance_count = &game->render_instance_count[group.shader_id][group.texture_id][group.mesh_id];
		assert(*instance_count > 0);
		s_instance_data* instance_data = game->render_instance_arr[group.shader_id][group.texture_id][group.mesh_id];

		gl(glUseProgram(game->shader_arr[group.shader_id].id));

		int in_texture_loc = glGetUniformLocation(game->shader_arr[group.shader_id].id, "in_texture");
		int noise_loc = glGetUniformLocation(game->shader_arr[group.shader_id].id, "noise");
		if(in_texture_loc >= 0) {
			glUniform1i(in_texture_loc, 0);
			glActiveTexture(GL_TEXTURE0);
			gl(glBindTexture(GL_TEXTURE_2D, game->texture_arr[group.texture_id].id));
		}
		if(noise_loc >= 0) {
			glUniform1i(noise_loc, 2);
			glActiveTexture(GL_TEXTURE2);
			gl(glBindTexture(GL_TEXTURE_2D, game->texture_arr[e_texture_noise].id));
		}

		gl(glBindVertexArray(mesh->vao));
		gl(glBindBuffer(GL_ARRAY_BUFFER, mesh->instance_vbo.id));
		gl(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(s_instance_data) * *instance_count, instance_data));
		gl(glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertex_count, *instance_count));
		if(reset_render_count) {
			game->render_group_arr.remove_and_swap(group_i);
			game->render_group_index_arr[group.shader_id][group.texture_id][group.mesh_id] = -1;
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

	char* shared_src = (char*)try_really_hard_to_read_file("src/shader_shared.h", arena);
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

func s_render_flush_data make_render_flush_data(s_v3 cam_pos, s_v3 player_pos)
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
	result.player_pos = player_pos;
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
		constexpr int stride = sizeof(float) * 24;

		gl(glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, stride, offset)); // instance color
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 4;

		gl(glVertexAttribPointer(attrib_index, 2, GL_FLOAT, GL_FALSE, stride, offset)); // uv min
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 2;

		gl(glVertexAttribPointer(attrib_index, 2, GL_FLOAT, GL_FALSE, stride, offset)); // uv max
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 2;

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

func s_mesh make_mesh_from_obj_file(char* file, s_linear_arena* arena)
{
	s_obj_mesh obj_mesh = parse_obj_mesh(file, arena);
	int vertex_count = obj_mesh.face_count * 3;
	assert(vertex_count < c_max_vertices);
	s_vertex vertex_arr[c_max_vertices] = zero;
	for(int i = 0; i < obj_mesh.face_count; i += 1) {
		for(int j = 0; j < 3; j += 1) {
			vertex_arr[i * 3 + j].pos = obj_mesh.pos_arr[obj_mesh.face_arr[i].vertex_index[j] - 1];
			vertex_arr[i * 3 + j].normal = obj_mesh.normal_arr[obj_mesh.face_arr[i].normal_index[j] - 1];
			vertex_arr[i * 3 + j].uv = obj_mesh.uv_arr[obj_mesh.face_arr[i].uv_index[j] - 1];
			vertex_arr[i * 3 + j].color = make_color(1);
		}
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

func b8 is_boost_hovered(s_v3 mouse_point, s_v3 boost_pos)
{
	float dist = v3_distance(mouse_point, boost_pos);
	b8 result = dist <= c_boost_radius * 4;
	return result;
}

func Mix_Chunk* load_sound_from_file(char* path, u8 volume)
{
	Mix_Chunk* chunk = Mix_LoadWAV(path);
	assert(chunk);
	chunk->volume = volume;
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
		game->hard_data.soft_data.player.dash_target = maybe<int>();
	}
	game->hard_data.soft_data.player.state = state;
}

func void try_to_dash()
{
	s_soft_game_data* soft_data = &game->hard_data.soft_data;
	s_player* player = &soft_data->player;
	switch(player->state) {
		case e_player_state_default: {
			if(soft_data->hovered_boost >= 0) {
				set_player_state(e_player_state_dashing);
				player->dash_target = maybe(soft_data->hovered_boost);
				soft_data->want_dash_timestamp = 0;
				play_sound(e_sound_dash);
			}
		} break;

		case e_player_state_dashing: {

		} break;

		case e_player_state_post_dash: {
			if(soft_data->hovered_boost >= 0 && soft_data->hovered_boost != player->dash_target.value) {
				set_player_state(e_player_state_dashing);
				player->dash_target = maybe(soft_data->hovered_boost);
				soft_data->want_dash_timestamp = 0;
				play_sound(e_sound_dash);
			}
			else {
				s_v3 dir = v3_normalized(player->wanted_pos - player->pos);
				player->vel += dir * 1.5f;
				set_player_state(e_player_state_default);
				soft_data->want_dash_timestamp = 0;
				play_sound(e_sound_dash);
			}
		} break;
	}
}

func s_texture load_texture_from_file(char* path)
{
	int width, height, num_channels;
	void* data = stbi_load(path, &width, &height, &num_channels, 4);
	assert(data);

	s_texture result = load_texture_from_data(data, width, height, GL_RGBA);
	return result;
}

func s_texture load_texture_from_data(void* data, int width, int height, int format)
{
	s_texture result = zero;

	gl(glGenTextures(1, &result.id));
	gl(glBindTexture(GL_TEXTURE_2D, result.id));
	gl(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	gl(glGenerateMipmap(GL_TEXTURE_2D));

	return result;
}

template <typename t>
func s_maybe<t> maybe()
{
	s_maybe<t> result = zero;
	return result;
}

template <typename t>
func s_maybe<t> maybe(t value)
{
	s_maybe<t> result = zero;
	result.valid = true;
	result.value = value;
	return result;
}

// @TODO(tkap, 13/10/2024): premultiply??
func s_font load_font_from_file(char* file, int font_size, s_linear_arena* arena)
{
	u8* file_data = read_file(file, arena);
	s_font font = {};
	font.size = (float)font_size;

	assert(file_data);

	stbtt_fontinfo info = {};
	stbtt_InitFont(&info, file_data, 0);

	stbtt_GetFontVMetrics(&info, &font.ascent, &font.descent, &font.line_gap);

	font.scale = stbtt_ScaleForPixelHeight(&info, (float)font_size);
	constexpr int max_chars = 128;
	int bitmap_count = 0;
	u8* bitmap_arr[max_chars];
	const int padding = 10;

	int columns = floorfi((float)(4096 / (font_size + padding)));
	int rows = ceilfi((max_chars - columns) / (float)columns) + 1;

	int total_width = floorfi((float)(columns * (font_size + padding)));
	// @Note(tkap, 20/10/2023): We need to align the texture width to 4 bytes! Very important! Thanks to tk_dev
	total_width = (total_width + 3) & ~3;
	int total_height = floorfi((float)(rows * (font_size + padding)));

	for(int char_i = 0; char_i < max_chars; char_i++)
	{
		s_glyph glyph = {};
		u8* bitmap = stbtt_GetCodepointBitmap(&info, 0, font.scale, char_i, &glyph.width, &glyph.height, 0, 0);
		stbtt_GetCodepointBox(&info, char_i, &glyph.x0, &glyph.y0, &glyph.x1, &glyph.y1);
		stbtt_GetGlyphHMetrics(&info, char_i, &glyph.advance_width, NULL);

		font.glyph_arr[char_i] = glyph;
		bitmap_arr[bitmap_count++] = bitmap;
	}

	u8* gl_bitmap = (u8*)arena_alloc_zero(arena, sizeof(u8) * 1 * total_width * total_height);

	for(int char_i = 0; char_i < max_chars; char_i++) {
		s_glyph* glyph = &font.glyph_arr[char_i];
		u8* bitmap = bitmap_arr[char_i];
		int column = char_i % columns;
		int row = char_i / columns;
		for(int y = 0; y < glyph->height; y++) {
			for(int x = 0; x < glyph->width; x++) {
				int current_x = floorfi((float)(column * (font_size + padding)));
				int current_y = floorfi((float)(row * (font_size + padding)));
				u8 src_pixel = bitmap[x + y * glyph->width];
				u8* dst_pixel = &gl_bitmap[((current_x + x) + (current_y + y) * total_width)];
				dst_pixel[0] = src_pixel;
			}
		}

		glyph->uv_min.x = column / (float)columns;
		glyph->uv_max.x = glyph->uv_min.x + (glyph->width / (float)total_width);

		// @Hack(tkap, 07/04/2025): Without this letters are cut off on the left side
		glyph->uv_min.x -= 0.001f;

		glyph->uv_min.y = row / (float)rows;

		// @Note(tkap, 17/05/2023): For some reason uv_max.y is off by 1 pixel (checked the texture in renderoc), which causes the text to be slightly miss-positioned
		// in the Y axis. "glyph->height - 1" fixes it.
		glyph->uv_max.y = glyph->uv_min.y + (glyph->height / (float)total_height);

		// @Note(tkap, 17/05/2023): Otherwise the line above makes the text be cut off at the bottom by 1 pixel...
		// glyph->uv_max.y += 0.01f;
	}

	for(int bitmap_i = 0; bitmap_i < bitmap_count; bitmap_i++) {
		stbtt_FreeBitmap(bitmap_arr[bitmap_i], NULL);
	}

	game->texture_arr[e_texture_font] = load_texture_from_data(gl_bitmap, total_width, total_height, m_gl_single_channel);

	return font;
}

func s_v2 draw_text(s_len_str text, s_v2 in_pos, float font_size, s_v4 color, b8 centered, s_font* font)
{
	float scale = font->scale * (font_size / font->size);

	assert(text.len > 0);
	if(centered) {
		s_v2 text_size = get_text_size(text, font, font_size);
		in_pos.x -= text_size.x / 2;
		in_pos.y -= text_size.y / 2;
	}
	s_v2 pos = in_pos;
	pos.y += font->ascent * scale;

	s_text_iterator it = {};
	while(iterate_text(&it, text, color)) {
		for(int char_i = 0; char_i < it.text.len; char_i++) {
			int c = it.text[char_i];
			if(c <= 0 || c >= 128) { continue; }

			if(c == '\n' || c == '\r') {
				pos.x = in_pos.x;
				pos.y += font_size;
				continue;
			}

			s_glyph glyph = font->glyph_arr[c];
			s_v2 draw_size = v2((glyph.x1 - glyph.x0) * scale, (glyph.y1 - glyph.y0) * scale);

			s_v2 glyph_pos = pos;
			glyph_pos.x += glyph.x0 * scale;
			glyph_pos.y += -glyph.y0 * scale;

			// t.flags |= e_render_flag_use_texture | e_render_flag_text;
			s_v3 tpos = v3(glyph_pos, 0.0f);

			s_v2 center = tpos.xy + draw_size / 2 * v2(1, -1);
			s_v2 bottomleft = tpos.xy;


			// pos.xy = v2_rotate_around(center, in_pos, t.rotation) + (bottomleft - center);

			s_m4 model = m4_translate(v3(tpos.xy, 0));
			model = m4_multiply(model, m4_scale(v3(draw_size, 1)));

			// t.color = it.color;
			s_v2 uv_min = glyph.uv_min;
			s_v2 uv_max = glyph.uv_max;
			// swap(&uv_min.y, &uv_max.y);
			// t.origin_offset = c_origin_bottomleft;

			draw_texture_screen(tpos.xy, draw_size, it.color, e_texture_font, e_shader_text, uv_min, uv_max);

			// draw_generic(game_renderer, &t, render_pass, render_data.shader, font->texture.game_id, e_mesh_rect);

			pos.x += glyph.advance_width * scale;

		}
	}

	return v2(pos.x, in_pos.y);
}

// @TODO(tkap, 31/10/2023): Handle new lines
func s_v2 get_text_size_with_count(s_len_str in_text, s_font* font, float font_size, int count, int in_column)
{
	assert(count >= 0);
	if(count <= 0) { return {}; }

	int column = in_column;

	s_v2 size = {};
	float max_width = 0;
	float scale = font->scale * (font_size / font->size);
	size.y = font_size;

	s_len_str text = substr_from_to_exclusive(in_text, 0, count);
	s_text_iterator it = {};
	while(iterate_text(&it, text, make_color(0))) {
		for(int char_i = 0; char_i < it.text.len; char_i++) {
			char c = it.text[char_i];
			s_glyph glyph = font->glyph_arr[c];
			if(c == '\t') {
				int spaces = get_spaces_for_column(column);
				size.x += glyph.advance_width * scale * spaces;
				column += spaces;
			}
			else if(c == '\n') {
				size.y += font_size;
				size.x = 0;
				column = 0;
			}
			else {
				size.x += glyph.advance_width * scale;
				column += 1;
			}
			max_width = max(size.x, max_width);
		}
	}
	size.x = max_width;

	return size;
}

func s_v2 get_text_size(s_len_str text, s_font* font, float font_size)
{
	return get_text_size_with_count(text, font, font_size, text.len, 0);
}

func b8 iterate_text(s_text_iterator* it, s_len_str text, s_v4 color)
{
	if(it->index >= text.len) { return false; }

	if(it->color_stack.count <= 0) {
		it->color_stack.add(color);
	}

	it->color = it->color_stack.get_last();

	int index = it->index;
	int advance = 0;
	while(index < text.len) {
		char c = text[index];
		char next_c = index < text.len - 1 ? text[index + 1] : 0;
		if(c == '$' && next_c == '$') {
			s_len_str red_str = substr_from_to_exclusive(text, index + 2, index + 4);
			s_len_str green_str = substr_from_to_exclusive(text, index + 4, index + 6);
			s_len_str blue_str = substr_from_to_exclusive(text, index + 6, index + 8);
			float red = hex_str_to_int(red_str) / 255.0f;
			float green = hex_str_to_int(green_str) / 255.0f;
			float blue = hex_str_to_int(blue_str) / 255.0f;
			s_v4 temp_color = make_color(red, green, blue);
			it->color_stack.add(temp_color);

			if(index == it->index) {
				index += 8;
				it->index += 8;
				it->color = it->color_stack.get_last();
				continue;
			}
			else {
				advance = 8;
				break;
			}
		}
		else if(c == '$' && next_c == '.') {
			if(index == it->index) {
				it->color_stack.pop_last();
				it->color = it->color_stack.get_last();
				index += 2;
				it->index += 2;
				continue;
			}
			else {
				advance = 2;
				it->color_stack.pop_last();
				break;
			}
		}
		index += 1;
	}
	it->text = substr_from_to_exclusive(text, it->index, index);
	it->index = index + advance;
	return true;
}

[[nodiscard]] func s_len_str substr_from_to_exclusive(s_len_str x, int start, int end)
{
	assert(start >= 0);
	assert(end > start);
	return {.str = x.str + start, .len = end - start};
}

func int get_spaces_for_column(int column)
{
	constexpr int tab_size = 4;
	if(tab_size <= 0) { return 0; }
	return tab_size - (column % tab_size);
}

func int hex_str_to_int(s_len_str str)
{
	int result = 0;
	int tens = 0;
	for(int i = str.len - 1; i >= 0; i -= 1) {
		char c = str[i];
		int val = 0;
		if(is_number(c)) {
			val = c - '0';
		}
		else if(c >= 'a' && c <= 'f') {
			val = c - 'a' + 10;
		}
		else if(c >= 'A' && c <= 'F') {
			val = c - 'A' + 10;
		}
		result += val * (int)powf(16, (float)tens);
		tens += 1;
	}
	return result;
}

func b8 is_number(char c)
{
	return c >= '0' && c <= '9';
}

func b8 is_alpha(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

func b8 is_alpha_numeric(char c)
{
	return is_number(c) || is_alpha(c);
}

func b8 can_start_identifier(char c)
{
	return is_alpha(c) || c == '_';
}

func b8 can_continue_identifier(char c)
{
	return is_alpha(c) || is_number(c) || c == '_';
}

func s_len_str format_text(const char* text, ...)
{
	static constexpr int max_format_text_buffers = 16;
	static constexpr int max_text_buffer_length = 512;

	static char buffers[max_format_text_buffers][max_text_buffer_length] = {};
	static int index = 0;

	char* current_buffer = buffers[index];
	memset(current_buffer, 0, max_text_buffer_length);

	va_list args;
	va_start(args, text);
	#ifdef m_debug
	int written = vsnprintf(current_buffer, max_text_buffer_length, text, args);
	assert(written > 0 && written < max_text_buffer_length);
	#else
	vsnprintf(current_buffer, max_text_buffer_length, text, args);
	#endif
	va_end(args);

	index += 1;
	if(index >= max_format_text_buffers) { index = 0; }

	return S(current_buffer);
}

func void spawn_particles(int count, s_v3 pos, s_particle_spawn_data data)
{
	s_soft_game_data* soft_data = &game->hard_data.soft_data;
	for(int i = 0; i < count; i += 1) {
		s_particle particle = zero;
		particle.pos = pos;
		particle.radius = data.radius * (1.0f - randf32(&game->rng) * data.radius_rand);
		particle.spawn_timestamp = game->render_time;
		particle.duration = data.duration * (1.0f - randf32(&game->rng) * data.duration_rand);
		particle.speed = data.speed * (1.0f - randf32(&game->rng) * data.speed_rand);
		particle.shrink = data.shrink;
		if(data.color_rand_per_channel) {
			float r = (1.0f - randf32(&game->rng) * data.color_rand);
			float g = (1.0f - randf32(&game->rng) * data.color_rand);
			float b = (1.0f - randf32(&game->rng) * data.color_rand);
			particle.color = data.color;
			particle.color.r *= r;
			particle.color.g *= g;
			particle.color.b *= b;
		}
		else {
			float r = (1.0f - randf32(&game->rng) * data.color_rand);
			particle.color = multiply_rgb(data.color, r);
		}
		particle.dir.x = data.dir.x * (1.0f - randf32(&game->rng) * data.dir_rand.x * 2);
		particle.dir.y = data.dir.y * (1.0f - randf32(&game->rng) * data.dir_rand.y * 2);
		particle.dir.z = data.dir.z * (1.0f - randf32(&game->rng) * data.dir_rand.z * 2);
		particle.dir = v3_normalized(particle.dir);
		soft_data->particle_arr.add(particle);
	}
}

func void update_particles()
{
	s_soft_game_data* soft_data = &game->hard_data.soft_data;
	foreach_ptr(particle_i, particle, soft_data->particle_arr) {
		float passed = game->render_time - particle->spawn_timestamp;
		float percent_done = passed / particle->duration;
		float speed = particle->speed;
		particle->pos += particle->dir * speed;
		s_v4 color = particle->color;
		color.a = 1.0f - percent_done;
		float radius = particle->radius * (1.0f - percent_done * particle->shrink);
		{
			s_instance_data data = zero;
			data.model = m4_translate(particle->pos);
			scale_m4_by_radius(&data.model, radius);
			data.color = color;
			add_to_render_group(data, e_shader_circle, e_texture_white, e_mesh_quad);
		}
		if(passed >= particle->duration) {
			soft_data->particle_arr.remove_and_swap(particle_i);
			particle_i -= 1;
		}
	}
}

func s_v4 rand_color(s_rng* rng)
{
	s_v4 result;
	result.x = randf32(rng);
	result.y = randf32(rng);
	result.z = randf32(rng);
	result.a = 1;
	return result;
}

func s_v4 rand_color_normalized(s_rng* rng)
{
	s_v4 result;
	result.x = randf32(rng);
	result.y = randf32(rng);
	result.z = randf32(rng);
	result.xyz = v3_normalized(result.xyz);
	result.a = 1;
	return result;
}

func s_v3 random_point_in_sphere(s_rng* rng, float radius)
{
	s_v3 pos;
	while(true) {
		pos = v3(
			randf_range(rng, -radius, radius),
			randf_range(rng, -radius, radius),
			randf_range(rng, -radius, radius)
		);
		float d = v3_length(pos);
		if(d <= radius) { break; }
	}
	return pos;
}

func u8* try_really_hard_to_read_file(char* file, s_linear_arena* arena)
{
	u8* result = null;
	for(int i = 0; i < 100; i += 1) {
		result = read_file(file, arena);
		if(result) {
			break;
		}
		SDL_Delay(10);
	}
	return result;
}

func float update_time_to_render_time(float time, float interp_dt)
{
	float result = time + (float)c_update_delay * interp_dt;
	return result;
}

func s_m4 fullscreen_m4()
{
	s_m4 result = m4_translate(v3(c_world_center, 0.0f));
	result = m4_multiply(result, m4_scale(v3(c_world_size, 0.0f)));
	return result;
}

func s_time_data get_time_data(float curr, float timestamp, float duration)
{
	s_time_data result = zero;
	result.passed = curr - timestamp;
	result.percent = result.passed / duration;
	result.inv_percent = 1.0f - result.percent;
	return result;
}

func s_time_format update_count_to_time_format(int update_count)
{
	s_time_format result = zero;
	float milliseconds = update_count * (float)c_update_delay * 1000;

	result.hours = floorfi(milliseconds / 1000 / 60 / 60);
	milliseconds -= result.hours * 1000 * 60 * 60;

	result.minutes = floorfi(milliseconds / 1000 / 60);
	milliseconds -= result.minutes * 1000 * 60;

	result.seconds = floorfi(milliseconds / 1000);
	milliseconds -= result.seconds * 1000;

	result.milliseconds = floorfi(milliseconds);

	return result;
}

func s_obj_mesh parse_obj_mesh(char* path, s_linear_arena* arena)
{
	s_obj_mesh result = zero;
	char* data = (char*)read_file(path, arena);
	assert(data);
	char* cursor = strstr(data, "\nv ") + 1;
	while(memcmp(cursor, "v ", 2) == 0) {
		cursor += 2;
		char* end = null;
		result.pos_arr[result.vertex_count].x = strtof(cursor, &end);
		assert(end > cursor);
		cursor = end;
		result.pos_arr[result.vertex_count].y = strtof(cursor, &end);
		assert(end > cursor);
		cursor = end;
		result.pos_arr[result.vertex_count].z = strtof(cursor, &end);
		assert(end > cursor);
		cursor = end;
		result.vertex_count += 1;

		while(*cursor == '\n' || *cursor == '\r' || *cursor == ' ') {
			cursor += 1;
		}
	}

	while(memcmp(cursor, "vn ", 3) == 0) {
		cursor += 3;
		char* end = null;
		result.normal_arr[result.normal_count].x = strtof(cursor, &end);
		assert(end > cursor);
		cursor = end;
		result.normal_arr[result.normal_count].y = strtof(cursor, &end);
		assert(end > cursor);
		cursor = end;
		result.normal_arr[result.normal_count].z = strtof(cursor, &end);
		assert(end > cursor);
		cursor = end;
		result.normal_count += 1;

		while(*cursor == '\n' || *cursor == '\r' || *cursor == ' ') {
			cursor += 1;
		}
	}

	while(memcmp(cursor, "vt ", 3) == 0) {
		cursor += 3;
		char* end = null;
		result.uv_arr[result.uv_count].x = strtof(cursor, &end);
		assert(end > cursor);
		cursor = end;
		result.uv_arr[result.uv_count].y = strtof(cursor, &end);
		assert(end > cursor);
		cursor = end;
		result.uv_count += 1;

		while(*cursor == '\n' || *cursor == '\r' || *cursor == ' ') {
			cursor += 1;
		}
	}

	while(memcmp(cursor, "f ", 2) != 0) {
		cursor += 1;
	}

	while(memcmp(cursor, "f ", 2) == 0) {
		cursor += 2;
		char* end = null;

		for(int i = 0; i < 3; i += 1) {
			result.face_arr[result.face_count].vertex_index[i] = (int)strtol(cursor, &end, 10);
			assert(end > cursor);
			cursor = end + 1;
			result.face_arr[result.face_count].uv_index[i] = (int)strtol(cursor, &end, 10);
			assert(end > cursor);
			cursor = end + 1;
			result.face_arr[result.face_count].normal_index[i] = (int)strtol(cursor, &end, 10);
			assert(end > cursor);
			cursor = end;

			while(*cursor == '\n' || *cursor == '\r' || *cursor == ' ') {
				cursor += 1;
			}
		}
		result.face_count += 1;

	}

	return result;
}

func char* skip_whitespace(char* str)
{
	while(true) {
		if(*str == '0') { break; }
		else if(*str <= ' ') { str += 1; }
		else { break; }
	}
	return str;
}

func void set_state0_next_frame(e_game_state0 state)
{
	s_state next_state = zero;
	next_state.value = state;
	game->next_state = maybe(next_state);
}

func void set_temp_state0_next_frame(e_game_state0 state)
{
	s_state next_state = zero;
	next_state.value = state;
	next_state.temporary = true;
	game->next_state = maybe(next_state);
}

func void pop_game_state()
{
	game->pop_state = true;
}

func s_v2 wxy(float x, float y)
{
	s_v2 result = c_world_size * v2(x, y);
	return result;
}

func s_v2 wcxy(float x, float y)
{
	s_v2 result = c_world_center * v2(x, y);
	return result;
}

func b8 do_button(s_len_str text, s_v2 pos, b8 centered)
{
	s_v2 size = v2(256, 48);
	b8 result = false;
	if(!centered) {
		pos += size * 0.5f;
	}

	b8 hovered = mouse_vs_rect_center(pos, size);
	s_v4 color = make_color(0.25f);
	if(hovered) {
		size += v2(8);
		if(!centered) {
			pos += v2(8) * 0.5f;
		}
		color = make_color(0.5f);
		if(g_click) {
			result = true;
			play_sound(e_sound_click);
		}
	}

	{
		s_instance_data data = zero;
		data.model = m4_translate(v3(pos, 0));
		data.model = m4_multiply(data.model, m4_scale(v3(size, 1)));
		data.color = color,
		add_to_render_group(data, e_shader_button, e_texture_white, e_mesh_quad);
	}

	draw_text(text, pos, 32.0f, make_color(1), true, &game->font);

	return result;
}


func b8 rect_vs_rect_topleft(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1)
{
	b8 result = pos0.x + size0.x > pos1.x && pos0.x < pos1.x + size1.x &&
		pos0.y + size0.y > pos1.y && pos0.y < pos1.y + size1.y;
	return result;
}

func b8 rect_vs_rect_center(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1)
{
	b8 result = rect_vs_rect_topleft(pos0 - size0 * 0.5f, size0, pos1 - size1 * 0.5f, size1);
	return result;
}

func b8 mouse_vs_rect_topleft(s_v2 pos, s_v2 size)
{
	b8 result = rect_vs_rect_topleft(g_mouse, v2(1), pos, size);
	return result;
}

func b8 mouse_vs_rect_center(s_v2 pos, s_v2 size)
{
	b8 result = rect_vs_rect_center(g_mouse, v2(1), pos, size);
	return result;
}

func b8 is_key_pressed(int key, b8 consume)
{
	b8 result = false;
	if(key < c_max_keys) {
		result = game->input_arr[key].half_transition_count > 1 || (game->input_arr[key].half_transition_count > 0 && game->input_arr[key].is_down);
	}
	if(result && consume) {
		game->input_arr[key].half_transition_count = 0;
		game->input_arr[key].is_down = false;
	}
	return result;
}

template <int n>
func void cstr_into_builder(s_str_builder<n>* builder, char* str)
{
	assert(str);

	int len = (int)strlen(str);
	assert(len <= n);
	memcpy(builder->data, str, len);
	builder->count = len;
}

template <int n>
func s_len_str builder_to_len_str(s_str_builder<n>* builder)
{
	s_len_str result = zero;
	result.str = builder->data;
	result.len = builder->count;
	return result;
}

template <int n>
func char* builder_to_cstr(s_str_builder<n>* builder, s_circular_arena* arena)
{
	char* result = (char*)circular_arena_alloc(arena, builder->count + 1);
	memcpy(result, builder->data, builder->count);
	result[builder->count] = '\0';
	return result;
}

template <int n0, int n1>
func b8 builder_equals(s_str_builder<n0>* a, s_str_builder<n1>* b)
{
	b8 result = a->count == b->count && memcmp(a->data, b->data, a->count) == 0;
	return result;
}

template <int n>
func b8 handle_string_input(s_input_str<n>* str, float time)
{
	b8 result = false;
	if(!str->cursor.valid) {
		str->cursor = maybe(0);
	}
	foreach_val(c_i, c, game->char_events) {
		if(is_alpha_numeric(c) || c == '_') {
			if(!is_builder_full(&str->str)) {
				builder_insert(&str->str, str->cursor.value, c);
				str->cursor.value += 1;
				str->last_edit_time = time;
				str->last_action_time = str->last_edit_time;
			}
		}
	}

	foreach_val(event_i, event, game->key_events) {
		if(!event.went_down) { continue; }
		if(event.key == SDLK_RETURN) {
			result = true;
			str->last_action_time = time;
		}
		else if(event.key == SDLK_ESCAPE) {
			str->cursor.value = 0;
			str->str.count = 0;
			str->str.data[0] = 0;
			str->last_edit_time = time;
			str->last_action_time = str->last_edit_time;
		}
		else if(event.key == SDLK_BACKSPACE) {
			if(str->cursor.value > 0) {
				str->cursor.value -= 1;
				builder_remove_char_at(&str->str, str->cursor.value);
				str->last_edit_time = time;
				str->last_action_time = str->last_edit_time;
			}
		}
	}
	return result;
}

func void handle_key_event(int key, b8 is_down, b8 is_repeat)
{
	if(is_down) {
		game->any_key_pressed = true;
	}
	if(key < c_max_keys) {
		if(!is_repeat) {
			game->any_key_pressed = true;
		}

		{
			s_key_event key_event = {};
			key_event.went_down = is_down;
			key_event.key = key;
			// key_event.modifiers |= e_input_modifier_ctrl * is_key_down(&g_platform_data.input, c_key_left_ctrl);
			game->key_events.add(key_event);
		}
	}
}

template <int n>
func b8 is_builder_full(s_str_builder<n>* builder)
{
	b8 result = builder->count >= n;
	return result;
}

template <int n>
func void builder_insert(s_str_builder<n>* builder, int index, char c)
{
	assert(index >= 0);
	assert(index <= builder->count);
	int num_to_the_right = builder->count - index;
	if(num_to_the_right > 0) {
		memmove(&builder->data[index + 1], &builder->data[index], num_to_the_right);
	}
	builder->data[index] = c;
	builder->count += 1;
}

template <int n>
func void builder_remove_char_at(s_str_builder<n>* builder, int index)
{
	assert(index >= 0);
	assert(index < builder->count);

	builder->count -= 1;
	memmove(&builder->data[index], &builder->data[index + 1], builder->count - index);
	builder->data[builder->count] = 0;
}

func void do_leaderboard()
{
	b8 escape = is_key_pressed(SDLK_ESCAPE, true);
	if(do_button(S("Back"), wxy(0.87f, 0.92f), true) || escape) {
		set_state0_next_frame(e_game_state0_main_menu);
		game->clear_state = true;
	}

	{
		if(!game->leaderboard_received) {
			draw_text(S("Getting leaderboard..."), c_world_center, 48, make_color(0.66f), true, &game->font);
		}
		else if(game->leaderboard_arr.count <= 0) {
			draw_text(S("No scores yet :("), c_world_center, 48, make_color(0.66f), true, &game->font);
		}

		constexpr int c_max_visible_entries = 10;
		s_v2 pos = c_world_center * v2(1.0f, 0.7f);
		for(int entry_i = 0; entry_i < at_most(c_max_visible_entries + 1, game->leaderboard_arr.count); entry_i += 1) {
			s_leaderboard_entry entry = game->leaderboard_arr[entry_i];
			s_time_format data = update_count_to_time_format(entry.time);
			s_v4 color = make_color(0.8f);
			int rank_number = entry_i + 1;
			if(entry_i == c_max_visible_entries || builder_equals(&game->leaderboard_public_uid, &entry.internal_name)) {
				color = hex_to_rgb(0xD3A861);
				rank_number = entry.rank;
			}
			char* name = entry.internal_name.data;
			if(entry.nice_name.count > 0) {
				name = entry.nice_name.data;
			}
			draw_text(format_text("%i %s", rank_number, name), v2(c_world_size.x * 0.1f, pos.y - 24), 32, color, false, &game->font);
			s_len_str text = format_text("%02i:%02i.%i", data.minutes, data.seconds, data.milliseconds);
			draw_text(text, v2(c_world_size.x * 0.5f, pos.y - 24), 32, color, false, &game->font);
			pos.y += 48;
		}
	}
}

func void init_obstacles()
{
	s_soft_game_data* soft_data = &game->hard_data.soft_data;
	s_rng rng = make_rng(0);

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn speed boosts start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		int z = c_boost_z_start;
		while(z < c_boost_z_end) {
			if(chance100(&rng, c_boost_chance)) {
				int count = rand_range_ii(&rng, 1, 5);
				for(int i = 0; i < count; i += 1) {
					s_speed_boost boost = zero;
					boost.pos = v3(
						-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, -z
					);
					z += c_boost_z_step;
					soft_data->speed_boost_arr.add(boost);
				}
			}
			z += c_boost_z_step;
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		spawn speed boosts end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	{
		int z = -10;
		while(z > -190) {
			if(chance100(&rng, 75)) {
				s_projectile proj = zero;
				proj.pos = v3(
					-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, z
				);
				proj.type = e_projectile_type_bounce;
				proj.prev_pos = proj.pos;
				proj.radius = randf_range(&rng, 0.5f, 1.5f);
				proj.going_right = rand_bool(&rng);
				proj.color = make_color(0.1f, 0.1f, 1.0f);
				soft_data->projectile_arr.add(proj);
			}
			z -= 4;
		}
	}

	{
		int z = -210;
		while(z > -390) {
			if(chance100(&rng, 40)) {
				s_projectile proj = zero;
				proj.pos = v3(
					-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, z
				);
				proj.type = e_projectile_type_default;
				proj.prev_pos = proj.pos;
				proj.radius = randf_range(&rng, 0.5f, 1.5f);
				proj.going_right = rand_bool(&rng);
				proj.color = make_color(1.0f, 0.1f, 0.1f);
				soft_data->projectile_arr.add(proj);
			}
			z -= 4;
		}
	}

	{
		int z = -410;
		while(z > -590) {
			constexpr float radius = 1.0f;
			int gap = rand_range_ii(&rng, 4, 8);
			float center_x = randf_range(&rng, -c_wall_x + radius, c_wall_x - radius);

			s_list<float, 16> x_arr;

			{
				float x = center_x - gap;
				while(x >= -c_wall_x + radius) {
					x_arr.add(x);
					x -= radius * 2;
				}
			}
			{
				float x = center_x + gap;
				while(x <= c_wall_x - radius) {
					x_arr.add(x);
					x += radius * 2;
				}
			}

			foreach_val(x_pos_i, x_pos, x_arr) {
				s_projectile proj = zero;
				proj.pos = v3(
					x_pos, 0, z
				);
				proj.type = e_projectile_type_static;
				proj.prev_pos = proj.pos;
				proj.radius = radius;
				proj.color = hex_to_rgb(0xA18594);
				soft_data->projectile_arr.add(proj);
			}
			z -= 10;
		}
	}

	{
		int z = -610;
		while(z > -790) {
			if(chance100(&rng, 40)) {
				s_projectile proj = zero;
				proj.pos = v3(
					-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, z
				);
				proj.type = e_projectile_type_follow;
				proj.prev_pos = proj.pos;
				proj.radius = 2.0f;
				proj.color = hex_to_rgb(0xFAD201);
				soft_data->projectile_arr.add(proj);
			}
			z -= 4;
		}
	}

	{
		int z = -820;
		while(z > -980) {
			constexpr float radius = 1.0f;

			{
				float x = -c_wall_x + radius * 2;
				while(x <= c_wall_x - radius * 2) {
					s_projectile proj = zero;
					proj.pos = v3(
						x, 0, z
					);
					proj.type = e_projectile_type_static;
					proj.prev_pos = proj.pos;
					proj.radius = radius;
					proj.color = hex_to_rgb(0x781F19);
					soft_data->projectile_arr.add(proj);
					x += radius * 2;
				}
			}

			{
				s_teleporter teleporter = zero;
				float x_pos = randf_range(&rng, -c_wall_x + radius, c_wall_x - radius);
				float x_destination = randf_range(&rng, -c_wall_x + radius, c_wall_x - radius);
				teleporter.pos = v3(x_pos, 0.0f, z + 5);
				teleporter.destination = v3(x_destination, 0.0f, z - 5);
				soft_data->teleporter_arr.add(teleporter);
			}
			z -= 20;
		}
	}

	{
		int z = -1010;
		while(z > -1190) {
			if(chance100(&rng, 45)) {
				float radius = randf_range(&rng, 1.0f, 1.2f);
				float x = randf_range(&rng, -c_wall_x + radius, c_wall_x - radius);
				s_projectile proj = zero;
				proj.pos = v3(x, 0, z);
				proj.type = e_projectile_type_diagonal;
				proj.prev_pos = proj.pos;
				proj.radius = radius;
				proj.color = hex_to_rgb(0x3E5F8A);
				float angle = randf_range(&rng, -c_pi * 0.25f, c_pi * 0.25f);
				s_v2 dir = v2_from_angle(angle);
				if(rand_bool(&rng)) {
					dir *= -1;
				}
				proj.dir = v3(dir.x, 0.0f, dir.y);
				soft_data->projectile_arr.add(proj);
			}
			z -= 5;
		}
	}

	{
		int z = -1210;
		while(z > -1390) {
			if(chance100(&rng, 50)) {

				s_projectile proj = zero;
				proj.pos = v3(
					-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, z
				);
				proj.type = e_projectile_type_bounce;
				proj.prev_pos = proj.pos;
				proj.radius = randf_range(&rng, 0.5f, 1.5f);
				proj.going_right = rand_bool(&rng);
				proj.color = make_color(0.1f, 0.1f, 1.0f);
				soft_data->projectile_arr.add(proj);

			}
			else if(chance100(&rng, 50)) {

				s_projectile proj = zero;
				proj.pos = v3(
					-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, z
				);
				proj.type = e_projectile_type_default;
				proj.prev_pos = proj.pos;
				proj.radius = randf_range(&rng, 0.5f, 1.5f);
				proj.going_right = rand_bool(&rng);
				proj.color = make_color(1.0f, 0.1f, 0.1f);
				soft_data->projectile_arr.add(proj);
			}
			z -= 5;
		}
	}

	{
		int z = -1410;
		while(z > -1590) {

			{
				constexpr float radius = 1.0f;
				int gap = rand_range_ii(&rng, 4, 6);
				float center_x = randf_range(&rng, -c_wall_x + radius, c_wall_x - radius);

				s_list<float, 16> x_arr;

				{
					float x = center_x - gap;
					while(x >= -c_wall_x + radius) {
						x_arr.add(x);
						x -= radius * 2;
					}
				}
				{
					float x = center_x + gap;
					while(x <= c_wall_x - radius) {
						x_arr.add(x);
						x += radius * 2;
					}
				}

				foreach_val(x_pos_i, x_pos, x_arr) {
					s_projectile proj = zero;
					proj.pos = v3(
						x_pos, 0, z
					);
					proj.type = e_projectile_type_static;
					proj.prev_pos = proj.pos;
					proj.radius = radius;
					proj.color = hex_to_rgb(0xA18594);
					soft_data->projectile_arr.add(proj);
				}
			}

			int temp_z = z - 3;
			for(int i = 0; i < 5; i += 1) {

				if(chance100(&rng, 50)) {
					s_projectile proj = zero;
					proj.pos = v3(
						-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, temp_z
					);
					proj.type = e_projectile_type_bounce;
					proj.prev_pos = proj.pos;
					proj.radius = randf_range(&rng, 0.5f, 1.5f);
					proj.going_right = rand_bool(&rng);
					proj.color = make_color(0.1f, 0.1f, 1.0f);
					soft_data->projectile_arr.add(proj);
				}
				temp_z -= 3;
			}

			z -= 20;
		}
	}

	{
		int z = -1610;
		while(z > -1790) {
			for(int i = 0; i < 5; i += 1) {
				if(chance100(&rng, 55)) {
					s_projectile proj = zero;
					proj.pos = v3(
						-c_wall_x + 2 + randf32(&rng) * (c_wall_x * 2 - 4), 0, z
					);
					proj.type = e_projectile_type_static;
					proj.prev_pos = proj.pos;
					proj.radius = randf_range(&rng, 0.5f, 0.75f);
					proj.going_right = rand_bool(&rng);
					proj.color = hex_to_rgb(0xB32428);
					soft_data->projectile_arr.add(proj);
				}
			}
			z -= 4;
		}
	}
}

func b8 sphere_out_of_bounds_left(s_v3 pos, float radius)
{
	b8 result = pos.x < -c_wall_x + radius * 2;
	return result;
}

func b8 sphere_out_of_bounds_right(s_v3 pos, float radius)
{
	b8 result = pos.x > c_wall_x - radius * 2;
	return result;
}
