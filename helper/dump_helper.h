// Copyright (c), lemon19900815.
// All rights reserved.
// mini dump file generator for Windows, 
// use virtual studio to open the *.dmp file, locate the dump position.
// dependency: c++11 needed on Windows platform.

#ifndef __DUMP_HELPER_H__
#define __DUMP_HELPER_H__

#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#include <ImageHlp.h>
#include <atlstr.h>

#pragma comment(lib, "DbgHelp.lib")

static std::string timestamp2Str(time_t ts) {
    struct tm* t = std::localtime(&ts);
    if (!t) return "";

    char buff[64];
    snprintf(buff, sizeof(buff),
             "%d-%02d-%02d-%02d-%02d-%02d",
             t->tm_year + 1900, t->tm_mon + 1,
             t->tm_mday, t->tm_hour,
             t->tm_min, t->tm_sec);

    return buff;
}

std::wstring stringToWstring(std::string orig)
{
    std::wstring result;
    result.resize(orig.length() - 1, 0);

    size_t origsize = orig.length() + 1;
    size_t convertedChars = 0;

    mbstowcs_s(&convertedChars, result.data(),
               origsize, orig.c_str(), _TRUNCATE);

    return result;
}

long __stdcall crashHandler(_EXCEPTION_POINTERS* excp) {
    auto now_ts = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    auto dump_time = timestamp2Str(now_ts);

#ifdef UNICODE
    auto dump_filename = stringToWstring(dump_time + ".dmp");
#else
    auto dump_filename = dump_time + ".dmp";
#endif

    HANDLE dump_file = CreateFile(dump_filename.c_str(),
                                  GENERIC_WRITE, 0, NULL,
                                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (dump_file != nullptr)
    {
        MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
        loExceptionInfo.ExceptionPointers = excp;
        loExceptionInfo.ThreadId = GetCurrentThreadId();
        loExceptionInfo.ClientPointers = TRUE;

        MiniDumpWriteDump(GetCurrentProcess(),
                          GetCurrentProcessId(), dump_file,
                          MiniDumpNormal, &loExceptionInfo, NULL, NULL);

        CloseHandle(dump_file);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

// 防止CRT（C runtime）函数报错可能捕捉不到
void disableSetUnhandledExceptionFilter()
{
    auto lib = LoadLibrary(L"kernel32.dll");
    if (!lib)
    {
        return;
    }

    void* addr = (void*)GetProcAddress(lib, "SetUnhandledExceptionFilter");
    if (addr)
    {
        unsigned char code[16];
        int size = 0;

        code[size++] = 0x33;
        code[size++] = 0xC0;
        code[size++] = 0xC2;
        code[size++] = 0x04;
        code[size++] = 0x00;

        DWORD dwOldFlag, dwTempFlag;
        VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
        WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
        VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
    }
}

// windows平台宕机生成mini.dmp文件，使用vs打开dmp文件辅助定位问题
class DumpHelper {
public:
    DumpHelper() {
        SetUnhandledExceptionFilter(crashHandler);
        disableSetUnhandledExceptionFilter();
    }

    ~DumpHelper() {
    }
};
#else
// 其他平台（如：Linux）可以通过gdb来定位
class DumpHelper {
public:
    DumpHelper() {
    }

    ~DumpHelper() {
    }
};
#endif // _WIN32

#endif // __DUMP_HELPER_H__
