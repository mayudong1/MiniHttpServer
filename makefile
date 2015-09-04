objs += main.o 
objs += HttpServer.o 
objs += HttpSession.o

LIBS += -lws2_32
CFLAGS = -g -O2 -Wall

Main : $(objs)
	g++ -o Main.exe $(objs) $(LIBS)

main.o : main.cpp
	g++ -c $(CFLAGS) main.cpp

HttpServer.o : HttpServer.cpp
	g++ -c $(CFLAGS) HttpServer.cpp

HttpSession.o : HttpSession.cpp
	g++ -c $(CFLAGS) HttpSession.cpp

clean : 
	rm -rf Main.exe *.o