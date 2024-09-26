#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>

#define PANIC(msg) do { \
    std::cerr << "panic occurred at " << __FILE__ << ":" << __LINE__ << " - " << msg << std::endl; \
    std::exit(1); \
} while (0)

#define PANICF(fmt, ...) do { \
    char buffer[256]; \
    std::snprintf(buffer, sizeof(buffer), fmt, __VA_ARGS__); \
    std::cerr << "panic occurred at " << __FILE__ << ":" << __LINE__ << " - " << buffer << std::endl; \
    std::exit(1); \
} while (0)
