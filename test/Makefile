MAKEFLAGS += -s -j4

AR = ar
CXX = g++

CFLAGS = -Wall -Werror -Wformat -O3 -Wall -Os -ffunction-sections -fdata-sections -static -flto -s -mavx
CFLAGS += -I../libraries/imgui-1.91.9b -I../libraries/imgui-1.91.9b/backends
CFLAGS += -I../libraries/MinHook/include
CFLAGS += -I../libraries/kiero
CFLAGS += -I../src

LFLAGS = -lgdi32 -ld3d12 -ld3d11 -ldwmapi -ld3dcompiler -lole32
LFLAGS += -L../libraries/imgui-1.91.9b -limgui -limgui_impl_win32 -limgui_impl_dx12 -limgui_impl_dx11 -limgui_demo
LFLAGS += -L../libraries/kiero -lkiero
LFLAGS += -L../libraries/MinHook -lMinHook
LFLAGS += -L.. -luglhook

./uglhook-test.dll: ./dllmain.o
	$(CXX) $(CFLAGS) $^ -shared -o $@ $(LFLAGS)

./dllmain.o: ./dllmain.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	-del *.o
	-del .\uglhook-test.dll