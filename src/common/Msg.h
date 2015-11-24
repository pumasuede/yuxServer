namespace yux
{
namespace common
{

class Msg
{
    public:
        Msg() : hdrBase_(NULL), hdrLen_(0), bdyLen_(0); msgType(0), bytes_(NULL);

    private:
        void     *hdrBase_;
        size_t   hdrLen_;
        size_t   bdyLen_;
        size_t   msgType_;
        uint8_t  *bytes_;
};

}}

