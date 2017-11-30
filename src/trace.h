#pragma once

#include <map>
#include <stdint.h>

struct TraceEntry
{
    int pc = -1;
    std::string tag = "";
    int cycle_issued = -1;
    int cycle_executed_start = -1;
    int cycle_executed_end   = -1;
    int cycle_write_cdb      = -1;
};

extern std::map < std::string , TraceEntry> InstructionTrace;

extern int CLOCK;

#define IT InstructionTrace
