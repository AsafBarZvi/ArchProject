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


const int INST_UNIT_SIZE  = 16; // Size of instruction unit queue
const int REGISTERS_NUM   = 16; // Number of FP registers
const int NUM_ISSUES      = 2; // NOTE: was defined to be 2, non-configurable in project


// ----------------------------------------------------------------------------------------------------------------------- //
void resevatoryToUnit(AsyncQueue<FuncTableEntry>& reservatory , std::vector< std::shared_ptr<BaseFunction> > & units , Register& register_file)
{
    for (auto & req : reservatory.get_queue())
    {
         if (!req.busy &&  req.VQS.first.is_ready() && req.VQS.second.is_ready())
         {
             for (auto & unit : units)
             {
                 if (!unit->is_busy())
                 {
                     req.busy = true;
                     auto & cmd = unit->write();
                     cmd = req;
                     cmd.creator = &req;
                     auto & trace = IT[req.pc];
                     trace.cycle_executed_start = CLOCK + 1;
                     break;
                 }
             }
         }
    }
}
// ----------------------------------------------------------------------------------------------------------------------- //
bool resevatoryToUnit(AsyncQueue<FuncTableEntry>& reservatory , MemAccess & mem_write)
{
    for (auto & req : reservatory.get_queue())
    {
         if (!req.busy)
         {
             req.busy = true;
             mem_write.port[2] = req;
             mem_write.port[2].creator = &req;
             auto & trace = IT[req.pc];
             trace.cycle_executed_start = CLOCK + 1; 
             return true;
         }
    }
    return false;
}
// ----------------------------------------------------------------------------------------------------------------------- //
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
// ----------------------------------------------------------------------------------------------------------------------- //
void updateTableWithUnitsOutout(std::list< AsyncQueue<FuncTableEntry> * > &       function_unit_tables ,
                                std::vector< std::shared_ptr<BaseFunction> > &    adders,
                                std::vector< std::shared_ptr<BaseFunction> > &    multipliers,
                                std::vector< std::shared_ptr<BaseFunction> > &    dividers,
                                Register&                                         register_file,
                                const MemAccess&                                  mem_read)
{

    std::list< std::vector< std::shared_ptr<BaseFunction> > * > units = {&adders , &multipliers , &dividers };
    for (auto & unit_group : units)
    {

        for (auto & unit : *unit_group)
        {
            auto & out = unit->read();
            if (out.op == OP::DONE)
            {
                auto & trace  = IT[out.pc];
                trace.cycle_write_cdb = CLOCK + 1;
                updateUnit(function_unit_tables , out , register_file);
                break; // Single CDB per functional unit group , .i.e. unit is busy while CDB is not available 

            }
        }
    }


    // Assuming mem data has a dedicated CDB, so arbitration on mem out data 
    auto & out = mem_read.port[2];
    if (out.op == OP::DONE)
    {
        auto & trace  = IT[out.pc];
        trace.cycle_write_cdb = CLOCK + 1; 
        updateUnit(function_unit_tables, out , register_file);
        assert(!register_file.write().at(mem_read.port[2].instruction.dst).is_ready() ||
               register_file.write().at(mem_read.port[2].instruction.dst).val() == out.result.as_float);
    }

}
// ----------------------------------------------------------------------------------------------------------------------- //
int main(int argc , char ** argv)
{
    // Check input args
    std::vector< std::string > inputFiles;
    if (argc == 7)
    {
        for (int i=1 ; i < argc ; i++)
            inputFiles.push_back(argv[i]);
    }
    else
    {
        std::cout << "Incorrect input arguments!\n" 
            << "Usage:./sim cfg.txt memin.txt memout.txt regout.txt traceinst.txt tracecdb.txt\n";
        exit(1);
    }

    // Read cfg.txt 
    int add_nr_units;
    int mul_nr_units;
    int div_nr_units;
    int add_nr_reservation;
    int mul_nr_reservation;
    int div_nr_reservation;
    int add_delay;
    int mul_delay;
    int div_delay;
    int mem_delay;
    int mem_nr_load_buffers;
    int mem_nr_store_buffers;

    std::ifstream cfgFile(inputFiles[0]);
    if (!cfgFile.is_open())
        throw std::runtime_error("unable to open file " + inputFiles[0]);
    
    std::string str;

    cfgFile >> str >> str >> add_nr_units;
    cfgFile >> str >> str >> mul_nr_units;
    cfgFile >> str >> str >> div_nr_units;
    cfgFile >> str >> str >> add_nr_reservation;
    cfgFile >> str >> str >> mul_nr_reservation;
    cfgFile >> str >> str >> div_nr_reservation;
    cfgFile >> str >> str >> add_delay;
    cfgFile >> str >> str >> mul_delay;
    cfgFile >> str >> str >> div_delay;
    cfgFile >> str >> str >> mem_delay;
    cfgFile >> str >> str >> mem_nr_load_buffers;
    cfgFile >> str >> str >> mem_nr_store_buffers;

    // ----------------------------------------------------------------------------------------------------------------------- //
    /*
     *  Init Units
     */

   std::list< SyncBlockBase* > blocks;
   std::list< AsyncQueue<FuncTableEntry> * > function_unit_tables;
   
   // Assuming AsyncQueue for instruction queue
   AsyncQueue<std::pair<int , Instruction> > inst_queue(INST_UNIT_SIZE);

   Queue<FuncTableEntry> store_buffer(mem_nr_store_buffers);
   function_unit_tables.push_back(&store_buffer);
   blocks.push_back(&store_buffer);
   
   Queue<FuncTableEntry> load_buffer(mem_nr_load_buffers);
   function_unit_tables.push_back(&load_buffer);
   blocks.push_back(&load_buffer);

   Register register_file(REGISTERS_NUM);
   blocks.push_back(&register_file);

   Queue<FuncTableEntry>  adders_reservatory(add_nr_reservation);
   function_unit_tables.push_back(&adders_reservatory);
   blocks.push_back(&adders_reservatory);

   Queue<FuncTableEntry>  mult_reservatory(mul_nr_reservation);
   function_unit_tables.push_back(&mult_reservatory);
   blocks.push_back(&mult_reservatory);

   Queue<FuncTableEntry>  div_reservatory(div_nr_reservation);
   function_unit_tables.push_back(&div_reservatory);
   blocks.push_back(&div_reservatory);

   std::vector< std::shared_ptr<BaseFunction> >       adders;
   for (int i = 0 ; i < add_nr_units ; i++)
   {
       adders.push_back(std::shared_ptr<BaseFunction> (new Add(add_delay)));
       blocks.push_back(adders.back().get());

   }

   std::vector< std::shared_ptr<BaseFunction> >      multipliers;
   for (int i = 0 ; i < mul_nr_units ; i++)
   {
       multipliers.push_back(std::shared_ptr<BaseFunction>( new Mult(mul_delay)) );
       blocks.push_back(multipliers.back().get());
   }

   std::vector<std::shared_ptr<BaseFunction> >       dividers;
   for (int i = 0 ; i < div_nr_units ; i++)
   {
       dividers.push_back(std::shared_ptr<BaseFunction>(new Div(div_delay)) );
       blocks.push_back(dividers.back().get());
   }

   Memory mem_unit = Memory(inputFiles[1] , mem_delay);
   blocks.push_back(&mem_unit);
   // ----------------------------------------------------------------------------------------------------------------------- //

   // Set CLOCK to mem_delay for instruction pre-fetch
   int preFetchCycles = mem_delay - 1;
   while (preFetchCycles)
   {
       CLOCK--;
       preFetchCycles--;
   }

   bool is_halt = false;
   int pc = 0;
   while (true)
   {
       CLOCK++;
       // ----------------------------------------------------------------------------------------------------------------------- //
       /*
        * FETCH
        */
       auto & mem_write = mem_unit.write();
       if (!is_halt)
       {
           if (!inst_queue.is_half_full())
           {
                // FETCH 0
                mem_write.port[0].op = OP::LD;
                mem_write.port[0].instruction.dst = 0;
                mem_write.port[0].instruction.imm  = pc;
                mem_write.port[0].instruction.op = OP::LD;
                mem_write.port[0].pc = pc;
                // FETCH 1
                mem_write.port[1].op = OP::LD;
                mem_write.port[1].instruction.dst = 0;
                mem_write.port[1].instruction.imm  = pc + 1;
                mem_write.port[1].instruction.op = OP::LD;
                mem_write.port[1].pc = pc + 1;
                pc +=2;
           }
       }

       auto & mem_read = mem_unit.read();
       if (!is_halt)
       {
           assert(!inst_queue.is_full());
           // Assuming write to instruction queue is not through CDB, 
           // i.e. every clock DATA can be written to register file
           if (mem_read.port[0].op == OP::DONE)
           {
               inst_queue.push(std::make_pair(mem_read.port[0].pc  , decode(mem_read.port[0].result.as_int)));
               auto & trace = IT[mem_read.port[0].pc];
               trace.pc     = mem_read.port[0].pc;
               trace.inst_hex = decode(mem_read.port[0].result.as_int).as_string();
           }

           if (mem_read.port[1].op == OP::DONE)
           {
               inst_queue.push(std::make_pair(mem_read.port[1].pc  , decode(mem_read.port[1].result.as_int)));
               auto & trace = IT[mem_read.port[1].pc];
               trace.pc     = mem_read.port[1].pc;
               trace.inst_hex = decode(mem_read.port[1].result.as_int).as_string();
           }

           // ----------------------------------------------------------------------------------------------------------------------- //
           
           /*
            * ISSUE
            * -> reservatory
            */

           for (int i = 0 ; i < NUM_ISSUES ; i ++)
           {
                auto & inst_pair = inst_queue.peek();
                auto & inst = inst_pair.second;
                
                if (inst.op == OP::HALT)
                {
                    auto & trace = IT[inst_pair.first];
                    trace.cycle_issued = CLOCK;
                    is_halt = true;
                    break;
                }

                if (inst.op == OP::NOPE)
                    inst_queue.pop();

                bool available_reservatory = ( (inst.op == OP::ADD || inst.op == OP::SUB) && (!adders_reservatory.is_full()) ) ||
                                             ( inst.op  == OP::MULT && !mult_reservatory.is_full() )                           ||
                                             ( inst.op  == OP::DIV  && !div_reservatory.is_full()  )                           ||
                                             ( inst.op  == OP::ST   && !store_buffer.is_full()     )                           ||
                                             ( inst.op  == OP::LD   && !load_buffer.is_full()      )                           ;

                /* Check if not accumulator, dst == src and dst is pending result, in that case must wait for result,
                 * since re-tagging dst with cause deadlock, infinitely waiting for src
                 */
                bool is_accumulator       = (( inst.dst == inst.src0 || inst.dst == inst.src1 ) & !register_file.write().at(inst.dst).is_ready());

                if (available_reservatory && !is_accumulator)
                {
                    FuncTableEntry tentry;
                    tentry.busy = false;
                    tentry.instruction = inst;
                    tentry.op   = OP(inst.op);
                    tentry.pc   = inst_pair.first;
                    tentry.VQS.first = register_file.write().at(inst.src0);         
                    tentry.VQS.second = register_file.write().at(inst.src1);
                    if       (inst.op == OP::ADD || inst.op == OP::SUB)
                    {
                         auto  tag = adders_reservatory.push(tentry,"ADD");
                         register_file.write().at(inst.dst).set_tag(tag);
                         auto & trace = IT[inst_pair.first];
                         trace.tag = tag;
                         trace.cycle_issued = CLOCK;
                    }
                    else if  (inst.op == OP::MULT)
                    {
                         auto  tag = mult_reservatory.push(tentry,"MUL");
                         register_file.write().at(inst.dst).set_tag(tag);
                         auto & trace = IT[inst_pair.first];
                         trace.tag = tag;
                         trace.cycle_issued = CLOCK;
                    }
                    else if  (inst.op == OP::DIV)
                    {
                         auto  tag = div_reservatory.push(tentry,"DIV");
                         register_file.write().at(inst.dst).set_tag(tag);
                         auto & trace = IT[inst_pair.first];
                         trace.tag = tag;
                         trace.cycle_issued = CLOCK;
                    }
                    else if  (inst.op == OP::ST)
                    {
                         auto  tag = store_buffer.push(tentry,"ST");
                         auto & trace = IT[inst_pair.first];
                         trace.tag = tag;
                         trace.cycle_issued = CLOCK;
                    }
                    else if  (inst.op == OP::LD)
                    {
                         auto  tag = load_buffer.push(tentry,"LD");
                         register_file.write().at(inst.dst).set_tag(tag);
                         auto & trace = IT[inst_pair.first];
                         trace.tag = tag;
                         trace.cycle_issued = CLOCK;
                    }
                        
                    std::cout << "In Resv -> " << inst << std::endl;
                    inst_queue.pop();
                }
                else
                    break;
           }
       }


      // Wait for functional units to complete all instruction  
       if (is_halt)
           if (adders_reservatory.is_empt() && mult_reservatory.is_empt() && div_reservatory.is_empt() && store_buffer.is_empt() && load_buffer.is_empt())
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
       resevatoryToUnit(adders_reservatory,adders,register_file);
       resevatoryToUnit(mult_reservatory,multipliers,register_file);
       resevatoryToUnit(div_reservatory,dividers,register_file);
       resevatoryToUnit(load_buffer,mem_write) || resevatoryToUnit(store_buffer,mem_write);    //Always tring to load and then store


       // ----------------------------------------------------------------------------------------------------------------------- //
        
       /*
        * Do clock to all 
        * Clocked units
        */

       std::for_each(blocks.begin() , blocks.end() , []( SyncBlockBase* b) { b->clock(); } );

   }


   std::ofstream regoutFile;
   regoutFile.open(inputFiles[3]);
   regoutFile << register_file << std::endl;
   std::cout << register_file << std::endl;

   std::ofstream traceinstFile;
   traceinstFile.open(inputFiles[4]);
   traceinstFile << IT << std::endl;
   std::cout << IT << std::endl;
   std::cout << register_file << std::endl;

}
