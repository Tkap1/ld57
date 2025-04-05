func s_m4 make_orthographic(float left, float right, float bottom, float top, float near, float far);
func s_m4 make_perspective(float FOV, float AspectRatio, float Near, float Far);
func s_m4 look_at(s_v3 eye, s_v3 target, s_v3 up);
func s_m4 m4_rotate(float angle, s_v3 axis);
func s_v3 v3_cross(s_v3 a, s_v3 b);
func float v3_dot(s_v3 a, s_v3 b);
func s_v3 v3_normalized(s_v3 v);
func float v3_length_squared(s_v3 v);
func float v3_length(s_v3 v);
func s_quaternion dir_to_quaternion(s_v3 dir);
func s_v3 v3_rotate(s_v3 v, s_quaternion q);
func s_m4 quaternion_to_m4(s_quaternion left);
func s_v4 v4_multiply_m4(s_v4 v, s_m4 m);
func s_quaternion quaternion_from_axis_angle(s_v3 axis, float angle);
func s_quaternion quaternion_divide_f(s_quaternion Left, float Dividend);
func s_quaternion quaternion_divide(s_quaternion Left, float Right);
func float quaternion_dot(s_quaternion Left, s_quaternion Right);
func s_quaternion quaternion_normalized(s_quaternion Left);
template <typename t>
func void swap(t* a, t* b);
func s_ray get_camera_ray(s_v3 cam_pos, s_m4 view, s_m4 projection, s_v2 mouse, s_v2 world_size);
func s_m4 m4_inverse(const s_m4 m);
func s_v3 ray_at_y(s_ray ray, float y);
func float lerp(float a, float b, float t);
func s_v3 lerp_v3(s_v3 a, s_v3 b, float t);
func float range_lerp(float input_val, float input_start, float input_end, float output_start, float output_end);
func s_v3 v3_set_mag(s_v3 v, float len);
template <typename t>
func void at_most_ptr(t* ptr, t max_val);
template <typename t>
func void at_least_ptr(t* ptr, t min_val);
func int ceilfi(float x);
func int roundfi(float x);
func float v3_distance(s_v3 a, s_v3 b);
func float go_towards(float from, float to, float amount);
func s_v3 go_towards(s_v3 from, s_v3 to, float amount);
func float sign(float x);
func b8 sphere_vs_sphere(s_v3 pos1, float r1, s_v3 pos2, float r2);
func void scale_m4_by_radius(s_m4* out, float radius);
