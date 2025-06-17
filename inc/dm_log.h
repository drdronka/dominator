#pragma once

enum class dm_log_level 
{
    none = 0,
    error = 1,
    info = 2,
    debug = 3
};

enum class dm_log_format 
{
    clean = 0,
    with_prefix = 1
};

class dm_log
{
    public:
        dm_log();
        dm_log(dm_log_level level, dm_log_format format);
        
        void set_level(dm_log_level level);
        void set_format(dm_log_format format);
        dm_log_level get_level();
        dm_log_format get_format();

        // format, new line added
        void debug(const char* format, ...);
        void info(const char* format, ...);
        void error(const char* format, ...);

        // no format, no new line
        void debug_naked(const char* format, ...);
        void info_naked(const char* format, ...);
        void error_naked(const char* format, ...);

    protected:
        dm_log_level log_level;
        dm_log_format log_format;
};