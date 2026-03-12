#pragma once

#define ENG_CONCAT_IMPL(x, y) x##y
#define ENG_CONCAT(x, y) ENG_CONCAT_IMPL(x, y)

#if defined(ENG_ENABLE_TRACY)
#include <tracy/Tracy.hpp>

#define ENG_PROFILE_ZONE() ZoneNamed(ENG_CONCAT(__tracy_zone_, __LINE__), true)

#define ENG_PROFILE_ZONE_N(name) ZoneNamedN(ENG_CONCAT(__tracy_zone_, __LINE__), name, true)

#define ENG_PROFILE_ZONE_C(name, color) \
  ZoneNamedNC(ENG_CONCAT(__tracy_zone_, __LINE__), name, color, true)

#define ENG_PROFILE_FRAME() FrameMark
#else
#define ENG_PROFILE_ZONE()
#define ENG_PROFILE_ZONE_N(name)
#define ENG_PROFILE_ZONE_C(name, color)
#define ENG_PROFILE_FRAME()
#endif