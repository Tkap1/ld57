

global constexpr s_v2 c_world_size = {1366, 768};
global constexpr s_v2 c_world_center = {c_world_size.x * 0.5f, c_world_size.y * 0.5f};
global constexpr int c_max_vertices = 16384;
global constexpr int c_max_faces = 8192;
global constexpr s_v3 c_up_axis = {0, 0, 1};
global constexpr int c_shadow_map_res = 1024;
global constexpr int c_updates_per_second = 100;
global constexpr f64 c_update_delay = 1.0 / c_updates_per_second;
global constexpr float c_wall_x = 10.0f;
global constexpr float c_boost_radius = 1;
global constexpr float c_player_radius = 1;
global constexpr float c_teleporter_radius = 8;
global constexpr float c_bottom = 2000;
global constexpr int c_max_particle_emitters = 1024;
global constexpr float c_transition_time = 0.25f;

global constexpr float c_game_speed_arr[] = {
	0.0f, 0.01f, 0.1f, 0.25f, 0.5f,
	1,
	2, 4, 8, 16
};

global constexpr s_v3 c_goal_pos = {0.0f, 0.0f, -c_bottom};

global constexpr int c_boost_z_start = 10;
global constexpr int c_boost_z_end = 1900;
global constexpr int c_boost_z_step = 9;
global constexpr int c_boost_chance = 65;

global constexpr int c_checkpoint_step = 200;

global constexpr int c_max_keys = 1024;
global constexpr int c_max_leaderboard_entries = 16;

global constexpr float c_pre_victory_duration = 2.0f;

global constexpr int c_leaderboard_id = 30592;

global constexpr s_len_str c_game_name = S("Ball Pit");
