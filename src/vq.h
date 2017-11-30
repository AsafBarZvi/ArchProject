
#pragma once

#include <string>
#include "sync_block.h"

class VQ : public SyncBlock < std::pair<std::string , float> > 
{
    std::pair <std::string , float > current;
    std::pair <std::string , float > update;

    const std::pair <std::string , float >& read() { return this->current; } 
    std::pair <std::string , float >& write()      { return this->update; }

public:

    VQ()
    {
        this->current.first  = "none";
        this->current.second = -1;
        this->update.first  = "none";
        this->update.second = -1;
    }
    
    
    void clock() { this->current = this->update; }
    bool is_ready() const { return const_cast<VQ*>(this)->read().first.empty();}
    void set_tag(const std::string & ntag) { this->write().first = ntag ; this->write().second = -1;}
    void set_val(const float val) { this->write().first = ""; this->write().second = val; }
    float val() const { return const_cast<VQ*>(this)->read().second; }
    const std::string& tag() const {return const_cast<VQ*>(this)->read().first;}
};
