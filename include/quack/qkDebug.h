#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#define QK_ZONE_SCOPED ZoneScoped
#define QK_ZONE_NAMED(name) ZoneScopedN(name)
#define QK_FRAME_MARK FrameMark
#else
#define ZONE_SCOPED
#define FRAME_MARK
#endif