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
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
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

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);
	int val;
	int status, exit, threadID, programID;
	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");
	
	// TODO begin
	char *filename;
	char* buffer;
	int fileID;
	int numChar;
	// end

	switch (which)
	{
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			SysHalt();
			cout << "in exception\n";
			ASSERTNOTREACHED();
			break;
		case SC_MSG:
			DEBUG(dbgSys, "Message received.\n");
			val = kernel->machine->ReadRegister(4);
			{
				char *msg = &(kernel->machine->mainMemory[val]);
				cout << msg << endl;
			}
			SysHalt();
			ASSERTNOTREACHED();
			break;
		// MP4 mod tag
#ifdef FILESYS_STUB
		case SC_Create:
			val = kernel->machine->ReadRegister(4);
			{
				char *filename = &(kernel->machine->mainMemory[val]);
				//cout << filename << endl;
				status = SysCreate(filename);
				kernel->machine->WriteRegister(2, (int)status);
			}
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			return;
			ASSERTNOTREACHED();
			break;
#endif

		// TODO begin
		case SC_Create:
			val = kernel->machine->ReadRegister(4);
			// create the file (filename store in the reg)
			{
				// read the addr's data from memory(store the filename)
				filename = &(kernel->machine->mainMemory[val]);
				// cout << filename << endl;
				int init = (int)kernel->machine->ReadRegister(5);
				// call syscreate to create a file with filename->filename
				status = SysCreate(filename , init);
				// store the result back to reg 2
				kernel->machine->WriteRegister(2, (int) status);
			}
			// write the state back to reg
        	// renew the PrevPCReg、PCReg and NextPCReg
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);

			return;
			ASSERTNOTREACHED();
			break;

		case SC_Open:
			// read the addr in reg 4
			val = kernel->machine->ReadRegister(4);
			// open the file (filename store in the reg)
			{
				// read the addr's data from memory (store the filename)
				filename = &(kernel->machine->mainMemory[val]); 
				// cout << filename << endl;
				// call SysOpen to open a file with filename
				status = SysOpen(filename);
				// store the result back to reg 2
				kernel->machine->WriteRegister(2, (int) status);
			}
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			return;	
			ASSERTNOTREACHED();
			break;

		case SC_Read:
			// read the addr in reg 4
			val = kernel->machine->ReadRegister(4); 
			numChar = kernel->machine->ReadRegister(5);
			fileID = kernel->machine->ReadRegister(6);
			// read the file
			{
				// read the addr's data from memory
				buffer = &(kernel->machine->mainMemory[val]);
                // call SysRead to read a file
				status = SysRead(buffer , numChar , fileID);
				// store the result back to reg 2
				kernel->machine->WriteRegister(2, (int) status);
			}
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			return;	
			ASSERTNOTREACHED();
			break;

		case SC_Write:
			// read the addr in reg 4
			val = kernel->machine->ReadRegister(4); 
			numChar = kernel->machine->ReadRegister(5);
			fileID = kernel->machine->ReadRegister(6);
			{
				// read the addr's data from memory
				buffer = &(kernel->machine->mainMemory[val]);
				// call SysWrite to write a file
				status = SysWrite(buffer , numChar , fileID);
				// store the result back to reg 2
				kernel->machine->WriteRegister(2, (int) status);
			}
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			return;	
			ASSERTNOTREACHED();
			break;

		case SC_Close:
			{
				OpenFileId id = kernel->machine->ReadRegister(4);
				status = SysClose(id);
				kernel->machine->WriteRegister(2, (int) status);
			}
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			return;	
			ASSERTNOTREACHED();
			break;


		// end

		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));
			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);
			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
			cout << "result is " << result << "\n";
			return;
			ASSERTNOTREACHED();
			break;
		case SC_Exit:
			DEBUG(dbgAddr, "Program exit\n");
			val = kernel->machine->ReadRegister(4);
			cout << "return value:" << val << endl;
			kernel->currentThread->Finish();
			break;
		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	default:
		cerr << "Unexpected user mode exception " << (int)which << "\n";
		break;
	}
	ASSERTNOTREACHED();
}
