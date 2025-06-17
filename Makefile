DIST_DIR = ./dist
SRC_DIR = ./src

C_SRC = $(wildcard $(SRC_DIR)/*.c)
CPP_SRC = $(wildcard $(SRC_DIR)/*.cpp)
C_OBJ = $(patsubst %.c, $(DIST_DIR)/%.o, $(notdir $(C_SRC)))
CPP_OBJ = $(patsubst %.cpp, $(DIST_DIR)/%.o, $(notdir $(CPP_SRC)))

TARGET = uni-imgui-hook-mingw.dll
BIN_TARGET = $(DIST_DIR)/$(TARGET)

CC = gcc
CXX = g++

CFLAGS = -Wall -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -static -flto -s -mavx
CFLAGS += -I./libraries/imgui-1.91.9b -I./libraries/imgui-1.91.9b/backends
CFLAGS += -I./libraries/MinHook/include
CFLAGS += -I./libraries/kiero

LFLAGS = -lgdi32 -ld3d12 -ldwmapi -ld3dcompiler -lstdc++ -lole32
LFLAGS += -L./libraries/imgui-1.91.9b -limgui -limgui_impl_win32 -limgui_impl_dx12 -limgui_demo
LFLAGS += -L./libraries/MinHook -lMinHook
LFLAGS += -L./libraries/kiero -lkiero

$(BIN_TARGET): $(C_OBJ) $(CPP_OBJ)
	$(CXX) $(CFLAGS) $^ -shared -o $@ $(LFLAGS)

$(DIST_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DIST_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	del .\dist\*.o
	del .\dist\$(TARGET)
