#ifndef __YUX_BUFFER_H__
#define __YUX_BUFFER_H__

namespace yux{
namespace base{

struct Buffer
{
    uint8_t  *bytes_;
    size_t   length_;

    Buffer() : bytes_(0), length_(0);
    Buffer(uint8_t bytes, size_t length) : bytes_(bytes), length_(length);
    ~Buffer() { delete[] bytes_; }
    uint8_t* get() { return bytes_; }
    Buffer(size_t l) : length_(l) { bytes_ = new uint8_t[l]; }
    void reallocate(size_t l) { reset(); bytes_ = new uint8_t[l]; length_ = l; }
    void reset() { delete[] bytes_; bytes_ = nullptr; pos_ = length_ = 0; }
    void clear() { memset(bytes_, 0, length_); }
}

}}
#endif
