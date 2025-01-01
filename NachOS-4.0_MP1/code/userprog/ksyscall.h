/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

#include "kernel.h"
#include "synchconsole.h"

void SysHalt() {
    // call interrupt->Halt to stop cpu, crush the simulation ans system
    kernel->interrupt->Halt();
}

void SysPrintInt(int val) {
    // print the int to console
    //  record the time spend
    DEBUG(dbgTraCode, "In ksyscall.h:SysPrintInt, into synchConsoleOut->PutInt, " << kernel->stats->totalTicks);
    // print the value out
    kernel->synchConsoleOut->PutInt(val);
    DEBUG(dbgTraCode, "In ksyscall.h:SysPrintInt, return from synchConsoleOut->PutInt, " << kernel->stats->totalTicks);
}

int SysAdd(int op1, int op2) {
    return op1 + op2;
}

int SysCreate(char *filename) {
    // return value
    // 1: success
    // 0: failed
    // call filesystem->create to create a new file
    return kernel->fileSystem->Create(filename);
}

// When you finish the function "OpenAFile", you can remove the comment below.

OpenFileId SysOpen(char *name) {
    return kernel->fileSystem->OpenAFile(name);
}

int SysWrite(char *buffer, int size, OpenFileId id) {
    // Change the name into WriteAFile, else the function can't call the syscall.h because the function's name are the same :)
    return kernel->fileSystem->WriteAFile(buffer, size, id);
}

int SysRead(char *buffer, int size, OpenFileId id) {
    return kernel->fileSystem->ReadFile(buffer, size, id);
}

int SysClose(OpenFileId id) {
    int status = kernel->fileSystem->CloseFile(id);
    return status;
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
