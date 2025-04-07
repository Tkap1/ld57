

static s_v4 make_color(float r)
{
	s_v4 result = zero;
	result.x = r;
	result.y = r;
	result.z = r;
	result.w = 1;
	return result;
}

static s_v4 make_color(float r, float g, float b)
{
	s_v4 result = zero;
	result.x = r;
	result.y = g;
	result.z = b;
	result.w = 1;
	return result;
}

static s_v4 make_color(float r, float a)
{
	s_v4 result = zero;
	result.x = r;
	result.y = r;
	result.z = r;
	result.w = a;
	return result;
}

func s_v4 multiply_rgb(s_v4 color, float t)
{
	color.r *= t;
	color.g *= t;
	color.b *= t;
	return color;
}

func s_v4 hex_to_rgb(u32 hex)
{
	s_v4 result;
	result.x = ((hex & 0xFF0000) >> 16) / 255.0f;
	result.y = ((hex & 0x00FF00) >> 8) / 255.0f;
	result.z = ((hex & 0x0000FF) >> 0) / 255.0f;
	result.w = 1;
	return result;
}