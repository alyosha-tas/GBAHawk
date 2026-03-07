#ifndef SNESHAWK_H
#define SNESHAWK_H

#ifdef _WIN32 // msvc garbage needs this
#define SNESHawk_EXPORT extern "C" __declspec(dllexport)
#else
#define SNESHawk_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#endif
