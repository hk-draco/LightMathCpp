#include <malloc.h>

constexpr int block_size = 1000 * 1024; // 1MB
char* mem_blocks[10000]; // 10GB
char* curr_mem = nullptr;
int curr_size = block_size;
int block_num = 0;

//#define OS_ALLOC

void* allocate(unsigned int size)
{
#ifdef OS_ALLOC
	return malloc(size);
#endif
	auto res = curr_mem;
	curr_size += size;

	if (curr_size > block_size) {
		if (size > block_size) {
			curr_size -= size;
			curr_mem -= size;
			res = (char*)malloc(size);
			mem_blocks[block_num] = res;
			block_num++;
		} else {
			curr_size = size;
			res = curr_mem = (char*)malloc(block_size);
			mem_blocks[block_num] = curr_mem;
			block_num++;
		}
	}

	curr_mem += size;
	return res;
}

MemoryManager::MemoryManager()
{
	mem_block_front = curr_mem;
}

void MemoryManager::free()
{
	for (char* block = mem_block_front; block <= curr_mem; block++) {
		::free(block);
		block_num--;
	}
}