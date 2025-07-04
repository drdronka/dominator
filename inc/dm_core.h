#pragma once

#include <windows.h>

#include "dm_log.h"
#include "dm_cmd.h"

class dm_core
{
    public:
        dm_core(dm_log* log_i);
        ~dm_core();

        void add_cmd(dm_cmd* cmd);
        void start_cmd_loop();
        void stop_cmd_loop();
        void cmd_loop(); // started by dm_core_cmd_loop(), not to be used directly

    protected:
        bool process_debug_event(DEBUG_EVENT* event, PROCESS_INFORMATION* proc_info, CREATE_PROCESS_DEBUG_INFO* proc_debug_info);
        void start_process(dm_cmd_start_process* cmd);
        bool attach_to_process(UINT32 uuid);
        void find_u32(dm_cmd_fu32* cmd);
        void read_u32(dm_cmd_ru32* cmd);
        void write_u32(dm_cmd_wu32* cmd);
        PVOID scan_memory(PROCESS_INFORMATION* proc_info, UINT32 wanted);

        dm_log* log;
        dm_cmd_list* cmd_list;
        HANDLE cmd_thread;
        DWORD cmd_thread_id;

        bool attached;
        STARTUPINFOA startup_info; 
        PROCESS_INFORMATION proc_info; 
        CREATE_PROCESS_DEBUG_INFO proc_debug_info;

        static char const debug_event_id_name[][27];
};

static DWORD WINAPI dm_core_cmd_loop(LPVOID ref);