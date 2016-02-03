CC = g++
Sources = main.cpp
Modules = configurator_linux.o
CFlags = -c
#WallFlag = -Wall
WallFlag = 
OutPut = petri_dish
curses = -lncurses

all: configurator
	$(CC) $(CFlags) $(WallFlag) $(Sources) -o main.o
	$(CC) main.o $(Modules) -o $(OutPut) $(curses)
	rm main.o
	./$(OutPut)

configurator:
	$(CC) $(CFlags) $(WallFlag) header/configurator.cpp -o configurator_linux.o

clean:
	rm -rf *.o