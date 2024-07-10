.PHONY: all clean
TARGET_NAME = controls

OBJPATH = ./obj
BINPATH = ./bin
XPF_LIBPATH = ../../xpf/lib
TOOLSBIN= ../../tools/bin

CPP=clang++
EMBED=$(TOOLSBIN)/embed

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
	TARGET = "./bin/${TARGET_NAME}"
else ifeq ($(PLATFORM_OS),WINDOWS)
	TARGET = ".\bin\${TARGET_NAME}.exe"
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
# Includes
INCLUDES = \
	-I ../.. \
	-I ../../xpf \
	-I ../../xpf/external \
	-I ../../xpf/external/glfw/include \

LIBS = \
	$(XPF_LIBPATH)/xpflib.a \
#	../../jsonlite/lib/jsonlitelib.a \

SOURCES = \
	main.cpp \
	MainWindow.cpp \
	$(OBJPATH)/resources.cpp \

CPPFLAGS = \
	-std=c++20 \
	-Werror \
	-fcxx-exceptions \
	-g

ifeq ($(PLATFORM_OS),MACOS)
	CC += -x objective-c
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

#-arch arm64

else ifeq ($(PLATFORM_OS),WINDOWS)
	CC += -D_WIN32
	CPPFLAGS += \
		-DPLATFORM_WINDOWS \
		-D_WIN32

	LIBS += -lopengl32 -lgdi32 -lwinmm -lkernel32 -lshell32 -luser32 -ld3dcompiler -lDXGI -lD3D11
else
	$(error unsupported platform)
endif

RESOURCES = \
	-b folder_closed_png ./resources/folder_closed.png \
	-b folder_open_png ./resources/folder_open.png \
	-b test_pattern_01_png ./resources/test_pattern_01.png \

all: clean example_resources controls

example_resources:
	$(EMBED) -ns resources -of $(OBJPATH) -o resources $(RESOURCES)
	$(CPP) -c $(OBJPATH)/resources.cpp $(CPPFLAGS) $(INCLUDES) -o $(OBJPATH)/resources.o

controls:
ifeq ($(PLATFORM_OS),MACOS)
	$(CP) "info.plist" "$(BINPATH)/info.plist"
endif
	$(CP) "../../xpf/external/fonts/segoeicons.ttf" "$(BINPATH)/segoeicons.ttf"
	$(CPP) $(SOURCES) $(INCLUDES) -o $(TARGET) $(CPPFLAGS) $(LIBS)

clean:
	$(RM) "$(OBJPATH)"
	mkdir "$(OBJPATH)"
	$(RM) "$(BINPATH)"
	mkdir "$(BINPATH)"
	@echo Cleaned