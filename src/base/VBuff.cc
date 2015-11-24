#include "VBuff.h"

bool VBuff::bytesExist(size_t offset, size_t length) const
{
    return offset+length <= length_;
}

