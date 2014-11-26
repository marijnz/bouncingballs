#pragma once

#include "gep/gepmodule.h"
#include <stdio.h>

namespace gep
{

    /**
    * wrapper around C FILE for raw file handling
    */
    struct RawFile {
        FILE* m_pHandle;

        /**
        * opens a file
        * Params:
        *  pFilename = the filename
        *  pMode = the mode for the filestream
        */
        inline void open(const char* pFilename, const char* pMode)
        {
            m_pHandle = fopen(pFilename, pMode);
        }

        inline RawFile() : m_pHandle(nullptr) {}

        /**
        * constructor
        * Params:
        *  pFilename = the filename
        *  pMode = the mode for the filestream
        */
        inline RawFile(const char* pFilename, const char* pMode)
        {
            open(pFilename, pMode);
        }

        inline ~RawFile()
        {
            if(m_pHandle != nullptr)
            {
                fclose(m_pHandle);
                m_pHandle = nullptr;
            }
        }

        /**
        * Returns: if the file is open or not
        */
        inline bool isOpen() const {
            return (m_pHandle != nullptr);
        }

        /**
        * writes a value to the file as raw data
        * Params:
        *    value = the value to write
        * Returns: number of bytes written
        */
        template <typename T>
        inline size_t write(const T& value)
        {
            GEP_ASSERT(m_pHandle != nullptr);
            return fwrite(&value, sizeof(T), 1, m_pHandle) * sizeof(T);
        }

        /**
        * writes a array of values as raw data
        * Params:
        *  values = the values
        * Returns: number of bytes written
        */
        template <typename T>
        size_t writeArray(T* values, size_t length)
        {
            GEP_ASSERT(m_pHandle != nullptr);
            return fwrite(values, sizeof(T), length, m_pHandle) * sizeof(T);
        }

        /**
        * reads a value as raw data
        * Params:
        *  value = the value
        * Returns: number of bytes read
        */
        template <typename T>
        inline size_t read(T& value)
        {
            GEP_ASSERT(m_pHandle != nullptr);
            return fread(&value, sizeof(T), 1, m_pHandle) * sizeof(T);
        }

        /**
        * reads a value array as raw data
        * Params:
        *  values = the values
        * Returns: number of bytes read
        */
        template <typename T>
        inline size_t readArray(T* values, size_t length)
        {
            GEP_ASSERT(m_pHandle != nullptr);
            return fread(values, sizeof(T), length, m_pHandle) * sizeof(T);
        }

        /**
        * closes the file
        * $(BR) this is done automatically upon destruction of the wrapper object
        */
        inline void close()
        {
            if(m_pHandle != nullptr)
            {
                fclose(m_pHandle);
                m_pHandle = nullptr;
            }
        }

        /**
        * gets the size of the file
        */
        size_t getSize()
        {
            if( m_pHandle != nullptr)
            {
                auto cur = ftell(m_pHandle);
                fseek(m_pHandle, 0, SEEK_END);
                auto len = ftell(m_pHandle);
                fseek(m_pHandle, cur, SEEK_SET);
                return len;
            }
            return 0;
        }

        /**
        * gets the current position in the file
        */
        inline size_t position()
        {
            if(m_pHandle != nullptr)
            {
                return ftell(m_pHandle);
            }
            return 0;
        }

        /**
        * sets the current position in the file
        */
        inline void seek(size_t position)
        {
            if( m_pHandle != nullptr)
            {
                fseek(m_pHandle, static_cast<int>(position), SEEK_SET);
            }
        }

        /**
        * sets the position to the end of the file
        */
        inline void seekEnd()
        {
            if(m_pHandle != nullptr)
            {
                fseek(m_pHandle, 0, SEEK_END);
            }
        }

        /**
        * skips a given number of bytes
        */
        inline void skip(size_t bytes)
        {
            if(m_pHandle != nullptr)
            {
                fseek(m_pHandle, static_cast<int>(bytes), SEEK_CUR);
            }
        }

        inline bool eof()
        {
            if( m_pHandle != nullptr)
                return !!feof(m_pHandle);
            return true;
        }
    };

    /// \brief checks if the given file exists
    GEP_API bool fileExists(const char* pathToFile);
}
