#include <windows.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include "dm_core.h"
#include "dm_scan.h"
#include "dm_log.h"
#include "dm_reg.h"

dm_core::dm_core(dm_log* log)
{
    this->log = log;
    cmd_list = new dm_cmd_list(log);
    reg = new dm_reg(log);
    scan = new dm_scan(log, reg);

    cmd_thread = NULL;
    cmd_thread_id = 0;
    attached = false;
}

dm_core::~dm_core()
{
    stop_cmd_loop();
    delete reg;
    delete scan;
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

        log->debug("core: cmd loop stopped");
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
    log->debug("core: cmd_loop");

    bool loop_exit = false;
    while(!loop_exit)
    {
        // process command
        dm_cmd* cmd = cmd_list->get();
        if(cmd != nullptr)
        {
            if(cmd->attached && !attached)
            {
                log->error("not attached");
            }
            else
            {
                switch(cmd->type)
                {
                    case dm_cmd_type::exit_cmd_loop:                    
                        loop_exit = true;
                        break;

                    case dm_cmd_type::start_process:
                        start_process((dm_cmd_start_process*)cmd);
                        break;

                    case dm_cmd_type::fu32:
                        scan->find_u32(&proc_info, ((dm_cmd_fu32*)cmd)->val);
                        break;

                    case dm_cmd_type::fu32_replace:
                        scan->replace_u32(&proc_info, ((dm_cmd_fu32*)cmd)->val);
                        break;

                    case dm_cmd_type::fu32_reset:
                        scan->reset_u32();
                        break;
                        
                    case dm_cmd_type::ru32:
                        reg->read_u32(&proc_info, ((dm_cmd_ru32*)cmd)->addr);
                        break;
                    
                    case dm_cmd_type::wu32:
                        reg->write_u32(&proc_info, ((dm_cmd_wu32*)cmd)->addr, ((dm_cmd_wu32*)cmd)->val);
                        break;

                    default:
                        log->error("unknown cmd [%d]", cmd->type);
                }
            }
            cmd_list->next();
        }

        // handle process events
        DEBUG_EVENT debug_event = {0};
        while(WaitForDebugEvent(&debug_event, 10))
        {
            process_debug_event(&debug_event, &proc_info, &proc_debug_info);

            if(!ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE))
            {   
                log->error("continue debug event failed");
            }       
        }
    }
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
            char* buff = new char[event->u.DebugString.nDebugStringLength * 2];
            
            ReadProcessMemory(
                proc_info->hProcess,         // HANDLE to Debuggee
                ds_info->lpDebugStringData,  // Target process' valid pointer
                buff,                        // Copy to this address space
                ds_info->nDebugStringLength,
                NULL);

            log->info("debug message [%s]", buff);
            
            delete[] buff;
            break;
        }
        case RIP_EVENT:
            break;

        defualt:
            break;
    }

    return true;
}

void dm_core::start_process(dm_cmd_start_process* cmd)
{
    log->info("core: start_process: path [%s]", cmd->path);

    if(!attached)
    {
        ZeroMemory(&proc_info, sizeof(proc_info));
        ZeroMemory(&startup_info, sizeof(startup_info));
        startup_info.cb = sizeof(startup_info);

        log->info("creating process [%s]", cmd->path);

        if(!CreateProcessA(cmd->path, NULL, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE, NULL,NULL, &startup_info, &proc_info))
        {
            log->error("failed to create process [%s]", cmd->path);
        }
        else
        {
            this->attached = true;
        }
    }
    else
    {
        log->error("already attached");
    }
}

bool dm_core::attach_to_process(UINT32 UUID)
{
    //log->debug("core: attach to process - UUID [%d]", cmd->uuid);
    return false;
}