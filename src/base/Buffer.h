#ifndef __YUX_BUFFER_H__
#define __YUX_BUFFER_H__

struct Buffer
{
    uint8_t  *bytes_;
    size_t   length_;

    Buffer() : bytes(0), length_(0);
    Buffer(size_t l) : length_(l) { bytes_ = new uint8_t[l]; }
    void reallocate(size_t l) { reset(); bytes_ = new uint8_t[l]; length_ = l; }
    void reset() { delete[] bytes_; bytes_ = nullptr; pos_ = length_ = 0; }
    void clear() { memset(bytes_, 0, length_); }
}

#endif
