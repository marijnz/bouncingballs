#pragma once

#include <Windows.h>
#include <gep/types.h>
#include <functional>

namespace gep
{
    class DirectoryWatcher
    {
    public:
        struct Watch
        {
            enum Enum
            {
              reads =  1 << 0,
              writes = 1 << 1,
              creates = 1 << 2,
              renames = 1 << 3
            };
        };

        struct WatchSubdirs
        {
            enum Enum
            {
              no = 0,
              yes = 1
            };
        };

        struct Action
        {
            enum Enum
            {
              added = FILE_ACTION_ADDED,
              removed = FILE_ACTION_REMOVED,
              modified = FILE_ACTION_MODIFIED,
              renamedOldName = FILE_ACTION_RENAMED_OLD_NAME,
              renamedNewName = FILE_ACTION_RENAMED_NEW_NAME
            };
        };

    private:
        HANDLE m_directoryHandle;
        HANDLE m_completionPort;
        WatchSubdirs::Enum m_watchSubdirs;
        DWORD m_filter;
        OVERLAPPED m_overlapped;
        char m_buffer[4096];

        void doRead();

    public:
        /// \brief constructor
        /// \param path
        ///   the path to watch
        /// \param watchSubdirs
        ///   if the subdirectoriers should be watched or not
        /// \param watch
        ///   a combination of Watch::Enum flags or 0
        DirectoryWatcher(const char* path, WatchSubdirs::Enum watchSubdirs, uint32 watch);

        ~DirectoryWatcher();

        /// \brief interates over all changes that occured since the last iteration
        /// \param func
        ///    the callback function to call for each change
        void enumerateChanges(std::function<void(const char* filename, Action::Enum action)> func);
    };
}
