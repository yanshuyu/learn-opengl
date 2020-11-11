#include"StackAllocator.h"


StackAllocator<1024 * 1024> g_stackAlloc;

void* stack_alloc(size_t cnt) {
	return g_stackAlloc.alloc(cnt);
}

void stack_free(void* data) {
	g_stackAlloc.dealloc((UINT8*)data);
}