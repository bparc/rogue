typedef struct alloc_t alloc_t;

struct alloc_t
{
	char name[16];
	s32 offset;
	s32 size;
	s32 padding;
	alloc_t *next;
};

typedef struct
{
	u8 *_memory;
	s32 size;
	s32 offset;

	alloc_t *node;
	alloc_t *head;
} memory_t;

#define PushStruct(Type, Memory) (Type *)_Push(Memory, sizeof(Type), # Type)
#define PushArray(Type, Memory, Count) (Type *)_Push(Memory, sizeof(Type) * Count, # Type)
#define PushSize(Memory, Count)  (void *)_Push(Memory, Count, "???")

fn void *_Push(memory_t *_memory, s32 count, const char name[])
{
	s32 block_header_sz = 0;

	#if _DEBUG
	if (_memory->offset == 0)
		_memory->head = _memory->node = (alloc_t *)_memory->_memory;
	block_header_sz = sizeof(alloc_t);
	#endif

	count += block_header_sz;

	void *result = NULL;
	if (_memory->offset + count <= _memory->size)
	{
		result = (void *)(_memory->_memory + _memory->offset + block_header_sz);
		_memory->offset += count;
	}

#if _DEBUG
	if (result && (block_header_sz > 0))
	{
		alloc_t *block = (alloc_t *)((u8 *)result - block_header_sz);
		strncpy(block->name, name, ArraySize(block->name));

		_memory->node->next = block;
		_memory->node = block;
	}
#endif
	
	if (result)
	{
		memset(result, 0, count);
	}
	return result;
}

fn void FlushMemory(memory_t *_memory)
{
	_memory->offset = 0;
}

fn memory_t Split(memory_t *_memory, s32 size)
{
	memory_t result = {0};
	result._memory = PushSize(_memory, size);
	result.size = size;
	Assert(result._memory);
	return result;
}