#pragma once

#include "gep/memory/allocator.h"
#include "gep/file.h"
#include "gep/container/DynamicArray.h"
#include "gep/traits.h"

namespace gep
{
    class GEP_API Chunkfile
    {
    public:
        enum class Operation
        {
            read,
            write,
            modify
        };

    private:
        Operation m_operation;
        ArrayPtr<uint8> m_oldData;
        uint8* m_readLocation;
        RawFile m_file;
        std::string m_filename;
        uint32 m_version;

        static const uint32 MAX_CHUNK_NAME_LENGTH = 27;

        struct ChunkReadInfo
        {
            char name[MAX_CHUNK_NAME_LENGTH];
            uint8 nameLength;
            uint32 bytesLeft;

            ChunkReadInfo() : nameLength(0), bytesLeft(0) {}
        };

        struct ChunkWriteInfo
        {
            size_t lengthPosition;
            size_t length;

            ChunkWriteInfo() : lengthPosition(0), length(0) {}
        };

        DynamicArray<ChunkReadInfo> m_readInfo;
        DynamicArray<ChunkWriteInfo> m_writeInfo;

    public:
        /// \brief creates or opens a chunkfile
        Chunkfile(const char* filename, Operation operation);
        ~Chunkfile();

        inline Operation getOperation() const
        {
            return m_operation;
        }

        void skipRead(size_t bytes);

        template <typename T>
        size_t read(T& val)
        {
            static_assert(isArrayPtr<T>::value == false, "for reading arrays use readArray");
            GEP_ASSERT(m_operation != Operation::write, "can not read in write operation");
            GEP_ASSERT(m_readInfo.length() == 0 || m_readInfo.lastElement().bytesLeft >= sizeof(T), "reading over chunk boundary");
            size_t size = 0;
            if(m_operation == Operation::read)
            {
                size = m_file.read(val);
            }
            else
            {
                GEP_ASSERT(m_readLocation + sizeof(T) <= m_oldData.getPtr() + m_oldData.length(), "out of bounds");
                val = *(T*)(m_readLocation);
                size = sizeof(T);
                m_readLocation += sizeof(T);
            }
            if(m_readInfo.length() > 0)
                m_readInfo.lastElement().bytesLeft -= (uint32)size;
            return size;
        }

        template <typename T>
        size_t readArray(ArrayPtr<T> val)
        {
            GEP_ASSERT(m_operation != Operation::write, "can not read in write operation");
            GEP_ASSERT(m_readInfo.length() == 0 || m_readInfo.lastElement().bytesLeft >= sizeof(T) * val.length(), "reading over chunk boundary");

            size_t size;
            if(m_operation == Operation::read)
            {
                size = m_file.readArray(val.getPtr(), val.length());
            }
            else
            {
                if(m_readLocation + sizeof(T) * val.length() <= m_oldData.getPtr() + m_oldData.length())
                {
                    GEP_ASSERT(false, "out of bounds");
                    return 0;
                }
                val.copyFrom(ArrayPtr<T>((T*)m_readLocation, val.length()));
                size = sizeof(T) * val.length();
                m_readLocation += size;
            }
            if(m_readInfo.length() > 0)
                m_readInfo.lastElement().bytesLeft -= (uint32)size;
            return size;
        }

        /// \brief Allocates and reads a array of a given type from the chunk file
        /// \param pAllocator
        ///   the allocator to use
        ///
        /// \return the correctly initialized array or an empty array on error
        template <typename T, typename SizeType>
        ArrayPtr<T> readAndAllocateArray(IAllocator* pAllocator)
        {
            SizeType len = 0;
            if( read(len) != sizeof(SizeType))
                return ArrayPtr<T>();
            if(len <= 0)
                return ArrayPtr<T>();
            ArrayPtr<T> data((T*)pAllocator->allocateMemory(sizeof(T) * len), len);
            if( readArray(data) != sizeof(T) * len )
            {
                pAllocator->freeMemory(data.getPtr());
                return ArrayPtr<T>();
            }
            return data;
        }

        template <typename T, typename SizeType>
        size_t writeArrayWithLength(ArrayPtr<T> data)
        {
            size_t size = write(static_cast<SizeType>(data.length()));
            size += writeArray(data);
            return size;
        }

        template <typename T>
        size_t write(const T& val)
        {
            static_assert(isArrayPtr<T>::value == false, "for writing arrays use writeArray");
            GEP_ASSERT(m_operation != Operation::read, "can not write in read operation");
            size_t size = m_file.write(val);
            GEP_ASSERT(size == sizeof(T), "writing failed");
            if(m_writeInfo.length() > 0)
                m_writeInfo.lastElement().length += size;
            return size;
        }

        template <class T>
        size_t writeArray(ArrayPtr<T> val)
        {
            GEP_ASSERT(m_operation != Operation::read, "can not write in read operation");
            size_t size = m_file.writeArray(val.getPtr(), val.length());
            GEP_ASSERT(size == sizeof(T) * val.length(), "writing failed");
            if(m_writeInfo.length() > 0)
                m_writeInfo.lastElement().length += size;
            return size;
        }

        void discardChanges();
        Result startReadChunk();

        inline bool currentChunkHasMoreData() const
        {
            GEP_ASSERT(m_readInfo.length() > 0, "no chunk is open");
            return m_readInfo.lastElement().bytesLeft > 0;
        }

        inline std::string getCurrentChunkName() const
        {
            GEP_ASSERT(m_readInfo.length() > 0, "no chunk is open");
            auto& el = m_readInfo.lastElement();
            return std::string(&el.name[0], el.nameLength);
        }

        inline uint32 getFileVersion() const
        {
            return m_version;
        }

        void endReadChunk();

        void startWriteChunk(const char* name);
        size_t endWriteChunk();

        /**
        * keeps the rest of the current chunk and ends the chunk
        */
        void keepRestOfCurrentChunk();

        /**
        * skips the rest of the current chunk and ends it
        */
        void skipCurrentChunk();

        void startWriting(const char* filetype, uint32 ver);

        void endWriting();

        Result startReading(const char* filetype);

        void endReading();
    };
}
