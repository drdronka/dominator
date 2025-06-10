#pragma once

#include "dm_core.h"

class dm_cmd
{
    public:
        dm_cmd();
        dm_cmd(const char* path);
        ~dm_cmd();

        void console();
        static void arg_strip(char* in, char* out);

    protected:
        dm_core* core;
};