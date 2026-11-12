#include "RavenaRenderer.h"
#include <EGL/eglext.h>

#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR 0x0040
#endif

bool RavenaRenderer::initialize(EGLNativeWindowType window) {

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (display == EGL_NO_DISPLAY)
        return false;

    EGLint major, minor;

    if (!eglInitialize(display, &major, &minor))
        return false;

    const EGLint configAttributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    EGLConfig config;
    EGLint count = 0;

    if (!eglChooseConfig(
        display, configAttributes,
        &config, 1, &count
    ) || count == 0)
        return false;

    const EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    surface = eglCreateWindowSurface(
        display, config, window, nullptr
    );

    if (surface == EGL_NO_SURFACE)
        return false;

    context = eglCreateContext(
        display, config,
        EGL_NO_CONTEXT, contextAttributes
    );

    if (context == EGL_NO_CONTEXT)
        return false;

    if (!eglMakeCurrent(display, surface, surface, context))
        return false;

    gpuName = reinterpret_cast<const char*>(
        glGetString(GL_RENDERER)
    );

    glEnable(GL_DEPTH_TEST);

    return true;
}

void RavenaRenderer::render() {

    if (display == EGL_NO_DISPLAY)
        return;

    EGLint width = 0;
    EGLint height = 0;

    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    glViewport(0, 0, width, height);

    glClearColor(0.025f, 0.035f, 0.055f, 1.0f);

    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );

    eglSwapBuffers(display, surface);
}

const char* RavenaRenderer::getGPUName() const {
    return gpuName;
}

void RavenaRenderer::shutdown() {

    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(
            display,
            EGL_NO_SURFACE,
            EGL_NO_SURFACE,
            EGL_NO_CONTEXT
        );

        if (context != EGL_NO_CONTEXT)
            eglDestroyContext(display, context);

        if (surface != EGL_NO_SURFACE)
            eglDestroySurface(display, surface);

        eglTerminate(display);
    }

    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
}
