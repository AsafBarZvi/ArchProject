#include <iostream>
#include "vq.h"
#include "defines.h"
#include "printers.hpp"
#include <map>


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
