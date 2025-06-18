#include <windows.h>
#include <stdio.h>
#include <thread>
#include <atomic>

#include "dm_core.h"

dm_core::dm_core(dm_log* log)
{
    this->log = log;
    cmd_list = new dm_cmd_list(log);
    cmd_thread = NULL;
    cmd_thread_id = 0;
}

dm_core::~dm_core()
{
    stop_cmd_loop();
    delete cmd_list;
}

void dm_core::add_cmd(dm_cmd* cmd)
{
    cmd_list->add(cmd);
}

void dm_core::start_cmd_loop()
{
    if(!cmd_thread)
    {
        cmd_thread = CreateThread(
            nullptr,              // Default security attributes
            0,                    // Default stack size
            dm_core_cmd_loop,     // Thread function name
            this,                 // Parameter passed to thread
            0,                    // Default creation flags
            &cmd_thread_id);      // Thread ID
    }
    else
    {
        log->error("cmd loop already running");
    }
}

void dm_core::stop_cmd_loop()
{
    if(cmd_thread)
    {
        add_cmd((dm_cmd*)new dm_cmd_exit_cmd_loop());

        DWORD result;
        do
        {
            result = WaitForSingleObject(cmd_thread, 0);
        }
        while(result != WAIT_OBJECT_0);

        cmd_thread = NULL;
        cmd_thread_id = 0;

        log->info("cmd loop stopped");
    }
}

static DWORD WINAPI dm_core_cmd_loop(LPVOID ref)
{
    dm_core* core = reinterpret_cast<dm_core*>(ref);
    core->cmd_loop();
    return 0;
}

void dm_core::cmd_loop()
{
    log->info("cmd loop started");

    bool loop_exit = false;
    while(!loop_exit)
    {
        dm_cmd* cmd = cmd_list->get();
        if(cmd != nullptr)
        {
            switch(cmd->type)
            {
                case dm_cmd_type::exit_cmd_loop:                    
                    loop_exit = true;
                    break;

                case dm_cmd_type::start_process:
                    start_process((dm_cmd_start_process*)cmd);
                    break;

                default:
                    log->error("unknown cmd [%d]", cmd->type);
            }

            cmd_list->next();
        }
    }
}

bool dm_core::start_process(dm_cmd_start_process* cmd)
{
    STARTUPINFOA startup_info; 
    PROCESS_INFORMATION proc_info; 
    CREATE_PROCESS_DEBUG_INFO proc_debug_info;

    ZeroMemory(&proc_info, sizeof(proc_info));
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    log->info("creating process [%s]", cmd->path);

    if(!CreateProcessA(cmd->path, NULL, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS, NULL,NULL, &startup_info, &proc_info))
    {
        log->error("failed to create process [%s]", cmd->path);
        return false;
    }
    
    DEBUG_EVENT debug_event = {0};
    while(1)
    {
        if(WaitForDebugEvent(&debug_event, 3000))
        {
            process_debug_event(&debug_event, &proc_info, &proc_debug_info);

            if(!ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE))
            {   
                log->error("continue debug event failed");
                return false;
            }       
        }
        else
        {

#if 0            
            log->info("no event");

            static UINT8 scan_done = false;
            if(!scan_done)
            {
                PVOID wanted_addr =  scan_memory(&proc_info, 555555);

                UINT32 new_val = 123456;
                SIZE_T write_size = 0;
                if(wanted_addr)
                {
                    WriteProcessMemory(proc_info.hProcess, wanted_addr, &new_val, sizeof(new_val), &write_size);
                    if(write_size == sizeof(new_val))
                    {
                        log->info("written [%d] to [0x%x]", new_val, wanted_addr);
                    }
                }

                scan_done = true;
            }
#endif            
        }
    }
}

bool dm_core::attach_to_process(UINT32 UUID)
{
    return false;
}

const char dm_core::debug_event_id_name[][27] = {
    "NULL", 
    "EXCEPTION_DEBUG_EVENT",      // 1
    "CREATE_THREAD_DEBUG_EVENT",  // 2
    "CREATE_PROCESS_DEBUG_EVENT", // 3
    "EXIT_THREAD_DEBUG_EVENT",    // 4
    "EXIT_PROCESS_DEBUG_EVENT",   // 5
    "LOAD_DLL_DEBUG_EVENT",       // 6
    "UNLOAD_DLL_DEBUG_EVENT",     // 7
    "OUTPUT_DEBUG_STRING_EVENT",  // 8
    "RIP_EVENT" };                // 9
            
bool dm_core::process_debug_event(DEBUG_EVENT* event, PROCESS_INFORMATION* proc_info, CREATE_PROCESS_DEBUG_INFO* proc_debug_info)
{
    if(event->dwDebugEventCode < EXCEPTION_DEBUG_EVENT || event->dwDebugEventCode > RIP_EVENT)
    {
        log->error("invalid debug event it [%d]", event->dwDebugEventCode);
        return false;
    }

    log->info("debug event [%s]", debug_event_id_name[event->dwDebugEventCode]);
    switch(event->dwDebugEventCode)
    {
        case EXCEPTION_DEBUG_EVENT:
            break;

        case CREATE_THREAD_DEBUG_EVENT:
            break;

        case CREATE_PROCESS_DEBUG_EVENT:
            memcpy(proc_debug_info, &(event->u.CreateProcessInfo), sizeof(proc_debug_info));
            break;

        case EXIT_THREAD_DEBUG_EVENT:
            break;

        case EXIT_PROCESS_DEBUG_EVENT:
            break;

        case LOAD_DLL_DEBUG_EVENT:
            break;

        case UNLOAD_DLL_DEBUG_EVENT:
            break;

        case OUTPUT_DEBUG_STRING_EVENT:            
        {
            OUTPUT_DEBUG_STRING_INFO* ds_info = &(event->u.DebugString);
            char* buff = (char*)malloc(event->u.DebugString.nDebugStringLength * 2); // handle WCHAR..
            ReadProcessMemory(
                proc_info->hProcess,         // HANDLE to Debuggee
                ds_info->lpDebugStringData,  // Target process' valid pointer
                buff,                        // Copy to this address space
                ds_info->nDebugStringLength,
                NULL);

            log->info("debug message [%s]", buff);
            free(buff);
            break;
        }
        case RIP_EVENT:
            break;

        defualt:
            break;
    }

    return true;
}

PVOID dm_core::scan_memory(PROCESS_INFORMATION* proc_info, UINT32 wanted)
{
    MEMORY_BASIC_INFORMATION mem_info;
    LPVOID base_addr = NULL;
    UINT32 reg_num = 0;
    UINT32 size_commited = 0;
    PVOID wanted_addr = 0;

    log->info("memory scan: wanted [%d]", wanted);
    while(VirtualQueryEx(proc_info->hProcess, base_addr, &mem_info, sizeof(mem_info)))
    {
        if(mem_info.State == MEM_COMMIT)
        {
            #if 0
            printf("D> reg_num [0x%x] base_addr [0x%x] alloc_base [0x%x] alloc_prot [0x%x] part_id [0x%x] reg_size [0x%x] state [0x%x] prot [0x%x] type [0x%x]\n",
                reg_num, 
                mem_info.BaseAddress,
                mem_info.AllocationBase,
                mem_info.AllocationProtect,
                mem_info.PartitionId,
                mem_info.RegionSize,
                mem_info.State,
                mem_info.Protect,
                mem_info.Type);
            #endif

            UINT32* reg_mem = (UINT32*)malloc(mem_info.RegionSize);
            SIZE_T read_size = 0;
            ReadProcessMemory(proc_info->hProcess, mem_info.BaseAddress, reg_mem, mem_info.RegionSize, &read_size);
            //printf("region read: size [%x]\n", read_size);

            for(UINT32 n = 0; n < read_size / 4; n++)
            {
                if(reg_mem[n] == wanted)
                {
                    log->info("wanted [%d] found at [0x%x]", wanted, (mem_info.BaseAddress + (n * 4)));
                    wanted_addr = mem_info.BaseAddress + (n * 4);
                }
            }

            free(reg_mem);

            size_commited += mem_info.RegionSize;
            reg_num++;
        }
        
        base_addr = (LPVOID)((DWORD_PTR)mem_info.BaseAddress + mem_info.RegionSize);
    }
    log->info("memory scanned [%d] KB", size_commited / 1024);
    log->info("wanted addr [0x%x]", wanted_addr);

    return wanted_addr;
}