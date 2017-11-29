#include "queue.h"
#include "defines.h"
#include "func_table.h"

const int add_nr_reservation = 2;
const int mul_nr_reservation = 2;
const int div_nr_reservation = 2;


int main(int argc , char ** argv)
{
   Queue<Instruction> inst_queue(16);
   Queue<FuncTableEntry>  adders_reservatory(add_nr_reservation);
   Queue<FuncTableEntry>  mult_reservatory(mul_nr_reservation);
   Queue<FuncTableEntry>  div_reservatory(div_nr_reservation);


}
