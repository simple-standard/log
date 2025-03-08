// SPDX-FileCopyrightText: 2025 Deo Hayer <deohayer@mail.com>
// SPDX-License-Identifier: MIT

#ifndef SIMPLE_STANDARD_LOG_HPP
#define SIMPLE_STANDARD_LOG_HPP

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

static const int LOG_LEVEL_QUIET = 0;
static const int LOG_LEVEL_ERROR = 1;
static const int LOG_LEVEL_PRINT = 2;
static const int LOG_LEVEL_DEBUG = 3;
static const int LOG_LEVEL_MIN = LOG_LEVEL_QUIET;
static const int LOG_LEVEL_MAX = LOG_LEVEL_DEBUG;

extern int log_l;

#define log_e(...) log_x(__FILE__, __func__, __LINE__, LOG_LEVEL_ERROR, __VA_ARGS__)

#define log_p(...) log_x(__FILE__, __func__, __LINE__, LOG_LEVEL_PRINT, __VA_ARGS__)

#define log_d(...) log_x(__FILE__, __func__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)

static inline void log_x(
    const char* file,
    const char* func,
    int line,
    int level,
    const char* format,
    ...)
{
    static const char levels[] = " EPD";
    // Handle level.
    if (log_l < level) return;
    level = level > LOG_LEVEL_MAX ? LOG_LEVEL_MAX : level;
    // Generate prefix: "YYYY-MM-DD HH:MM:SS.UUUUUU PPPPPPP TTTTTTT [L] ".
    char prefix[64];
    // Prefix - date and time.
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME, &timestamp);
    strftime(prefix, sizeof(prefix), "%Y-%m-%d %H:%M:%S.", localtime(&timestamp.tv_sec));
    // Prefix - microseconds, pid, tid, label.
    int usec = (int)(timestamp.tv_nsec / 1000);
    int pid = (int)getpid();
    int tid = (int)syscall(SYS_gettid);
    sprintf(prefix + 20, "%6d %7d:%-7d [%c]", usec, pid, tid, levels[level]);
    // Generate the message to guarantee that there is at least one newline,
    // and the last newline is immediately followed by the null character.
    va_list args;
    va_start(args, format);
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, format, args_copy) + 2;
    va_end(args_copy);
    char* message = (char*)malloc(size);
    vsprintf(message, format, args);
    message[size - 2] = '\n';
    message[size - 1] = '\0';
    // Reduce file path to file name.
    const char* name = strrchr(file, '/');
    file = name ? name + 1 : file;
    // Print message line by line, adding a prefix before each newline.
    char* current = message;
    char* newline = strchr(message, '\n');
    while (newline)
    {
        *newline = '\0';
        printf("%s %s::%s(%i): %s\n", prefix, file, func, line, current);
        current = newline + 1;
        newline = strchr(current, '\n');
    }
    free(message);
    va_end(args);
    return;
}

template <typename T>
std::string strc(const T& v)
{
    return (std::string)v;
}

#define cstr(v) (strc(v).c_str())

#endif
