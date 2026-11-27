#ifndef GBCHAWK_H
#define GBCHAWK_H

#ifdef _WIN32 // msvc garbage needs this
#define GBCHawk_EXPORT extern "C" __declspec(dllexport)
#else
#define GBCHawk_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#endif
