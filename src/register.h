
#pragma once

#include <vector>
#include "sync_block.h"
#include "vq.h"

class Register : public SyncBlock< std::vector<VQ> >
{

    std::vector<VQ> current;
    std::vector<VQ> update;

public:
    Register(int size)
    {
        this->current.resize(size);
        this->update.resize(size);
        for (int i = 0 ; i < this->current.size() ; i++)
            this->current.at(i).set_val(i);

        this->update = this->current;
    }


    void clock()
    {
        this->current = this->update;
    }


    const std::vector<VQ> &  read() { return this->current;}
    std::vector<VQ> & write() { return this->update; }

};
