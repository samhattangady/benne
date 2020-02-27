CC := g++

CFLAGS := -lm -lglfw -lGL -lGLEW
# For debugging...
# CFLAGS += -fsanitize-address -g
HDRS := imgui/imconfig.h  imgui/imgui.h  imgui/imgui_impl_glfw.h  imgui/imgui_impl_opengl3.h  imgui/imgui_internal.h  imgui/imstb_rectpack.h  imgui/imstb_textedit.h  imgui/imstb_truetype.h
PCH += imgui_pch.h
HDRS += shaders.h distance_fields.h benne_string.h


SRCS += ./imgui/imgui_impl_glfw.cpp ./imgui/imgui_impl_opengl3.cpp ./imgui/imgui.cpp ./imgui/imgui_demo.cpp ./imgui/imgui_draw.cpp ./imgui/imgui_widgets.cpp
SRCS += shaders.cpp distance_fields.cpp benne_string.cpp
SRCS += benne.cpp
OBJS := $(SRCS:.c=.o)
EXEC = benne

all: $(EXEC)
$(EXEC): $(OBJS) $(PCH) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS)
