func s_m4 make_orthographic(float left, float right, float bottom, float top, float near, float far);
func s_m4 make_perspective(float FOV, float AspectRatio, float Near, float Far);
func s_m4 look_at(s_v3 eye, s_v3 target, s_v3 up);
func s_m4 m4_rotate(float angle, s_v3 axis);
func s_v3 v3_cross(s_v3 a, s_v3 b);
func float v3_dot(s_v3 a, s_v3 b);
func s_v3 v3_normalized(s_v3 v);
func float v3_length_squared(s_v3 v);
func float v3_length(s_v3 v);
