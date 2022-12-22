
#ifndef SNEX_XPLAT_H
#define SNEX_XPLAT_H

#if (__linux__ || __APPLE__)
#  define XPIPC_UNIX 1
#elif _WIN32
#  define XPIPC_WINDOWS 1
#endif

#endif //SNEX_XPLAT_H
