#pragma once

#include <windows.h>
#include <vector>

#include "dm_log.h"
#include "dm_reg.h"

class dm_scan
{
    public:
        dm_scan(dm_log* log, dm_reg* reg);
        ~dm_scan();

        void find_u32(HANDLE proc_handle, UINT32 wanted);
        void replace_u32(HANDLE proc_handle, UINT32 val);
        void reset_u32();

    protected:
        dm_log* log;
        dm_reg* reg;
        std::vector<UINT64> regs;
        
};