#include <windows.h>
#include <stdio.h>

const char target_app[] = "C:\\Users\\Dron\\test\\test.exe";

static void debug_program(const char* path);
static void process_debug_event(DEBUG_EVENT* event, PROCESS_INFORMATION* proc_info, CREATE_PROCESS_DEBUG_INFO* proc_debug_info);
static PVOID scan_memory(PROCESS_INFORMATION* proc_info, UINT32 wanted);

int main(int argc, char* argv[])
{
    debug_program(target_app);

    return 0;
}

void debug_program(const char* path)
{
    STARTUPINFOA startup_info; 
    PROCESS_INFORMATION proc_info; 
    CREATE_PROCESS_DEBUG_INFO proc_debug_info;

    ZeroMemory(&proc_info, sizeof(proc_info));
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if(!CreateProcessA(path, NULL, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS, NULL,NULL, &startup_info, &proc_info))
    {
        printf("D> [ERROR] Failed to create process [%d]\n", path);
        return;
    }
    
    DEBUG_EVENT debug_event = {0};
    while(1)
    {
        if(WaitForDebugEvent(&debug_event, 3000))
        {
            process_debug_event(&debug_event, &proc_info, &proc_debug_info);

            if(!ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE))
            {   
                printf("D> [ERROR] ContinueDebugEvent");
                return;
            }       
        }
        else
        {
            printf("D> no event\n");

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
                        printf("D> written [%d] to [0x%x]\n", new_val, wanted_addr);
                    }
                }

                scan_done = true;
            }
        }
    }
}

char debug_event_id_name[][27] = {
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
            
void process_debug_event(DEBUG_EVENT* event, PROCESS_INFORMATION* proc_info, CREATE_PROCESS_DEBUG_INFO* proc_debug_info)
{
    if(event->dwDebugEventCode < EXCEPTION_DEBUG_EVENT || event->dwDebugEventCode > RIP_EVENT)
    {
        printf("D> [ERR] invalid debug event it [%d]\n", event->dwDebugEventCode);
        return;
    }

    printf("D> debug event [%s]\n", debug_event_id_name[event->dwDebugEventCode]);
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

            printf("D> debug message [%s]\n", buff);
            free(buff);
            break;
        }
        case RIP_EVENT:
            break;

        defualt:
            break;
    }
}

PVOID scan_memory(PROCESS_INFORMATION* proc_info, UINT32 wanted)
{
    MEMORY_BASIC_INFORMATION mem_info;
    LPVOID base_addr = NULL;
    UINT32 reg_num = 0;
    UINT32 size_commited = 0;
    PVOID wanted_addr = 0;

    printf("D> memory scan: wanted [%d]\n", wanted);
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
                    printf("D> wanted [%d] found at [0x%x]\n", wanted, (mem_info.BaseAddress + (n * 4)));
                    wanted_addr = mem_info.BaseAddress + (n * 4);
                }
            }

            free(reg_mem);

            size_commited += mem_info.RegionSize;
            reg_num++;
        }
        
        base_addr = (LPVOID)((DWORD_PTR)mem_info.BaseAddress + mem_info.RegionSize);
    }
    printf("D> memory scanned [%d] KB\n", size_commited / 1024);
    printf("D> wanted addr [0x%x]\n", wanted_addr);

    return wanted_addr;
}