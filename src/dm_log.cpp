#include <stdio.h>
#include <stdarg.h>

#include "dm_log.h"

#define DEFAULT_LOG_LEVEL  dm_log_level::debug
#define DEFAULT_LOG_FORMAT dm_log_format::clean
#define LOG_PREFIX        "D> "

dm_log::dm_log()
{
    log_level = DEFAULT_LOG_LEVEL;
    log_format = DEFAULT_LOG_FORMAT;
}

dm_log::dm_log(dm_log_level level, dm_log_format format)
{
    log_level = level;
    log_format = format;
}

dm_log::~dm_log()
{
}

void dm_log::set_level(dm_log_level level)
{
    printf("set log level [%d]", level);
    log_level = level;
}

dm_log_level dm_log::get_level()
{
    return log_level;
}

void dm_log::set_format(dm_log_format format)
{
    info("set log format [%d]", format);
    log_format = format;
}

dm_log_format dm_log::get_format()
{
    return log_format;
}

void dm_log::debug(const char* format, ...) 
{
    if(log_level >= dm_log_level::debug)
    {
        va_list args;
        va_start(args, format);
        if(log_format == dm_log_format::with_prefix)
        {
            printf(LOG_PREFIX);
        }
        vprintf(format, args);
        printf("\n");
        va_end(args);
    }
}

void dm_log::debug_naked(const char* format, ...) 
{
    if(log_level >= dm_log_level::error)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

void dm_log::info(const char* format, ...) 
{
    if(log_level >= dm_log_level::info)
    {
        va_list args;
        va_start(args, format);
        if(log_format == dm_log_format::with_prefix)
        {
            printf(LOG_PREFIX);
        }
        vprintf(format, args);
        printf("\n");
        va_end(args);
    }
}

void dm_log::info_naked(const char* format, ...) 
{
    if(log_level >= dm_log_level::info)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

void dm_log::error(const char* format, ...) 
{
    if(log_level >= dm_log_level::info)
    {    
        va_list args;
        va_start(args, format);
        if(log_format == dm_log_format::with_prefix)
        {
            printf(LOG_PREFIX);
        }
        fprintf(stderr, "[ERR]: ");
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
        va_end(args);
    }
}

void dm_log::error_naked(const char* format, ...) 
{
    if(log_level >= dm_log_level::info)
    {    
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
    }
}