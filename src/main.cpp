#include "queue.h"
#include "defines.h"
#include "func_table.h"
#include "functions.h"
#include <vector>
#include <list>
#include "sync_block.h"
#include "register.h"
#include "inst_decode.h"
#include <memory>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include "printers.hpp"
#include <map>
#include "trace.h"


const int NUM_ISSUES      = 2;

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




void resevatoryToUnit(AsyncQueue<FuncTableEntry>& reservatory , std::vector< std::shared_ptr<BaseFunction> > & units)
{
    for (auto & req : reservatory.get_queue())
    {
         if (!req.busy && req.VQS.first.is_ready() && req.VQS.second.is_ready())
         {
             for (auto & unit : units)
             {
                 if (!unit->is_busy())
                 {
                     req.busy = true;
                     auto & cmd = unit->write();
                     cmd = req;
                     cmd.creator = &req;
                     auto & trace = IT[req.instruction.as_string()];
                     trace.cycle_executed_start = CLOCK; 
                 }
             }
         }
    }
}

bool resevatoryToUnit(AsyncQueue<FuncTableEntry>& reservatory , MemAccess & mem_write)
{
    for (auto & req : reservatory.get_queue())
    {
         if (!req.busy && req.VQS.first.is_ready() && req.VQS.second.is_ready())
         {
             req.busy = true;
             mem_write.port[2] = req;
             mem_write.port[2].creator = &req;
             auto & trace = IT[req.instruction.as_string()];
             trace.cycle_executed_start = CLOCK; 
             return true;
         }
    }
    return false;
}



void updateUnit(std::list< AsyncQueue<FuncTableEntry> * > &     function_unit_tables,
                const FuncTableEntry &                          out,
                Register &                                      register_file)
{
    /***
     * Update reservatory with functional unit update
     */
    for (auto & unit_reservatory : function_unit_tables)
    {
        for (auto & unit_req : unit_reservatory->get_queue())
        {
            if (unit_req.VQS.first.tag() == out.tag)
                unit_req.VQS.first.set_val(out.result.as_float);
            if (unit_req.VQS.second.tag() == out.tag)
                unit_req.VQS.second.set_val(out.result.as_float);

        }

        // Remove Done job from reservatory
        auto i = unit_reservatory->get_queue().begin() ;
        while ( i != unit_reservatory->get_queue().end() )
        {
            if ( i->tag == out.tag)
            {
                assert(out.creator == &(*i));
                i = unit_reservatory->get_queue().erase(i);
                break;
            }
            i++;

        }
    }

    /***
     * Update register file with functional unit update
     */
    for (auto & reg : register_file.write())
    {
        if (reg.tag() == out.tag)
            reg.set_val(out.result.as_float);
    }
}


void updateTableWithUnitsOutout(std::list< AsyncQueue<FuncTableEntry> * > &       function_unit_tables ,
                                std::vector< std::shared_ptr<BaseFunction> > &    adders,
                                std::vector< std::shared_ptr<BaseFunction> > &    multipliers,
                                std::vector< std::shared_ptr<BaseFunction> > &    dividers,
                                Register&                                         register_file,
                                const MemAccess&                                  mem_read)
{

    //TODO reservatory  not pushed for current cycle
    
    std::list< std::vector< std::shared_ptr<BaseFunction> > * > units = {&adders , &multipliers , &dividers };
    for (auto & unit_group : units)
    {

        for (auto & unit : *unit_group)
        {
            auto & out = unit->read();
            if (out.op == OP::DONE)
            {
                auto & trace  = IT[out.creator->instruction.as_string()];
                trace.cycle_write_cdb = CLOCK;
                updateUnit(function_unit_tables , out , register_file);
                break; // TODO Single CDB per functional unit group , .i.e. unit is busy while CDB is not available 

            }
        }
    }


    // TODO Assuming mem data has a dedocated CDB, so arbitration on mem out data 
    auto & out = mem_read.port[2];
    if (out.op == OP::DONE)
    {
        auto & trace  = IT[out.creator->instruction.as_string()];
        trace.cycle_write_cdb = CLOCK;
        updateUnit(function_unit_tables, out , register_file);
        assert(register_file.write().at(mem_read.port[2].instruction.dst).val() == out.result.as_float);
    }

}


int main(int argc , char ** argv)
{
   /*
    * Init Units
    */


   std::list< SyncBlockBase* > blocks;
   std::list< AsyncQueue<FuncTableEntry> * > function_unit_tables;
   
   //TODO assuming AsyncQueue for instruction queue
   AsyncQueue<Instruction> inst_queue(16);
   //blocks.push_back(&inst_queue);

   AsyncQueue<FuncTableEntry> store_buffer(mem_nr_store_buffer);
   //blocks.push_back(&store_buffer);
   function_unit_tables.push_back(&store_buffer);
   
   AsyncQueue<FuncTableEntry> load_buffer(mem_nr_load_buffer);
   //blocks.push_back(&load_buffer);
   function_unit_tables.push_back(&load_buffer);

   Register register_file(16);
   blocks.push_back(&register_file);

   AsyncQueue<FuncTableEntry>  adders_reservatory(add_nr_reservation);
   //blocks.push_back(&adders_reservatory);
   function_unit_tables.push_back(&adders_reservatory);

   AsyncQueue<FuncTableEntry>  mult_reservatory(mul_nr_reservation);
   //blocks.push_back(&mult_reservatory);
   function_unit_tables.push_back(&mult_reservatory);

   AsyncQueue<FuncTableEntry>  div_reservatory(div_nr_reservation);
   //blocks.push_back(&div_reservatory);
   function_unit_tables.push_back(&div_reservatory);


   std::vector< std::shared_ptr<BaseFunction> >       adders;
   for (int i = 0 ; i < add_nr_units ; i++)
   {
       adders.push_back(std::shared_ptr<BaseFunction> (new Add(add_delay)));
       blocks.push_back(adders.back().get());

   }

   std::vector< std::shared_ptr<BaseFunction> >      multipliers;
   for (int i = 0 ; i < mul_nr_units ; i++)
   {
       multipliers.push_back(std::shared_ptr<BaseFunction>( new Mult(mult_delay)) );
       blocks.push_back(multipliers.back().get());
   }

   std::vector<std::shared_ptr<BaseFunction> >       dividers;
   for (int i = 0 ; i < div_nr_units ; i++)
   {
       dividers.push_back(std::shared_ptr<BaseFunction>(new Div(div_delay)) );
       blocks.push_back(dividers.back().get());
   }

   Memory mem_unit = Memory("memin.txt" , mem_delay);
   blocks.push_back(&mem_unit);

   int pc = 0;
   while (true)
   {
       CLOCK++;
       // ---------------------------------- //
       /*
        * FETCH
        */
       auto & mem_write = mem_unit.write();
       if (!inst_queue.is_half_full()) 
       {    // TODO is this logic true ? 
            // FETCH 0
            mem_write.port[0].op = OP::LD;
            mem_write.port[0].instruction.dst = 0;
            mem_write.port[0].instruction.imm  = pc;
            mem_write.port[0].pc = pc;
            // FETCH 1
            mem_write.port[1].op = OP::LD;
            mem_write.port[1].instruction.dst = 0;
            mem_write.port[1].instruction.imm  = pc + 1;
            mem_write.port[1].pc = pc + 1;
            pc +=2;
       }


       auto & mem_read = mem_unit.read();
       assert(!inst_queue.is_full());
       // TODO assuming write to instruction queue is not through CDB, 
       // i.e. every clock DATA can be written to register file
       if (mem_read.port[0].op == OP::DONE)
       {
           inst_queue.push(decode(mem_read.port[0].result.as_int));
           auto & trace = IT[mem_read.port[0].instruction.as_string()];
           trace.pc     = mem_read.port[0].pc;
       }

       if (mem_read.port[1].op == OP::DONE)
       {
           inst_queue.push(decode(mem_read.port[1].result.as_int));
           auto & trace = IT[mem_read.port[0].instruction.as_string()];
           trace.pc     = mem_read.port[0].pc;
       }

       // ---------------------------------- //
       
       /*
        * ISSUE
        * -> reservatory
        */

       bool is_halt = false;
       for (int i = 0 ; i < NUM_ISSUES ; i ++)
       {
            auto & inst = inst_queue.peek();
            
            if (inst.op == OP::HALT)
            {
                auto & trace = IT[inst.as_string()];
                trace.cycle_issued = CLOCK;
                is_halt = true;
                break;
            }

            bool available_reservatory = ( (inst.op == OP::ADD || inst.op == OP::SUB) && (!adders_reservatory.is_full()) ) ||
                                         ( inst.op  == OP::MULT && !mult_reservatory.is_full() )                           ||
                                         ( inst.op  == OP::DIV  && !div_reservatory.is_full()  )                           ||
                                         ( inst.op  == OP::ST   && !store_buffer.is_full()     )                           ||
                                         ( inst.op  == OP::LD   && !load_buffer.is_full()      )                           ;

            if (available_reservatory)
            {
                FuncTableEntry tentry;
                tentry.busy = false;
                tentry.instruction = inst;
                tentry.op   = OP(inst.op);
                tentry.VQS.first = register_file.read().at(inst.src0);
                tentry.VQS.second = register_file.read().at(inst.src1);
                if       (inst.op == OP::ADD || inst.op == OP::SUB)
                {
                     auto  tag = adders_reservatory.push(tentry,"add");
                     register_file.write().at(inst.dst).set_tag(tag);
                     auto & trace = IT[inst.as_string()];
                     trace.tag = tag;
                     trace.cycle_issued = CLOCK;
                }
                else if  (inst.op == OP::MULT)
                {
                     auto  tag = mult_reservatory.push(tentry,"mult");
                     register_file.write().at(inst.dst).set_tag(tag);
                     auto & trace = IT[inst.as_string()];
                     trace.tag = tag;
                     trace.cycle_issued = CLOCK;
                }
                else if  (inst.op == OP::DIV)
                {
                     auto  tag = div_reservatory.push(tentry,"div");
                     register_file.write().at(inst.dst).set_tag(tag);
                     auto & trace = IT[inst.as_string()];
                     trace.tag = tag;
                     trace.cycle_issued = CLOCK;
                }
                else if  (inst.op == OP::ST)
                {
                     auto  tag = store_buffer.push(tentry,"st");
                     auto & trace = IT[inst.as_string()];
                     trace.tag = tag;
                     trace.cycle_issued = CLOCK;
                }
                else if  (inst.op == OP::LD)
                {
                     auto  tag = load_buffer.push(tentry,"ld");
                     register_file.write().at(inst.dst).set_tag(tag);
                     auto & trace = IT[inst.as_string()];
                     trace.tag = tag;
                     trace.cycle_issued = CLOCK;
                }
                    
                std::cout << "In Resv -> " << inst << std::endl;
                inst_queue.pop();
            }
            else
                break;
       }


       if (is_halt)
           break;

       /*
        * ISSUE
        * -> update tables with new info from functional units and mem out
        */

       updateTableWithUnitsOutout(function_unit_tables,
                                  adders,
                                  multipliers,
                                  dividers,
                                  register_file,
                                  mem_read);
       
       /*
        * ISSUE
        * -> dispatch to functional units
        */
       resevatoryToUnit(adders_reservatory,adders);
       resevatoryToUnit(mult_reservatory,multipliers);
       resevatoryToUnit(div_reservatory,dividers);
       resevatoryToUnit(load_buffer,mem_write) || resevatoryToUnit(store_buffer,mem_write);    //Always tring to load and then store


       std::for_each(blocks.begin() , blocks.end() , []( SyncBlockBase* b) { b->clock(); } );

   }

}
