#pragma once

#include <cstdlib>
#include <cstdarg>
#include <cstring>

extern int log_level;

static void emu_set_debug(int level) { log_level = level; }
static void emu_log_printf(const char* fmt, ...);

enum {
    emu_log_none,
    emu_log_error,
    emu_log_info,
    emu_log_debug,
    emu_log_trace
};

#define emu_errorf(...) if(log_level >= emu_log_error) emu_log_printf(__VA_ARGS__)
#define emu_infof(...) if(log_level >= emu_log_info) emu_log_printf(__VA_ARGS__)
#define emu_debugf(...) if(log_level >= emu_log_debug) emu_log_printf(__VA_ARGS__)
#define emu_tracef(...) if(log_level >= emu_log_trace) emu_log_printf(__VA_ARGS__)

static void emu_log_printf(const char* fmt, ...)
{
    int len;
    char buf[128];
    char *pbuf = buf;
    char *hbuf = NULL;
    va_list ap;
    va_start(ap, fmt);
    len = vsnprintf(pbuf, sizeof(buf)-1, fmt, ap);
    pbuf[sizeof(buf)-1] = '\0';
    va_end(ap);
    if (len >= sizeof(buf)) {
        pbuf = hbuf = (char*)malloc(len + 1);
        va_start(ap, fmt);
        len = vsnprintf(pbuf, len + 1, fmt, ap);
        pbuf[len] = '\0';
        va_end(ap);
    }
    fwrite(pbuf, 1, len, stderr);
    if (hbuf) free(hbuf);
}
