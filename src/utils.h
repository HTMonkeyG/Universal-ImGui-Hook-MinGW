#ifndef __UGLH_UTILS_H__
#define __UGLH_UTILS_H__

template<typename T> inline void SafeRelease(T **obj) {
  if (*obj) {
    (*obj)->Release();
    *obj = nullptr;
  }
}

#endif
