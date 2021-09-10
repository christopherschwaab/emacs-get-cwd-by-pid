#include <Windows.h>
#include <winternl.h>
#include <wow64apiset.h>

#include <memory>
#include <stdexcept>
#include <string>

#include <iostream>
#include <cstdio>

// https://docs.microsoft.com/en-us/windows/win32/api/winternl/ns-winternl-rtl_user_process_parameters
// typedef struct _RTL_USER_PROCESS_PARAMETERS {
//   BYTE           Reserved1[16];   // 0  16
//   PVOID          Reserved2[10];   // 16 80
//   UNICODE_STRING ImagePathName;   // 96
//   UNICODE_STRING CommandLine;
// } RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

// typedef struct _RTL_USER_PROCESS_PARAMETERS {
//   ULONG                   MaximumLength;           // 0  4
//   ULONG                   Length;                  // 4  4
//   ULONG                   Flags;                   // 8  4
//   ULONG                   DebugFlags;              // 12 4
//   PVOID                   ConsoleHandle;           // 16 8
//   ULONG                   ConsoleFlags;            // 24 4
//   HANDLE                  StdInputHandle;          // 32 8 (32=28+4 bytes padding for self-alignment)
//   HANDLE                  StdOutputHandle;         // 40 8
//   HANDLE                  StdErrorHandle;          // 48 8
//   UNICODE_STRING          CurrentDirectoryPath;    // 56 (PVOID Reserved2[5] = 16+5*8 = 56)
//   HANDLE                  CurrentDirectoryHandle;
//   UNICODE_STRING          DllPath;
//   UNICODE_STRING          ImagePathName;
//   UNICODE_STRING          CommandLine;
//   PVOID                   Environment;
//   ULONG                   StartingPositionLeft;
//   ULONG                   StartingPositionTop;
//   ULONG                   Width;
//   ULONG                   Height;
//   ULONG                   CharWidth;
//   ULONG                   CharHeight;
//   ULONG                   ConsoleTextAttributes;
//   ULONG                   WindowFlags;
//   ULONG                   ShowWindowFlags;
//   UNICODE_STRING          WindowTitle;
//   UNICODE_STRING          DesktopName;
//   UNICODE_STRING          ShellInfo;
//   UNICODE_STRING          RuntimeData;
//   RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
// } RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

namespace {
  using unique_handle = std::unique_ptr<std::remove_pointer<HANDLE>::type, BOOL(*)(HANDLE)>;
} // namespace

static unique_handle make_unique_handle(HANDLE handle) noexcept {
  return unique_handle(handle, &CloseHandle);
}

static bool ensure_privilege(const LPWSTR privelege) {
  HANDLE token = INVALID_HANDLE_VALUE;
  TOKEN_PRIVILEGES tokenPrivileges;
  LUID luidDebug;

  if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token) != FALSE) {
    auto uniqueTokenHandle = make_unique_handle(token);
    if (LookupPrivilegeValue(NULL, privelege, &luidDebug) != FALSE) {
      tokenPrivileges.PrivilegeCount = 1;
      tokenPrivileges.Privileges[0].Luid = luidDebug;
      tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      return AdjustTokenPrivileges(token, FALSE, &tokenPrivileges, 0, nullptr, nullptr) != FALSE;
    }
  }

  return false;
}

static bool ensure_debug_privilege() {
  return ensure_privilege(SE_DEBUG_NAME);
}

decltype(&NtQueryInformationProcess) ntQueryInformationProcess;
HMODULE ntdll = nullptr;
struct NtQueryInformationProcessLoader {
  NtQueryInformationProcessLoader() {
    if (!ensure_debug_privilege()) {
      throw std::runtime_error("failed to adjust debug privilege");
    }
    ntdll = LoadLibraryA("ntdll");

    if (ntdll == nullptr) {
      throw std::runtime_error("ntdll failed to load");
    }

    const FARPROC ntQueryInformationProcessAddr = GetProcAddress(ntdll, "NtQueryInformationProcess");
    if (ntQueryInformationProcessAddr == nullptr) {
      throw std::runtime_error("NtQueryInformationProcess not found");
    }

    ntQueryInformationProcess = reinterpret_cast<decltype(&NtQueryInformationProcess)>(ntQueryInformationProcessAddr);
  }

  ~NtQueryInformationProcessLoader() {
    if (ntdll != nullptr) {
      FreeLibrary(ntdll);
    }
  }
};

static NtQueryInformationProcessLoader ntQueryInformationProcessLoader;

static PEB read_process_peb(HANDLE proc) {
  DWORD minBufLength;
  PROCESS_BASIC_INFORMATION procInfo;
  const NTSTATUS result = ntQueryInformationProcess(proc, ProcessBasicInformation, &procInfo, sizeof(PROCESS_BASIC_INFORMATION), &minBufLength);

  if (result != 0 || minBufLength < sizeof(PROCESS_BASIC_INFORMATION)) {
    // TODO what do we do here?
    throw std::runtime_error("NtQueryInformationProcess: " + result);
  }

  SIZE_T bytesRead;
  PEB peb;
  if (!ReadProcessMemory(proc, procInfo.PebBaseAddress, &peb, sizeof(PEB), &bytesRead) || bytesRead < sizeof(PEB)) {
    // TODO what to do here
    throw std::runtime_error("ReadProcessMemory: " + std::to_string(GetLastError()));
  }

  return peb;
}

static WCHAR* get_current_directory_path(HANDLE proc, const RTL_USER_PROCESS_PARAMETERS* userProcParams, size_t* length) {
    UNICODE_STRING cwd;
    ReadProcessMemory(proc, userProcParams->Reserved2 + 5, &cwd, sizeof(UNICODE_STRING), nullptr);
    const size_t pathLength = cwd.Length / 2;
    auto path = std::make_unique<wchar_t[]>(pathLength + 1);
    ReadProcessMemory(proc, cwd.Buffer, path.get(), cwd.Length, nullptr);
    path[pathLength] = '\0';
    *length = pathLength;
    return path.release();
}

// https://stackoverflow.com/questions/14018280/how-to-get-a-process-working-dir-on-windows
// https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntqueryinformationprocess
extern "C" WCHAR *get_cwd_by_pid(const DWORD pid, size_t* length) {
  const HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if (proc == nullptr) {
    // TODO failed
    return nullptr;
  }

  // FIXME guess we need to load this dynamically for msys?
  // USHORT processMachine;
  // USHORT nativeMachine;
  // if (IsWow64Process2(proc, &processMachine, &nativeMachine) == 0) {
  //   // TODO what do we do here?
  //   return nullptr;
  // }
  // if (processMachine != IMAGE_FILE_MACHINE_UNKNOWN) {
  //   // TODO how do we need to handle WOW?
  //   return nullptr;
  // }

  auto peb = read_process_peb(proc);
  return get_current_directory_path(proc, peb.ProcessParameters, length);
}
