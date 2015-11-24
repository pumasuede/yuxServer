struct Buffer
{
    uint8_t  bytes_;
    size_t   length_;
    size_t   pos_;

    Buffer() : bytes(0), length_(0), pos_(0);
    Buffer(size_t l) : length_(l), pos_(0) { bytes_ = new uint8_t[l]; }
    void reallocate(size_t l) { reset(); bytes_ = new uint8_t[l]; length_ = l; pos = 0; }
    void reset() { delete[] bytes_; bytes_ = NULL; pos_ = length_ = 0;}
    void clear() { memset(bytes_, 0, length_); }
}
