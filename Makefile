CC := g++

# CFLAGS := -lglfw -lGL -lGLEW
CFLAGS := -lm -lglfw -lGL -lGLEW -fsanitize=address
HDRS := imgui/imconfig.h  imgui/imgui.h  imgui/imgui_impl_glfw.h  imgui/imgui_impl_opengl3.h  imgui/imgui_internal.h  imgui/imstb_rectpack.h  imgui/imstb_textedit.h  imgui/imstb_truetype.h
HDRS += shaders.h distance_fields.h

SRCS += ./imgui/imgui_impl_glfw.cpp ./imgui/imgui_impl_opengl3.cpp
SRCS += ./imgui/imgui.cpp ./imgui/imgui_demo.cpp ./imgui/imgui_draw.cpp ./imgui/imgui_widgets.cpp
SRCS += shaders.cpp distance_fields.cpp
SRCS += benne.cpp
OBJS := $(SRCS:.c=.o)
EXEC = benne

all: $(EXEC)
$(EXEC): $(OBJS) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS)
