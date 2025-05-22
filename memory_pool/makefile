client: client.o memory_pool.o
	g++ client.o memory_pool.o -o client
memory_pool.o:
	g++ -c memory_pool.cpp -o memory_pool.o
client.o:
	g++ -c client.cpp -o client.o
clean:
	del /f *.o client.exe  