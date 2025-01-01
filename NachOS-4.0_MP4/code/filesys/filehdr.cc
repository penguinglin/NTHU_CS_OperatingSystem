// filehdr.cc
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector,
//
//      Unlike in a real system, we do not keep track of file permissions,
//	ownership, last modification date, etc., in the file header.
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"

//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::FileHeader
//	There is no need to initialize a fileheader,
//	since all the information should be initialized by Allocate or FetchFrom.
//	The purpose of this function is to keep valgrind happy.
//----------------------------------------------------------------------

// TODO begin

FileHeader::FileHeader()
{
	this->nextHeader = NULL; 
	this->nextSector = -1; 
	numBytes = -1;
	numSectors = -1;
	memset(dataSectors, -1, sizeof(dataSectors));
}

// end

//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::~FileHeader
//	Currently, there is not need to do anything in destructor function.
//	However, if you decide to add some "in-core" data in header
//	Always remember to deallocate their space or you will leak memory
//----------------------------------------------------------------------

// TODO begin

FileHeader::~FileHeader()
{
	if (this->nextHeader != NULL) delete this->nextHeader;
}

// end

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

// TODO begin

bool FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{

	int remain = fileSize;
	if(fileSize > MaxFileSize) fileSize = MaxFileSize;
	
	remain = remain - MaxFileSize; 
	numBytes = fileSize;
	numSectors = divRoundUp(fileSize, SectorSize);    		// How many sector needed in a block

	if (freeMap->NumClear() < numSectors) return FALSE;  	// There are not enough free blocks to accomodate new file.

	for (int i = 0; i < numSectors; i++){
		dataSectors[i] = freeMap->FindAndSet();
		ASSERT(dataSectors[i] >= 0);
	}

	if(remain > 0){
		if(this->nextHeader != NULL) return false;			// Already exist

		nextSector = freeMap->FindAndSet();
		if(nextSector == -1) return FALSE;

		this->nextHeader = new FileHeader();
		return nextHeader->Allocate(freeMap , remain);
	}

	return TRUE;
}

// end



//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void FileHeader::Deallocate(PersistentBitmap *freeMap)
{
	for (int i = 0; i < numSectors; i++)
	{
		ASSERT(freeMap->Test((int)dataSectors[i])); // ought to be marked!
		freeMap->Clear((int)dataSectors[i]);
	}
	// TODO begin
	if(nextHeader != NULL) nextHeader->Deallocate(freeMap);
	// end
}

/*
void FileHeader::Deallocate(PersistentBitmap *freeMap)
{
    for (int i = 0; i < numSectors; i++) {
        if (dataSectors[i] >= 0 && dataSectors[i] < NumSectors) {
            ASSERT(freeMap->Test((int)dataSectors[i])); // ought to be marked!
            freeMap->Clear((int)dataSectors[i]);
        } else {
            printf("Invalid data sector: %d\n", dataSectors[i]);
        }
    }

    // 遍歷並釋放鏈結的 FileHeader
    FileHeader* currentHeader = nextHeader;
    while (currentHeader != NULL) {
        for (int i = 0; i < currentHeader->numSectors; i++) {
            if (currentHeader->dataSectors[i] >= 0 && currentHeader->dataSectors[i] < NumSectors) {
                ASSERT(freeMap->Test((int)currentHeader->dataSectors[i]));
                freeMap->Clear((int)currentHeader->dataSectors[i]);
            } else {
                printf("Invalid data sector in linked header: %d\n", currentHeader->dataSectors[i]);
            }
        }
        FileHeader* next = currentHeader->nextHeader;
        delete currentHeader; // 釋放當前的 FileHeader
        currentHeader = next;
    }

    nextHeader = NULL; // 確保鏈結指標被清空
}
*/
//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk.
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

// TODO begin

void FileHeader::FetchFrom(int sector)
{
	kernel->synchDisk->ReadSector(sector, (char *)this + sizeof(FileHeader*));
	/*
		MP4 Hint:
		After you add some in-core informations, you will need to rebuild the header's structure
	*/
	if(this->nextSector != -1){
		nextHeader = new FileHeader;
		nextHeader->FetchFrom(nextSector);
	}
}

// end

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk.
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void FileHeader::WriteBack(int sector)
{
	// TODO begin
	kernel->synchDisk->WriteSector(sector, (char *)this + sizeof(FileHeader*)); 

	if(this->nextHeader != NULL) nextHeader->WriteBack(nextSector);
	
	// end
}
	/*
		MP4 Hint:
		After you add some in-core informations, you may not want to write all fields into disk.
		Use this instead:
		char buf[SectorSize];
		memcpy(buf + offset, &dataToBeWritten, sizeof(dataToBeWritten));
		...
	*/


//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------
// TODO begin

int FileHeader::ByteToSector(int offset)
{
	int idx = divRoundDown(offset, SectorSize);
	if(idx < NumDirect) return dataSectors[idx];
	else return nextHeader->ByteToSector(offset - MaxFileSize);
}

//end
//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------
// TODO begin

int FileHeader::FileLength()
{
	if(this->nextHeader) return numBytes + this->nextHeader->FileLength();
	return numBytes;
}

// end
//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

// TODO begin

void FileHeader::Print()
{
	int fileLen = FileLength();
	printf("header size : %d \n", sizeof(FileHeader) * divRoundUp(fileLen, MaxFileSize));
	
}

void FileHeader::Print_Data_Sector()
{
	for (int i = 0; i < numSectors; i++) printf("%d ", dataSectors[i]);

	if(this->nextHeader != NULL) this->nextHeader->Print_Data_Sector();
}

void FileHeader::Print_File_Content()
{
	int i, j, k;
	char *data = new char[SectorSize];

	for (i = k = 0; i < numSectors; i++)
	{
		kernel->synchDisk->ReadSector(dataSectors[i], data);
		for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++)
		{
			if ('\040' <= data[j] && data[j] <= '\176')
				printf("%c", data[j]);
			else
				printf("\\%x", (unsigned char)data[j]);
		}
		printf("\n");
	}
	
	delete[] data;

	if(this->nextHeader != NULL) this->nextHeader->Print_File_Content();
	
}

// end