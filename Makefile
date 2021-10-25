OUTPUT_PATH = ./build
OBJECT_FILES = $(OUTPUT_PATH)/misc.cpp.o $(OUTPUT_PATH)/shell.cpp.o $(OUTPUT_PATH)/seabang.cpp.o

mkdir -p $(OUTPUT_PATH)

seabang : $(OBJECT_FILES) cc -o $(OUTPUT_PATH)/seabang $(OBJECT_FILES)

misc.cpp.o : misc.cpp
	cc misc.cpp

shell.cpp.o : shell.cpp
	cc shell.cpp

seabang.cpp.o : seabang.cpp
	cc seabang.cpp

clean :
	rmdir -drf  $(OUTPUT_PATH)
