#include "dm_reg.h"

dm_reg::dm_reg(dm_log* log)
{
    this->log = log;
}

dm_reg::~dm_reg()
{
}

UINT32 dm_reg::read_u32(PROCESS_INFORMATION* proc_info, UINT64 addr)
{
    log->info("reg: read_u32: addr [0x%llx]", addr);

    UINT32 reg_mem;
    SIZE_T read_size = 0;
    if(ReadProcessMemory(proc_info->hProcess, (LPCVOID)addr, &reg_mem, 4, &read_size))
    {
        log->info("val read [%d]", reg_mem);
    }
    else
    {
        log->error("read failed: winapi error [%d]", GetLastError());
    }

    return reg_mem;
}

void dm_reg::write_u32(PROCESS_INFORMATION* proc_info, UINT64 addr, UINT32 val)
{
    log->info("reg: write_u32: addr [0x%llx] val [%lu]", addr, val);

    SIZE_T written = 0;
    if(WriteProcessMemory(proc_info->hProcess, (LPVOID)addr, &val, 4, &written))
    {
        log->info("write complete");
    }
    else
    {
        log->error("write failed: winapi error [%d]", GetLastError());
    }
}