#CC specifies which compiler we're using
CC = i686-w64-mingw32-g++.exe # 32 bit compiler
# CC = x86_64-w64-mingw32-g++.exe # 64 bit compiler

#INCLUDE_PATHS specifies the additional include paths we'll need -IC:\mingw_dev_lib\include\SDL2
INCLUDE_PATHS =  -Idependencies/include/SDL2

#LIBRARY_PATHS specifies the additional library paths we'll need -LC:\mingw_dev_lib\lib
LIBRARY_PATHS =  -Ldependencies/lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window

PATHS=$(LIBRARY_PATHS) $(INCLUDE_PATHS)

#LINKER_FLAGS specifies the libraries we're linking against
FLAGS =-Wall -m32 -g -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = main.exe


SRCS = $(wildcard src/*.cpp)

$(warning SRCS IS $(SRCS) \n)

all: $(SRCS)
	$(CC) $(SRCS) $(PATHS) $(FLAGS) -o $(OBJ_NAME)
clean: 
	rm -f $(PROGS)