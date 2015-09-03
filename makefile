Main : main.o HttpServer.o
	g++ -o Main.exe main.o HttpServer.o

clean : 
	rm -rf Main.exe main.o HttpServer.o