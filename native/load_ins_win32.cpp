//
// Created by Karlatemp on 2020/8/13.
//

#include "load_ins.h"
#include <winbase.h>

typedef jint(*InsAgentStartup)(JavaVM *vm, char *options, void *reserved);

void startIns(JavaVM *vm, const char *path, char *jar) {
//    DWORD wd;
    auto lib = LoadLibrary(path);
    auto func = (InsAgentStartup) GetProcAddress(lib, "Agent_OnAttach");
    void *reserved;
    func(vm, jar, &reserved);
//    free(&reserved);
}

