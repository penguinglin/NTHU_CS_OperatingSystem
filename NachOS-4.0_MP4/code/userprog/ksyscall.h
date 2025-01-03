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

void SysHalt()
{
	kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
	return op1 + op2;
}

// TODO begin

int SysCreate(char *filename , int init){
    // return value
    // 1: success
    // 0: failed
    // call filesystem->create to create a new file
	return kernel->fileSystem->Create(filename , init);
}

OpenFileId SysOpen(char *name){
	return kernel->fileSystem->OpenAFile(name);
}

int SysRead(char *buffer, int size, OpenFileId id){
	return kernel->fileSystem->Read(buffer, size, id);
}

int SysWrite(char *buffer, int size, OpenFileId id){
	return kernel->fileSystem->Write(buffer, size, id);
}

int SysClose(OpenFileId id){
	return kernel->fileSystem->Close(id);
}

// end

#ifdef FILESYS_STUB
int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename);
}
#endif

#endif /* ! __USERPROG_KSYSCALL_H__ */
