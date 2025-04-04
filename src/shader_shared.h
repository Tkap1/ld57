
#if !defined(m_cpu_side)
#define s_v2 vec2
#define s_v3 vec3
#define s_v4 vec4
#define s_m4 mat4
#endif

#if defined(m_cpu_side)
#pragma pack(push, 1)
struct s_uniform_data
#else // m_cpu_side
layout(std140) uniform ublock
#endif
{
	s_m4 view;
	s_m4 projection;
	s_v2 base_res;
	s_v2 window_size;
	float render_time;
	float pad_0;
	float pad_1;
	float pad_2;
};
#if defined(m_cpu_side)
#pragma pack(pop)
#endif // m_cpu_side