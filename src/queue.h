
#pragma once

#include "sync_block.h"
#include "func_table.h"
#include <list>
#include <assert.h>



template< typename T>
class Queue : public SyncBlock< std::list< std::pair< std::string , T >  > >
{

    std::list< std::pair<std::string , T> > queue_;
    std::list< std::pair<std::string , T> > update_;
    std::pair< std::string , T >            null = std::make_pair("",T());
    int size ;
    int id  = 0;


public:
    Queue(int size)
    : size(size)
    {}

    std::list< std::pair< std::string , T > > & write() { return update_; }
    const std::list< std::pair< std::string , T > > & read() { return queue_; }
    void clock() { this->queue_ = this->update_ ; }

    std::string push(const T& inst , const std::string tag = "")
    {
        assert(this->update_.size() <= this->size);
        id++;
        id = id % this->size;
        if (!tag.empty())
        {
            std::string ntag = tag + std::to_string(id);
            this->write().push_back( std::make_pair(ntag,  inst) );
            return ntag;
        }
        else
            this->write().push_back( std::make_pair("",  inst) );
        
        return "";
    }


    bool is_full() {return this->update_.size() == size ; }

    bool is_half_full() { return this->update_.size() > size/2 ;} 


    const std::pair<std::string , T> & peek()
    {
        if (this->queue_.empty())
            return null;
        return this->read().front();
    }

    void pop()
    {
        if (!this->write().empty())
            this->write().pop_front();
        if (!this->read().empty())
            this->read().pop_front();
    }

};


template< typename T>
class AsyncQueue 
{

    std::list< T > queue_;
    T                                       null = T();
    int size ;
    int id = 0;


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







