#include <stdint.h>
#include <string>

struct Event
{
    Event (uint32_t id, const char* desc) : id_(id), desc_(desc) {}
    uint32_t id_;
    std::string desc_;
};

typedef enum
{
    EVENT_1 = 1,  // 1
    EVENT_2       // 2
} enumEvent;
