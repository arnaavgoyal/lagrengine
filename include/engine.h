#ifndef ENGINE_H
#define ENGINE_H

#include <windows.h>
#include <iostream>

#include "utils/log.h"

class WindowCloseRequestedEvent { };
class WindowDestroyStartEvent { };
class WindowDestroyEndEvent { };

void engineInit();

#endif
