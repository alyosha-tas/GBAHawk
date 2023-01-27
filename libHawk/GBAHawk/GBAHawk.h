#ifndef GBAHAWK_H
#define GBAHAWK_H

#ifdef _WIN32 // msvc garbage needs this
#define GBAHawk_EXPORT extern "C" __declspec(dllexport)
#else
#define GBAHawk_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#endif
