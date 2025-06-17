MAKEFLAGS += -s -j4

DIST_DIR = ./dist
SRC_DIR = ./src

CPP_SRC = $(wildcard $(SRC_DIR)/*.cpp)
CPP_OBJ = $(patsubst %.cpp, $(DIST_DIR)/%.o, $(notdir $(CPP_SRC)))

AR = ar
CXX = g++

CFLAGS = -Wall -Werror -Wformat -O3
CFLAGS += -I./libraries/imgui-1.91.9b -I./libraries/imgui-1.91.9b/backends
CFLAGS += -I./libraries/MinHook/include
CFLAGS += -I./libraries/kiero

.PHONY: clean test

libuglhook.a: $(CPP_OBJ)
	$(AR) rcs $@ $^ 

$(DIST_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

test:
	make -C .\test

clean:
	del .\dist\*.o
	del libuglhook.a

clean_test:
	make -C .\test clean
