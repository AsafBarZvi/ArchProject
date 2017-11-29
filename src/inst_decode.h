#pragma once

#include "defines.h"
#include <stdint.h>


Instruction decode(uint32_t bin)
{
    Instruction inst = *(Instruction*)(&bin);
    return inst;
}
