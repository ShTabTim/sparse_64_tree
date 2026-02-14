#include "highgl.hpp"

#define high_gl_create(output, func, ...) func##_proc func= nullptr;

HIGH_GL_USING_LIST(high_gl_create)

#undef high_gl_create

#define high_gl_find(output, func, ...) \
    func = (func##_proc)wglGetProcAddress(#func); \
    if (func == nullptr) { \
        MessageBoxW(nullptr, TEXT(#func), TEXT("Õ¿≈¡Õ”À¿—‹ ‘”Õ ÷»ﬂ"), MB_ICONERROR | MB_OK);\
    }


void high_gl_init() {
    HIGH_GL_USING_LIST(high_gl_find)
}

#undef high_gl_find