#ifndef ENGINE_H
#define ENGINE_H

#include <syncstream>
#include <windows.h>

#include "utils/log.h"

class WindowCloseRequestedEvent { };
class WindowDestroyStartEvent { };
class WindowDestroyEndEvent { };

void engineInit();

#endif
