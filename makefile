Main : main.o HttpServer.o
	g++ -o Main.exe main.o HttpServer.o -lws2_32

clean : 
	rm -rf Main.exe *.o