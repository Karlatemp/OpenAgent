#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "load_ins.h"

typedef jint(*InsAgentStartup)(JavaVM *vm, char *options, void *reserved);

void startIns(JavaVM *vm, const char *path, char *jar) {
//    DWORD wd;
    auto lib = dlopen(path, RTLD_LAZY);
    auto func = (InsAgentStartup) dlsym(lib, "Agent_OnAttach");
    void *reserved;
    func(vm, jar, &reserved);
//    free(&reserved);
}