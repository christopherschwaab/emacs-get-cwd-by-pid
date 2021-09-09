#include <stdio.h>
#include <Windows.h>

#include "get_process_cwd.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        return 1;
    }

    const DWORD pid = atoi(argv[1]);
    wprintf(L"doing pid %i\n", pid);
    wprintf(L"got: %ls\n", get_cwd_by_pid(pid));

    return 0;
}
