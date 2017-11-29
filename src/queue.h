
#pragma once

#include "sync_block.h"
#include "func_table.h"
#include <list>
#include <assert.h>



template< typename T>
class Queue : public SyncBlock< std::list< T > >
{

    std::list< T > queue_;
    std::list< T > update_;
    int size ;


public:
    Queue(int size)
    : size(size)
    {}

    std::list< T > & write() { return update_; }
    const std::list < T > & read() { return queue_; }
    void clock() { this->queue_ = this->update_ ; }

    void push(const T& inst)
    {
        assert(this->update_.size() <= this->size);
        this->write().push_back(inst);
    }


    bool is_full() {return this->update_.size() == size ; }


    const T& peek()
    {
        return this->read().front();
    }

    void pop()
    {
        this->write().pop_front();
        this->read().pop_front();
    }

};



