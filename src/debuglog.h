#ifndef DEBUGLOG_H_
#define DEBUGLOG_H_

#include <cstdio>
#include <cstdarg>

inline void DBG(const char* fmt, ...) {
    FILE* f = fopen("C:\\iwd2-re\\debug.log", "a");
    if (f) {
        va_list args;
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        va_end(args);
        fprintf(f, "\n");
        fflush(f);
        fclose(f);
    }
}

#endif
