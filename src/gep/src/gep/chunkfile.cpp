#include "stdafx.h"
#include "gep/chunkfile.h"

gep::Chunkfile::Chunkfile(const char* filename, Operation operation)
{
      m_filename = filename;
      m_operation = operation;
      m_readLocation = nullptr;
      switch(m_operation)
      {
      case Operation::read:
          m_file.open(filename, "rb");
          break;
      case Operation::write:
          m_file.open(filename, "wb");
          break;
      case Operation::modify:
          m_file.open(filename, "rb");
          m_oldData = GEP_NEW_ARRAY(g_stdAllocator, uint8, m_file.getSize());
          m_readLocation = m_oldData.getPtr();
          m_file.readArray(m_oldData.getPtr(), m_oldData.length());
          m_file.close();
          m_file.open(filename, "wb");
          break;
      }
}

gep::Chunkfile::~Chunkfile()
{
    GEP_ASSERT(m_readInfo.length() == 0, "there are still chunks open for reading");
    GEP_ASSERT(m_writeInfo.length() == 0, "there are still chunks open for writing");
    GEP_DELETE_ARRAY(g_stdAllocator, m_oldData);
}

void gep::Chunkfile::discardChanges()
{
    GEP_ASSERT(m_operation == Operation::modify, "discarding only possible when modifying");
    m_file.close();
    m_file.open(m_filename.c_str(), "wb");
    m_file.writeArray(m_oldData.getPtr(), m_oldData.length());
    m_file.close();
}

gep::Result gep::Chunkfile::startReadChunk()
{
    Result result = FAILURE;
    SCOPE_EXIT
    {
        if(result != SUCCESS && m_operation == Operation::modify)
            discardChanges();
    });
    GEP_ASSERT(m_operation != Operation::write, "opening a existing chunk is not possible in write mode");
    ChunkReadInfo info;

    //read the chunk name
    if(read(info.nameLength) != 1 || info.nameLength > MAX_CHUNK_NAME_LENGTH)
    {
        return FAILURE;
    }
    if(readArray(ArrayPtr<char>(info.name, info.nameLength)) != info.nameLength)
        return FAILURE;

    //read the chunk length
    if(read(info.bytesLeft) != sizeof(info.bytesLeft))
        return FAILURE;

    if(m_readInfo.length() > 0)
    {
        if(m_readInfo.lastElement().bytesLeft < info.bytesLeft)
        {
            GEP_ASSERT(0, "inner chunk is to long");
            return FAILURE;
        }
        m_readInfo.lastElement().bytesLeft -= info.bytesLeft;
    }

    m_readInfo.append(info);

    result = SUCCESS;
    return SUCCESS;
}

void gep::Chunkfile::endReadChunk()
{
    GEP_ASSERT(m_readInfo.length() > 0, "no chunk to end");
    GEP_ASSERT(m_readInfo.lastElement().bytesLeft == 0, "there is still data left in the chunk");
    m_readInfo.removeLastElement();
}

void gep::Chunkfile::startWriteChunk(const char* name)
{
    GEP_ASSERT(strlen(name) <= MAX_CHUNK_NAME_LENGTH, "chunk name is to long");
    writeArrayWithLength<char, uint8>(ArrayPtr<char>((char*)name, strlen(name)));
    ChunkWriteInfo info;
    info.lengthPosition = m_file.position();
    write<uint32>(0);
    m_writeInfo.append(info);
}

size_t gep::Chunkfile::endWriteChunk()
{
    GEP_ASSERT(m_writeInfo.length() > 0, "there is no chunk to end");
    auto length = m_writeInfo.lastElement().length;
    m_file.seek(m_writeInfo.lastElement().lengthPosition);
    m_file.write<uint32>(static_cast<uint32>(length));
    m_file.seekEnd();
    m_writeInfo.removeLastElement();
    if(m_writeInfo.length() > 0)
        m_writeInfo.lastElement().length += length;
    return length;
}

void gep::Chunkfile::keepRestOfCurrentChunk()
{
    GEP_ASSERT(m_operation == Operation::modify, "can only keep chunks in modifiy operation");
    ptrdiff_t bytesRemaining = (m_oldData.getPtr() + m_oldData.length()) - m_readLocation;
    GEP_ASSERT(bytesRemaining >= 0, "privous read did go out of bounds");
    if(bytesRemaining > 0)
    {
        m_file.writeArray(m_readLocation, bytesRemaining);
    }
    endWriteChunk();
}

void gep::Chunkfile::skipRead(size_t bytes)
{
    GEP_ASSERT(m_operation != Operation::write, "can not read in write operation");
    GEP_ASSERT(m_readInfo.length() == 0 || m_readInfo.lastElement().bytesLeft >= bytes, "reading over chunk boundary");
    if(m_operation == Operation::read)
    {
        m_file.skip(bytes);
    }
    else
    {
        GEP_ASSERT(m_readLocation + bytes <= m_oldData.getPtr() + m_oldData.length(), "out of bounds");
        m_readLocation += bytes;
    }
    if(m_readInfo.length() > 0)
        m_readInfo.lastElement().bytesLeft -= (uint32)bytes;
}

void gep::Chunkfile::skipCurrentChunk()
{
    GEP_ASSERT(m_operation != Operation::write, "can not skip chunks in write operation");
    if(m_operation == Operation::read)
    {
        m_file.skip(m_readInfo.lastElement().bytesLeft);
        m_readInfo.lastElement().bytesLeft = 0;
        endReadChunk();
    }
    else
    {
        m_readLocation += m_readInfo.lastElement().bytesLeft;
        m_readInfo.lastElement().bytesLeft = 0;
        endReadChunk();
    }
}

void gep::Chunkfile::startWriting(const char* filetype, uint32 ver)
{
    GEP_ASSERT(m_operation != Operation::read, "can't write in reading operation");
    GEP_ASSERT(ver > 0, "version has to be greater then 0");
    startWriteChunk(filetype);
    write(ver);
    write<uint32>(1); // DebugMode = off
}

void gep::Chunkfile::endWriting()
{
    GEP_ASSERT(m_operation != Operation::read, "can't write in reading operation");
    GEP_ASSERT(m_writeInfo.length() == 1, "there is still more then 1 chunk open");
    endWriteChunk();
}

gep::Result gep::Chunkfile::startReading(const char* filetype)
{
    GEP_ASSERT(m_operation != Operation::write, "can't read in write operation");
    if( startReadChunk() != SUCCESS )
    {
        return FAILURE;
    }
    if( getCurrentChunkName() != filetype)
    {
        skipCurrentChunk();
        return FAILURE;
    }
    if( read(m_version) < sizeof(m_version) )
    {
        skipCurrentChunk();
        return FAILURE;
    }
    if( m_version == 0 )
    {
        GEP_ASSERT(0, "invalid version in chunk file");
        skipCurrentChunk();
        return FAILURE;
    }
    uint32 debugMode;
    if( read(debugMode) < sizeof(debugMode) )
    {
        skipCurrentChunk();
        return FAILURE;
    }
    if( debugMode != 1 )
    {
        GEP_ASSERT(0, "this implementation does only support DebugMode = off");
        skipCurrentChunk();
        return FAILURE;
    }
    return SUCCESS;
}

void gep::Chunkfile::endReading()
{
    endReadChunk();
}
