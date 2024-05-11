main: main.cpp window_manager.o util.o
	g++ -o main main.cpp window_manager.o util.o -lX11 -lglog -lXpm

window_manager.o: window_manager.cpp window_manager.h
	g++ -o window_manager.o -c window_manager.cpp -lX11 -lglog -lXpm

util.o: util.cpp util.h
	g++ -o util.o -c util.cpp

cleanall:
	rm *.o main