
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


#if defined(m_debug)
#define gl(...) __VA_ARGS__; {int error = glGetError(); if(error != 0) { on_gl_error(#__VA_ARGS__, __FILE__, __LINE__, error); }}
#else // m_debug
#define gl(...) __VA_ARGS__
#endif // m_debug

#include "tklib.h"
#include "shared.h"

#include "config.h"
#include "leaderboard.h"
#include "engine.h"
#include "game.h"
#include "shader_shared.h"


#if defined(__EMSCRIPTEN__)
global constexpr b8 c_on_web = true;
#else
global constexpr b8 c_on_web = false;
#endif



global s_platform_data* g_platform_data;
global s_game* game;
global s_v2 g_mouse;
global b8 g_click;

#if defined(__EMSCRIPTEN__)
#include "leaderboard.cpp"
#endif

#include "engine.cpp"

m_dll_export void init(s_platform_data* platform_data)
{
	g_platform_data = platform_data;
	game = (s_game*)platform_data->memory;
	game->speed_index = 5;
	game->rng = make_rng(1234);
	game->reload_shaders = true;
	game->speed = 1;

	SDL_StartTextInput();

	add_state(&game->state0, e_game_state0_main_menu);

	u8* cursor = platform_data->memory + sizeof(s_game);

	{
		game->update_frame_arena = make_arena_from_memory(cursor, 10 * c_mb);
		cursor += 10 * c_mb;
	}
	{
		game->render_frame_arena = make_arena_from_memory(cursor, 500 * c_mb);
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

	{
		s_mesh* mesh = &game->mesh_arr[e_mesh_line];
		mesh->vertex_count = 6;
		gl(glGenVertexArrays(1, &mesh->vao));
		gl(glBindVertexArray(mesh->vao));

		gl(glGenBuffers(1, &mesh->instance_vbo.id));
		gl(glBindBuffer(GL_ARRAY_BUFFER, mesh->instance_vbo.id));

		u8* offset = 0;
		constexpr int stride = sizeof(float) * 9;
		int attrib_index = 0;

		gl(glVertexAttribPointer(attrib_index, 2, GL_FLOAT, GL_FALSE, stride, offset)); // line start
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 2;

		gl(glVertexAttribPointer(attrib_index, 2, GL_FLOAT, GL_FALSE, stride, offset)); // line end
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 2;

		gl(glVertexAttribPointer(attrib_index, 1, GL_FLOAT, GL_FALSE, stride, offset)); // line width
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 1;

		gl(glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, stride, offset)); // instance color
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 4;
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

	{
		u32* texture = &game->texture_arr[e_texture_light].id;
		game->light_fbo.size.x = (int)c_world_size.x;
		game->light_fbo.size.y = (int)c_world_size.y;
		gl(glGenFramebuffers(1, &game->light_fbo.id));
		bind_framebuffer(game->light_fbo.id);
		gl(glGenTextures(1, texture));
		gl(glBindTexture(GL_TEXTURE_2D, *texture));
		gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, game->light_fbo.size.x, game->light_fbo.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		gl(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0));
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		bind_framebuffer(0);
	}

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
	{
		b8 state_changed = handle_state(&game->state0, game->render_time);
		if(state_changed) {
			game->accumulator += c_update_delay;
		}
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

	e_game_state0 state0 = (e_game_state0)get_state(&game->state0);
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

				entity_manager_reset(&soft_data->emitter_a_arr);

				{
					s_particle_emitter_a data = zero;
					data.shrink = 0.5f;
					data.particle_duration = 0.5f;
					data.particle_duration_rand = 0.5f;
					data.radius = 1.5f;
					data.radius_rand = 0.25f;
					{
						s_particle_color c = zero;
						c.color = hex_to_rgb(0xFAD201);
						c.color_rand = 1.0f;
						data.color_arr.add(c);
					}
					{
						s_particle_color c = zero;
						c.color = make_color(1,0,0);
						c.color_rand = 1.0f;
						c.percent = 0.5f;
						data.color_arr.add(c);
					}
					data.dir = v3(0.5f, 1, 1.0f);
					data.dir_rand = v3(1);
					data.speed = 0.01f;
					data.speed_rand = 0.5f;

					s_particle_emitter_b b = zero;
					b.duration = -1;
					b.particles_per_second = 200;
					b.spawn_type = e_emitter_spawn_type_sphere;
					b.spawn_data.x = c_player_radius;
					add_emitter(data, b);
				}


				// @Note(tkap, 07/04/2025): If restarting without a checkpoint, reset the timer so that the player doesn't have to
				if(!hard_data->curr_checkpoint.valid) {
					hard_data->update_count = 0;
				}

				if(hard_data->curr_checkpoint.valid) {
					int index = hard_data->curr_checkpoint.value;
					player->pos.z = (index + 1) * (float)-c_checkpoint_step;
					player->prev_pos = player->pos;
				}
				game->cam_pos = get_wanted_cam_pos(player->pos);

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
					player->pos = go_towards_v3(player->pos, boost.pos, 1.0f);
					if(v3_distance(player->pos, boost.pos) < 0.01f) {
						soft_data->speed_boost_arr.remove_and_swap(soft_data->player.dash_target.value);
						set_player_state(e_player_state_post_dash);
						player->dash_target = maybe<int>();
						player->post_dash_timestamp = game->update_time;
						player->vel = zero;
						play_sound(e_sound_pop);

						{
							s_particle_emitter_a a = zero;
							a.shrink = 0.5f;
							a.particle_duration = 0.5f;
							a.particle_duration_rand = 0.5f;
							a.radius = 1.5f;
							a.radius_rand = 0.25f;
							{
								s_particle_color c = zero;
								c.color = make_color(0.5f, 1.0f, 0.5f);
								c.color_rand = 1.0f;
								a.color_arr.add(c);
							}
							a.dir = v3(1);
							a.dir_rand = v3(1);
							a.speed = 0.1f;
							a.speed_rand = 0.5f;
							a.pos = boost.pos;

							s_particle_emitter_b b = zero;
							b.particle_count = 64;
							add_emitter(a, b);
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
							hard_data->checkpoint_hit_timestamp_arr[index] = game->update_time;

							{
								s_particle_emitter_a data = zero;
								data.shrink = 0.5f;
								data.particle_duration = 3.5f;
								data.particle_duration_rand = 0.5f;
								data.radius = 1.5f;
								data.radius_rand = 0.25f;

								{
									s_particle_color c = zero;
									c.color = hex_to_rgb(0x3B83BD);
									c.color_rand = 1.0f;
									data.color_arr.add(c);
								}


								data.dir = v3(1);
								data.dir_rand = v3(1);
								data.speed = 0.1f;
								data.speed_rand = 0.5f;
								data.pos = pos;

								s_particle_emitter_b b = zero;
								b.particle_count = 128;
								add_emitter(data, b);
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
							if(!game->turn_off_death_sound) {
								play_sound(e_sound_defeat);
							}
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

		default: {}
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

	s_v3 wanted_cam_pos = get_wanted_cam_pos(player_pos);
	game->cam_pos = lerp_v3(game->cam_pos, wanted_cam_pos, delta * 10);
	s_v3 cam_pos = game->cam_pos;

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

	bind_framebuffer(0);
	clear_framebuffer_depth(0);
	clear_framebuffer_color(0, v4(0.0f, 0, 0, 0));

	e_game_state0 state0 = (e_game_state0)get_state(&game->state0);


	switch(state0) {

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		main menu start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_main_menu: {

			if(do_button(S("Play"), wxy(0.5f, 0.5f), true) || is_key_pressed(SDLK_RETURN, true)) {
				add_state_transition(&game->state0, e_game_state0_play, game->render_time, c_transition_time);
				game->do_hard_reset = true;
			}

			if(do_button(S("Leaderboard"), wxy(0.5f, 0.6f), true)) {
				#if defined(__EMSCRIPTEN__)
				get_leaderboard(c_leaderboard_id);
				#endif
				add_state_transition(&game->state0, e_game_state0_leaderboard, game->render_time, c_transition_time);
			}

			if(do_button(S("Options"), wxy(0.5f, 0.7f), true)) {
				add_state_transition(&game->state0, e_game_state0_options, game->render_time, c_transition_time);
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
				pop_state_transition(&game->state0, game->render_time, c_transition_time);
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
			s_v2 pos = wxy(0.5f, 0.4f);

			{
				s_len_str text = format_text("Death sound: %s", game->turn_off_death_sound ? "Off" : "On");
				do_bool_button(text, pos, true, &game->turn_off_death_sound);
				pos.y += 80;
			}

			{
				s_len_str text = format_text("Sound: %s", game->turn_off_all_sounds ? "Off" : "On");
				do_bool_button(text, pos, true, &game->turn_off_all_sounds);
				pos.y += 80;
			}

			{
				s_len_str text = format_text("Show timer: %s", game->hide_timer ? "Off" : "On");
				do_bool_button(text, pos, true, &game->hide_timer);
				pos.y += 80;
			}

			{
				s_len_str text = format_text("Show depth: %s", game->hide_depth ? "Off" : "On");
				do_bool_button(text, pos, true, &game->hide_depth);
				pos.y += 80;
			}

			b8 escape = is_key_pressed(SDLK_ESCAPE, true);
			if(do_button(S("Back"), wxy(0.87f, 0.92f), true) || escape) {
				pop_state_transition(&game->state0, game->render_time, c_transition_time);
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
							int index = 0;
							float z = c_checkpoint_step;
							while(z < c_bottom) {
								s_instance_data data = zero;
								data.model = m4_translate(v3(0.0f, 0.0f, -z));
								data.model = m4_multiply(data.model, m4_rotate(game->render_time, v3(0, 0, 1)));
								s_time_data time_data = zero;
								if(hard_data->checkpoint_hit_timestamp_arr[index] > 0) {
									time_data = get_time_data(
										update_time_to_render_time(game->update_time, interp_dt), hard_data->checkpoint_hit_timestamp_arr[index], 1.0f
									);
								}
								else {
									time_data.percent = 1;
								}
								float scale = ease_out_back_advanced(time_data.percent, 0, 2.0f, 3.0f, 2.0f);
								data.model = m4_multiply(data.model, m4_scale(v3(scale)));
								data.color = make_color(ease_out_back_advanced(time_data.percent, 0, 2.0f, 2.0f, 0.75f));
								add_to_render_group(data, e_shader_mesh, e_texture_checkpoint, e_mesh_cube);
								z += c_checkpoint_step;
								index += 1;
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
							data.color = make_color(1.0f);
							add_to_render_group(data, e_shader_teleporter, e_texture_white, e_mesh_quad);
						}
						{
							s_instance_data data = zero;
							data.model = m4_translate(teleporter.destination);
							scale_m4_by_radius(&data.model, c_teleporter_radius);
							data.color = make_color(1, 0.1f, 0.1f);
							add_to_render_group(data, e_shader_teleporter, e_texture_white, e_mesh_quad);
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

					soft_data->emitter_a_arr.data[0].pos = player_pos;

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

						if(!game->hide_timer) {
							s_time_format format = update_count_to_time_format(hard_data->update_count);
							s_len_str text = format_text("%02d:%02d.%i", format.minutes, format.seconds, format.milliseconds);
							draw_text(text, pos, font_size, make_color(1), false, &game->font);
							pos.y += font_size;
						}

						if(!game->hide_depth) {
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

						pos.y += font_size * 0.5f;
						{
							s_len_str text = format_text("Hold $$C0832Eleft click$. to go faster");
							draw_text(text, pos, font_size * 0.5f, make_color(1), false, &game->font);
							pos.y += font_size;
						}

						{
							s_len_str text = format_text("Press $$C0832Eright click$. or $$C0832EF$.\nto dash into or out of green balls");
							draw_text(text, pos, font_size * 0.5f, make_color(1), false, &game->font);
							pos.y += font_size * 1.5f;
						}

						{
							s_len_str text = format_text("Press $$C0832ER$. to go back to\nprevious checkpoint");
							draw_text(text, pos, font_size * 0.5f, make_color(1), false, &game->font);
							pos.y += font_size * 1.5f;
						}

						{
							s_len_str text = format_text("Press $$C0832ECTRL + R$. to fully restart");
							draw_text(text, pos, font_size * 0.5f, make_color(1), false, &game->font);
							pos.y += font_size;
						}
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
									add_temporary_state_transition(&game->state0, e_game_state0_input_name, game->render_time, c_transition_time);
								}
								else {
									add_state_transition(&game->state0, e_game_state0_win_leaderboard, game->render_time, c_transition_time);
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
			s_v4 color = lerp_color(hex_to_rgb(0xffdddd), multiply_rgb_clamped(hex_to_rgb(0xABC28F), 0.8f), 1 - powf(1 - t2, 3));
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

	s_state_transition transition = get_state_transition(&game->state0, game->render_time);
	if(transition.active) {
		{
			float alpha = 0;
			if(transition.time_data.percent <= 0.5f) {
				alpha = transition.time_data.percent * 2;
			}
			else {
				alpha = transition.time_data.inv_percent * 2;
			}
			s_instance_data data = zero;
			data.model = fullscreen_m4();
			data.color = make_color(0.0f, 0, 0, alpha);
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
		void* instance_data = game->render_instance_arr[group.shader_id][group.texture_id][group.mesh_id];

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
		gl(glBufferSubData(GL_ARRAY_BUFFER, 0, group.element_size * *instance_count, instance_data));
		gl(glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertex_count, *instance_count));
		if(reset_render_count) {
			game->render_group_arr.remove_and_swap(group_i);
			game->render_group_index_arr[group.shader_id][group.texture_id][group.mesh_id] = -1;
			group_i -= 1;
			*instance_count = 0;
		}
	}
}

template <typename t>
func void add_to_render_group(t data, e_shader shader_id, e_texture texture_id, e_mesh mesh_id)
{
	s_render_group render_group = zero;
	render_group.shader_id = shader_id;
	render_group.texture_id = texture_id;
	render_group.mesh_id = mesh_id;
	render_group.element_size = sizeof(t);

	s_mesh* mesh = &game->mesh_arr[render_group.mesh_id];

	int* render_group_index = &game->render_group_index_arr[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	if(*render_group_index < 0) {
		game->render_group_arr.add(render_group);
		*render_group_index = game->render_group_arr.count - 1;
	}
	int* count = &game->render_instance_count[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	int* max_elements = &game->render_instance_max_elements[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	t* ptr = (t*)game->render_instance_arr[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
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
			gl(glBufferData(GL_ARRAY_BUFFER, sizeof(t) * new_max_elements, null, GL_DYNAMIC_DRAW));
			mesh->instance_vbo.max_elements = new_max_elements;
		}
	}
	if(get_new_ptr) {
		t* temp = (t*)arena_alloc(&game->render_frame_arena, sizeof(t) * new_max_elements);
		if(*count > 0) {
			memcpy(temp, ptr, *count * sizeof(t));
		}
		game->render_instance_arr[render_group.shader_id][render_group.texture_id][render_group.mesh_id] = (void*)temp;
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

func b8 is_boost_hovered(s_v3 mouse_point, s_v3 boost_pos)
{
	float dist = v3_distance(mouse_point, boost_pos);
	b8 result = dist <= c_boost_radius * 4;
	return result;
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

func b8 do_button(s_len_str text, s_v2 pos, b8 centered)
{
	s_v2 size = v2(256, 48);
	b8 result = false;
	if(!centered) {
		pos += size * 0.5f;
	}

	b8 hovered = mouse_vs_rect_center(g_mouse, pos, size);
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

func b8 do_bool_button(s_len_str text, s_v2 pos, b8 centered, b8* out)
{
	assert(out);
	b8 result = false;
	if(do_button(text, pos, centered)) {
		result = true;
		*out = !(*out);
	}
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
	memcpy(builder->str, str, len);
	builder->count = len;
}

template <int n>
func s_len_str builder_to_len_str(s_str_builder<n>* builder)
{
	s_len_str result = zero;
	result.str = builder->str;
	result.count = builder->count;
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
				builder_insert_char(&str->str, str->cursor.value, c);
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
			str->str.str[0] = 0;
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

func void do_leaderboard()
{
	b8 escape = is_key_pressed(SDLK_ESCAPE, true);
	if(do_button(S("Back"), wxy(0.87f, 0.92f), true) || escape) {
		add_state_transition(&game->state0, e_game_state0_main_menu, game->render_time, c_transition_time);
		clear_state(&game->state0);
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
			char* name = entry.internal_name.str;
			if(entry.nice_name.count > 0) {
				name = entry.nice_name.str;
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

func s_v3 get_wanted_cam_pos(s_v3 player_pos)
{
	s_v3 result = v3(
		0, -25, player_pos.z - 5
	);
	return result;
}

func s_v2 get_rect_normal_of_closest_edge(s_v2 p, s_v2 center, s_v2 size)
{
	s_v2 edge_arr[] = {
		v2(center.x - size.x * 0.5f, center.y),
		v2(center.x + size.x * 0.5f, center.y),
		v2(center.x, center.y - size.y * 0.5f),
		v2(center.x, center.y + size.y * 0.5f),
	};
	s_v2 normal_arr[] = {
		v2(-1, 0),
		v2(1, 0),
		v2(0, -1),
		v2(0, 1),
	};
	float closest_dist[2] = {9999999, 9999999};
	int closest_index[2] = {0, 0};

	for(int i = 0; i < 4; i += 1) {
		float dist;
		if(i <= 1) {
			dist = fabsf(p.x - edge_arr[i].x);
		}
		else {
			dist = fabsf(p.y - edge_arr[i].y);
		}
		if(dist < closest_dist[0]) {
			closest_dist[0] = dist;
			closest_index[0] = i;
		}
		else if(dist < closest_dist[1]) {
			closest_dist[1] = dist;
			closest_index[1] = i;
		}
	}
	s_v2 result = normal_arr[closest_index[0]];

	// @Note(tkap, 19/04/2025): Probably breaks for very thin rectangles
	if(fabsf(closest_dist[0] - closest_dist[1]) <= 0.01f) {
		result += normal_arr[closest_index[1]];
		result = v2_normalized(result);
	}
	return result;
}