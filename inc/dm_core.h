#pragma once

#include <windows.h>

#include "dm_log.h"
#include "dm_cmd.h"

enum class dm_core_err {
    ok = 0,
    create_process = 1,
    coninue_debug_event = 2,
    process_debug_event = 3,
};

class dm_core
{
    public:
        dm_core(dm_log* log_i);
        ~dm_core();

        void add_cmd(dm_cmd* cmd);
        void cmd_loop();

    protected:
        bool start_process(dm_cmd_start_process* cmd);
        bool attach_to_process(UINT32 uuid);
        bool process_debug_event(DEBUG_EVENT* event, PROCESS_INFORMATION* proc_info, CREATE_PROCESS_DEBUG_INFO* proc_debug_info);
        PVOID scan_memory(PROCESS_INFORMATION* proc_info, UINT32 wanted);

        dm_log* log;
        dm_cmd_list* cmd_list;

        static char const debug_event_id_name[][27];
};