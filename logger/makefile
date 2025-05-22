log: main.o Logger.o
	g++ main.o Logger.o -o log
main.o:
	g++  -c main.cpp -o main.o
Logger.o:
	g++ -c Logger.cpp -o Logger.o
clean:
	del /f *.o 