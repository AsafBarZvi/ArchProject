#include <iostream>
#include "vq.h"
#include "defines.h"
#include "printers.hpp"
#include <map>
#include "trace.h"
#include <algorithm>


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
    return osObject << "tag:" <<  obj.tag() << ",val:" << obj.val(); 
}

std::ostream& operator <<(std::ostream& osObject, const Instruction & obj)
{
    return osObject << "op:"  <<  OP_names.at(obj.op) 
                    << ",d:"  <<  obj.dst
                    << ",s0:" <<  obj.src0
                    << ",s1:" <<  obj.src1
                    << ",i:"  <<  obj.imm;
}



std::ostream& operator <<(std::ostream& osObject , const std::map<std::string , TraceEntry> & obj)
{
    int pc_count = 0;
    while (true)
    {
        auto pc_iter = std::find_if(obj.begin(),
                                    obj.end(),
                                    [pc_count](std::pair<std::string , TraceEntry> ent){ return ent.second.pc = pc_count; } );

        if (pc_iter == obj.end())
            return osObject;

        osObject << pc_iter->first                                      << " " 
                 << pc_iter->second.pc                                  << " "
                 << pc_iter->second.tag                                 << " "
                 << pc_iter->second.cycle_issued                        << " "
                 << pc_iter->second.cycle_executed_start                << " "
                 << pc_iter->second.cycle_executed_end                  << " "
                 << pc_iter->second.cycle_write_cdb                     << "\n";


        pc_count++;
    }
}
