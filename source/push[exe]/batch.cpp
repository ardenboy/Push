#include <sltypes.h>
#include <slntuapi.h>
#include <slfile.h>
#include <slc.h>
#include <wchar.h>

#include "push.h"
#include "file.h"
#include "ini.h"
#include "batch.h"


extern "C"
WCHAR*
memchrW(const WCHAR *ptr, WCHAR ch, UINT_B n);


VOID
GetBatchFile( WCHAR* GameId, WCHAR* Buffer )
{
    WCHAR *gameName, *dot;
    WCHAR batchFile[260] = L"cache\\";

    gameName = IniReadSubKey(L"Game Settings", GameId, L"Name");

    wcscat(batchFile, gameName);
    RtlFreeHeap(PushHeapHandle, 0, gameName);

    dot = SlStringFindLastChar(batchFile, '.');

    if (dot)
        wcscpy(dot, L".txt");
    else
        wcscat(batchFile, L".txt");

    wcscpy(Buffer, batchFile);
}


BfBatchFile::BfBatchFile( WCHAR* GameId )
{
    UINT64 fileSize;
    VOID *buffer = NULL, *bufferOffset;
    WCHAR *lineStart, *nextLine, *end;
    FILE_LIST_ENTRY fileEntry;
    WCHAR line[260];
    UINT16 lineLength;
    WCHAR batchFile[260];

    FileList = NULL;

    // Get the batchfile name and path
    GetBatchFile(GameId, batchFile);

    // Allocate some memory for the batchfile name
    BatchFileName = (WCHAR*) RtlAllocateHeap(
        PushHeapHandle,
        0,
        (SlStringGetLength(batchFile) + 1) * sizeof(WCHAR)
        );

    // Save new batchfile name
    wcscpy(BatchFileName, batchFile);

    // Open the batchfile and read the entire file into memory
    buffer = FsFileLoad(batchFile, &fileSize);

    if (!buffer) 
        return;

    // Start our reads after the UTF16-LE character marker
    bufferOffset = (WCHAR*) buffer + 1;

    // Update fileSize to reflect. It is now invalid for use as the 
    // actual file size.
    fileSize -= sizeof(WCHAR);

    // Get the end of the buffer
    end = (WCHAR *)((UINT_B)bufferOffset + fileSize);

    // Let's start the day off nicely
    nextLine = (WCHAR*) bufferOffset;
    fileSize = 0;

    // Get all file names and queue them for caching
    while (nextLine < end)
    {
        lineStart = nextLine;

        // Try to find new line character
        nextLine = memchrW(lineStart, '\n', end - lineStart);

        if (!nextLine)
            // Didn't find it? How about return?
            nextLine = memchrW(lineStart, '\r', end - lineStart);

        if (!nextLine)
            // Still didn't find any? Must not be one.
            nextLine = end + 1;

        lineLength = (nextLine - 1) - lineStart;

        // Copy line into a buffer
        wcsncpy(line, lineStart, lineLength);

        // Terminate the buffer;
        line[lineLength] = L'\0';

        nextLine++;

        // Add to file list
        fileEntry.Name = line;
        fileEntry.Bytes = FsFileGetSize(line);

        PushAddToFileList(&FileList, &fileEntry);

        // Increment total batch size counter
        fileSize += fileEntry.Bytes;
    }

    // Be a nice guy and give back what was given to you
    RtlFreeHeap(PushHeapHandle, 0, buffer);

    // Update batch size;
    BatchSize = fileSize;
}


BfBatchFile::~BfBatchFile()
{
    //destroy file list
}


/**
* Checks whether a file is included in a batchfile.
*
* \param BatchFile The batchfile.
* \param File A pointer to a FILE_LIST_ENTRY
* structure representing the file.
*/

BOOLEAN
BfBatchFile::IsBatchedFile( FILE_LIST_ENTRY* File )
{
    FILE_LIST_ENTRY *file;

    file = (FILE_LIST_ENTRY*) FileList;

    while (file != NULL)
    {
        if (File->Bytes == file->Bytes
            && wcscmp(File->Name, file->Name) == 0)
            return TRUE;

        file = file->NextEntry;
    }

    return FALSE;
}


/**
* Gets the size (in bytes) of
* a batchfile.
*
* \param BatchFile The batchfile.
*/

UINT64
BfBatchFile::GetBatchSize()
{
    return BatchSize;
}


/**
* Saves the batchfile to disk.
*
* \param BatchFile The batchfile.
*/

VOID
BfBatchFile::SaveBatchFile()
{
    VOID *fileHandle;
    IO_STATUS_BLOCK isb;
    FILE_LIST_ENTRY *file;
    WCHAR marker = 0xFEFF;
    WCHAR end[] = L"\r\n";

    SlFileCreate(
        &fileHandle,
        BatchFileName,
        SYNCHRONIZE | FILE_READ_ATTRIBUTES | GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OVERWRITE_IF,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
        );

    file = (FILE_LIST_ENTRY*) FileList;

    // Write character marker
    NtWriteFile(
        fileHandle,
        NULL,
        NULL,
        NULL,
        &isb,
        &marker,
        sizeof(marker),
        NULL,
        NULL
        );

    // Write all batch items to file
    while (file != 0)
    {
        // Write a single item to batch file
        NtWriteFile(
            fileHandle,
            NULL,
            NULL,
            NULL,
            &isb,
            file->Name,
            SlStringGetLength(file->Name) * sizeof(WCHAR),
            NULL,
            NULL
            );

        // Write new line
        NtWriteFile(
            fileHandle,
            NULL,
            NULL,
            NULL,
            &isb,
            &end,
            4,
            NULL,
            NULL
            );

        file = file->NextEntry;
    }

    NtClose(fileHandle);
}


/**
* Adds an item to a
* batchfile.
*
* \param BatchFile The
* batchfile.
* \param File Pointer to a
* FILE_LIST_ENTRY structure
* with details of the file.
*/

VOID
BfBatchFile::AddItem(
    FILE_LIST_ENTRY* File
    )
{
    FILE_LIST_ENTRY file;

    file.Name = File->Name;
    file.Bytes = File->Bytes;

    PushAddToFileList(
        &FileList,
        &file
        );
}


/**
* Removes an item from a batchfile.
*
* \param File Pointer to a FILE_LIST_ENTRY structure with
* details of the file.
*/

VOID
BfBatchFile::RemoveItem( FILE_LIST_ENTRY* File )
{
    FILE_LIST_ENTRY *file, *previousEntry;

    file = FileList;

    while (file != 0)
    {
        if (file->Bytes == File->Bytes
            && wcscmp(file->Name, File->Name) == 0)
        {
            if (file == FileList)
                FileList = file->NextEntry;
            else
                previousEntry->NextEntry = file->NextEntry;

            break;
        }

        previousEntry = file;
        file = file->NextEntry;
    }
}


/**
* Gets a pointer to
* the batchfile list
* used by the
* BatchFile class.
*/

FILE_LIST
BfBatchFile::GetBatchList()
{
    return FileList;
}
