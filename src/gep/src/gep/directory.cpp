#include "stdafx.h"
#include "gep/directory.h"
#include "gep/exception.h"
#include <sstream>

gep::DirectoryWatcher::DirectoryWatcher(const char* path, WatchSubdirs::Enum watchSubdirs, uint32 watch)
{
    m_watchSubdirs = watchSubdirs;
    m_filter = 0;
    if(watch & Watch::reads)
        m_filter |= FILE_NOTIFY_CHANGE_LAST_ACCESS;
    if(watch & Watch::writes)
        m_filter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
    if(watch & Watch::creates)
        m_filter |= FILE_NOTIFY_CHANGE_CREATION;
    if(watch & Watch::renames)
        m_filter |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;

    m_directoryHandle = CreateFileA(
        path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr);

    if(m_directoryHandle == INVALID_HANDLE_VALUE)
    {
        std::ostringstream msg;
        msg << "Couldn't open directory '" << path << "'. Maybe it does not exist?";
        throw Exception(msg.str());
    }

    m_completionPort = CreateIoCompletionPort(m_directoryHandle, nullptr, 0, 1);
    if(m_completionPort == INVALID_HANDLE_VALUE)
    {
        std::ostringstream msg;
        msg << "Couldn't create io completion port for directory '" << path << "'";
        throw Exception(msg.str());
    }

    doRead();
}

gep::DirectoryWatcher::~DirectoryWatcher()
{
    CancelIo(m_directoryHandle);
    CloseHandle(m_completionPort);
    CloseHandle(m_directoryHandle);
}

void gep::DirectoryWatcher::doRead()
{
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    ReadDirectoryChangesW(m_directoryHandle, m_buffer, GEP_ARRAY_SIZE(m_buffer),
        (m_watchSubdirs == WatchSubdirs::yes),
        m_filter, nullptr, &m_overlapped, nullptr);
}

void gep::DirectoryWatcher::enumerateChanges(std::function<void(const char* filename, Action::Enum action)> func)
{
    OVERLAPPED* lpOverlapped;
    DWORD numberOfBytes;
    ULONG_PTR completionKey;
    while( GetQueuedCompletionStatus(m_completionPort, &numberOfBytes, &completionKey, &lpOverlapped, 0) != 0)
    {
        //Copy the buffer
        GEP_ASSERT(numberOfBytes > 0);
        void* buffer = alloca(numberOfBytes);
        memcpy(buffer, m_buffer, numberOfBytes);

        //Reissue the read request
        doRead();

        //Progress the messages
        auto info = (const FILE_NOTIFY_INFORMATION*)buffer;
        while(true)
        {
            int bytesNeeded = WideCharToMultiByte(CP_UTF8, 0, info->FileName, info->FileNameLength/2, nullptr, 0, nullptr, nullptr);
            if(bytesNeeded > 0)
            {
                char* dir = (char*)alloca(bytesNeeded+1);
                WideCharToMultiByte(CP_UTF8, 0, info->FileName, info->FileNameLength/2, dir, bytesNeeded, nullptr, nullptr);
                dir[bytesNeeded] = '\0';
                func(dir, (Action::Enum)info->Action);
            }
            if(info->NextEntryOffset == 0)
                break;
            else
                info = (const FILE_NOTIFY_INFORMATION*)(((char*)info) + info->NextEntryOffset);
        }
    }
}
