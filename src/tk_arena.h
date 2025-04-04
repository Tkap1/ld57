

struct s_linear_arena
{
	int used;
	int capacity;
	u8* memory;
};

static s_linear_arena make_arena_from_memory(u8* memory, int requested_capacity)
{
	int capacity = (requested_capacity + 7) & ~7;
	s_linear_arena result = zero;
	result.capacity = capacity;
	result.memory = memory;
	return result;
}

static s_linear_arena make_arena_from_malloc(int requested_capacity)
{
	int capacity = (requested_capacity + 7) & ~7;
	u8* memory = (u8*)malloc(capacity);
	s_linear_arena result = make_arena_from_memory(memory, capacity);
	return result;
}

static u8* arena_alloc(s_linear_arena* arena, int requested_size)
{
	assert(requested_size > 0);
	int size = (requested_size + 7) & ~7;
	assert(arena->used + size <= arena->capacity);
	u8* result = arena->memory + arena->used;
	arena->used += size;
	return result;
}
