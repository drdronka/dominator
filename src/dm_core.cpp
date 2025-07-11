#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <vector>
#include <algorithm>

#include "dm_core.h"
#include "dm_scan.h"
#include "dm_log.h"
#include "dm_reg.h"

#define PROC_LIST_SIZE 1024

dm_core::dm_core(dm_log* log)
{
    this->log = log;
    cmd_list = new dm_cmd_list(log);
    reg = new dm_reg(log);
    scan = new dm_scan(log, reg);

    cmd_thread = NULL;
    cmd_thread_id = 0;
    proc_handle = 0;
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

                    case dm_cmd_type::proc_show:
                        show_process_list();
                        break;

                    case dm_cmd_type::proc_run:
                        run_process(((dm_cmd_proc_run*)cmd)->path);
                        break;

                    case dm_cmd_type::proc_attach:
                        attach_to_process(((dm_cmd_proc_attach*)cmd)->pid);
                        break;                        

                    case dm_cmd_type::proc_stop:
                        pause_process();
                        break;

                    case dm_cmd_type::proc_start:
                        resume_process();
                        break;

                    case dm_cmd_type::reg_read_u32:
                        reg->read_u32(proc_handle, ((dm_cmd_reg_read_u32*)cmd)->addr);
                        break;
                    
                    case dm_cmd_type::reg_write_u32:
                        reg->write_u32(proc_handle, ((dm_cmd_reg_write_u32*)cmd)->addr, ((dm_cmd_reg_write_u32*)cmd)->val);
                        break;

                    case dm_cmd_type::scan_find_u32:
                        scan->find_u32(proc_handle, ((dm_cmd_scan_find_u32*)cmd)->val);
                        break;

                    case dm_cmd_type::scan_replace_u32:
                        scan->replace_u32(proc_handle, ((dm_cmd_scan_replace_u32*)cmd)->val);
                        break;

                    case dm_cmd_type::scan_reset_u32:
                        scan->reset_u32();
                        break;

                    default:
                        log->error("unknown cmd [%d]", cmd->type);
                }
            }
            cmd_list->next();
        }

        // handle process events
        if(attached)
        {
            DEBUG_EVENT debug_event = {0};
            while(WaitForDebugEvent(&debug_event, 10))
            {
                process_debug_event(&debug_event, &proc_debug_info);

                if(!ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE))
                {   
                    log->error("continue debug event failed");
                }       
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
            
bool dm_core::process_debug_event(DEBUG_EVENT* event, CREATE_PROCESS_DEBUG_INFO* proc_debug_info)
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
                proc_handle,         // HANDLE to Debuggee
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

void dm_core::show_process_list()
{
    log->info("core: show_process_list");

    DWORD proc_list[PROC_LIST_SIZE];
    DWORD proc_list_size = 0;

    if(!EnumProcesses(proc_list, PROC_LIST_SIZE, &proc_list_size))
    {
        log->error("failed to get proc list: winapi error [%d]", GetLastError());
        return;
    }

    log->info("PID\tname");

    for(UINT32 n = 0 ; n < proc_list_size; n++)
    {
        DWORD proc_id = proc_list[n];
        TCHAR proc_name[MAX_PATH] = TEXT("<unknown>");

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, proc_id);

        if(NULL != hProcess)
        {
            HMODULE hMod;
            DWORD cbNeeded;

            if(EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
            {
                GetModuleBaseName(hProcess, hMod, proc_name, sizeof(proc_name)/sizeof(TCHAR));
            }

            log->info("%lu\t%s", proc_id, proc_name);
        }
    }
}

void dm_core::run_process(char const* const path)
{
    log->info("core: start_process: path [%s]", path);

    if(!attached)
    {
        STARTUPINFOA startup_info; 
        PROCESS_INFORMATION proc_info; 

        ZeroMemory(&proc_info, sizeof(proc_info));
        ZeroMemory(&startup_info, sizeof(startup_info));
        startup_info.cb = sizeof(startup_info);

        log->info("creating process [%s]", path);

        if(!CreateProcessA(
            path, 
            NULL, 
            NULL, 
            NULL, 
            FALSE, 
            DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE, 
            NULL,
            NULL, 
            &startup_info, &proc_info))
        {
            log->error("failed to create process [%s]", path);
        }
        else
        {
            proc_handle = proc_info.hProcess;
            this->attached = true;
        }
    }
    else
    {
        log->error("already attached");
    }
}

void dm_core::attach_to_process(UINT32 pid)
{
    log->info("core: attach_to_process: UUID [%lu]", pid);

    proc_handle = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);

    if(!proc_handle)
    {
        log->error("failed to open process [%lu]: winapi error [%d]", pid, GetLastError());
    }
    else
    {
        log->info("attached");
        attached = true;

        if(!DebugActiveProcess(pid))
        {
            log->error("failed to debug process [%lu]: winapi error [%d]", pid, GetLastError());
        }
        else
        {
            log->info("debug enabled");
        }
    }
}

void dm_core::pause_process()
{
    log->info("core: pause_process");

    DebugBreakProcess(proc_handle);
}

void dm_core::resume_process()
{
    log->info("core: resume_process");
}