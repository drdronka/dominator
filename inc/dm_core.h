#pragma once

#include <windows.h>

enum class dm_core_err {
    ok = 0,
    create_process = 1,
    coninue_debug_event = 2,
    process_debug_event = 3,
};

class dm_core
{
    public:
        dm_core();
        ~dm_core();
        dm_core_err start_process(const char* path);
        dm_core_err attach_to_process(UINT32 uuid);

    protected:
        dm_core_err process_debug_event(
            DEBUG_EVENT* event, 
            PROCESS_INFORMATION* proc_info, 
            CREATE_PROCESS_DEBUG_INFO* proc_debug_info);
        PVOID scan_memory(
            PROCESS_INFORMATION* proc_info, 
            UINT32 wanted);

        static const char debug_event_id_name[][27];
};