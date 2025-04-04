
#include "tk_types.h"

struct s_rng
{
	u64 seed;
};

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		function declarations start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
[[nodiscard]] static u32 randu(s_rng* rng);
[[nodiscard]] static float randf32(s_rng* rng);
[[nodiscard]] static float randf32_11(s_rng* rng);
[[nodiscard]] static u64 randu64(s_rng* rng);
[[nodiscard]] static int rand_range_ii(s_rng* rng, int min, int max);
[[nodiscard]] static int rand_range_ie(s_rng* rng, int min, int max);
[[nodiscard]] static float randf_range(s_rng* rng, float min_val, float max_val);
[[nodiscard]] static b8 chance100(s_rng* rng, float chance);
[[nodiscard]] static b8 chance1(s_rng* rng, f32 chance);
[[nodiscard]] static b8 rand_bool(s_rng* rng);
[[nodiscard]] static int while_chance1(s_rng* rng, float chance);
[[nodiscard]] static s_rng make_rng(u64 seed);
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		function declarations end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#if defined(m_tk_random_impl)

#pragma warning(push, 0)
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#endif // __clang__
// @Note(tkap, 12/06/2024): https://www.pcg-random.org/download.html
[[nodiscard]] static u32 randu(s_rng* rng)
{
	uint64_t oldstate = rng->seed;
	// Advance internal state
	rng->seed = oldstate * 6364136223846793005ULL + 1;
	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__
#pragma warning(pop)

[[nodiscard]] static float randf32(s_rng* rng)
{
	float result = (float)randu(rng) / (float)4294967295;
	return result;
}

[[nodiscard]] static float randf32_11(s_rng* rng)
{
	float result = randf32(rng) * 2 - 1;
	return result;
}

[[nodiscard]] static u64 randu64(s_rng* rng)
{
	u64 result = (u64)(randf32(rng) * (double)c_max_u64);
	return result;
}

// min inclusive, max inclusive
[[nodiscard]] static int rand_range_ii(s_rng* rng, int min, int max)
{
	if(min > max) {
		int temp = min;
		min = max;
		max = temp;
	}

	int result = (int)(min + (randu(rng) % (max - min + 1)));
	return result;
}

// min inclusive, max exclusive
[[nodiscard]] static int rand_range_ie(s_rng* rng, int min, int max)
{
	if(min > max) {
		int temp = min;
		min = max;
		max = temp;
	}
	int result = (int)(min + (randu(rng) % (max - min)));
	return result;
}

[[nodiscard]] static float randf_range(s_rng* rng, float min_val, float max_val)
{
	if(min_val > max_val)
	{
		float temp = min_val;
		min_val = max_val;
		max_val = temp;
	}

	float r = randf32(rng);
	return min_val + (max_val - min_val) * r;
}

[[nodiscard]] static b8 chance100(s_rng* rng, float chance)
{
	assert(chance >= 0);
	b8 result = chance / 100 >= randf32(rng);
	return result;
}

[[nodiscard]] static b8 chance1(s_rng* rng, f32 chance)
{
	assert(chance >= 0);
	return chance >= randf32(rng);
}

[[nodiscard]] static b8 rand_bool(s_rng* rng)
{
	return (b8)(randu(rng) & 1);
}

[[nodiscard]] static int while_chance1(s_rng* rng, float chance)
{
	int result = 0;
	while(chance1(rng, chance)) {
		result += 1;
	}
	return result;
}

[[nodiscard]] static s_rng make_rng(u64 seed)
{
	s_rng rng = {};
	u32 _ = randu(&rng);
	rng.seed += seed;
	_ = randu(&rng);
	return rng;
}


#endif // m_tk_random_impl