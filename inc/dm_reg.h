#pragma once

#include <windows.h>

#include "dm_log.h"

class dm_reg
{
    public:
        dm_reg(dm_log* log);
        ~dm_reg();

        UINT32 read_u32(HANDLE proc_handle, UINT64 addr);
        void write_u32(HANDLE proc_handle, UINT64 addr, UINT32 val);

    private:
        dm_log* log;
};

