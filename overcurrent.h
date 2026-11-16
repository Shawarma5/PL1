#pragma once
#include <cstdint>


namespace oc {


void init();

//true when allowed, false when not
bool motors_allowed();

//tripcount
uint32_t trip_count();

}  
