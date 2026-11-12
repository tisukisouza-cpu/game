#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <cmath>

#define LOG_TAG "RavenaEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static GLuint program = 0;
static GLuint vao = 0;
static GLuint vbo = 0;

static GLint mvpLocation = -1;

static int screenWidth = 1;
static int screenHeight = 1;

static float angle = 0.0f;

static const char* vertexShaderSource = R"(
#version 300 es

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

uniform mat4 uMVP;

out vec3 vColor;

void main() {
    gl_Position = uMVP * vec4(aPosition, 1.0);
    vColor = aColor;
}
)";

static const char* fragmentShaderSource = R"(
#version 300 es

precision mediump float;

in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);

    if (!shader) {
        LOGE("Nao foi possivel criar shader");
        return 0;
    }

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char log[2048] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOGE("Falha no shader: %s", log);

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static void multiplyMatrix(
    const float* a,
    const float* b,
    float* result
) {
    for (int column = 0; column < 4; column++) {
        for (int row = 0; row < 4; row++) {
            result[column * 4 + row] = 0.0f;

            for (int k = 0; k < 4; k++) {
                result[column * 4 + row] +=
                    a[k * 4 + row] * b[column * 4 + k];
            }
        }
    }
}

static void createMatrices(float* mvp) {
    const float pi = 3.14159265f;

    float aspect = (float)screenWidth / (float)screenHeight;
    float f = 1.0f / tanf(45.0f * pi / 360.0f);

    float projection[16] = {};

    projection[0] = f / aspect;
    projection[5] = f;
    projection[10] = (100.0f + 0.1f) / (0.1f - 100.0f);
    projection[11] = -1.0f;
    projection[14] = (2.0f * 100.0f * 0.1f) / (0.1f - 100.0f);

    float c = cosf(angle);
    float s = sinf(angle);

    float modelView[16] = {
         c,  0.0f,  s,  0.0f,
         0.0f, 1.0f, 0.0f, 0.0f,
        -s,  0.0f,  c,  0.0f,
         0.0f, 0.0f, -5.0f, 1.0f
    };

    multiplyMatrix(projection, modelView, mvp);
}

static bool createRenderer() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    if (!vs) return false;

    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    if (!fs) {
        glDeleteShader(vs);
        return false;
    }

    program = glCreateProgram();

    if (!program) {
        LOGE("Falha ao criar programa OpenGL");
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked) {
        char log[2048] = {};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        LOGE("Falha ao vincular shaders: %s", log);

        glDeleteProgram(program);
        program = 0;
        return false;
    }

    mvpLocation = glGetUniformLocation(program, "uMVP");

    const float vertices[] = {
        // Frente
        -1,-1, 1,  1,0,0,   1,-1, 1,  1,0,0,   1, 1, 1,  1,0,0,
        -1,-1, 1,  1,0,0,   1, 1, 1,  1,0,0,  -1, 1, 1,  1,0,0,

        // Traseira
         1,-1,-1,  0,1,0,  -1,-1,-1,  0,1,0,  -1, 1,-1,  0,1,0,
         1,-1,-1,  0,1,0,  -1, 1,-1,  0,1,0,   1, 1,-1,  0,1,0,

        // Esquerda
        -1,-1,-1,  0,0,1,  -1,-1, 1,  0,0,1,  -1, 1, 1,  0,0,1,
        -1,-1,-1,  0,0,1,  -1, 1, 1,  0,0,1,  -1, 1,-1,  0,0,1,

        // Direita
         1,-1, 1,  1,1,0,   1,-1,-1,  1,1,0,   1, 1,-1,  1,1,0,
         1,-1, 1,  1,1,0,   1, 1,-1,  1,1,0,   1, 1, 1,  1,1,0,

        // Topo
        -1, 1, 1,  1,0,1,   1, 1, 1,  1,0,1,   1, 1,-1,  1,0,1,
        -1, 1, 1,  1,0,1,   1, 1,-1,  1,0,1,  -1, 1,-1,  1,0,1,

        // Base
        -1,-1,-1,  0,1,1,   1,-1,-1,  0,1,1,   1,-1, 1,  0,1,1,
        -1,-1,-1,  0,1,1,   1,-1, 1,  0,1,1,  -1,-1, 1,  0,1,1
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), (void*)0
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    LOGI("Ravena Engine: renderizador 3D criado!");
    return true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ravena_engine_NativeRenderer_nativeInit(
    JNIEnv*, jclass
) {
    program = 0;
    vao = 0;
    vbo = 0;
    angle = 0.0f;

    createRenderer();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ravena_engine_NativeRenderer_nativeResize(
    JNIEnv*, jclass, jint width, jint height
) {
    screenWidth = width > 0 ? width : 1;
    screenHeight = height > 0 ? height : 1;

    glViewport(0, 0, screenWidth, screenHeight);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ravena_engine_NativeRenderer_nativeRender(
    JNIEnv*, jclass
) {
    glClearColor(0.035f, 0.045f, 0.065f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!program || !vao || !vbo) return;

    float mvp[16] = {};
    createMatrices(mvp);

    glUseProgram(program);

    glUniformMatrix4fv(
        mvpLocation, 1, GL_FALSE, mvp
    );

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    angle += 0.01f;
    if (angle > 6.2831853f) angle -= 6.2831853f;
}
