#pragma once

#include "vq.h"
#include "sync_block.h"
#include "defines.h"
#include <stdint.h>
#include <memory>
#include "sync_block.h"

struct FuncTableEntry : public SyncBlockBase
{
    int  time = 0;
    bool busy = false;
    OP   op   = OP::INVALID;
    std::pair<VQ , VQ> VQS;

    //For Bookkeeping
    int  pc   = -1;
    std::string tag = "";
    Instruction instruction;

    union result_t
    {
        uint32_t as_int;
        float    as_float;
    } result;

    FuncTableEntry* creator = NULL;

    FuncTableEntry()
    {
        this->result.as_int = 0;
    }

    void clock()
    {
        this->VQS.first.clock();
        this->VQS.second.clock();
    }

};

