#pragma once

#include "func_table.h"
#include <string>
#include "vq.h"
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdint.h>


class BaseFunction : public SyncBlock <FuncTableEntry>
{

protected:
    int delay;
    int time = -1;

    FuncTableEntry cmd;
    FuncTableEntry new_cmd;
    FuncTableEntry result;
    FuncTableEntry null;

    virtual const FuncTableEntry& op() = 0;

public:

    BaseFunction(int delay)
    : delay(delay)
    {}

    bool is_busy() { return time != -1;}

    FuncTableEntry&  write()
    {
        assert(!is_busy());
        time = delay;
        return new_cmd;
    }

    const FuncTableEntry& read()
    {
        if (time == 0)
        {
            time = -1;
            return op();
        }

        return null;
    }

   
    void clock()
    {
        cmd = new_cmd;
        if (time > 0 )
        {
            time--;
        }
    }

};


class Add : public BaseFunction
{
    using BaseFunction::BaseFunction;
    const FuncTableEntry& op()
    {
        assert(cmd.op == OP::ADD || cmd.op == OP::SUB);
        assert(cmd.VQS.first.is_ready() && cmd.VQS.second.is_ready());
        if (cmd.op == OP::ADD)
        {
            this->result.result.as_float = cmd.VQS.first.val() + cmd.VQS.second.val() ;
            this->result.op = OP::DONE;
            return this->result;
        }
        if (cmd.op == OP::SUB)
        {
            this->result.result.as_float = cmd.VQS.first.val() - cmd.VQS.second.val() ;
            this->result.op = OP::DONE;
            return this->result;
        }

        return null;
    }

};


class Mult : public BaseFunction
{
    using BaseFunction::BaseFunction;
    const FuncTableEntry& op()
    {
        assert(cmd.op == OP::MULT);
        assert(cmd.VQS.first.is_ready() && cmd.VQS.second.is_ready());
        this->result.result.as_float = cmd.VQS.first.val() * cmd.VQS.second.val();
        this->result.op = OP::DONE;
        return this->result;
    }

};

class Div : public BaseFunction
{
    using BaseFunction::BaseFunction;
    const FuncTableEntry& op()
    {
        assert(cmd.op == OP::DIV);
        assert(cmd.VQS.first.is_ready() && cmd.VQS.second.is_ready());
        this->result.result.as_float = cmd.VQS.first.val() / cmd.VQS.second.val();
        this->result.op = OP::DONE;
        return this->result;
    }

};


struct MemAccess
{
    std::vector<FuncTableEntry> port;
    
    MemAccess()
    {
        this->port.resize(3); //2 Fetch Instruciton ports + 1 Data port    
    }
};



class Memory : public SyncBlock < MemAccess >  
{

    class PipeDelay : public SyncBlock < MemAccess >
    {
        MemAccess current_;
        MemAccess update_;

    public:

        MemAccess& write() { return update_;}
        const MemAccess& read() { return current_;}
        void clock() { this->current_ = this->update_ ; }
    };


    std::vector<uint32_t>   mem_cache_;
    std::vector<PipeDelay>  pipe_delay_;
    bool                    new_request = false;


public:
    Memory(const std::string mem_file , int delay)
    {
        mem_cache_.resize(4096,0);
        std::ifstream file("mem_file.txt");
        std::string str;
        std::stringstream converter;
        while (std::getline(file , str))
        {
            uint32_t temp;
            converter << std::hex << str;
            converter >> temp;
            mem_cache_.push_back( temp );
        }

        this->pipe_delay_.resize(delay);
    }


    MemAccess& write() { this->new_request = true ; this->pipe_delay_.back().write() = MemAccess() ; return this->pipe_delay_.back().write(); }
    const MemAccess& read() {return this->pipe_delay_.front().read(); }
    


    void clock()
    {
        if (this->new_request)
            this->new_request = false;
        else
        {
            this->pipe_delay_.back().write() = MemAccess();
        }
        
        for (auto & cmd :  this->pipe_delay_.front().write().port)
        {
            assert(cmd.op == OP::ST || cmd.op == OP::LD || cmd.op == OP::INVALID);
            if (cmd.op == OP::ST)
            {
                assert(cmd.VQS.second.is_ready() && cmd.imm != -1 );
                this->mem_cache_.at(cmd.imm) = cmd.VQS.second.val();
                cmd.op = OP::DONE;
            }

            if (cmd.op == OP::LD)
            {
                assert(cmd.dest != -1 && cmd.imm != -1);
                cmd.op = OP::DONE;
                cmd.result.as_int = this->mem_cache_.at(cmd.imm);
            }
        }

        for (auto & duint : this->pipe_delay_)
            duint.clock();

        for (int i =1 ; i < this->pipe_delay_.size() ; i++)
            this->pipe_delay_.at(i).write() = this->pipe_delay_.at(i-1).read();

    }

};
