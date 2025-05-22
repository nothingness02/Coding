#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "memory_pool.h"
using namespace Memory;// 测试用例
class P1 
{
    int id_;
};

class P2 
{
    int id_[5];
};

class P3
{
    int id_[10];
};

class P4
{
    int id_[20];
};

// 单轮次申请释放次数 线程数 轮次
void BenchmarkMemoryPool(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks); // 线程池
	std::chrono::duration<double> total_costtime(0);
	for (size_t k = 0; k < nworks; ++k) // 创建 nworks 个线程
	{
		vthread[k] = std::thread([&]() {
			for (size_t j = 0; j < rounds; ++j)
			{
				auto begin1 = std::chrono::high_resolution_clock::now(); 
				for (size_t i = 0; i < ntimes; i++)
				{
                    P1* p1 = newElement<P1>(); // 内存池对外接口
                    deleteElement<P1>(p1);
                    P2* p2 = newElement<P2>();
                    deleteElement<P2>(p2);
                    P3* p3 = newElement<P3>();
                    deleteElement<P3>(p3);
                    P4* p4 = newElement<P4>();
                    deleteElement<P4>(p4);
				}
				auto end1 = std::chrono::high_resolution_clock::now(); // 

				std::chrono::duration<double> elapsed = end1 - begin1;
                total_costtime += elapsed;
			}
		});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	    std::cout << "Total cost time for memory pool: " << total_costtime.count() << " seconds" << std::endl;
}

void BenchmarkNew(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	 std::chrono::duration<double> total_costtime(0); 
	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			for (size_t j = 0; j < rounds; ++j)
			{
				auto begin1 = std::chrono::high_resolution_clock::now(); 
				for (size_t i = 0; i < ntimes; i++)
				{
                    P1* p1 = new P1;
                    delete p1;
                    P2* p2 = new P2;
                    delete p2;
                    P3* p3 = new P3;
                    delete p3;
                    P4* p4 = new P4;
                    delete p4;
				}

				auto end1 = std::chrono::high_resolution_clock::now(); // 

				std::chrono::duration<double> elapsed = end1 - begin1;
                total_costtime += elapsed;
			}
		});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	    std::cout << "Total cost time for  new: " << total_costtime.count() << " seconds" << std::endl;
}

int main()
{
    HashBucket::initMemoryPool(); // 使用内存池接口前一定要先调用该函数
    
	BenchmarkMemoryPool(100, 10 , 10000); // 测试内存池
	std::cout << "===========================================================================" << std::endl;
	std::cout << "===========================================================================" << std::endl;
	BenchmarkNew(100, 10, 10000); // 测试 new delete
	
	return 0;
}