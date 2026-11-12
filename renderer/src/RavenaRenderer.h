#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>

class RavenaRenderer {
public:
    bool initialize(EGLNativeWindowType window);
    void render();
    void shutdown();

    const char* getGPUName() const;

private:
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;

    const char* gpuName = "Unknown";
};
