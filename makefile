Main : main.o HttpServer.o
	g++ -o Main.exe main.o HttpServer.o -lws2_32

main.o :
	g++ -g -c main.cpp

HttpServer.o :
	g++ -g -c HttpServer.cpp
	
clean : 
	rm -rf Main.exe *.o