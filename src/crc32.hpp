/* CRC32 Implementation
 * 
 * Copyright (c) 2012-2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 04/22/2014 - Initial open source release
 * 04/23/2014 - Change return type of crc32()
 * 04/25/2014 - change len to const size_t
 *
 */

#ifndef __CRC32__
#define __CRC32__

#include <array>

class CRC32 {

public:
    CRC32(const uint32_t chunk_size);

    int64_t crc32(const std::string& filename) const;
    
private:
    uint32_t _crc32(uint32_t crc, const uint8_t *ptr, const size_t len) const;
    
    unsigned int _chunk;
    std::array<uint32_t, 256> _table;	
};

#endif
