#ifndef GENERAL_SYSTEM_EVENTS
#define GENERAL_SYSTEM_EVENTS

#include "DelegateHandlers/Delegate.h"

DEFINE_DELEGATE_TwoParam(WindowResizeDelegate, WindowResizePayload, int, width, int, height);
static WindowResizeDelegate WindowResizeEvent;



#endif