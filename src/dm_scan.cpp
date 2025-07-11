#include <windows.h>

#include "dm_scan.h"
#include "dm_reg.h"
#include "dm_log.h"

dm_scan::dm_scan(dm_log* log, dm_reg* reg)
{
    this->log = log;
    this->reg = reg;
}

dm_scan::~dm_scan()
{
}

void dm_scan::find_u32(HANDLE proc_handle, UINT32 wanted)
{
    log->info("scan: find_u32: val [%lu]", wanted);

    MEMORY_BASIC_INFORMATION mem_info;
    LPVOID base_addr = NULL;
    UINT32 reg_num = 0;
    UINT32 size_commited = 0;

    if(regs.size() == 0)
    {
        while(VirtualQueryEx(proc_handle, base_addr, &mem_info, sizeof(mem_info)))
        {
            if(mem_info.State == MEM_COMMIT)
            {
                UINT32* reg_mem = (UINT32*)malloc(mem_info.RegionSize);
                SIZE_T read_size = 0;

                if(!ReadProcessMemory(proc_handle, mem_info.BaseAddress, reg_mem, mem_info.RegionSize, &read_size))
                {
                    log->error(
                        "failed to read region [%lu] addr [0x%llx] size [0x%lx]: winapi error [%d]", 
                        reg_num, mem_info.BaseAddress, mem_info.RegionSize, GetLastError());
                }
                else
                {
                    for(UINT32 n = 0; n < read_size / 4; n++)
                    {
                        if(reg_mem[n] == wanted)
                        {
                            UINT64 wanted_addr = (UINT64)(mem_info.BaseAddress + (n * 4));
                            regs.push_back(wanted_addr);
                            log->info("val [%lu] found at [0x%llx]", wanted, wanted_addr);
                        }
                    }
                }

                free(reg_mem);

                size_commited += mem_info.RegionSize;
                reg_num++;
            }
            
            base_addr = (LPVOID)((DWORD_PTR)mem_info.BaseAddress + mem_info.RegionSize);
        }
        log->info("memory scanned [%d] KB", size_commited / 1024);
    }
    else
    {
        for(size_t n = 0; n < regs.size();) 
        {
            UINT32 reg;
            UINT64 reg_addr = regs[n];
            SIZE_T read_size = 0;

            if(!ReadProcessMemory(proc_handle, (PVOID*)reg_addr, &reg, 4, &read_size))
            {
                log->error("ReadProcessMemory failed: winapi error [%d]", GetLastError());
                return;
            }

            if(reg != wanted)
            {
                regs.erase(regs.begin() + n);
                log->info("addr [%llx] removed", reg_addr);
            }
            else
            {
                n++;
            }
        }

        for(size_t n = 0; n < regs.size(); n++)
        {
            log->info("addr [%llx] persisted", regs[n]);
        }

        if(regs.size() == 0)
        {
            log->info("addr list empty");
        }
    }
}

void dm_scan::replace_u32(HANDLE proc_handle, UINT32 val)
{
    log->info("scan: replace_u32: val [%lu]", val);

    if(regs.size() != 0)
    {
        for(size_t n = 0; n < regs.size(); n++)
        {
            reg->write_u32(proc_handle, regs[n], val);
        }
    }
    else
    {
        log->error("addr list empty");
    }
}

void dm_scan::reset_u32()
{
    log->info("scan: reset_u32");
    regs.clear();
}