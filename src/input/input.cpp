#include "input/input.h"

bool KeyInput::s = 0;
decltype(KeyInput::keystates) KeyInput::keystates;
InputContext KeyInput::context;
std::mutex KeyInput::context_sync;
