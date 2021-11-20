#pragma once

// `PRIu64`-alike for printing size_t
#ifdef __MINGW64__
  // Microsoft’s Visual C Runtime (msvcrt) ships a printf that does not
  // understand "%zu". MSVC itself uses a different printf that does not rely on
  // this, but MinGW uses msvcrt and so cannot handle "%zu".
#define PRISIZE_T "llu"
#elif defined(__MINGW32__)
#define PRISIZE_T "u"
#else
#define PRISIZE_T "zu"
#endif
