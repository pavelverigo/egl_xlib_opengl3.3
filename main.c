#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
 
// HACK: Linux only
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>

#include <EGL/egl.h>

const int WIDTH = 640, HEIGHT = 480;

// a.k.a. VSYNC 0/1
const int SWAP_INTERVAL = 0;

static void print_egl_details(EGLDisplay egl_display, EGLConfig egl_conf) {
    printf("EGL Client APIs: %s\n", eglQueryString(egl_display, EGL_CLIENT_APIS));
    printf("EGL Vendor: %s\n", eglQueryString(egl_display, EGL_VENDOR));
    printf("EGL Version: %s\n", eglQueryString(egl_display, EGL_VERSION));
    printf("EGL Extensions: %s\n", eglQueryString(egl_display, EGL_EXTENSIONS));
   
    int i = -1;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_CONFIG_ID, &i);
    printf("EGL_CONFIG_ID = %d\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_RED_SIZE, &i);
    printf("EGL_RED_SIZE = %d\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_GREEN_SIZE, &i);
    printf("EGL_GREEN_SIZE = %d\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_BLUE_SIZE, &i);
    printf("EGL_BLUE_SIZE = %d\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_ALPHA_SIZE, &i);
    printf("EGL_ALPHA_SIZE = %d\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_DEPTH_SIZE, &i);
    printf("EGL_DEPTH_SIZE = %d\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_LEVEL, &i);
    printf("EGL_LEVEL = %d\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_NATIVE_RENDERABLE, &i);
    printf("EGL_NATIVE_RENDERABLE = %s\n", i ? "EGL_TRUE" : "EGL_FALSE");
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_NATIVE_VISUAL_TYPE, &i);
    printf("EGL_NATIVE_VISUAL_TYPE = %d\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_RENDERABLE_TYPE, &i);
    printf("EGL_RENDERABLE_TYPE = 0x%04x\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_SURFACE_TYPE, &i);
    printf("EGL_SURFACE_TYPE = 0x%04x\n", i);
    
    i = 0;
    eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_TYPE, &i);
    if (i == EGL_TRANSPARENT_RGB) {
        printf("EGL_TRANSPARENT_TYPE = EGL_TRANSPARENT_RGB\n");
        
        i = 0;
        eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_RED_VALUE, &i);
        printf("EGL_TRANSPARENT_RED = 0x%02x\n", i);
        
        i = 0;
        eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_GREEN_VALUE, &i);
        printf("EGL_TRANSPARENT_GREEN = 0x%02x\n", i);
        
        i = 0;
        eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_BLUE_VALUE, &i);
        printf("EGL_TRANSPARENT_BLUE = 0x%02x\n", i);
    } else {
        printf("EGL_TRANSPARENT_TYPE = EGL_NONE\n");
    }
}

int main(int argc, char **argv) {
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        printf("Cannot connect to X server\n");
        exit(1);
    }

    Window root = DefaultRootWindow(display);

    XSetWindowAttributes attributes;
    attributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask;

    Window window =
        XCreateWindow(display, root, 0, 0, WIDTH, HEIGHT, 0,
                      CopyFromParent, InputOutput, CopyFromParent,
                      CWEventMask, &attributes);
    
    if (!window) {
        printf("Unable to create window\n");
        exit(1);
    }

    XMapWindow(display, window);
    XStoreName(display, window, "EGL + Xlib + OpenGL 3.3");

    EGLDisplay egl_display = eglGetDisplay((EGLNativeDisplayType) display);
    if (egl_display == EGL_NO_DISPLAY) {
        printf("Cannot get EGL display (eglError: %d)\n", eglGetError());
        exit(1);
    }

    EGLint egl_version_major, egl_version_minor;
    if (!eglInitialize(egl_display, &egl_version_major, &egl_version_minor)) {
        printf("Cannot initialize EGL display (eglError: %d)\n", eglGetError());
        exit(1);
    }

    printf("Initialized EGL version %d.%d\n", egl_version_major, egl_version_minor);

    EGLint config_attrib_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_CONFIG_CAVEAT, EGL_NONE,
        EGL_NONE
    };

    EGLConfig egl_config;
    EGLint num_config;
    if (!eglChooseConfig(egl_display, config_attrib_list, &egl_config, 1, &num_config)) {
        printf("Failed to choose config (eglError: %d)\n", eglGetError());
        exit(1);
    }

    if (num_config != 1) {
        printf("Didn't get exactly one config, but %d\n", num_config);
        exit(1);
    }

    EGLSurface egl_surface = eglCreateWindowSurface(egl_display, egl_config, window, NULL);
    if (egl_surface == EGL_NO_SURFACE) {
        printf("Unable to create EGL surface (eglError: %d)\n", eglGetError());
        exit(1);
    }

    // IMPORTANT
    if (!eglBindAPI(EGL_OPENGL_API)) {
        printf("Cannot make EGL context curren (eglError: %d)\n", eglGetError());
        exit(1);
    }

    EGLint context_attrib_list[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_NONE
    };
    
    EGLContext egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attrib_list);
    if (egl_context == EGL_NO_CONTEXT) {
        printf("Unable to create EGL context (eglError: %d)\n", eglGetError());
        exit(1);
    }

    if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
        printf("Cannot make EGL context current (eglError: %d)\n", eglGetError());
        exit(1);
    }

    print_egl_details(egl_display, egl_config);
  
    if (!eglSwapInterval(egl_display, SWAP_INTERVAL)) {
        printf("eglSwapInterval failed: %d\n", eglGetError());
        exit(1);
    }

    // Well just to be sure. Works without this line
    glViewport(0, 0, WIDTH, HEIGHT);
    
    // Create, compile, and link shaders
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 vertexColor;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(aPos, 1.0);\n"
        "    vertexColor = aColor;\n"
        "}\n";

    const char* fragmentShaderSource = "#version 330 core\n"
        "in vec3 vertexColor;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(vertexColor, 1.0);\n"
        "}\n";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLfloat vertices[] = {
        // positions          // colors
        -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, // red
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, // green
         0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f  // blue
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    Atom WM_PROTOCOLS = XInternAtom(display, "WM_PROTOCOLS", False);
    Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);

    while (1) {
        XEvent event;
        while (XPending(display)) {
            XNextEvent(display, &event);
            if (event.type == Expose) {
                glClear(GL_COLOR_BUFFER_BIT);

                glUseProgram(shaderProgram);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 3);

                eglSwapBuffers(egl_display, egl_surface);
            } else if (event.type == KeyPress) {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                if (key == XK_Escape) {
                    printf("ESC is pressed\n");
                    exit(1);
                }
            } else if (event.type == ClientMessage) {
                if (event.xclient.message_type == WM_PROTOCOLS && event.xclient.data.l[0] == WM_DELETE_WINDOW) {
                    printf("WM_DELETE_WINDOW\n");
                    exit(1);
                }
            } else if (event.type == ConfigureNotify) {
                XConfigureEvent xce = event.xconfigure;
                glViewport(0, 0, xce.width, xce.height);
            }
        }
    }

    return 0;
}