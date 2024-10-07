typedef struct
{
	u8 *memory;
	s32 size;
	s32 offset;
} memory_t;

#define PushStruct(Type, Memory) (Type *)Push(Memory, sizeof(Type))
#define PushArray(Type, Memory, Count) (Type *)Push(Memory, sizeof(Type) * Count)

fn void *Push(memory_t *memory, s32 count)
{
	void *result = NULL;
	if (memory->offset + count <= memory->size)
	{
		result = (void *)(memory->memory + memory->offset);
		memory->offset += count;
	}
	return result;
}

fn void FlushMemory(memory_t *memory)
{
	memory->offset = 0;
}

fn memory_t PartitionMemory(memory_t *memory, s32 size)
{
	memory_t result = {0};
	result.memory = Push(memory, size);
	result.size = size;
	Assert(result.memory);
	return result;
}