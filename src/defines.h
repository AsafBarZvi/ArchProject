
#pragma once


enum OP
{
    LD,
    ST,
    ADD,
    SUB,
    MULT,
    DIV,
    HALT,
    INVALID,
    DONE
};


struct Instruction
{
    unsigned int imm  : 12;
    unsigned int src1 : 4;
    unsigned int src0 : 4;
    unsigned int dst  : 4;
    unsigned int op   : 4;
    unsigned int reserved : 4;


    Instruction()
    : imm(0xFFF)
    , src1(0xF)
    , src0(0xF)
    , dst(0xF)
    , op(0x7) // INVALID
    , reserved(0x0)
    {}

} __attribute__((packed));



