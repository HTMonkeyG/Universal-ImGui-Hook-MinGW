#include "imgui.h"
#include "imgui_impl_win32.h"

#define USE_GL 1

#if USE_GL == 0
#include "imgui_impl_dx12.h"
#elif USE_GL == 1
#include "imgui_impl_dx11.h"
#endif

#include "uglhook.h"
