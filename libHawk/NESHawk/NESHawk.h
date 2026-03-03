#ifndef NESHAWK_H
#define NESHAWK_H

#ifdef _WIN32 // msvc garbage needs this
#define NESHawk_EXPORT extern "C" __declspec(dllexport)
#else
#define NESHawk_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#endif
