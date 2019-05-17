#pragma once
#include <cstddef>

void* allocate(unsigned int size);

inline void* __CRTDECL operator new(std::size_t size)
{
	return allocate(size);
}

inline void* __CRTDECL operator new(std::size_t size, int array_size)
{
	return allocate(size + sizeof(void*) * array_size);
}

inline void* __CRTDECL operator new(std::size_t size, void* ptr)
{
	return ptr;
}

inline void __CRTDECL operator delete(void* ptr) noexcept {}

inline void __CRTDECL operator delete(void* ptr, std::size_t size) noexcept {}

inline void* __CRTDECL operator new[](std::size_t size) 
{
	return allocate(size);
}

inline void __CRTDECL operator delete[](void* ptr) noexcept {}

inline void __CRTDECL operator delete[](void* ptr, std::size_t size) noexcept  {}

class MemoryManager
{
private:
	char* mem_block_front;

public:
	MemoryManager();
	void free();
};