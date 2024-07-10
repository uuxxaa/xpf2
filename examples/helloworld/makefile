.PHONY: all clean
TARGET_NAME = helloworld

CPP=clang++

OBJPATH = ./obj
BINPATH = ./bin
XPF_LIBPATH = ../../xpf/lib

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
	main_raster.cpp \
#	main_text_triangulated.cpp \
#	main.cpp \

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

all: clean helloworld

helloworld:
ifeq ($(PLATFORM_OS),MACOS)
	$(CP) "info.plist" "$(BINPATH)/info.plist"
endif
	$(CP) "LittleDreamsDemo.otf" "$(BINPATH)/ld.otf"
	$(CP) "alien.png" "$(BINPATH)/alien.png"
	$(CP) "medal.png" "$(BINPATH)/medal.png"
	$(CPP) $(SOURCES) $(INCLUDES) -o $(TARGET) $(CPPFLAGS) $(LIBS)

#	$(CP) "$(XPF_LIBPATH)/xpf.metallib" "$(BINPATH)/xpf.metallib"

clean:
	$(RM) "$(BINPATH)"
	mkdir "$(BINPATH)"
	@echo Cleaned