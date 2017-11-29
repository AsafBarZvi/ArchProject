

#pragma once


class SyncBlockBase
{
public:
    virtual void     clock() = 0;
    virtual bool     is_busy() {return false;}
};


template<typename T>
class SyncBlock : public SyncBlockBase
{
public:
    virtual const T& read()  = 0;
    virtual T&       write() = 0;
    virtual void     clock() = 0;
    virtual bool     is_busy() {return false;}
};

