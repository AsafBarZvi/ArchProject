

#pragma once

template<typename T>
class SyncBlock
{
public:
    virtual const T& read()  = 0;
    virtual T&       write() = 0;
    virtual void     clock() = 0;
};

