#include <iostream>
#include "vq.h"
#include "defines.h"
#include "printers.hpp"
#include <map>
#include "trace.h"
#include <algorithm>
#include "register.h"


const std::map<int , std::string> OP_names =
{ 
    { 0 , "LD"},
    { 1 , "ST"},
    { 2 , "ADD"},
    { 3 , "SUB"},
    { 4 , "MULT"},
    { 5 , "DIV"},
    { 6 , "HALT"},
};




std::ostream& operator <<(std::ostream& osObject, const VQ& obj)
{
    if (obj.is_ready())
        osObject << obj.val();
    else
        osObject << obj.tag();

    return osObject;
}

std::ostream& operator <<(std::ostream& osObject, const Instruction & obj)
{
    return osObject << "op:"  <<  OP_names.at(obj.op) 
                    << ",d:"  <<  obj.dst
                    << ",s0:" <<  obj.src0
                    << ",s1:" <<  obj.src1
                    << ",i:"  <<  obj.imm;
}


std::ostream& operator <<(std::ostream& osObject, const Register & obj)
{
    int offset = 0;
    for (const auto & reg : const_cast<Register*>(&obj)->read())
    {
        osObject << "F" << offset << ":" << reg << "\n";
        offset ++;
    }

    return osObject;
}

std::ostream& operator <<(std::ostream& osObject, const std::vector<uint32_t> & obj)
{
    for (const auto &e : obj)
    {
        std::stringstream converter;
        converter << std::setw(8) << std::setfill('0') << std::hex << e;
        osObject << converter.str() << "\n";
    }

    return osObject;
}

std::ostream& operator <<(std::ostream& osObject , const std::map<int , TraceEntry> & obj)
{
    for (auto & pc_iter : obj)
    {
        if (pc_iter.second.cycle_issued == -1)
            continue;

        osObject << pc_iter.second.inst_hex                            << " " 
                 << pc_iter.second.pc                                  << " "
                 << pc_iter.second.tag                                 << " "
                 << pc_iter.second.cycle_issued                        << " "
                 << pc_iter.second.cycle_executed_start                << " "
                 << pc_iter.second.cycle_executed_end                  << " "
                 << pc_iter.second.cycle_write_cdb                     << "\n";

    }

    return osObject;
}

std::ostream& operator <(std::ostream& osObject , const std::map<int , TraceEntry> & obj)
{
    for (auto & pc_iter : obj)
    {
        if (pc_iter.second.cycle_issued == -1)
            continue;

        std::string cdbname = pc_iter.second.tag;
        for(int i = 0; i < cdbname.size(); ++i)
            if (!((cdbname[i] >= 'A' && cdbname[i]<='z') || (cdbname[i] >= 'A' && cdbname[i]<='Z')))
                cdbname[i] = '\0';
        
        if (cdbname.length() == 3)
            cdbname = std::string("MEM");

        osObject << pc_iter.second.cycle_write_cdb                     << " " 
                 << pc_iter.second.pc                                  << " "
                 << cdbname                                            << " "
                 << pc_iter.second.data                                << " "
                 << pc_iter.second.tag                                 << "\n";

    }

    return osObject;
}
