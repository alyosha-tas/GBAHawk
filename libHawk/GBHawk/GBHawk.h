#ifndef GBHAWK_H
#define GBHAWK_H

#ifdef _WIN32 // msvc garbage needs this
#define GBHawk_EXPORT extern "C" __declspec(dllexport)
#else
#define GBHawk_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#endif
