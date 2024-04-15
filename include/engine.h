#ifndef ENGINE_H
#define ENGINE_H

#include <windows.h>

class WindowCloseRequestedEvent { };
class WindowDestroyStartEvent { };
class WindowDestroyEndEvent { };

void engineInit();

#endif
