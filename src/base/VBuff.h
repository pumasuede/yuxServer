#include <stdint.h>
#include <iostream>
class VBuff
{
    public:

    VBuff() : bytes_(NULL), length_(0) { }
    VBuff(const uint8_t *bytes, size_t length) : bytes_(bytes), length_(length) { }
    VBuff(const VBuff& v, size_t offset, size_t length) : bytes_(v.bytes_+offset), length_(v.length_) { }
    VBuff(const VBuff& v) : bytes_(v.bytes_), length_(v.length_) {}
    VBuff& operator=(const VBuff& v) //assignment operator
    {
        if (this == &v)
        {
            return *this;
        }
        bytes_ = v.bytes_;
        length_ = v.length_;

        return *this;
    }


    size_t length() const { return length_; }

    bool bytesExist(size_t offset, size_t length) const; 

    private:
    const uint8_t *bytes_;
    size_t length_;
};
