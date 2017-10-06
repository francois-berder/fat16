#ifndef __FAT16_DEBUG_H
#define __FAT16_DEBUG_H

#ifndef LOG
#define LOG printf
#endif

#ifndef NDEBUG
#define FAT16DBG(...) do { LOG(__VA_ARGS__); } while (0)
#elif
#define FAT16DBG(...)
#endif

#endif
