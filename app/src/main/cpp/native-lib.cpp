#include <jni.h>
#include <GLES3/gl3.h>
#include <cmath>
#include <android/log.h>

#define LOG_TAG "RavenaEngine"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static GLuint program = 0;
static GLuint vbo = 0;
static int shaderError = 0;
static float angle = 0.0f;
static int width = 1;
static int height = 1;

static const char* vertexShader = R"(
#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

uniform mat4 uMVP;

out vec3 vColor;

void main() {
    gl_Position = vec4(aPosition * 0.5, 1.0);
    vColor = aColor;
}
)";

static const char* fragmentShader = R"(
#version 300 es
precision mediump float;
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.5, 0.0, 1.0);
}
)";

static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char log[2048] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOGE("Erro ao compilar shader: %s", log);
        shaderError = (type == GL_VERTEX_SHADER) ? 1 : 2;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static void createRenderer() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    if (!vs || !fs) return;

    program = glCreateProgram();
    if (!program) shaderError = 4;
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked) {
        shaderError = 3;
        char log[2048] = {};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        LOGE("Erro ao vincular programa: %s", log);
        glDeleteProgram(program);
        program = 0;
        return;
    }

    LOGE("Renderizador OpenGL inicializado com sucesso");

    const float vertices[] = {
        // Frente
        -1,-1, 1,  1,0,0,  1,-1, 1,  1,0,0,  1,1,1,  1,0,0,
        -1,-1, 1,  1,0,0,  1,1,1,  1,0,0, -1,1,1,  1,0,0,

        // Trás
         1,-1,-1,  0,1,0, -1,-1,-1,  0,1,0, -1,1,-1,  0,1,0,
         1,-1,-1,  0,1,0, -1,1,-1,  0,1,0,  1,1,-1,  0,1,0,

        // Esquerda
        -1,-1,-1,  0,0,1, -1,-1, 1,  0,0,1, -1,1, 1,  0,0,1,
        -1,-1,-1,  0,0,1, -1,1, 1,  0,0,1, -1,1,-1,  0,0,1,

        // Direita
         1,-1, 1,  1,1,0,  1,-1,-1,  1,1,0,  1,1,-1,  1,1,0,
         1,-1, 1,  1,1,0,  1,1,-1,  1,1,0,  1,1, 1,  1,1,0,

        // Topo
        -1,1, 1,  1,0,1,  1,1, 1,  1,0,1,  1,1,-1,  1,0,1,
        -1,1, 1,  1,0,1,  1,1,-1,  1,0,1, -1,1,-1,  1,0,1,

        // Base
        -1,-1,-1,  0,1,1,  1,-1,-1,  0,1,1,  1,-1, 1,  0,1,1,
        -1,-1,-1,  0,1,1,  1,-1, 1,  0,1,1, -1,-1, 1,  0,1,1
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    LOGE("Diagnostico: program=%u, vbo=%u, vertices=%zu",
         program, vbo, sizeof(vertices) / (6 * sizeof(float)));
    LOGE("OpenGL: %s", glGetString(GL_VERSION));
    LOGE("GPU: %s", glGetString(GL_RENDERER));
    LOGE("Erro OpenGL apos criar VBO: 0x%x", glGetError());

    glEnable(GL_DEPTH_TEST);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ravena_engine_NativeRenderer_nativeInit(
    JNIEnv*, jclass) {
    createRenderer();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ravena_engine_NativeRenderer_nativeResize(
    JNIEnv*, jclass, jint w, jint h) {
    width = w > 0 ? w : 1;
    height = h > 0 ? h : 1;
    glViewport(0, 0, width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ravena_engine_NativeRenderer_nativeRender(
    JNIEnv*, jclass) {

    if (shaderError == 1) {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    } else if (shaderError == 2) {
        glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    } else if (shaderError == 3) {
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    } else if (shaderError == 4) {
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    } else {
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!program || !vbo) return;

    float aspect = (float)width / (float)height;
    float f = 1.0f / tanf(0.785398f / 2.0f);
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    float projection[16] = {};
    projection[0] = f / aspect;
    projection[5] = f;
    projection[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    projection[11] = -1.0f;
    projection[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);

    float c = cosf(angle);
    float s = sinf(angle);

    float modelView[16] = {
         c, 0, s, 0,
         0, 1, 0, 0,
        -s, 0, c, 0,
         0, 0,-5, 1
    };

    float mvp[16] = {};

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            for (int k = 0; k < 4; k++) {
                mvp[col * 4 + row] +=
                    projection[k * 4 + row] *
                    modelView[col * 4 + k];
            }
        }
    }

    glUseProgram(program);

    GLint location = glGetUniformLocation(program, "uMVP");
    glUniformMatrix4fv(location, 1, GL_FALSE, mvp);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)(3 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 36);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("Erro ao desenhar o cubo: 0x%x", error);
    }

    angle += 0.01f;
}
