.PHONY: all clean
TARGET_NAME = embed

CPP=clang++

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

OBJPATH = ./obj
BINPATH = ../../bin

###############################################################################
# Target
ifeq ($(PLATFORM_OS),MACOS)
	TARGET = "$(BINPATH)/${TARGET_NAME}"
else ifeq ($(PLATFORM_OS),WINDOWS)
	TARGET = "$(BINPATH)\${TARGET_NAME}.exe"
else
	$(error unsupported platform)
endif

###############################################################################
# Tools
ifeq ($(PLATFORM_OS),MACOS)
	AR = @ar
	RM = @rm -rf
	CP = @cp
	DEL = @rm -f
	IGNORE_ERROR =
	MKDIR_BIN = @mkdir -p "$(BINPATH)"

else ifeq ($(PLATFORM_OS),WINDOWS)
	AR=@llvm-ar
	RM = @IF EXIST "${BINPATH}" rd /s /q
	CP = @copy
	DEL = @del
	IGNORE_ERROR = 2>nul
	MKDIR_BIN = @IF NOT EXIST "$(BINPATH)" mkdir "$(BINPATH)"

else
	$(error unsupported platform)
endif

###############################################################################
# Includes
INCLUDES = \

LIBS = \

SOURCES = \
	main.cpp \

CPPFLAGS = \
	-std=c++20 \
	-Werror \
	-fcxx-exceptions

ifeq ($(PLATFORM_OS),MACOS)
	CC += -x objective-c
	ifeq ($(PLATFORM_CPU),ARM)
		CPPFLAGS += -arch arm64
	endif

else ifeq ($(PLATFORM_OS),WINDOWS)
	CC += -D_WIN32
	CPPFLAGS += -D_WIN32

else
	$(error unsupported platform)
endif

all: clean tool

tool:
	$(MKDIR_BIN)
	$(DEL) $(TARGET) $(IGNORE_ERROR)
	$(CPP) $(SOURCES) $(INCLUDES) -o $(TARGET) $(CPPFLAGS) $(LIBS)

clean:
	$(DEL) $(TARGET) $(IGNORE_ERROR)
	@echo Cleaned
