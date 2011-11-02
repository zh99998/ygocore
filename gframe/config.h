#define _IRR_STATIC_LIB_

#include <irrlicht.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "CGUITTFont.h"
#include "CGUIImageButton.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "timer.h"
#include "mutex.h"
#include "signal.h"
#include "thread.h"
#include "../ocgcore/ocgapi.h"
#include "../ocgcore/card.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
