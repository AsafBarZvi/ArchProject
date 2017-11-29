#pragma once

#include "vq.h"
#include "sync_block.h"
#include "defines.h"
#include <map>
#include <stdint.h>

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

};


class FuncTable : public SyncBlock< std::map< std::string , FuncTableEntry> >
{
    std::map< std::string , FuncTableEntry> current_;
    std::map< std::string , FuncTableEntry> update_;

public:

    const std::map< std::string , FuncTableEntry>& read() { return this->current_;}
    std::map< std::string , FuncTableEntry>& write() {return this->update_;}
    void clock() { this->current_ = this->update_ ; }

};

