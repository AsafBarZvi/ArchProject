#include "queue.h"
#include "defines.h"
#include "func_table.h"
#include "functions.h"
#include <vector>
#include <list>
#include "sync_block.h"
#include "register.h"

const int add_nr_reservation = 2;
const int mul_nr_reservation = 2;
const int div_nr_reservation = 2;
const int add_nr_units = 2;
const int mul_nr_units = 2;
const int div_nr_units = 2;
const int add_delay    = 2;
const int mult_delay   = 4;
const int div_delay    = 6;
const int mem_nr_store_buffer = 3;
const int mem_nr_load_buffer  = 3;
const int mem_delay    = 3;




int main(int argc , char ** argv)
{

   /*
    * Init Units
    */

   std::list< SyncBlockBase* > blocks; 
   
   Queue<Instruction> inst_queue(16);
   blocks.push_back(&inst_queue);

   Queue<Mem> store_buffer(mem_nr_store_buffer);
   blocks.push_back(&store_buffer);
   
   Queue<Mem> load_buffer(mem_nr_load_buffer);
   blocks.push_back(&load_buffer);

   Register register_file(16);
   blocks.push_back(&register_file);

   Queue<FuncTableEntry>  adders_reservatory(add_nr_reservation);
   blocks.push_back(&adders_reservatory);

   Queue<FuncTableEntry>  mult_reservatory(mul_nr_reservation);
   blocks.push_back(&mult_reservatory);

   Queue<FuncTableEntry>  div_reservatory(div_nr_reservation);
   blocks.push_back(&div_reservatory);


   std::vector<Add>       adders;
   for (int i = 0 ; i < add_nr_units ; i++)
   {
       adders.push_back(Add(add_delay));
       blocks.push_back(&adders.back());
   }

   std::vector<Mult>      multipliers;
   for (int i = 0 ; i < mul_nr_units ; i++)
   {
       multipliers.push_back(Mult(mult_delay));
       blocks.push_back(&multipliers.back());
   }

   std::vector<Div>       dividers;
   for (int i = 0 ; i < div_nr_units ; i++)
   {
       dividers.push_back(Div(div_delay));
       blocks.push_back(&dividers.back());
   }

   Memory mem_unit = Memory("mem_file.txt" , mem_delay);
   blocks.push_back(&mem_unit);


   

}
