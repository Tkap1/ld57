
#if defined(m_vertex)
layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec4 vertex_color;
layout (location = 3) in vec2 vertex_uv;
layout (location = 4) in vec4 instance_color;
layout (location = 5) in mat4 instance_model;
#endif

shared_var vec4 v_color;
shared_var vec3 v_normal;

#if defined(m_vertex)
void main()
{
	vec3 vertex = vertex_pos;
	gl_Position = projection * view * instance_model * vec4(vertex, 1);
	v_color = vertex_color * instance_color;
	v_normal = vertex_normal;
}
#endif

#if !defined(m_vertex)

out vec4 out_color;

void main()
{
	vec3 normal = normalize(v_normal);
	// vec3 color = normal * 0.5 + 0.5;
	vec3 color = vec3(0.0);

	vec3 light_dir = normalize(vec3(1, 1, -1));

	float d = max(0.0, dot(-light_dir, normal)) * 0.5 + 0.5;
	color = v_color.rgb * d;
	// color = vec3(d);
	out_color = vec4(color, 1.0);
}
#endif