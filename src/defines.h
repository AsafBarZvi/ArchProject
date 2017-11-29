#ifndef DEFINES_H
#define DEFINES_H

enum OP
{
    LD,
    ST,
    ADD,
    SUB,
    MULT,
    DIV,
    HALT,
    INVALID
};


struct Instruction
{
    OP  op  = OP::INVALID;
    int dst  = -1;
    int src0 = -1;
    int src1 = -1;
    int imm  = -1;
};

#endif
