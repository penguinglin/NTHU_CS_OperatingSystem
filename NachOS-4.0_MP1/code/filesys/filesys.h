// filesys.h
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system.
//	The "STUB" version just re-defines the Nachos file system
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.
//
//	The other version is a "real" file system, built on top of
//	a disk simulator.  The disk is simulated using the native UNIX
//	file system (in a file named "DISK").
//
//	In the "real" implementation, there are two key data structures used
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "debug.h"  //just for test!!!
#include "openfile.h"
#include "sysdep.h"

#ifdef FILESYS_STUB  // Temporarily implement file system calls as
                     // calls to UNIX, until the real file system
                     // implementation is available
typedef int OpenFileId;

class FileSystem {
   public:
    OpenFile *OpenFileTable[20];          
    char openfiletable_name[20][100];    

    FileSystem() {
        for (int i = 0; i < 20; i++){
            OpenFileTable[i] = NULL; 
            openfiletable_name[i][0] = '\0'; // 
        }
    }

    bool Create(char *name) {
        int fileDescriptor = OpenForWrite(name);
        if (fileDescriptor == -1)
            return FALSE;
        Close(fileDescriptor);
        return TRUE;
    }
    // The OpenFile function is used for open user program  [userprog/addrspace.cc]
    OpenFile *Open(char *name) {
        // OpenForReadWrite ->
        int fileDescriptor = OpenForReadWrite(name, FALSE);

        if (fileDescriptor == -1)
            return NULL;
        return new OpenFile(fileDescriptor);
    }

    int OpenAFile(char *name) {
        // fileDescriptor is unique

        char newArr[100]; // 定義一個足夠大的 char 陣列來存儲字元
        char *ptr = name; // 指向原始陣列的指標
        int index = 0; // 用來追蹤 newArr 的索引

        while (*ptr != '\0') {
            newArr[index] = *ptr; 
            ptr++; 
            index++; 
        }
        // duplicate file opening
        for(int idx = 0; idx <= 20; idx++){
            if(strcmp(openfiletable_name[idx], newArr) == 0){
                return -1;
            }
        }

        int fileDescriptor = OpenForReadWrite(name, FALSE);
        if (fileDescriptor == -1)
            return -1;
        // insert file into table
        for (int i = 0; i < 20; i++) {
            if (OpenFileTable[i] == NULL) {
                OpenFileTable[i] = new OpenFile(fileDescriptor);
                strcpy(openfiletable_name[i], newArr);
                return i;  // return index
            }
        }
        return -1;
    }

    int WriteAFile(char *buffer, int size, OpenFileId id) {
        // call writeFile
        // open for read write
        // table index
        if (size <= 0) {
            return -1;
        }
        // check if there exsit the same file

        if (id < 0 || id >= 20 || OpenFileTable[id] == NULL)
            return -1;

        int writeID = OpenFileTable[id]->file;
        WriteFile(writeID, buffer, size);
        return size;
    }

    int ReadFile(char *buffer, int size, OpenFileId id) {
        if (size <= 0) {
            return -1;
        }
        if (id < 0 || id >= 20 || OpenFileTable[id] == NULL)
            return -1;
        int readID = OpenFileTable[id]->file;
        Read(readID, buffer, size);
        return size;
    }

    int CloseFile(OpenFileId id) {
        if (id < 0 || id >= 20 || OpenFileTable[id] == NULL) {
            return -1;
        }
        delete OpenFileTable[id];
        OpenFileTable[id] = NULL;
        openfiletable_name[id][0] = '\0';
        return 1;
    }

    bool Remove(char *name) {
        return Unlink(name) == 0;
    }

};

#else  // FILESYS
class FileSystem {
   public:
    FileSystem(bool format);  // Initialize the file system.
                              // Must be called *after* "synchDisk"
                              // has been initialized.
                              // If "format", there is nothing on
                              // the disk, so initialize the directory
                              // and the bitmap of free blocks.

    bool Create(char *name, int initialSize);
    // Create a file (UNIX creat)

    OpenFile *Open(char *name);  // Open a file (UNIX open)

    bool Remove(char *name);  // Delete a file (UNIX unlink)

    void List();  // List all the files in the file system

    void Print();  // List all the files and their contents

   private:
    OpenFile *freeMapFile;    // Bit map of free disk blocks,
                              // represented as a file
    OpenFile *directoryFile;  // "Root" directory -- list of
                              // file names, represented as a file
};

#endif  // FILESYS

#endif  // FS_H
