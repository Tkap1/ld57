
global constexpr float c_pi = 3.1415926f;

struct s_v2
{
	float x;
	float y;
};

struct s_v3
{
	union {
		struct {
			float x;
			float y;
		};
		s_v2 xy;
	};
	float z;
};


union s_v4
{
	struct
	{
		union
		{
			struct
			{
				union
				{
					struct
					{
						float x;
						float y;
					};
					s_v2 xy;
				};
				float z;
			};
			s_v3 xyz;
		};
		float w;
	};

	struct
	{
		union
		{
			struct
			{
				union
				{
					struct
					{
						float r;
						float g;
					};
					s_v2 rg;
				};
				float b;
			};
			s_v3 rgb;
		};
		float a;
	};
	float elements[4];
};

struct s_rect
{
	union
	{
		struct
		{
			float x;
			float y;
		};
		s_v2 pos;
	};

	union
	{
		struct
		{
			float w;
			float h;
		};
		s_v2 size;
	};
};

struct s_quaternion
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		s_v3 xyz;
	};
	float w;
};


union s_m4
{
	float all[16];
	float all2[4][4];
};

struct s_ray
{
	s_v3 pos;
	s_v3 dir;
};

struct s_ray_collision
{
	b8 hit;
	float distance;
	s_v3 normal;
	s_v3 point;
};

#include "generated/generated_tk_math.h"

// -------------------------------------------------------------------------------------------


template <typename t>
func constexpr t at_least(t a, t b)
{
	return a > b ? a : b;
}

template <typename t>
func constexpr t at_most(t a, t b)
{
	return b > a ? a : b;
}

template <typename t>
func constexpr t clamp(t current, t min_val, t max_val)
{
	return at_most(max_val, at_least(min_val, current));
}


template <typename t0, typename t1>
func constexpr s_v2 v2(t0 x, t1 y)
{
	s_v2 result = zero;
	result.x = (float)x;
	result.y = (float)y;
	return result;
}

template <typename t0>
func constexpr s_v2 v2(t0 x)
{
	s_v2 result = zero;
	result.x = (float)x;
	result.y = (float)x;
	return result;
}

func constexpr s_v2 v2(s_v2i v)
{
	s_v2 result = zero;
	result.x = (float)v.x;
	result.y = (float)v.y;
	return result;
}

template <typename t0, typename t1, typename t2>
func constexpr s_v3 v3(t0 x, t1 y, t2 z)
{
	s_v3 result = zero;
	result.x = (float)x;
	result.y = (float)y;
	result.z = (float)z;
	return result;
}

template <typename t0>
func constexpr s_v3 v3(s_v2 xy, t0 z)
{
	s_v3 result = zero;
	result.x = xy.x;
	result.y = xy.y;
	result.z = (float)z;
	return result;
}

template <typename t0>
func constexpr s_v3 v3(t0 x)
{
	s_v3 result = zero;
	result.x = (float)x;
	result.y = (float)x;
	result.z = (float)x;
	return result;
}

template <typename t0, typename t1, typename t2, typename t3>
func constexpr s_v4 v4(t0 x, t1 y, t2 z, t3 w)
{
	s_v4 result = zero;
	result.x = (float)x;
	result.y = (float)y;
	result.z = (float)z;
	result.w = (float)w;
	return result;
}

template <typename t0>
func constexpr s_v4 v4(s_v3 v, t0 w)
{
	s_v4 result = zero;
	result.x = v.x;
	result.y = v.y;
	result.z = v.z;
	result.w = (float)w;
	return result;
}

func constexpr s_v2 operator+(s_v2 a, s_v2 b)
{
	return v2(
		a.x + b.x,
		a.y + b.y
	);
}

func constexpr s_v2 operator-(s_v2 a, s_v2 b)
{
	return v2(
		a.x - b.x,
		a.y - b.y
	);
}

func constexpr s_v3 operator+(s_v3 a, s_v3 b)
{
	return v3(
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	);
}

func constexpr s_v2 operator*(s_v2 a, float b)
{
	return v2(
		a.x * b,
		a.y * b
	);
}

func constexpr s_v2 operator*(s_v2 a, s_v2 b)
{
	return v2(
		a.x * b.x,
		a.y * b.y
	);
}

func constexpr s_v3 operator*(s_v3 a, float b)
{
	return v3(
		a.x * b,
		a.y * b,
		a.z * b
	);
}

func constexpr s_v2 operator/(s_v2 a, float b)
{
	return v2(
		a.x / b,
		a.y / b
	);
}

func constexpr s_v3 operator/(s_v3 a, float b)
{
	return v3(
		a.x / b,
		a.y / b,
		a.z / b
	);
}

func void operator+=(s_v2& a, s_v2 b)
{
	a.x += b.x;
	a.y += b.y;
}

func void operator-=(s_v2& a, s_v2 b)
{
	a.x -= b.x;
	a.y -= b.y;
}

func void operator*=(s_v2& a, float b)
{
	a.x *= b;
	a.y *= b;
}

func void operator+=(s_v3& a, s_v3 b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}

func void operator*=(s_v3& a, float b)
{
	a.x *= b;
	a.y *= b;
	a.z *= b;
}

func void operator-=(s_v3& a, s_v3 b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
}

func constexpr s_v3 operator-(s_v3 a, s_v3 b)
{
	return v3(
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	);
}

func constexpr s_quaternion make_quaternion()
{
	return {.x = 0, .y = 0, .z = 0, .w = 1};
}

// -------------------------------------------------------------------------------------------

func s_m4 m4_scale(s_v3 v)
{
	s_m4 result = {
		v.x, 0, 0, 0,
		0, v.y, 0, 0,
		0, 0, v.z, 0,
		0, 0, 0, 1,
	};
	return result;
}

func s_m4 m4_translate(s_v3 v)
{
	s_m4 result = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		v.x, v.y, v.z, 1,
	};
	return result;
}

func s_m4 m4_identity()
{
	s_m4 result = zero;
	result.all2[0][0] = 1;
	result.all2[1][1] = 1;
	result.all2[2][2] = 1;
	result.all2[3][3] = 1;
	return result;
}

func s_m4 m4_multiply(s_m4 a, s_m4 b)
{
	s_m4 result = zero;
	for(int i = 0; i < 4; i += 1) {
		for(int j = 0; j < 4; j += 1) {
			for(int k = 0; k < 4; k += 1) {
				result.all2[j][i] += a.all2[k][i] * b.all2[j][k];
			}
		}
	}
	return result;
}

func s_m4 make_orthographic(float left, float right, float bottom, float top, float near, float far)
{
	s_m4 result = zero;

	// xy values are mapped to -1 to 1 range for viewport mapping
	// z values are mapped to 0 to 1 range instead of -1 to 1 for depth writing
	result.all2[0][0] = 2.0f / (right - left);
	result.all2[1][1] = 2.0f / (top - bottom);
	result.all2[2][2] = 1.0f / (near - far);
	result.all2[3][3] = 1.0f;

	result.all2[3][0] = (left + right) / (left - right);
	result.all2[3][1] = (bottom + top) / (bottom - top);
	result.all2[3][2] = 0.5f * (near + far) / (near - far);

	return result;
}

func s_m4 make_perspective(float FOV, float AspectRatio, float Near, float Far)
{
	s_m4 Result = zero;

	// See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml

	float Cotangent = 1.0f / tanf(FOV * (c_pi / 360.0f));

	Result.all2[0][0] = Cotangent / AspectRatio;

	Result.all2[1][1] = Cotangent;

	Result.all2[2][3] = -1.0f;
	Result.all2[2][2] = (Near + Far) / (Near - Far);
	Result.all2[3][2] = (2.0f * Near * Far) / (Near - Far);
	Result.all2[3][3] = 0.0f;

	return (Result);
}

func s_m4 look_at(s_v3 eye, s_v3 target, s_v3 up)
{
	s_m4 world_to_cam = zero;
	s_v3 front = v3_normalized(target - eye);
	s_v3 side = v3_normalized(v3_cross(front, up));
	s_v3 top = v3_normalized(v3_cross(side, front));

	world_to_cam.all[0] = side.x;
	world_to_cam.all[1] = top.x;
	world_to_cam.all[2] = -front.x;
	world_to_cam.all[3] = 0;

	world_to_cam.all[4] = side.y;
	world_to_cam.all[5] = top.y;
	world_to_cam.all[6] = -front.y;
	world_to_cam.all[7] = 0;

	world_to_cam.all[8] = side.z;
	world_to_cam.all[9] = top.z;
	world_to_cam.all[10] = -front.z;
	world_to_cam.all[11] = 0;

	s_v3 x = v3(world_to_cam.all[0], world_to_cam.all[4], world_to_cam.all[8]);
	s_v3 y = v3(world_to_cam.all[1], world_to_cam.all[5], world_to_cam.all[9]);
	s_v3 z = v3(world_to_cam.all[2], world_to_cam.all[6], world_to_cam.all[10]);

	world_to_cam.all[12] = -v3_dot(x, eye);
	world_to_cam.all[13] = -v3_dot(y, eye);
	world_to_cam.all[14] = -v3_dot(z, eye);
	world_to_cam.all[15] = 1.0f;

	return world_to_cam;
}

func s_m4 m4_rotate(float angle, s_v3 axis)
{

	s_m4 result = m4_identity();

	axis = v3_normalized(axis);

	float SinTheta = sinf(angle);
	float CosTheta = cosf(angle);
	float CosValue = 1.0f - CosTheta;

	result.all2[0][0] = (axis.x * axis.x * CosValue) + CosTheta;
	result.all2[0][1] = (axis.x * axis.y * CosValue) + (axis.z * SinTheta);
	result.all2[0][2] = (axis.x * axis.z * CosValue) - (axis.y * SinTheta);

	result.all2[1][0] = (axis.y * axis.x * CosValue) - (axis.z * SinTheta);
	result.all2[1][1] = (axis.y * axis.y * CosValue) + CosTheta;
	result.all2[1][2] = (axis.y * axis.z * CosValue) + (axis.x * SinTheta);

	result.all2[2][0] = (axis.z * axis.x * CosValue) + (axis.y * SinTheta);
	result.all2[2][1] = (axis.z * axis.y * CosValue) - (axis.x * SinTheta);
	result.all2[2][2] = (axis.z * axis.z * CosValue) + CosTheta;

	return result;
}


func s_v3 v3_cross(s_v3 a, s_v3 b)
{
	s_v3 Result;

	Result.x = (a.y * b.z) - (a.z * b.y);
	Result.y = (a.z * b.x) - (a.x * b.z);
	Result.z = (a.x * b.y) - (a.y * b.x);

	return (Result);
}

func float v3_dot(s_v3 a, s_v3 b)
{
	float Result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	return (Result);
}

func s_v3 v3_normalized(s_v3 v)
{
	s_v3 result = v;
	float length = v3_length(v);
	if(length != 0) {
		result.x /= length;
		result.y /= length;
		result.z /= length;
	}
	return result;
}

func float v3_length_squared(s_v3 v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

func float v3_length(s_v3 v)
{
	return sqrtf(v3_length_squared(v));
}

func s_quaternion dir_to_quaternion(s_v3 dir)
{
	dir.y *= -1;
	swap(&dir.x, &dir.y);

	s_v3 v1 = v3(1, 0, 0);
	s_v3 y_vec = v3(0, 1, 0);
	float dot = v3_dot(v1, dir);
	if(dot < -0.999999f) {
		s_v3 temp = v3_cross(v1, dir);
		if(v3_length(temp) < 0.000001f) {
			temp = v3_cross(y_vec, dir);
		}
		temp = v3_normalized(temp);
		return quaternion_from_axis_angle(temp, c_pi);
	}
	else if (dot > 0.999999f) {
		return make_quaternion();
	}
	else {
		s_quaternion result = zero;
		result.xyz = v3_cross(v1, dir);
		result.w = 1 + dot;
		return result;
	}
}

func s_v3 v3_rotate(s_v3 v, s_quaternion q)
{
	s_m4 m4 = quaternion_to_m4(q);
	return v4_multiply_m4(v4(v, 0), m4).xyz;
}

func s_m4 quaternion_to_m4(s_quaternion left)
{
	s_m4 result;

	s_quaternion NormalizedQuaternion = quaternion_normalized(left);

	float XX, YY, ZZ,
			XY, XZ, YZ,
			WX, WY, WZ;

	XX = NormalizedQuaternion.x * NormalizedQuaternion.x;
	YY = NormalizedQuaternion.y * NormalizedQuaternion.y;
	ZZ = NormalizedQuaternion.z * NormalizedQuaternion.z;
	XY = NormalizedQuaternion.x * NormalizedQuaternion.y;
	XZ = NormalizedQuaternion.x * NormalizedQuaternion.z;
	YZ = NormalizedQuaternion.y * NormalizedQuaternion.z;
	WX = NormalizedQuaternion.w * NormalizedQuaternion.x;
	WY = NormalizedQuaternion.w * NormalizedQuaternion.y;
	WZ = NormalizedQuaternion.w * NormalizedQuaternion.z;

	result.all2[0][0] = 1.0f - 2.0f * (YY + ZZ);
	result.all2[0][1] = 2.0f * (XY + WZ);
	result.all2[0][2] = 2.0f * (XZ - WY);
	result.all2[0][3] = 0.0f;

	result.all2[1][0] = 2.0f * (XY - WZ);
	result.all2[1][1] = 1.0f - 2.0f * (XX + ZZ);
	result.all2[1][2] = 2.0f * (YZ + WX);
	result.all2[1][3] = 0.0f;

	result.all2[2][0] = 2.0f * (XZ + WY);
	result.all2[2][1] = 2.0f * (YZ - WX);
	result.all2[2][2] = 1.0f - 2.0f * (XX + YY);
	result.all2[2][3] = 0.0f;

	result.all2[3][0] = 0.0f;
	result.all2[3][1] = 0.0f;
	result.all2[3][2] = 0.0f;
	result.all2[3][3] = 1.0f;

	return (result);
}

func s_v4 v4_multiply_m4(s_v4 v, s_m4 m)
{
	s_v4 result;

	result.x = m.all[0] * v.x + m.all[4] * v.y + m.all[8]  * v.z + m.all[12] * v.w;
	result.y = m.all[1] * v.x + m.all[5] * v.y + m.all[9]  * v.z + m.all[13] * v.w;
	result.z = m.all[2] * v.x + m.all[6] * v.y + m.all[10] * v.z + m.all[14] * v.w;
	result.w = m.all[3] * v.x + m.all[7] * v.y + m.all[11] * v.z + m.all[15] * v.w;

	return result;
}


func s_quaternion quaternion_from_axis_angle(s_v3 axis, float angle)
{
	s_quaternion result;

	s_v3 axis_normalized = v3_normalized(axis);
	float sin_rot = sinf(angle * 0.5f);

	result.xyz = axis_normalized * sin_rot;
	result.w = cosf(angle * 0.5f);

	return result;
}

func s_quaternion quaternion_divide_f(s_quaternion Left, float Dividend)
{
	s_quaternion Result;

	Result.x = Left.x / Dividend;
	Result.y = Left.y / Dividend;
	Result.z = Left.z / Dividend;
	Result.w = Left.w / Dividend;

	return (Result);
}

func s_quaternion quaternion_divide(s_quaternion Left, float Right)
{
	s_quaternion Result = quaternion_divide_f(Left, Right);
	return (Result);
}

func float quaternion_dot(s_quaternion Left, s_quaternion Right)
{
	float Result;

	Result = (Left.x * Right.x) + (Left.y * Right.y) + (Left.z * Right.z) + (Left.w * Right.w);

	return (Result);
}

func s_quaternion quaternion_normalized(s_quaternion Left)
{
	s_quaternion Result;

	float Length = sqrtf(quaternion_dot(Left, Left));
	Result = quaternion_divide(Left, Length);

	return (Result);
}

template <typename t>
func void swap(t* a, t* b)
{
	t c = *a;
	*a = *b;
	*b = c;
}

[[nodiscard]]
func int circular_index(int index, int size)
{
	assert(size > 0);
	if(index >= 0) {
		return index % size;
	}
	return (size - 1) - ((-index - 1) % size);
}

// -------------------------------------------------------------------------------------------------------------------------


func s_ray get_camera_ray(s_v3 cam_pos, s_m4 view, s_m4 projection, s_v2 mouse, s_v2 world_size)
{

	float x = (2.0f * mouse.x) / world_size.x - 1.0f;

	// @Note(tkap, 19/06/2022): y may not need to be reversed
	float y = 1.0f - (2.0f * mouse.y) / world_size.y;

	s_v3 ray_nds = v3(x, y, -1);

	s_v4 ray_clip = v4(ray_nds.x, ray_nds.y, ray_nds.z, 1);

	s_v4 ray_eye = v4_multiply_m4(ray_clip, m4_inverse(projection));
	ray_eye = v4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	s_v3 ray_wor = v4_multiply_m4(ray_eye, m4_inverse(view)).xyz;
	// don't forget to normalize the vector at some point
	ray_wor = v3_normalized(ray_wor);

	s_ray result = zero;
	result.pos = cam_pos;
	result.dir = ray_wor;
	return result;
}

func s_m4 m4_inverse(const s_m4 m)
{
	s_m4 result = zero;
	float det;
	int i;

	result.all[0] = m.all[5]  * m.all[10] * m.all[15] -
				m.all[5]  * m.all[11] * m.all[14] -
				m.all[9]  * m.all[6]  * m.all[15] +
				m.all[9]  * m.all[7]  * m.all[14] +
				m.all[13] * m.all[6]  * m.all[11] -
				m.all[13] * m.all[7]  * m.all[10];

	result.all[4] = -m.all[4]  * m.all[10] * m.all[15] +
				m.all[4]  * m.all[11] * m.all[14] +
				m.all[8]  * m.all[6]  * m.all[15] -
				m.all[8]  * m.all[7]  * m.all[14] -
				m.all[12] * m.all[6]  * m.all[11] +
				m.all[12] * m.all[7]  * m.all[10];

	result.all[8] = m.all[4]  * m.all[9] * m.all[15] -
				m.all[4]  * m.all[11] * m.all[13] -
				m.all[8]  * m.all[5] * m.all[15] +
				m.all[8]  * m.all[7] * m.all[13] +
				m.all[12] * m.all[5] * m.all[11] -
				m.all[12] * m.all[7] * m.all[9];

	result.all[12] = -m.all[4]  * m.all[9] * m.all[14] +
					m.all[4]  * m.all[10] * m.all[13] +
					m.all[8]  * m.all[5] * m.all[14] -
					m.all[8]  * m.all[6] * m.all[13] -
					m.all[12] * m.all[5] * m.all[10] +
					m.all[12] * m.all[6] * m.all[9];

	result.all[1] = -m.all[1]  * m.all[10] * m.all[15] +
				m.all[1]  * m.all[11] * m.all[14] +
				m.all[9]  * m.all[2] * m.all[15] -
				m.all[9]  * m.all[3] * m.all[14] -
				m.all[13] * m.all[2] * m.all[11] +
				m.all[13] * m.all[3] * m.all[10];

	result.all[5] = m.all[0]  * m.all[10] * m.all[15] -
				m.all[0]  * m.all[11] * m.all[14] -
				m.all[8]  * m.all[2] * m.all[15] +
				m.all[8]  * m.all[3] * m.all[14] +
				m.all[12] * m.all[2] * m.all[11] -
				m.all[12] * m.all[3] * m.all[10];

	result.all[9] = -m.all[0]  * m.all[9] * m.all[15] +
				m.all[0]  * m.all[11] * m.all[13] +
				m.all[8]  * m.all[1] * m.all[15] -
				m.all[8]  * m.all[3] * m.all[13] -
				m.all[12] * m.all[1] * m.all[11] +
				m.all[12] * m.all[3] * m.all[9];

	result.all[13] = m.all[0]  * m.all[9] * m.all[14] -
				m.all[0]  * m.all[10] * m.all[13] -
				m.all[8]  * m.all[1] * m.all[14] +
				m.all[8]  * m.all[2] * m.all[13] +
				m.all[12] * m.all[1] * m.all[10] -
				m.all[12] * m.all[2] * m.all[9];

	result.all[2] = m.all[1]  * m.all[6] * m.all[15] -
				m.all[1]  * m.all[7] * m.all[14] -
				m.all[5]  * m.all[2] * m.all[15] +
				m.all[5]  * m.all[3] * m.all[14] +
				m.all[13] * m.all[2] * m.all[7] -
				m.all[13] * m.all[3] * m.all[6];

	result.all[6] = -m.all[0]  * m.all[6] * m.all[15] +
				m.all[0]  * m.all[7] * m.all[14] +
				m.all[4]  * m.all[2] * m.all[15] -
				m.all[4]  * m.all[3] * m.all[14] -
				m.all[12] * m.all[2] * m.all[7] +
				m.all[12] * m.all[3] * m.all[6];

	result.all[10] = m.all[0]  * m.all[5] * m.all[15] -
				m.all[0]  * m.all[7] * m.all[13] -
				m.all[4]  * m.all[1] * m.all[15] +
				m.all[4]  * m.all[3] * m.all[13] +
				m.all[12] * m.all[1] * m.all[7] -
				m.all[12] * m.all[3] * m.all[5];

	result.all[14] = -m.all[0]  * m.all[5] * m.all[14] +
					m.all[0]  * m.all[6] * m.all[13] +
					m.all[4]  * m.all[1] * m.all[14] -
					m.all[4]  * m.all[2] * m.all[13] -
					m.all[12] * m.all[1] * m.all[6] +
					m.all[12] * m.all[2] * m.all[5];

	result.all[3] = -m.all[1] * m.all[6] * m.all[11] +
				m.all[1] * m.all[7] * m.all[10] +
				m.all[5] * m.all[2] * m.all[11] -
				m.all[5] * m.all[3] * m.all[10] -
				m.all[9] * m.all[2] * m.all[7] +
				m.all[9] * m.all[3] * m.all[6];

	result.all[7] = m.all[0] * m.all[6] * m.all[11] -
				m.all[0] * m.all[7] * m.all[10] -
				m.all[4] * m.all[2] * m.all[11] +
				m.all[4] * m.all[3] * m.all[10] +
				m.all[8] * m.all[2] * m.all[7] -
				m.all[8] * m.all[3] * m.all[6];

	result.all[11] = -m.all[0] * m.all[5] * m.all[11] +
					m.all[0] * m.all[7] * m.all[9] +
					m.all[4] * m.all[1] * m.all[11] -
					m.all[4] * m.all[3] * m.all[9] -
					m.all[8] * m.all[1] * m.all[7] +
					m.all[8] * m.all[3] * m.all[5];

	result.all[15] = m.all[0] * m.all[5] * m.all[10] -
				m.all[0] * m.all[6] * m.all[9] -
				m.all[4] * m.all[1] * m.all[10] +
				m.all[4] * m.all[2] * m.all[9] +
				m.all[8] * m.all[1] * m.all[6] -
				m.all[8] * m.all[2] * m.all[5];

	det = m.all[0] * result.all[0] + m.all[1] * result.all[4] + m.all[2] * result.all[8] + m.all[3] * result.all[12];

	if(det == 0)
		return result;

	det = 1.0f / det;

	for (i = 0; i < 16; i += 1) {
		result.all[i] = result.all[i] * det;
	}

	return result;
}

func s_v3 ray_at_y(s_ray ray, float y)
{
	float t = (y - ray.pos.y) / ray.dir.y;
	return ray.pos + ray.dir * t;
}

func float lerp(float a, float b, float t)
{
	float result = a + (b - a) * t;
	return result;
}

func s_v3 lerp_v3(s_v3 a, s_v3 b, float t)
{
	s_v3 result = zero;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	result.z = lerp(a.z, b.z, t);
	return result;
}

func float range_lerp(float input_val, float input_start, float input_end, float output_start, float output_end)
{
	return output_start + ((output_end - output_start) / (input_end - input_start)) * (input_val - input_start);
}

template <typename t>
func t min(t a, t b)
{
	return a <= b ? a : b;
}

func s_v3 v3_set_mag(s_v3 v, float len)
{
	v = v3_normalized(v);
	v *= len;
	return v;
}

func float smoothstep(float edge0, float edge1, float x)
{
	// Scale, bias and saturate x to 0..1 range
	x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

func float ilerp(float start, float end, float val)
{
	float b = end - start;
	if(b == 0) { return val; }
	return (val - start) / b;
}

template <typename t>
func void at_most_ptr(t* ptr, t max_val)
{
	*ptr = at_most(max_val, *ptr);
}

template <typename t>
func void at_least_ptr(t* ptr, t min_val)
{
	*ptr = at_least(min_val, *ptr);
}

func int ceilfi(float x)
{
	int result = (int)ceil(x);
	return result;
}

func int roundfi(float x)
{
	int result = (int)roundf(x);
	return result;
}

func float v3_distance(s_v3 a, s_v3 b)
{
	s_v3 c = a - b;
	float result = v3_length(c);
	return result;
}

func float go_towards(float from, float to, float amount)
{
	assert(amount >= 0);

	float result = from;
	float dist = to - from;
	int s = (int)sign(dist);
	result += min(fabsf(dist), amount) * s;
	return result;
}

func s_v3 go_towards(s_v3 from, s_v3 to, float amount)
{
	assert(amount >= 0);

	s_v3 result = from;
	s_v3 dir = to - from;
	s_v3 dir_n = v3_normalized(dir);
	result.x = go_towards(from.x, to.x, amount * fabsf(dir_n.x));
	result.y = go_towards(from.y, to.y, amount * fabsf(dir_n.y));
	result.z = go_towards(from.z, to.z, amount * fabsf(dir_n.z));
	return result;
}


func float sign(float x)
{
	return x >= 0 ? 1.0f : -1.0f;
}

func b8 sphere_vs_sphere(s_v3 pos1, float r1, s_v3 pos2, float r2)
{
	float dist = v3_distance(pos1, pos2);
	b8 result = dist < (r1 + r2);
	return result;
}

func void scale_m4_by_radius(s_m4* out, float radius)
{
	*out = m4_multiply(*out, m4_scale(v3(radius * 2)));
}

template <typename t>
func t max(t a, t b)
{
	t result = a > b ? a : b;
	return result;
}

[[nodiscard]]
func float ilerp_clamp(float start, float end, float value)
{
	return ilerp(start, end, clamp(value, start, end));
}

func float handle_advanced_easing(float x, float x_start, float x_end)
{
	x = clamp(ilerp_clamp(x_start, x_end, x), 0.0f, 1.0f);
	return x;
}

func float ease_in_expo(float x)
{
	if(x == 0) { return 0; }
	return powf(2, 10 * x - 10);
}

func float ease_linear(float x)
{
	return x;
}

func float ease_in_quad(float x)
{
	return x * x;
}

func float ease_out_quad(float x)
{
	float x2 = 1 - x;
	return 1 - x2 * x2;
}

func float ease_out_expo(float x)
{
	if(x == 1) { return 1; }
	return 1 - powf(2, -10 * x);
}

func float ease_out_elastic(float x)
{
	constexpr float c4 = (2 * c_pi) / 3;
	if(x == 0 || x == 1) { return x; }
	return powf(2, -5 * x) * sinf((x * 5 - 0.75f) * c4) + 1;
}

func float ease_out_elastic2(float x)
{
	constexpr float c4 = (2 * c_pi) / 3;
	if(x == 0 || x == 1) { return x; }
	return powf(2, -10 * x) * sinf((x * 10 - 0.75f) * c4) + 1;
}

func float ease_out_back(float x)
{
	float c1 = 1.70158f;
	float c3 = c1 + 1;
	return 1 + c3 * powf(x - 1, 3) + c1 * powf(x - 1, 2);
}

#define m_advanced_easings \
X(ease_linear, e_ease_linear) \
X(ease_in_expo, e_ease_in_expo) \
X(ease_in_quad, e_ease_in_quad) \
X(ease_out_quad, e_ease_out_quad) \
X(ease_out_expo, e_ease_out_expo) \
X(ease_out_elastic, e_ease_out_elastic) \
X(ease_out_elastic2, e_ease_out_elastic2) \
X(ease_out_back, e_ease_out_back) \

#define X(fname, ename) \
func float fname##_advanced(float x, float x_start, float x_end, float target_start, float target_end) \
{ \
	x = handle_advanced_easing(x, x_start, x_end); \
	return lerp(target_start, target_end, fname(x)); \
}
m_advanced_easings
#undef X

func float lerp_snap(float a, float b, float t, float max_diff)
{
	float result = a + (b - a) * t;
	if(fabsf(result - b) < max_diff) {
		result = b;
	}
	return result;
}

func s_v2 lerp_snap(s_v2 a, s_v2 b, float t)
{
	s_v2 result;
	float dist = v2_distance(a, b);
	if(dist < 1.0f) {
		t = 1;
	}
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	return result;
}

func s_v2 lerp_snap(s_v2 a, s_v2 b, float t, s_v2 max_diff)
{
	s_v2 result;
	result.x = lerp_snap(a.x, b.x, t, max_diff.x);
	result.y = lerp_snap(a.y, b.y, t, max_diff.y);
	return result;
}

func float v2_distance(s_v2 a, s_v2 b)
{
	return v2_length(a - b);
}

func float v2_length(s_v2 a)
{
	return sqrtf(a.x * a.x + a.y * a.y);
}

static float sin_range(float min_val, float max_val, float x)
{
	return lerp(min_val, max_val, sinf(x) * 0.5f + 0.5f);
}

func s_v2 v2_from_angle(float angle)
{
	s_v2 result = zero;
	result.x = cosf(angle);
	result.y = sinf(angle);
	return result;
}