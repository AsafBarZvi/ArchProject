#pragma once

#include "vq.h"
#include "sync_block.h"
#include "defines.h"
#include <map>
#include <stdint.h>
#include <memory>


struct FuncTableEntry
{
    int  time = 0;
    bool busy = false;
    OP   op   = OP::INVALID;
    std::pair<VQ , VQ> VQS;

    //For Bookkeeping
    std::string tag = "";
    int dest  = -1;
    int imm   = -1;

    union result_t
    {
        uint32_t as_int;
        float    as_float;
    } result;

    FuncTableEntry* creator = NULL;

};

