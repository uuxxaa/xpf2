.PHONY: all clean
TARGET_NAME = xpflib

OBJPATH = ./obj
BINPATH = ./lib
TOOLSBIN= ../tools/bin

###############################################################################
# Define required environment variables
# Define target platform: PLATFORM_DESKTOP
PLATFORM ?= PLATFORM_DESKTOP
PLATFORM_OS ?= WINDOWS

# Determine PLATFORM_OS in case PLATFORM_DESKTOP selected
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
	# No uname.exe on MinGW!, but OS=Windows_NT on Windows!
	# ifeq ($(UNAME),Msys) -> Windows
	ifeq ($(OS),Windows_NT)
		PLATFORM_OS = WINDOWS
	else
		UNAMEOS = $(shell uname)
		ifeq ($(UNAMEOS),Linux)
			PLATFORM_OS = LINUX
		endif
		ifeq ($(UNAMEOS),FreeBSD)
			PLATFORM_OS = BSD
		endif
		ifeq ($(UNAMEOS),OpenBSD)
			PLATFORM_OS = BSD
		endif
		ifeq ($(UNAMEOS),NetBSD)
			PLATFORM_OS = BSD
		endif
		ifeq ($(UNAMEOS),DragonFly)
			PLATFORM_OS = BSD
		endif
		ifeq ($(UNAMEOS),Darwin)
			PLATFORM_OS = MACOS
			UNAMECPU = $(shell uname -p)
			ifeq ($(UNAMEOS),i386)
				PLATFORM_CPU=INTEL
			else ifeq ($(UNAMEOS),arm)
				PLATFORM_CPU=APPLE
			endif
		endif
		ifndef PLATFORM_SHELL
			PLATFORM_SHELL = sh
		endif
	endif
	endif

###############################################################################
# Target
ifeq ($(PLATFORM_OS),MACOS)
else ifeq ($(PLATFORM_OS),WINDOWS)
else
	$(error unsupported platform)
endif

###############################################################################
# Tools
ifeq ($(PLATFORM_OS),MACOS)
	AR=ar
	RM = rm -rf
	CP = cp
else ifeq ($(PLATFORM_OS),WINDOWS)
	AR=llvm-ar
	RM = IF EXIST "${BINPATH}" rd /s /q
	CP = copy
else
	$(error unsupported platform)
endif

###############################################################################
# Compiler and tools
CPP=clang++
CC=clang
EMBED=$(TOOLSBIN)/embed

###############################################################################
# Flags
CPPFLAGS = \
	-std=c++20 \
	-Wall \
	-Wno-macro-redefined \
	-Wno-unknown-pragmas \
	-Wno-missing-braces \
	-Wno-mismatched-tags \
	-Wno-undefined-var-template \
	-Wno-unused-private-field \
	-Wshadow \
	-Wreserved-id-macro \
	-Wrange-loop-analysis \
	-Wshorten-64-to-32 \
	-Wshadow-uncaptured-local \
	-Wunused-variable \
	-Wimplicit-fallthrough \
	-Wextra \
	-Wno-error=unused-value \
	-Wno-reserved-id-macro \
	-Wno-null-conversion \
	-Wno-deprecated-copy \
	-Wunreachable-code \
	-Werror \
	-fcxx-exceptions \
	-g \

CFLAGS =

ifeq ($(PLATFORM_OS),MACOS)
	CC += -x objective-c

	CFLAGS += \
		-DMACOS \
		-DPLATFORM_APPLE \

	CPPFLAGS += \
		-D_GLFW_COCOA \
		-DPLATFORM_APPLE \

	LIBS += \
		-framework AppKit \
		-framework Metal \
		-framework MetalKit \
		-framework Foundation \
		-framework QuartzCore \
		-framework CoreVideo \
		-framework IOKit \
		-framework Cocoa \
		-framework GLUT \
		-framework OpenGL \

else ifeq ($(PLATFORM_OS),WINDOWS)
	CFLAGS += \
		-D_WIN32 \

	CPPFLAGS += \
		-DPLATFORM_WINDOWS \
		-D_WIN32

	LIBS += -lopengl32 -lgdi32 -lwinmm -lkernel32 -lshell32 -luser32
else
	$(error unsupported platform)
endif

###############################################################################
# Includes
INCLUDES = \
	-I . \
	-I renderer \
	-I windows \
	-I external \
	-I external/glfw/deps \
	-I external/glfw/include \

###############################################################################
# objects & resources
OBJS = \
	$(OBJPATH)/application.o \
	$(OBJPATH)/clipboard_service.o \
	$(OBJPATH)/color.o \
	$(OBJPATH)/common_drawcircle.o \
	$(OBJPATH)/common_drawimage.o \
	$(OBJPATH)/common_drawline.o \
	$(OBJPATH)/common_drawrectangle.o \
	$(OBJPATH)/common_drawtext.o \
	$(OBJPATH)/common_font.o \
	$(OBJPATH)/common_renderer.o \
	$(OBJPATH)/event.o \
	$(OBJPATH)/glad.o \
	$(OBJPATH)/glfw.o \
	$(OBJPATH)/image.o \
	$(OBJPATH)/input_service.o \
	$(OBJPATH)/m3_t.o \
	$(OBJPATH)/m4_t.o \
	$(OBJPATH)/null_renderer.o \
	$(OBJPATH)/opengl_buffer.o \
	$(OBJPATH)/opengl_renderer.o \
	$(OBJPATH)/opengl_shader.o \
	$(OBJPATH)/opengl_texture.o \
	$(OBJPATH)/rectangle.o \
	$(OBJPATH)/render_batch_builder.o \
	$(OBJPATH)/stringex.o \
	$(OBJPATH)/ttf2mesh.o \
	$(OBJPATH)/time.o \
	$(OBJPATH)/v2_t.o \
	$(OBJPATH)/v3_t.o \
	$(OBJPATH)/v4_t.o \
	$(OBJPATH)/window.o \
	$(OBJPATH)/xpf_resources.o \
	$(OBJPATH)/FormattedText.o \
	$(OBJPATH)/Glyph.o \

RESOURCES = \
	-t opengl_shader_vert ./renderer/opengl/glshader.vert \
	-t opengl_shader_frag ./renderer/opengl/glshader.frag \

ifeq ($(PLATFORM_OS),MACOS)
	OBJS += \
		$(OBJPATH)/convert_apple.o \
		$(OBJPATH)/metal_renderer.o \
		$(OBJPATH)/metal_buffer.o \
		$(OBJPATH)/metal_texture.o \

	RESOURCES += \
		-t xpf_metal ./renderer/metal/xpf.metal \
#		-b xpf_metallib $(OBJPATH)/xpf.metallib \

else ifeq ($(PLATFORM_OS),WINDOWS)
	OBJS += \
		$(OBJPATH)/directx11_renderer.o \

	RESOURCES += \
		-t dx11_shaders ./renderer/directx11/dx_shaders.hlsl \

else
	$(error unsupported platform)
endif

###############################################################################
# xpflib
all: xpfresources xpflib

xpflib: $(OBJS)
	$(AR) rcs $(BINPATH)/$(TARGET_NAME).a $(OBJS)

xpfresources: # $(OBJPATH)/xpf.metallib
	$(EMBED) -ns xpf::resources -of $(OBJPATH) -o xpf_resources $(RESOURCES)
	$(CPP) -c $(OBJPATH)/xpf_resources.cpp $(CPPFLAGS) $(INCLUDES) -o $(OBJPATH)/xpf_resources.o

###############################################################################
# External sources
$(OBJPATH)/glfw.o : external/glfw.c
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/glad.o : external/glad/glad.c
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/ttf2mesh.o : external/ttf2mesh/ttf2mesh.c
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -o $@

###############################################################################
# window & app
$(OBJPATH)/application.o : windows/Application.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/window.o : windows/Window.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/clipboard_service.o : windows/ClipboardService.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/input_service.o : windows/InputService.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

###############################################################################
# directx11 renderer
$(OBJPATH)/directx11_renderer.o : renderer/directx11/DirectX_Renderer.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@ -Wno-conversion -Wno-class-conversion

###############################################################################
# metal renderer
$(OBJPATH)/metal_renderer.o : renderer/metal/Metal_Renderer.mm
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/metal_buffer.o : renderer/metal/Metal_Buffer.mm
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/metal_texture.o : renderer/metal/Metal_Texture.mm
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/xpf.air : renderer/metal/xpf.metal
	xcrun -sdk macosx metal -c $< -o $@

$(OBJPATH)/xpf.metallib : $(OBJPATH)/xpf.air
	xcrun -sdk macosx metallib $< -o $(OBJPATH)/xpf.metallib

###############################################################################
# opengl renderer
$(OBJPATH)/opengl_renderer.o : renderer/opengl/OpenGL_Renderer.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/opengl_buffer.o : renderer/opengl/OpenGL_Buffer.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/opengl_texture.o : renderer/opengl/OpenGL_Texture.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/opengl_shader.o : renderer/opengl/Shader.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

###############################################################################
# null renderer
$(OBJPATH)/null_renderer.o : renderer/null/Null_Renderer.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

###############################################################################
# common renderer
$(OBJPATH)/common_renderer.o : renderer/common/Common_Renderer.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/common_drawcircle.o : renderer/common/DrawCircle.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/common_drawimage.o : renderer/common/DrawImage.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/common_drawline.o : renderer/common/DrawLine.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/common_drawrectangle.o : renderer/common/DrawRectangle.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/common_drawtext.o : renderer/common/DrawText.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/render_batch_builder.o : renderer/common/RenderBatchBuilder.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/FormattedText.o : renderer/common/FormattedText.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

###############################################################################
# core
$(OBJPATH)/color.o : core/Color.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/convert_apple.o : core/Convert_Apple.mm
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/event.o : core/Event.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/rectangle.o : core/Rectangle.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/image.o : core/Image.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/stringex.o : core/stringex.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/time.o : core/Time.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

###############################################################################
# font
$(OBJPATH)/common_font.o : renderer/common/Font.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/Glyph.o : renderer/common/Glyph.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

###############################################################################
# math
$(OBJPATH)/m3_t.o : math/m3_t.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/m4_t.o : math/m4_t.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/v2_t.o : math/v2_t.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/v3_t.o : math/v3_t.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

$(OBJPATH)/v4_t.o : math/v4_t.cpp
	$(CPP) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

###############################################################################
# xpflib.a
$(OBJPATH)/xpflib.a : $(OBJS)
	$(AR) rcs $(OBJPATH)/xpflib.a $(OBJS)

clean:
	$(RM) "$(OBJPATH)"
	mkdir "$(OBJPATH)"
	$(RM) "$(BINPATH)"
	mkdir "$(BINPATH)"
	@echo Cleaned