# EGL + Xlib + OpenGL 3.3

Will probably work only on Linux.
Refactored from https://gist.github.com/jfuerth/82b816510bb2cc063c9945baf1093fd9.

Build:
```sh
gcc -Wall -o triangle main.c -lX11 -lEGL -lGL
```

Run:
```sh
./triangle
```

Lesson learned (https://registry.khronos.org/EGL/sdk/docs/man/html/eglBindAPI.xhtml):
```c
// This is important for OpenGL context to work
eglBindAPI(EGL_OPENGL_API)
```