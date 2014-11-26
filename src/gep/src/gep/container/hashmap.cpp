#include "stdafx.h"
#include "gep/container/hashmap.h"

namespace
{
    inline unsigned int get16bits( const unsigned char* x )
    {
        return *reinterpret_cast<const unsigned short*>(x);
    }
}

unsigned int gep::hashOf( const void* buf, size_t len, unsigned int seed)
{
    //
    // This is Paul Hsieh's SuperFastHash algorithm, described here:
    //  http://www.azillionmonkeys.com/qed/hash.html
    // It is protected by the following open source license:
    //   http://www.azillionmonkeys.com/qed/weblicense.html
    //


    // NOTE: SuperFastHash normally starts with a zero hash value.  The seed
    //       value was incorporated to allow chaining.
    auto data = reinterpret_cast<const unsigned char*>(buf);
    auto hash = seed;
    int  rem;

    if( len <= 0 || data == nullptr )
    return 0;

    rem = len & 3;
    len >>= 2;

    for( ; len > 0; len-- )
    {
    hash += get16bits( data );
    auto tmp = (get16bits( data + 2 ) << 11) ^ hash;
    hash  = (hash << 16) ^ tmp;
    data += 2 * sizeof(unsigned short);
    hash += hash >> 11;
    }

    switch( rem )
    {
    case 3: hash += get16bits( data );
        hash ^= hash << 16;
        hash ^= data[sizeof(unsigned short)] << 18;
        hash += hash >> 11;
        break;
    case 2: hash += get16bits( data );
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1: hash += *data;
        hash ^= hash << 10;
        hash += hash >> 1;
        break;
    default:
        break;
    }

    // Force "avalanching" of final 127 bits
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}
