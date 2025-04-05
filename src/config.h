
global constexpr s_v2 c_world_size = {1366, 768};
global constexpr s_v2 c_world_center = {c_world_size.x * 0.5f, c_world_size.y * 0.5f};
global constexpr int c_max_vertices = 4096;
global constexpr int c_max_faces = 1024;
global constexpr s_v3 c_up_axis = v3(0, 0, 1);
global constexpr int c_shadow_map_res = 1024;
global constexpr int c_updates_per_second = 100;
global constexpr f64 c_update_delay = 1.0 / c_updates_per_second;
global constexpr float c_wall_x = 10.0f;
global constexpr float c_boost_radius = 1;
global constexpr float c_player_radius = 1;
global constexpr float c_game_speed_arr[] = {
	0.0f, 0.01f, 0.1f, 0.25f, 0.5f,
	1,
	2, 4, 8, 16
};

global constexpr int c_bounce_proj_z_start = 20;
global constexpr int c_bounce_proj_z_end = 300;
global constexpr int c_bounce_proj_z_step = 9;
global constexpr int c_bounce_proj_chance = 65;

global constexpr int c_default_proj_z_start = 200;
global constexpr int c_default_proj_z_end = 600;
global constexpr int c_default_proj_z_step = 9;
global constexpr int c_default_proj_chance = 65;