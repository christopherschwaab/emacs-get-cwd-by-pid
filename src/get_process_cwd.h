#ifndef GET_PROCESS_CWD_H_
#define GET_PROCESS_CWD_H_

#include <Windows.h>

WCHAR *get_cwd_by_pid(const DWORD pid, size_t* length);

#endif // GET_PROCESS_CWD_H_
