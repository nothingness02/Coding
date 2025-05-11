#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>

namespace Memory{
#define MEMORY_POOL_NUM 64
#define SLOT_BASE_SIZE 8
#define MAX_SLOT_SIZE 512 

struct Slot
{
    std::atomic<Slot*> next;
};

class MemoryPool{
public:
    MemoryPool(size_t BlockSize=4096);
    ~MemoryPool();
    void init(size_t);
    void* allocate();
    void deallocate(void* ptr);
private:
    void allocateNewBlock();
    size_t padPointer(char* p,size_t align);//划分
     // 使用CAS操作进行无锁入队和出队
    bool pushFreeList(Slot* slot);
    Slot* popFreeList();
private:
    int BlockSize_;
    int SlotSize_;
    Slot* firstBlock_;
    Slot* curSlot_;
    std::atomic<Slot*> freeList_;
    Slot* lastSlot_;
    std::mutex mutexForBlock_;
};
class HashBucket{
    public:
        static void initMemoryPool();
        static MemoryPool& getMemoryPool(int index);
        static void* useMemory(size_t size)//根据大小分配适合的内存池
        {
            if(size<=0)
                return nullptr;
            if(size> MAX_SLOT_SIZE)
            {
                return operator new(size);
            }
            return getMemoryPool(((size+7)/ SLOT_BASE_SIZE)-1).allocate();
        }
        static void freeMemory(void* ptr,size_t size)
        {
            if(ptr==nullptr)
            {
                return;
            }
            if(size>MAX_SLOT_SIZE)
            {
                operator delete(ptr);
                return;
            }
            getMemoryPool(((size+7)/SLOT_BASE_SIZE)-1).deallocate(ptr);
        }
        template<typename T,typename... Args>
        friend T* newElement(Args&&... args);
        template<typename T>
        friend void deleteElement(T* p);
    };
    template<typename T,typename... Args>
    T* newElement(Args&&... args)
    {
        T* p=nullptr;
        if((p=reinterpret_cast<T*>(HashBucket::useMemory(sizeof(T))))!=nullptr)
        {
            new(p) T(std::forward<Args>(args)...);
        }
        return p;
    }
    template<typename T>
    void deleteElement(T* p)
    {
        if(p)
        {
            p->~T();
            HashBucket::freeMemory(reinterpret_cast<void*>(p),sizeof(T));
        }
    }
}