
#pragma once

#include "sync_block.h"
#include "func_table.h"
#include <list>
#include <assert.h>
#include <algorithm>





template< typename T>
class AsyncQueue 
{

protected:
    std::list< T > queue_;
    T                                       null = T();
    int size ;
    int id = -1;


public:
    AsyncQueue(int size)
    : size(size)
    {}

    std::string push(const T& inst , const std::string & tag)
    {
        assert(this->queue_.size() <= this->size);
        id++;
        id = id % this->size;
        if (!tag.empty())
        {
            std::string ntag = tag + std::to_string(id);
            this->queue_.push_back(  inst );
            this->queue_.back().tag = ntag;
            return ntag;
        }
        
        this->queue_.push_back( inst );
        return "";
        
    }
    
    void push(const T& inst)
    {
        assert(this->queue_.size() <= this->size);
        this->queue_.push_back(  inst );
        
    }

    bool is_empt() {return this->queue_.size() == 0 ; }

    bool is_full() {return this->queue_.size() == size ; }

    bool is_half_full() { return this->queue_.size() > size/2 ;}

    std::list< T > &  get_queue() { return this->queue_; }

    T & peek()
    {
        if (this->queue_.empty())
            return null;
        return this->queue_.front();
    }

    void pop()
    {
        if (!this->queue_.empty())
            this->queue_.pop_front();
    }

};



template< typename T>
class Queue : public AsyncQueue< T > ,
              public SyncBlockBase

{


public:
    Queue(int size)
    : AsyncQueue<T>(size)
    {}

    void clock() {
        for (auto & entry : this->queue_)
            entry.clock();
    }


};




