#ifndef WEDGE3_MAIN_H
#define WEDGE3_MAIN_H

#include <shim3/shim3.h>

#ifdef _WIN32
#ifdef WEDGE3_STATIC
#define WEDGE3_EXPORT
#else
#ifdef WEDGE3_BUILD
#define WEDGE3_EXPORT __declspec(dllexport)
#else
#define WEDGE3_EXPORT __declspec(dllimport)
#endif
#endif
#else
#define WEDGE3_EXPORT
#endif

namespace wedge {

bool WEDGE3_EXPORT start(util::Size<int> base_screen_size, util::Size<int> window_size);
bool WEDGE3_EXPORT go();
void WEDGE3_EXPORT end();
void WEDGE3_EXPORT handle_event(TGUI_Event *event);

}

#endif // NOOSKEWL_EDGE_MAIN_H
