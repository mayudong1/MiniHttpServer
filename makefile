src += main.cpp
src += HttpServer.cpp
src += HttpSession.cpp

objs += main.o 
objs += HttpServer.o 
objs += HttpSession.o

LIBS += -lws2_32
CFLAGS = -g -O2 -Wall

Main : $(objs)
	g++ -o Main.exe $(objs) $(LIBS)

$(objs) : $(src)
	g++ -c $(CFLAGS) $^

clean : 
	rm -rf Main.exe *.o