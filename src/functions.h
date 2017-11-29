#include "func_table.h"
#include <string>
#include "vq.h"
#include <assert.h>

class BaseFunction : public SyncBlock <FuncTableEntry>
{

protected:
    int delay;
    std::string tag;
    int time = -1;

    FuncTableEntry cmd;
    FuncTableEntry result;
    FuncTableEntry null;

    virtual const FuncTableEntry& op() = 0;

public:

    BaseFunction(const std::string& tag ,int delay)
    : delay(delay)
    , tag(tag)
    {}

    bool is_busy() { return time != -1;}

    FuncTableEntry&  write()
    {
        assert(!is_busy());
        time = delay;
        return cmd;
    }

    const FuncTableEntry& read()
    {
        if (time == 0)
        {
            time = -1;
            return op();
        }
        if (time > 0 || time ==-1)
        {
            return null;
        }
    }

   
    void clock()
    {
        if (time > 0 )
        {
            time--;
        }
    }

};


class Add : public BaseFunction
{
    const FuncTableEntry& op()
    {
        assert(cmd.op == OP::ADD || cmd.op == OP::SUB);
        assert(cmd.VQS.first.is_ready() && cmd.VQS.second.is_ready());
        if (cmd.op == OP::ADD)
        {
            this->result.VQS.first.set_val( cmd.VQS.first.val() + cmd.VQS.second.val() );
            return this->result;
        }
        if (cmd.op == OP::SUB)
        {
            this->result.VQS.first.set_val( cmd.VQS.first.val() - cmd.VQS.second.val() );
            return this->result;
        }

    }

};


class Mult : public BaseFunction
{
    const FuncTableEntry& op()
    {
        assert(cmd.op == OP::MULT);
        assert(cmd.VQS.first.is_ready() && cmd.VQS.second.is_ready());
        this->result.VQS.first.set_val( cmd.VQS.first.val() * cmd.VQS.second.val() );
        return this->result;
    }

};

class Div : public BaseFunction
{
    const FuncTableEntry& op()
    {
        assert(cmd.op == OP::DIV);
        assert(cmd.VQS.first.is_ready() && cmd.VQS.second.is_ready());
        this->result.VQS.first.set_val( cmd.VQS.first.val() / cmd.VQS.second.val() );
        return this->result;
    }

};
