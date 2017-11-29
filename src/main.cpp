#include "queue.h"
#include "defines.h"
#include "func_table.h"
#include "functions.h"
#include <vector>
#include <list>
#include "sync_block.h"
#include "register.h"
#include "inst_decode.h"

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
   
   //TODO assuming AsyncQueue for instruction queue
   AsyncQueue<Instruction> inst_queue(16);
   //blocks.push_back(&inst_queue);

   AsyncQueue<Mem> store_buffer(mem_nr_store_buffer);
   //blocks.push_back(&store_buffer);
   
   AsyncQueue<Mem> load_buffer(mem_nr_load_buffer);
   //blocks.push_back(&load_buffer);

   Register register_file(16);
   blocks.push_back(&register_file);

   AsyncQueue<FuncTableEntry>  adders_reservatory(add_nr_reservation);
   //blocks.push_back(&adders_reservatory);

   AsyncQueue<FuncTableEntry>  mult_reservatory(mul_nr_reservation);
   //blocks.push_back(&mult_reservatory);

   AsyncQueue<FuncTableEntry>  div_reservatory(div_nr_reservation);
   //blocks.push_back(&div_reservatory);


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

   //TODO NO use of memory store load buffer

   int pc = 0;
   while (true)
   {
       /*
        * FETCH
        */
       auto mem_write = mem_unit.write();
       if (!inst_queue.is_half_full()) 
       {    // TODO is this logic true ? 
            // FETCH 0
            mem_write.port[0].op = OP::LD;
            mem_write.port[0].dest = 0;
            mem_write.port[0].imm  = pc;
            // FETCH 1
            mem_write.port[0].op = OP::LD;
            mem_write.port[0].dest = 0;
            mem_write.port[0].imm  = pc + 1;
       }

       pc +=2;

       auto mem_read = mem_unit.read();
       assert(!inst_queue.is_full());
       // TODO assuming write to instruction queue is not through CDB, 
       // i.e. every clock DATA can be written to register file
       if (mem_read.port[0].op == OP::DONE)
           inst_queue.push(decode(mem_read.port[0].result.as_int));
       if (mem_read.port[1].op == OP::DONE)
           inst_queue.push(decode(mem_read.port[1].result.as_int));
       if (mem_read.port[2].op == OP::DONE)
           register_file.write().at(mem_read.port[2].dest).set_val(mem_read.port[2].result.as_float);

       // ---------------------------------- //

       /*
        * ISSUE
        */

       auto inst = inst_queue.peek().second;
       bool available_reservatory = ( (inst.op == OP::ADD || inst.op == OP::SUB) && (!adders_reservatory.is_full()) ) ||
                                    ( inst.op  == OP::MULT && !mult_reservatory.is_full() )                           ||
                                    ( inst.op  == OP::DIV  && !div_reservatory.is_full()  )                            ;

       if (available_reservatory)
       {
           FuncTableEntry tentry;
           tentry.busy = true;
           tentry.dest = inst.dst;
           tentry.op   = OP(inst.op);
           tentry.VQS.first = register_file.read().at(inst.src0);
           tentry.VQS.second = register_file.read().at(inst.src1);
           if       (inst.op == OP::ADD || inst.op == OP::SUB)
           {
                auto tag = adders_reservatory.push(tentry,"add");
                register_file.write().at(inst.dst).set_tag(tag);
           }
           else if  (inst.op == OP::MULT)
           {
                auto tag = adders_reservatory.push(tentry,"mult");
                register_file.write().at(inst.dst).set_tag(tag);
           }
           else if  (inst.op == OP::DIV)
           {
                auto tag = adders_reservatory.push(tentry,"div");
                register_file.write().at(inst.dst).set_tag(tag);
           }
                
           inst_queue.pop();
       }

   }

   

}
