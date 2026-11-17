
//   tape::on_tape(tape::FL)   true for front left tape
//   tape::raw(tape::FL)       raw reading
#pragma once

#include <cstdint>

namespace tape {

enum Sensor { BL = 0, BR = 1, FL = 2, FR = 3 };
constexpr int kCount = 4;

void init();
void update();

uint32_t raw(int sensor);

//true while tape - 2 tapes for confirm
bool on_tape(int sensor);

//true if any 1tape
bool any_on_tape();

uint32_t reading_count();

}  
