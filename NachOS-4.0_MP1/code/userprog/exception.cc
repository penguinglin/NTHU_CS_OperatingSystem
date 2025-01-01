// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "ksyscall.h"
#include "main.h"
#include "syscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------
// 實作異常處理器, 尤其是系統呼叫的異常現象(處理用戶程式所引發的異常)
void ExceptionHandler(ExceptionType which) {
    char ch;
    int val;
    int type = kernel->machine->ReadRegister(2);  // read the type of exception
    int status, exit, threadID, programID, fileID, numChar;

    // output the typr and the system's type
    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");
    DEBUG(dbgTraCode, "In ExceptionHandler(), Received Exception " << which << " type: " << type << ", " << kernel->stats->totalTicks);

    // system call syscall exception
    switch (which) {
        case SyscallException:
            switch (type) {
                // sc_halt
                case SC_Halt:
                    DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
                    SysHalt();
                    cout << "in exception\n";
                    ASSERTNOTREACHED();
                    break;

                // call the function to print
                case SC_PrintInt:
                    DEBUG(dbgSys, "Print Int\n");
                    // read the value in regester 4
                    val = kernel->machine->ReadRegister(4);
                    // into the sysPrintInt, and record the time it spends
                    DEBUG(dbgTraCode, "In ExceptionHandler(), into SysPrintInt, " << kernel->stats->totalTicks);
                    // call function to print the value
                    SysPrintInt(val);
                    DEBUG(dbgTraCode, "In ExceptionHandler(), return from SysPrintInt, " << kernel->stats->totalTicks);
                    // Set Program Counter
                    // renew the regester to the next instructon's addr.
                    // renew the PrevPCReg、PCReg and NextPCReg
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    return;
                    ASSERTNOTREACHED();
                    break;

                // call the msg to print the message store in the reg
                case SC_MSG:
                    DEBUG(dbgSys, "Message received.\n");
                    // read the addr in the reg 4
                    val = kernel->machine->ReadRegister(4);
                    // print the message in the addr
                    {
                        // read the addr's data in memory/cache
                        char *msg = &(kernel->machine->mainMemory[val]);
                        // print the msg
                        cout << msg << endl;
                    }
                    // call halt to shut down the system
                    SysHalt();
                    // the same work as syscall sc_halt
                    ASSERTNOTREACHED();
                    break;

                // sc_create
                case SC_Create:
                    val = kernel->machine->ReadRegister(4);
                    // create the file (filename store in the reg)
                    {
                        // read the addr's data from memory(store the filename)
                        char *filename = &(kernel->machine->mainMemory[val]);
                        // cout << filename << endl;
                        // call syscreate to create a file with filename->filename
                        status = SysCreate(filename);
                        // store the result back to reg 2
                        kernel->machine->WriteRegister(2, (int)status);
                    }
                    // write the state back to reg
                    // renew the PrevPCReg、PCReg and NextPCReg, the same as sc_printint
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    return;
                    // the same work as syscall sc_halt
                    ASSERTNOTREACHED();
                    break;

                // TODO
                // sc_Open
                case SC_Open:
                    // read the addr in reg 4
                    val = kernel->machine->ReadRegister(4);
                    // open the file (filename store in the reg)
                    {
                        // read the addr's data from memory(store the filename)
                        char *filename = &(kernel->machine->mainMemory[val]);
                        // cout << filename << endl;
                        // call SysOpen to open a file with filename->filename
                        status = SysOpen(filename);
                        // store the result back to reg 2
                        kernel->machine->WriteRegister(2, (int)status);
                    }
                    // write the state back to reg
                    // renew the PrevPCReg、PCReg and NextPCReg, the same as sc_printint
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    return;
                    // the same work as syscall sc_halt
                    ASSERTNOTREACHED();
                    break;

                // SC_Read
                case SC_Read:
                    // read the addr in reg 4
                    val = kernel->machine->ReadRegister(4);
                    numChar = kernel->machine->ReadRegister(5);
                    fileID = kernel->machine->ReadRegister(6);
                    // open the file (filename store in the reg)
                    {
                        // read the addr's data from memory(store the filename)
                        char *buf = &(kernel->machine->mainMemory[val]);

                        // cout << filename << endl;
                        // call SysOpen to open a file with filename->filename
                        status = SysRead(buf, numChar, fileID);
                        // status = SysOpen(filename);
                        // store the result back to reg 2
                        kernel->machine->WriteRegister(2, (int)status);
                    }
                    // write the state back to reg
                    // renew the PrevPCReg、PCReg and NextPCReg, the same as sc_printint
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    return;
                    // the same work as syscall sc_halt
                    ASSERTNOTREACHED();
                    break;

                // SC_Write
                case SC_Write:
                    // read the addr in reg 4
                    val = kernel->machine->ReadRegister(4);
                    numChar = kernel->machine->ReadRegister(5);
                    fileID = kernel->machine->ReadRegister(6);
                    // open the file (filename store in the reg)
                    {
                        // read the addr's data from memory(store the filename)
                        char *buf = &(kernel->machine->mainMemory[val]);
                        // cout << filename << endl;
                        // call SysOpen to open a file with filename->filename
                        status = SysWrite(buf, numChar, fileID);
                        // store the result back to reg 2
                        kernel->machine->WriteRegister(2, (int)status);
                    }
                    // write the state back to reg
                    // renew the PrevPCReg、PCReg and NextPCReg, the same as sc_printint
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    return;

                    // the same work as syscall sc_halt
                    ASSERTNOTREACHED();
                    break;

                // SC_Close
                case SC_Close:
                    // read the addr in reg 4
                    fileID = kernel->machine->ReadRegister(4);
                    // open the file (filename store in the reg)
                    {
                        // read the addr's data from memory(store the filename)
                        // char *filename = &(kernel->machine->mainMemory[val]);
                        // cout << filename << endl;
                        // call SysOpen to open a file with filename->filename
                        // int regnumber = kernel->machine->ReadRegister(4);
                        // char *buf = &(kernel->machine->mainMemory[regnumber]);

                        status = SysClose(fileID);

                        // status = Remove(filename);
                        // store the result back to reg 2
                        kernel->machine->WriteRegister(2, (int)status);
                    }
                    // write the state back to reg
                    // renew the PrevPCReg、PCReg and NextPCReg, the same as sc_printint
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    return;
                    // the same work as syscall sc_halt
                    ASSERTNOTREACHED();
                    break;
                // end TODO

                // add
                case SC_Add:
                    DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
                    /* Process SysAdd Systemcall*/
                    int result;
                    // just like the add function in assembly code
                    result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
                                    /* int op2 */ (int)kernel->machine->ReadRegister(5));
                    DEBUG(dbgSys, "Add returning with " << result << "\n");
                    /* Prepare Result */
                    // write back the result to reg 2
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                        /* set previous programm counter (debugging only)*/
                        // renew the PrevPCReg、PCReg and NextPCReg
                        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                        /* set next programm counter for brach execution */
                        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    }
                    // print the result to the console
                    cout << "result is " << result << "\n";
                    return;
                    // the same work as syscall sc_halt
                    ASSERTNOTREACHED();
                    break;

                // exit
                case SC_Exit:
                    DEBUG(dbgAddr, "Program exit\n");
                    // read the addr in reg
                    val = kernel->machine->ReadRegister(4);
                    cout << "return value:" << val << endl;
                    // stop the process
                    kernel->currentThread->Finish();
                    break;

                // error message
                default:
                    cerr << "Unexpected system call " << type << "\n";
                    break;
            }
            break;

        // user mode error
        default:
            cerr << "Unexpected user mode exception " << (int)which << "\n";
            break;
    }
    ASSERTNOTREACHED();
}

/*
    PrevPCReg：記錄上一條指令的地址，用於debuging和tracking program執行過程。
    PCReg：記錄當前正在執行的指令的地址（program counter）。
    NextPCReg：記錄下一條將要執行的指令的地址，用於prefetching和brench操作。
*/