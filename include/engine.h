#ifndef ENGINE_H
#define ENGINE_H

#include "graphics/graphics.h"

class WindowCloseRequestedEvent { };
class WindowDestroyStartEvent { };
class WindowDestroyEndEvent { };

class EngineTickEvent { };

int engineInit(OpenGLWrapper graphics);

#endif
