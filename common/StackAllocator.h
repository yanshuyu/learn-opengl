#pragma once
#include"Util.h"
#include"MemoryAllocator.h"


/**
	 * Describes a memory stack of a certain block capacity. See MemStack for more information.
	 *
	 * @tparam	BlockCapacity Minimum size of a block. Larger blocks mean less memory allocations, but also potentially
	 *						  more wasted memory. If an allocation requests more bytes than BlockCapacity, first largest
	 *						  multiple is used instead.
	 */
template <int BlockCapacity = 1024 * 1024>
class StackAllocator
{
private:
	/**
	 * A single block of memory of BlockCapacity size. A pointer to the first free address is stored, and a remaining
	 * size.
	 */
	class MemBlock
	{
	public:
		MemBlock(UINT32 size) :mSize(size) { }

		~MemBlock() = default;

		/**
		 * Returns the first free address and increments the free pointer. Caller needs to ensure the remaining block
		 * size is adequate before calling.
		 */
		UINT8* alloc(UINT32 amount)
		{
			UINT8* freePtr = &mData[mFreePtr];
			mFreePtr += amount;

			return freePtr;
		}

		/**
		 * Deallocates the provided pointer. Deallocation must happen in opposite order from allocation otherwise
		 * corruption will occur.
		 *
		 * @note	Pointer to @p data isn't actually needed, but is provided for debug purposes in order to more
		 * 			easily track out-of-order deallocations.
		 */
		void dealloc(UINT8* data, UINT32 amount)
		{
			mFreePtr -= amount;
			assert((&mData[mFreePtr]) == data && "Out of order stack deallocation detected. Deallocations need to happen in order opposite of allocations.");
		}

		UINT8* mData = nullptr;
		UINT32 mFreePtr = 0;
		UINT32 mSize = 0;
		MemBlock* mNextBlock = nullptr;
		MemBlock* mPrevBlock = nullptr;
	};

public:
	StackAllocator()
	{
		mFreeBlock = allocBlock(BlockCapacity);
	}

	~StackAllocator()
	{
		assert(mFreeBlock->mFreePtr == 0 && "Not all blocks were released before shutting down the stack allocator.");

		MemBlock* curBlock = mFreeBlock;
		while (curBlock != nullptr)
		{
			MemBlock* nextBlock = curBlock->mNextBlock;
			deallocBlock(curBlock);

			curBlock = nextBlock;
		}
	}

	/**
	 * Allocates the given amount of memory on the stack.
	 *
	 * @param[in]	amount	The amount to allocate in bytes.
	 *
	 * @note
	 * Allocates the memory in the currently active block if it is large enough, otherwise a new block is allocated.
	 * If the allocation is larger than default block size a separate block will be allocated only for that allocation,
	 * making it essentially a slower heap allocator.
	 * @note
	 * Each allocation comes with a 4 byte overhead.
	 */
	UINT8* alloc(UINT32 amount)
	{
		amount += sizeof(UINT32);

		UINT32 freeMem = mFreeBlock->mSize - mFreeBlock->mFreePtr;
		if (amount > freeMem)
			allocBlock(amount);

		UINT8* data = mFreeBlock->alloc(amount);

		UINT32* storedSize = reinterpret_cast<UINT32*>(data);
		*storedSize = amount;

		return data + sizeof(UINT32);
	}

	/** Deallocates the given memory. Data must be deallocated in opposite order then when it was allocated. */
	void dealloc(UINT8* data)
	{
		data -= sizeof(UINT32);

		UINT32* storedSize = reinterpret_cast<UINT32*>(data);
		mFreeBlock->dealloc(data, *storedSize);

		if (mFreeBlock->mFreePtr == 0)
		{
			MemBlock* emptyBlock = mFreeBlock;

			if (emptyBlock->mPrevBlock != nullptr)
				mFreeBlock = emptyBlock->mPrevBlock;

			// Merge with next block
			if (emptyBlock->mNextBlock != nullptr)
			{
				UINT32 totalSize = emptyBlock->mSize + emptyBlock->mNextBlock->mSize;

				if (emptyBlock->mPrevBlock != nullptr)
					emptyBlock->mPrevBlock->mNextBlock = nullptr;
				else
					mFreeBlock = nullptr;

				deallocBlock(emptyBlock->mNextBlock);
				deallocBlock(emptyBlock);

				allocBlock(totalSize);
			}
		}
	}

private:
	MemBlock* mFreeBlock = nullptr;

	/**
	 * Allocates a new block of memory using a heap allocator. Block will never be smaller than BlockCapacity no matter
	 * the @p wantedSize.
	 */
	MemBlock* allocBlock(UINT32 wantedSize)
	{
		UINT32 blockSize = BlockCapacity;
		if (wantedSize > blockSize)
			blockSize = wantedSize;

		MemBlock* newBlock = nullptr;
		MemBlock* curBlock = mFreeBlock;

		while (curBlock != nullptr)
		{
			MemBlock* nextBlock = curBlock->mNextBlock;
			if (nextBlock != nullptr && nextBlock->mSize >= blockSize)
			{
				newBlock = nextBlock;
				break;
			}

			curBlock = nextBlock;
		}

		if (newBlock == nullptr)
		{
			UINT8* data = (UINT8*)reinterpret_cast<UINT8*>(_alloc(blockSize + sizeof(MemBlock)));
			newBlock = new (data)MemBlock(blockSize);
			data += sizeof(MemBlock);

			newBlock->mData = data;
			newBlock->mPrevBlock = mFreeBlock;

			if (mFreeBlock != nullptr)
			{
				if (mFreeBlock->mNextBlock != nullptr)
					mFreeBlock->mNextBlock->mPrevBlock = newBlock;

				newBlock->mNextBlock = mFreeBlock->mNextBlock;
				mFreeBlock->mNextBlock = newBlock;
			}
		}

		mFreeBlock = newBlock;
		return newBlock;
	}

	/** Deallocates a block of memory. */
	void deallocBlock(MemBlock* block)
	{
		block->~MemBlock();
		_free(block);
	}
};


extern StackAllocator<1024 * 1024> g_stackAlloc;


void* stack_alloc(size_t cnt);


template<typename T> 
T* stack_alloc() {
	return reinterpret_cast<T*>(g_stackAlloc.alloc(sizeof(T)));
}

template<typename T>
T* stack_alloc_n(size_t n) {
	return reinterpret_cast<T*>(g_stackAlloc.alloc(sizeof(T) * n));
}


template<typename T>
T*	stack_new() {
	T* obj = stack_alloc<T>();
	new ((void*)obj) T();
	
	return obj;
}


template<typename T>
T* stack_new_n(size_t n) {
	T* objs = stack_alloc_n<T>(n);
	for (size_t i = 0; i < n; i++) {
		T* obj = &objs[i];
		new ((void*)obj) T;
	}

	return objs;
}


template<typename T, typename ... Args>
T* stack_new(Args&& ... args) {
	T* obj = stack_alloc<T>();
	new ((void*)obj) T(std::forward<Args>(args) ...);

	return obj;
}


template<typename T, typename ... Args>
T* stack_new_n(size_t n, Args&& ... args) {
	T* objs = stack_alloc<T>(n);
	for (size_t i = 0; i < n; i++) {
		T* obj = objs[i];
		new ((void*)obj) T(std::forward<Args>(args) ...);
	}

	return objs;
}


void stack_free(void* data);


template<typename T>
void stack_free_n(T* datas, size_t n) {
	stack_free(datas);
}


template<typename T>
void stack_delete(T* data) {
	data->~T();
	g_stackAlloc.dealloc((UINT8*)data);
}


template<typename T>
void stack_delete_n(T* datas, size_t n) {
	for (size_t i = 0; i < n; i++) {
		datas[i].~T();
	}
	g_stackAlloc.dealloc((UINT8*)datas);
}