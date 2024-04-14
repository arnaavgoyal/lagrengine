# Variables

EXE    := engine.exe
CC     := clang++
SRCDIR := src
INCDIR := include
LIBDIR := lib
OBJDIR := build
LIBS   := user32 gdi32 gl opengl32
FLAGS  := -Wall -g -std=c++20

# Processing

#  Find all sources (without the src dir prefix)
SOURCES     := $(patsubst $(SRCDIR)/%,%,$(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/*/*.cpp))
#  Get all objects from the sources (with the obj dir prefix)
OBJECTS     := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))
#  Get all obj directories that must exist for compilation
OBJDIRSREQ  := $(sort $(dir $(OBJECTS)))
#  Create the library search path and include flags
LIBFLAGS    := -L$(LIBDIR) $(addprefix -l,$(LIBS))
#  Create the full compilation command (.cpp -> .o)
COMPILECMD  := $(CC) $(FLAGS) -I$(INCDIR) -c
#  Create the full build command (.o -> .exe)
BUILDCMD    := $(CC) $(LIBFLAGS)

# Rules

#  Ensures obj dirs exist, then checks project exe
all: $(OBJDIRSREQ) $(EXE)

#  Builds project exe from object files
$(EXE): $(OBJECTS)
	$(BUILDCMD) $^ -o $@

#  Compiles object files from source files
$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(COMPILECMD) $^ -o $@

#  Creates the object file directories
$(OBJDIRSREQ):
	mkdir $@

run: all
	./engine.exe
