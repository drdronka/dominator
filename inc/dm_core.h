#pragma once

#include <windows.h>
#include <vector>

#include "dm_log.h"
#include "dm_cmd.h"
#include "dm_scan.h"

class dm_core
{
    public:
        dm_core(dm_log* log_i);
        ~dm_core();

        void add_cmd(dm_cmd* cmd);
        void start_cmd_loop();
        void stop_cmd_loop();
        void cmd_loop(); // started by dm_core_cmd_loop(), not to be used directly
        
        void show_process_list();
        void run_process(char const* const path);
        void attach_to_process(UINT32 uuid);
        void pause_process();
        void resume_process();

    protected:
        bool process_debug_event(DEBUG_EVENT* event, PROCESS_INFORMATION* proc_info, CREATE_PROCESS_DEBUG_INFO* proc_debug_info);

        STARTUPINFOA startup_info; 
        PROCESS_INFORMATION proc_info; 
        CREATE_PROCESS_DEBUG_INFO proc_debug_info;

        dm_log* log;
        dm_scan* scan;
        dm_reg* reg;
        dm_cmd_list* cmd_list;

        HANDLE cmd_thread;
        DWORD cmd_thread_id;
        bool attached;

        static char const debug_event_id_name[][27];
};

static DWORD WINAPI dm_core_cmd_loop(LPVOID ref);