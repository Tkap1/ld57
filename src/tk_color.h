

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

