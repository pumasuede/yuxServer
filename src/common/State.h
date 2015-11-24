#include <stdint.h>
#include <string>
struct State
{
	uint32_t id;
	std::string desc;
};

typedef enum 
{
	STATE_1 , // 1
	STATE_2 ,  // 2
	STATE_3   // 2
} enumState;
