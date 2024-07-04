HDRS = \
       drawing.h\
       graphs.h\
       files.h\

OBJS = \
       main.o\
       drawing.o\
       graphs.o\
       files.o\

all: main

main: $(OBJS)
	g++ -o main $(OBJS) -lSDL2 -lglu32 -lopengl32

main.o: main.cpp $(HDRS)
	g++ -c main.cpp

drawing.o: drawing.cpp drawing.h
	g++ -c drawing.cpp

graphs.o: graphs.cpp graphs.h drawing.h
	g++ -c graphs.cpp

files.o: files.cpp graphs.h files.h
	g++ -c files.cpp

clean:
	rm -fv $(OBJS)
	rm -fv main.exe

