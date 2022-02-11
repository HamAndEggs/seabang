# This make file is used for platforms that do not have cmake.
# If your platform has cmake please use that instead.

DEFINES = -DSEABANG_TEMPORAY_FOLDER="/tmp/seabang/" -DSEABANG_CXX_COMPILER="gcc"

#COMPILE = cc $(DEFINES) -std=c++17 -o0 -g2 -o $@
COMPILE = cc $(DEFINES) -std=c++17 -o2 -o $@
BIN_FOLDER = /usr/local/bin
OUTPUT_PATH = ./build
SOURCE_PATH = ./source
OBJECT_FILES = $(OUTPUT_PATH)/dependencies.cpp.o $(OUTPUT_PATH)/TinyTools.cpp.o $(OUTPUT_PATH)/seabang.cpp.o
EXEC_NAME = seabang

$(OUTPUT_PATH)/$(EXEC_NAME) : $(OUTPUT_PATH) $(OBJECT_FILES)
	cc $(OBJECT_FILES) -lstdc++ -lpthread -lm -o $@

$(OUTPUT_PATH) :
	mkdir -p $(OUTPUT_PATH)

$(OUTPUT_PATH)/TinyTools.cpp.o : $(SOURCE_PATH)/TinyTools.cpp
	$(COMPILE) -c $(SOURCE_PATH)/TinyTools.cpp -o $@

$(OUTPUT_PATH)/dependencies.cpp.o : $(SOURCE_PATH)/dependencies.cpp
	$(COMPILE) -c $(SOURCE_PATH)/dependencies.cpp -o $@

$(OUTPUT_PATH)/seabang.cpp.o : $(SOURCE_PATH)/seabang.cpp
	$(COMPILE) -c $(SOURCE_PATH)/seabang.cpp -o $@

clean :
	rm -drf  $(OUTPUT_PATH)

install :
	sudo cp $(OUTPUT_PATH)/$(EXEC_NAME) $(BIN_FOLDER)/$(EXEC_NAME)