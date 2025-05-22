#include "memory_pool.h"
namespace Memory{

MemoryPool::MemoryPool(size_t BlockSize):BlockSize_(BlockSize)
            ,SlotSize_(0)
            ,firstBlock_(nullptr)
            ,curSlot_(nullptr)
            ,freeList_(nullptr)
            ,lastSlot_(nullptr)
            {}
MemoryPool::~MemoryPool()
{
    Slot* cur=firstBlock_;
    while(cur)
    {
        Slot* next=cur->next;
        operator delete(reinterpret_cast<void*>(cur));
        cur=next;
    }
}
void MemoryPool::init(size_t size)
{
    assert(size>0);
    SlotSize_=size;
    firstBlock_=nullptr;
    curSlot_=nullptr;
    lastSlot_=nullptr;
    freeList_=nullptr;
}
void* MemoryPool::allocate()
{
    Slot* slot = popFreeList();
    if(slot!=nullptr)
        return slot;
    Slot* temp;
    {
        std::lock_guard<std::mutex> lock(mutexForBlock_);
        if(curSlot_>=lastSlot_)
        {
            allocateNewBlock();
        }
        temp=curSlot_;
        curSlot_+=SlotSize_/sizeof(Slot);//
    }
    return temp;
}
void MemoryPool::deallocate(void* ptr)
{
    if(!ptr) return;
    Slot* slot =reinterpret_cast<Slot*>(ptr);
    pushFreeList(slot);
}
void MemoryPool::allocateNewBlock()
{
    void* newBlock=operator new(BlockSize_);
    reinterpret_cast<Slot*>(newBlock)->next.store(firstBlock_, std::memory_order_relaxed);
    firstBlock_=reinterpret_cast<Slot*>(newBlock);
    char* body =reinterpret_cast<char*>(newBlock)+sizeof(Slot*);
    size_t paddingSize = padPointer(body,SlotSize_);
    curSlot_=reinterpret_cast<Slot*>(body+paddingSize);
    lastSlot_ =reinterpret_cast<Slot*>(reinterpret_cast<size_t>(newBlock)+BlockSize_-SlotSize_);
    freeList_=nullptr;
}

//内存对齐提高释放和分配的效率和减少读取的次数
size_t MemoryPool::padPointer(char* p,size_t align)
{//align 是槽的大小
    return (align-reinterpret_cast<size_t>(p))%align;
}
//实现无锁操作
bool MemoryPool::pushFreeList(Slot* slot)
{
    //循环更新OldHead
    while(true)
    {
        //获取当前可用槽的槽首并且将传入的空闲节点指向头指针
        Slot* oldHead=freeList_.load(std::memory_order_relaxed);
        slot->next.store(oldHead,std::memory_order_relaxed);
        if(freeList_.compare_exchange_weak(oldHead,slot,
            std::memory_order_release,std::memory_order_relaxed))
            {
                return true;
            }
        //谁快谁插入假设多个进程同时去加载oldHead那么 假设有一个线程最快速的插入了自己的slot那么另一个进程在进行自旋锁的时候就会失效然后循环尝试
        //如果失败那么说明产生了竞争那么CAS失败并且尝试再次插入    
    }
}

Slot* MemoryPool::popFreeList()
{
    while(true)
    {
        Slot* oldHead = freeList_.load(std::memory_order_acquire);
        if(oldHead==nullptr)
            return nullptr;
        Slot* newHead =nullptr;
        //检验newHead的有效性
        try{
            newHead = oldHead->next.load(std::memory_order_relaxed);
        }
        catch(...){
            continue;
        }
        //尝试更新头节点
        if(freeList_.compare_exchange_weak(oldHead,newHead,
            std::memory_order_acquire,std::memory_order_relaxed))
            {
                return oldHead;
            }
    }
}
void HashBucket::initMemoryPool()
{
    for(int i=0;i<MEMORY_POOL_NUM;i++)
    {
        getMemoryPool(i).init((i+1)*SLOT_BASE_SIZE);
    }
}

MemoryPool& HashBucket::getMemoryPool(int index)
{
    static MemoryPool memorypool[MEMORY_POOL_NUM];
    return memorypool[index];
}

}