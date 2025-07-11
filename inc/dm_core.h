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

        void cmd_add(dm_cmd* cmd);
        void cmd_loop_start();
        void cmd_loop_stop();
        void cmd_loop(); // started by dm_core_cmd_loop(), not to be used directly
        
        void proc_show_list();
        void proc_run(char const* const path);
        void proc_attach(UINT32 pid);
        void proc_stop();
        void proc_start();

    protected:
        bool process_debug_event(DEBUG_EVENT* event, CREATE_PROCESS_DEBUG_INFO* proc_debug_info);

        HANDLE proc_handle;
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