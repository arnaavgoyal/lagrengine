CC = clang
LIBS = -luser32

build/engine.exe: build/win32_main.o
	$(CC) $^ $(LIBS) -o $@

build/win32_main.o: src/win32_main.cpp
	$(CC) -c $^ -o $@
