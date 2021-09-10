#include <stdio.h>
#include <Windows.h>

#include "get_process_cwd.h"

static inline CHAR* narrow(const WCHAR* ws, const size_t length) {
  // TODO error checking?
  const size_t bufLength = WideCharToMultiByte(CP_UTF8, 0, ws, length, NULL, 0, NULL, NULL);
  CHAR *s = (CHAR*) malloc(bufLength + 1);
  WideCharToMultiByte(CP_UTF8, 0, ws, length, s, bufLength, NULL, NULL);
  s[bufLength] = '\0';
  return s;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        return 1;
    }

    const DWORD pid = atoi(argv[1]);
    wprintf(L"doing pid %i\n", pid);

    size_t length = 0;
    WCHAR* ws = get_cwd_by_pid(pid, &length);
    wprintf(L"got: %ls\n", ws);

    CHAR* s = narrow(ws, length);
    printf("got: %s\n", s);

    free(ws);
    free(s);

    return 0;
}
