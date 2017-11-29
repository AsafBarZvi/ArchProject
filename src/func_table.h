#ifndef FUNC_TABLE_H
#define FUNC_TABLE_H


#include "vq.h"
#include "sync_block.h"
#include "defines.h"
#include <map>


struct FuncTableEntry
{
    int  time = 0;
    bool busy = false;
    OP   op   = OP::INVALID;
    std::pair<VQ , VQ> VQS;
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

#endif
