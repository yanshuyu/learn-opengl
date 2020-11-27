#pragma once
#include"Util.h"
#include"MemoryAllocator.h"
#include<vector>

template <typename T, typename A = StdAlloc<T>>
using Vector = std::vector<T, A>;

/**
 * Frame allocator. Performs very fast allocations but can only free all of its memory at once. Perfect for allocations
 * that last just a single frame.
 *
 * @note	Not thread safe with an exception. alloc() and clear() methods need to be called from the same thread.
 * 			dealloc() is thread safe and can be called from any thread.
 */
class  FrameAllocator {
	private:
		/** A single block of memory within a frame allocator. */
		class MemBlock
		{
		public:
			MemBlock(UINT32 size) :mSize(size) { }

			~MemBlock() = default;

			/** Allocates a piece of memory within the block. Caller must ensure the block has enough empty space. */
			UINT8* alloc(UINT32 amount);

			/** Releases all allocations within a block but doesn't actually free the memory. */
			void clear();

			UINT8* mData = nullptr;
			UINT32 mFreePtr = 0;
			UINT32 mSize;
		};

public:
	FrameAllocator(UINT32 blockSize = 1024 * 1024);
	~FrameAllocator();

	/**
	 * Allocates a new block of memory of the specified size.
	 *
	 * @param[in]	amount	Amount of memory to allocate, in bytes.
	 *
	 * @note	Not thread safe.
	 */
	UINT8* alloc(UINT32 amount);

	/**
	 * Allocates a new block of memory of the specified size aligned to the specified boundary. If the aligment is less
	 * or equal to 16 it is more efficient to use the allocAligned16() alternative of this method.
	 *
	 * @param[in]	amount		Amount of memory to allocate, in bytes.
	 * @param[in]	alignment	Alignment of the allocated memory. Must be power of two.
	 *
	 * @note	Not thread safe.
	 */
	UINT8* allocAligned(UINT32 amount, UINT32 alignment);

	/**
	 * Allocates and constructs a new object.
	 *
	 * @note	Not thread safe.
	 */
	template<typename T, typename ... Args>
	T* construct(Args&&...args) {
		return new ((T*)alloc(sizeof(T))) T(std::forward<Args>(args) ...);
	}

	/**
	 * Destructs and deallocates an object.
	 *
	 * @note	Not thread safe.
	 */
	template<typename T>
	void destruct(T* data) {
		data->~T();
		free((UINT8*)data);
	}

	/**
	 * Deallocates a previously allocated block of memory.
	 *
	 * @note
	 * No deallocation is actually done here. This method is only used for debug purposes so it is easier to track
	 * down memory leaks and corruption.
	 * @note
	 * Thread safe.
	 */
	void free(UINT8* data);

	/**
	 * Deallocates and destructs a previously allocated object.
	 *
	 * @note
	 * No deallocation is actually done here. This method is only used to call the destructor and for debug purposes
	 * so it is easier to track down memory leaks and corruption.
	 * @note
	 * Thread safe.
	 */
	template<typename T>
	void free(T* obj) {
		if (obj != nullptr)
			obj->~T();

		free((UINT8*)obj);
	}

	/** Starts a new frame. Next call to clear() will only clear memory allocated past this point. */
	void markFrame();

	/**
	 * Deallocates all allocated memory since the last call to markFrame() (or all the memory if there was no call
	 * to markFrame()).
	 *
	 * @note	Not thread safe.
	 */
	void clearFrame();

	void clearAllFrame();
private:
	UINT32 mBlockSize;
	Vector<MemBlock*> mBlocks;
	MemBlock* mFreeBlock;
	UINT32 mNextBlockIdx;
	UINT32 mTotalAllocBytes;
	void* mLastFrame;

	/**
	 * Allocates a dynamic block of memory of the wanted size. The exact allocation size might be slightly higher in
	 * order to store block meta data.
	 */
	MemBlock* allocBlock(UINT32 wantedSize);

	/** Frees a memory block. */
	void deallocBlock(MemBlock* block);
};




/**
 * Version of FrameAlloc that allows blocks size to be provided through the template argument instead of the
 * constructor. */
template<size_t BlockSize>
class TFrameAllocator : public FrameAllocator
{
public:
	TFrameAllocator():FrameAllocator(BlockSize) { }
};



/** Allocator for the standard library that internally uses a frame allocator. */
template <typename T>
class StdFrameAlloc {
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	FrameAllocator* mFrameAlloc = nullptr;

	StdFrameAlloc() noexcept = default;

	StdFrameAlloc(FrameAllocator* alloc) noexcept :mFrameAlloc(alloc) {}

	template<typename U> StdFrameAlloc(const StdFrameAlloc<U>& alloc) noexcept :mFrameAlloc(alloc.mFrameAlloc) {}	

	template<typename U> bool operator == (const StdFrameAlloc<U>&) const noexcept { return true; }
	template<typename U> bool operator != (const StdFrameAlloc<U>&) const noexcept { return false; }
	
	template<typename U> class rebind { 
		public: typedef StdFrameAlloc<U> other;
	};

	/** Allocate but don't initialize number elements of type T.*/
	T* allocate(const size_t num) const {
		if (num == 0)
			return nullptr;

		if (num > static_cast<size_t>(-1) / sizeof(T))
			return nullptr; // Error

		void* const pv = mFrameAlloc->alloc((UINT32)(num * sizeof(T)));
		if (!pv)
			return nullptr; // Error

		return static_cast<T*>(pv);
	}

	/** Deallocate storage p of deleted elements. */
	void deallocate(T* p, size_t num) const noexcept {
		mFrameAlloc->free((UINT8*)p);
	}


	size_t max_size() const { return std::numeric_limits<size_type>::max() / sizeof(T); }
	void construct(pointer p, const_reference t) { new (p) T(t); }
	void destroy(pointer p) { p->~T(); }
	template<class U, class... Args>
	void construct(U* p, Args&&... args) { new(p) U(std::forward<Args>(args)...); }
};



/** Return that all specializations of this allocator are interchangeable. */
template <typename T1, typename T2>
bool operator == (const StdFrameAlloc<T1>&, const StdFrameAlloc<T2>&) throw() {
	return true;
}

/** Return that all specializations of this allocator are interchangeable. */
template <typename T1, typename T2>
bool operator != (const StdFrameAlloc<T1>&, const StdFrameAlloc<T2>&) throw() {
	return false;
}


extern TFrameAllocator<1024*1024> g_frameAlloc;


/**
 * Allocates some memory using the global frame allocator.
 *
 * @param[in]	numBytes	Number of bytes to allocate.
 */
inline UINT8* frame_alloc(UINT32 numBytes) {
	return g_frameAlloc.alloc(numBytes);
}

/**
 * Allocates the specified number of bytes aligned to the provided boundary, using the global frame allocator. Boundary
 * is in bytes and must be a power of two.
 */
inline UINT8* frame_alloc_aligned(UINT32 count, UINT32 align) {
	return g_frameAlloc.allocAligned(count, align);
}

/**
 * Deallocates memory allocated with the global frame allocator.
 *
 * @note	Must be called on the same thread the memory was allocated on.
 */
inline void frame_free(void* data) {
	return g_frameAlloc.free((UINT8*)data);
}

/**
 * Frees memory previously allocated with bs_frame_alloc_aligned().
 *
 * @note	Must be called on the same thread the memory was allocated on.
 */
inline void frame_free_aligned(void* data) {
	return g_frameAlloc.free((UINT8*)data);
}

/**
 * Allocates enough memory to hold the object of specified type using the global frame allocator, but does not
 * construct the object.
 */
template<typename T>
T* frame_alloc() {
	return (T*)frame_alloc(sizeof(T));
}

/**
 * Allocates enough memory to hold N objects of specified type using the global frame allocator, but does not
 * construct the object.
 */
template<typename T>
T* frame_alloc(UINT32 count) {
	return (T*)frame_alloc(sizeof(T) * count);
}


/**
 * Allocates enough memory to hold the object(s) of specified type using the global frame allocator, and constructs them.
 */
template<typename T, typename ... Args>
T* frame_new(Args&& ... args)
{
	T* obj = frame_alloc<T>();
	new ((void*)obj) T(std::forward<Args>(args)...);

	return obj;
}


/**
 * Allocates enough memory to hold the object(s) of specified type using the global frame allocator, and constructs them.
 */
template<typename T>
T* frame_new_n(UINT32 count) {
	T* objs = frame_alloc<T>(count);

	for (unsigned int i = 0; i < count; i++)
		new ((void*)&objs[i]) T();

	return objs;
}

/**
 * Destructs and deallocates an object allocated with the global frame allocator.
 *
 * @note	Must be called on the same thread the memory was allocated on.
 */
template<typename T>
void frame_delete(T* data) {
	data->~T();

	frame_free((UINT8*)data);
}

/**
 * Destructs and deallocates an array of objects allocated with the global frame allocator.
 *
 * @note	Must be called on the same thread the memory was allocated on.
 */
template<typename T>
void frame_delete_n(T* data, UINT32 count) {
	for (unsigned int i = 0; i < count; i++)
		data[i].~T();

	frame_free((UINT8*)data);
}

/** @copydoc FrameAlloc::markFrame */
inline void frame_mark() {
	g_frameAlloc.markFrame();
}

/** @copydoc FrameAlloc::clear */
inline void frame_clear() {
	g_frameAlloc.clearFrame();
 }




/**
 * Specialized memory allocator implementations that allows use of a global frame allocator in normal
 * new/delete/free/dealloc operators.
 */
template<>
class MemoryAllocator<FrameAllocator> : public MemoryAllocatorBase
{
public:
	/** @copydoc MemoryAllocator::allocate */
	static void* allocate(size_t bytes) {
		return frame_alloc((UINT32)bytes);
	}

	/** @copydoc MemoryAllocator::allocateAligned */
	static void* allocateAligned(size_t bytes, size_t alignment) {
#if PROFILING_ENABLED
		incAllocCount();
#endif
		return frame_alloc_aligned((UINT32)bytes, (UINT32)alignment);
	}

	/** @copydoc MemoryAllocator::allocateAligned16 */
	static void* allocateAligned16(size_t bytes) {
#if PROFILING_ENABLED
		incAllocCount();
#endif

		return frame_alloc_aligned((UINT32)bytes, 16);
	}

	/** @copydoc MemoryAllocator::free */
	static void free(void* ptr) {
#if PROFILING_ENABLED
		incFreeCount();
#endif
		frame_free(ptr);
	}

	/** @copydoc MemoryAllocator::freeAligned */
	static void freeAligned(void* ptr) {
#if PROFILING_ENABLED
		incFreeCount();
#endif

		frame_free_aligned(ptr);
	}

	/** @copydoc MemoryAllocator::freeAligned16 */
	static void freeAligned16(void* ptr) {
#if PROFILING_ENABLED
		incFreeCount();
#endif

		frame_free_aligned(ptr);
	}
};