//
// Created by HEADS on 2021/2/20.
//

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

#define LOG_DEBUG_TYPE  0
#define LOG_MSG_TYPE    1
#define LOG_WARN_TYPE   2
#define LOG_ERR_TYPE    3

void lamp_log(int severity, const char *msg);

void lamp_logx(int severity, const char *errstr, const char *fmt, va_list ap);

void lamp_msgx(const char *fmt, ...);

void lamp_debugx(const char *fmt, ...);

#define LOG_MSG(msg) lamp_log(LOG_MSG_TYPE, msg)
#define LOG_ERROR(msg) lamp_log(LOG_ERR_TYPE, msg)

#endif //LOG_H
