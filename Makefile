COMPILE = cc -std=c++17 -o2 -o $@
BIN_FOLDER = /usr/bin
OUTPUT_PATH = ./build
SOURCE_PATH = ./source
OBJECT_FILES = $(OUTPUT_PATH)/misc.cpp.o $(OUTPUT_PATH)/shell.cpp.o $(OUTPUT_PATH)/seabang.cpp.o
EXEC_NAME = seabang

$(OUTPUT_PATH)/$(EXEC_NAME) : $(OUTPUT_PATH) $(OBJECT_FILES)
	cc $(OBJECT_FILES) -lstdc++ -lpthread -lm -o $@

$(OUTPUT_PATH) :
	mkdir -p $(OUTPUT_PATH)

$(OUTPUT_PATH)/misc.cpp.o : $(SOURCE_PATH)/misc.cpp
	$(COMPILE) -c $(SOURCE_PATH)/misc.cpp -o $@

$(OUTPUT_PATH)/shell.cpp.o : $(SOURCE_PATH)/shell.cpp
	$(COMPILE) -c $(SOURCE_PATH)/shell.cpp -o $@

$(OUTPUT_PATH)/seabang.cpp.o : $(SOURCE_PATH)/seabang.cpp
	$(COMPILE) -c $(SOURCE_PATH)/seabang.cpp -o $@

clean :
	rm -drf  $(OUTPUT_PATH)

install :
	sudo cp $(OUTPUT_PATH)/$(EXEC_NAME) $(BIN_FOLDER)/$(EXEC_NAME)