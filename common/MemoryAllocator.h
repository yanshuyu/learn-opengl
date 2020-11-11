#pragma once
#include"Util.h"
#include<new>

/**
 * Thread safe class used for storing total number of memory allocations and deallocations, primarily for statistic
 * purposes.
 */
class MemoryCounter
{
public:
	static uint64_t getNumAllocs()
	{
		return Allocs;
	}

	static  uint64_t getNumFrees()
	{
		return Frees;
	}

private:
	friend class MemoryAllocatorBase;

	// Threadlocal data can't be exported, so some magic to make it accessible from MemoryAllocator
	static  void incAllocCount() { ++Allocs; }
	static  void incFreeCount() { ++Frees; }

	static uint64_t Allocs;
	static uint64_t Frees;
};




/** Base class all memory allocators need to inherit. Provides allocation and free counting. */
class MemoryAllocatorBase
{
protected:
	static void incAllocCount() { MemoryCounter::incAllocCount(); }
	static void incFreeCount() { MemoryCounter::incFreeCount(); }
};





/**
 * Memory allocator providing a generic implementation. Specialize for specific categories as needed.
 *
 * @note	For example you might implement a pool allocator for specific types in order
 * 			to reduce allocation overhead. By default standard malloc/free are used.
 */
template<typename T>
class MemoryAllocator : public MemoryAllocatorBase
{
public:
	/** Allocates @p bytes bytes. */
	static void* allocate(size_t bytes)
	{
#if PROFILING_ENABLED
		incAllocCount();
#endif

		return malloc(bytes);
	}

	/**
	 * Allocates @p bytes and aligns them to the specified boundary (in bytes). If the aligment is less or equal to
	 * 16 it is more efficient to use the allocateAligned16() alternative of this method. Alignment must be power of two.
	 */
	static void* allocateAligned(size_t bytes, size_t alignment)
	{
#if PROFILING_ENABLED
		incAllocCount();
#endif

		return _aligned_malloc(bytes, alignment);
	}

	/** Allocates @p bytes and aligns them to a 16 byte boundary. */
	static void* allocateAligned16(size_t bytes)
	{
#if PROFILING_ENABLED
		incAllocCount();
#endif

		return _aligned_malloc(bytes, 16);
	}

	/** Frees the memory at the specified location. */
	static void free(void* ptr)
	{
#if PROFILING_ENABLED
		incFreeCount();
#endif

		::free(ptr);
	}

	/** Frees memory allocated with allocateAligned() */
	static void freeAligned(void* ptr)
	{
#if PROFILING_ENABLED
		incFreeCount();
#endif

		_aligned_free(ptr);
	}

	/** Frees memory allocated with allocateAligned16() */
	static void freeAligned16(void* ptr)
	{
#if PROFILING_ENABLED
		incFreeCount();
#endif

		_aligned_free(ptr);
	}
};


/**
 * General allocator provided by the OS. Use for persistent long term allocations, and allocations that don't
 * happen often.
 */
class GenAlloc {
};



/** Allocates the specified number of bytes. */
template<typename Alloc = GenAlloc>
void* _alloc(size_t count) {
	return MemoryAllocator<Alloc>::allocate(count);
}


/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
template<typename T, typename Alloc = GenAlloc>
T* _alloc() {
	return (T*)MemoryAllocator<Alloc>::allocate(sizeof(T));
}


/** Create a new object with the specified allocator and the specified parameters. */
template<typename T, typename Alloc = GenAlloc, typename ... Args>
T* _new(Args&& ... args) {
	return new (_alloc<T, Alloc>()) T(std::forward<Args>(args)...);
}


/** Creates and constructs an array of @p count elements. */
template<typename T, typename Alloc = GenAlloc>
T* _newN(size_t count)
{
	T* ptr = (T*)MemoryAllocator<Alloc>::allocate(sizeof(T) * count);

	for (size_t i = 0; i < count; ++i)
		new (&ptr[i]) T();

	return ptr;
}


/** Frees all the bytes allocated at the specified location. */
template<typename Alloc = GenAlloc>
void _free(void* ptr) {
	MemoryAllocator<Alloc>::free(ptr);
}


/** Destructs and frees the specified object. */
template<typename T, class Alloc = GenAlloc>
void _delete(T* ptr) {
	(ptr)->~T();

	MemoryAllocator<Alloc>::free(ptr);
}


/** Destructs and frees the specified array of objects. */
template<typename T, typename Alloc = GenAlloc>
void _deleteN(T* ptr, size_t count) {
	for (size_t i = 0; i < count; ++i)
		ptr[i].~T();

	MemoryAllocator<Alloc>::free(ptr);
}


/** Callable struct that acts as a proxy for _delete */
template<typename T, typename Alloc = GenAlloc>
struct Deleter {
	constexpr Deleter() noexcept = default;

	/** Constructor enabling deleter conversion and therefore polymorphism with smart points (if they use the same allocator). */
	template <class T2, std::enable_if_t<std::is_convertible<T2*, T*>::value, int> = 0>
	constexpr Deleter(const Deleter<T2, Alloc>& other) noexcept { }

	void operator()(T* ptr) const {
		_delete<T, Alloc>(ptr);
	}
};



/*****************************************************************************/
/* Default versions of all alloc/free/new/delete methods which call GenAlloc */
/*****************************************************************************/


/** Allocates the specified number of bytes. */
//inline void* _alloc(size_t count) {
//	return MemoryAllocator<GenAlloc>::allocate(count);
//}

/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
//template<class T>
//T* _alloc() {
//	return (T*)MemoryAllocator<GenAlloc>::allocate(sizeof(T));
//}

/**
 * Allocates the specified number of bytes aligned to the provided boundary. Boundary is in bytes and must be a power
 * of two.
 */
inline void* _alloc_aligned(size_t count, size_t align){
	return MemoryAllocator<GenAlloc>::allocateAligned(count, align);
}


/** Allocates the specified number of bytes aligned to a 16 bytes boundary. */
inline void* _alloc_aligned16(size_t count) {
	return MemoryAllocator<GenAlloc>::allocateAligned16(count);
}

/** Allocates enough bytes to hold an array of @p count elements the specified type, but doesn't construct them. */
//template<typename T>
//T* _allocN(size_t count) {
//	return (T*)MemoryAllocator<GenAlloc>::allocate(count * sizeof(T));
//}

/** Creates and constructs an array of @p count elements. */
//template<typename T>
//T* _newN(size_t count) {
//	T* ptr = (T*)MemoryAllocator<GenAlloc>::allocate(count * sizeof(T));
//
//	for (size_t i = 0; i < count; ++i)
//		new (&ptr[i]) T;
//
//	return ptr;
//}

/** Create a new object with the specified allocator and the specified parameters. */
//template<typename T, typename ... Args>
//T* _new(Args&&...args) {
//	return new (_alloc<T, GenAlloc>()) T(std::forward<Args>(args)...);
//}

/** Frees all the bytes allocated at the specified location. */
//inline void _free(void* ptr) {
//	MemoryAllocator<GenAlloc>::free(ptr);
//}

/** Frees memory previously allocated with bs_alloc_aligned(). */
inline void _free_aligned(void* ptr) {
	MemoryAllocator<GenAlloc>::freeAligned(ptr);
}

/** Frees memory previously allocated with bs_alloc_aligned16(). */
inline void _free_aligned16(void* ptr) {
	MemoryAllocator<GenAlloc>::freeAligned16(ptr);
}




/** Allocator for the standard library that internally uses bsf memory allocator. */
template <typename T, typename Alloc = GenAlloc>
class StdAlloc {
public:
	using value_type = T;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	constexpr StdAlloc() = default;
	constexpr StdAlloc(StdAlloc&&) = default;
	constexpr StdAlloc(const StdAlloc&) = default;
	template<typename U, typename Alloc2> constexpr StdAlloc(const StdAlloc<U, Alloc2>&) { };
	template<typename U, typename Alloc2> constexpr bool operator==(const StdAlloc<U, Alloc2>&) const noexcept { return true; }
	template<typename U, typename Alloc2> constexpr bool operator!=(const StdAlloc<U, Alloc2>&) const noexcept { return false; }

	template<typename U> class rebind { public: using other = StdAlloc<U, Alloc>; };

	/** Allocate but don't initialize number elements of type T. */
	static T* allocate(const size_t num) {
		if (num == 0)
			return nullptr;

		if (num > max_size())
			return nullptr; // Error

		void* const pv = _alloc<Alloc>(num * sizeof(T));
		if (!pv)
			return nullptr; // Error

		return static_cast<T*>(pv);
	}

	/** Deallocate storage p of deleted elements. */
	static void deallocate(pointer p, size_type) {
		_free<Alloc>(p);
	}

	static constexpr size_t max_size() { 
		return std::numeric_limits<size_type>::max() / sizeof(T); 
	}

	static constexpr void destroy(pointer p) {
		p->~T(); 
	}

	template<typename ... Args>
	static void construct(pointer p, Args&& ... args) {
		new(p) T(std::forward<Args>(args)...); 
	}
};






