/**************************************************************/
/* CS/COE 1541				 			
   compile with gcc -o pipeline five_stage.c			
   and execute using							
   ./pipeline  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr	0  
***************************************************************/
//team name
//modify
#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "CPU.h" 

int main(int argc, char **argv)
{
  struct instruction *tr_entry;
  struct instruction IF, ID, EX, MEM, WB;
  size_t size;
  char *trace_file_name;
  int trace_view_on = 0;
  int prediction_method = 0;
  int flush_counter = 4; //5 stage pipeline, so we have to move 4 instructions once trace is done
  // data_hazard == 0 -> No data hazard detected
  // data_hazard == 1 -> Data hazard detected
  int data_hazard = 0;
  int control_hazard = 0;

  
  unsigned int cycle_number = 0;

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }
    
  trace_file_name = argv[1];  
  if (argc == 4) 
  {
    prediction_method = atoi(argv[3]);
    trace_view_on = atoi(argv[2]) ;
  }
  else
  {
    printf("there should be 3 arguments");
    exit(0);
  }

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();
//////////////////////////////////////////////////////////////////
  //////////////////Start
  /////////////////////////////////////////////////////////////
  while(1) {
	if ((!data_hazard)&& (!control_hazard)){
		size = trace_get_item(&tr_entry); /* put the instruction into a buffer */
	}
      data_hazard = 0;
     control_hazard = 0;
    if (!size && flush_counter==0) {       /* no more instructions (instructions) to simulate */
      printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      break;
    }
    else
    {              /* move the pipeline forward */
      cycle_number++;

      /* move instructions one stage ahead */
	/////////////////////////////////////////////////////////////////////  
	  // Data Hazard Detection
	  if (ID.type == ti_LOAD)
    {
  		if (data_hazard_condition(IF))
      {
  			if (ID.dReg == IF.sReg_a || ID.dReg == IF.sReg_b){
  				data_hazard = 1;
  				WB = MEM;
  				MEM = EX;
          EX = ID;
  				ID.type = ti_NOP;
          IF = IF;
  			}
  		}
  		else if (data_hazard_condition2(ID))
      {
  			if (ID.dReg == IF.sReg_a)
        {
  				data_hazard = 1;
  				WB = MEM;
  				MEM = EX;
  				ID.type = ti_NOP;
          IF = IF;
  			}
  		}
	  }
	  
	///////////////////////////////////////////////////////////////////////////////////////////

	 //  // Handling control hazards
	 if (ID.type == ti_BRANCH)
    {
    	if (prediction_method){
				
			}
			else{

              printf("ID PC %d  and IF PC %d",ID.PC,IF.PC );
          if(branch_not_taken(ID,IF))
            {
              control_hazard = 0;
            }
          else
            {
              
              control_hazard = 1;
              WB = MEM;
              MEM = EX;
              EX = ID;
              ID.type = ti_NOP;
              IF = IF;
            }
			}
		}


/////////////////////////////
    if (!data_hazard && !control_hazard)
    {
      WB = MEM;
      MEM = EX;
      EX = ID;
      ID = IF;
    }

//////////////////////////////    


    if(!size)
    {    /* if no more instructions in trace, reduce flush_counter */
      flush_counter--;   
    }
    else
    {   /* copy trace entry into IF stage */
  		if (!data_hazard && (!control_hazard))
      {
  			memcpy(&IF, tr_entry , sizeof(IF));
  		}
    }	

      //printf("==============================================================================\n");
    }  


    if (trace_view_on && cycle_number>=5) {/* print the executed instruction if trace_view_on=1 */
      switch(WB.type) {
        case ti_NOP:
          printf("[cycle %d] NOP:\n",cycle_number) ;
          break;
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
          printf("[cycle %d] RTYPE:",cycle_number) ;
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", WB.PC, WB.sReg_a, WB.sReg_b, WB.dReg);
          break;
        case ti_ITYPE:
          printf("[cycle %d] ITYPE:",cycle_number) ;
          printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", WB.PC, WB.sReg_a, WB.dReg, WB.Addr);
          break;
        case ti_LOAD:
          printf("[cycle %d] LOAD:",cycle_number) ;      
          printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", WB.PC, WB.sReg_a, WB.dReg, WB.Addr);
          break;
        case ti_STORE:
          printf("[cycle %d] STORE:",cycle_number) ;      
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", WB.PC, WB.sReg_a, WB.sReg_b, WB.Addr);
          break;
        case ti_BRANCH:
          printf("[cycle %d] BRANCH:",cycle_number) ;
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", WB.PC, WB.sReg_a, WB.sReg_b, WB.Addr);
          break;
        case ti_JTYPE:
          printf("[cycle %d] JTYPE:",cycle_number) ;
          printf(" (PC: %d)(addr: %d)\n", WB.PC,WB.Addr);
          break;
        case ti_SPECIAL:
          printf("[cycle %d] SPECIAL:\n",cycle_number) ;      	
          break;
        case ti_JRTYPE:
          printf("[cycle %d] JRTYPE:",cycle_number) ;
          printf(" (PC: %d) (sReg_a: %d)(addr: %d)\n", WB.PC, WB.dReg, WB.Addr);
          break;
      }
    }
  }

  trace_uninit();

  exit(0);
}

int data_hazard_condition(struct instruction IF)
{
  return (IF.type == ti_RTYPE || IF.type == ti_STORE || IF.type == ti_BRANCH);
}

int data_hazard_condition2(struct  instruction IF)
{
 return (IF.type == ti_ITYPE || IF.type == ti_JRTYPE || IF.type == ti_LOAD); 
}

int branch_not_taken(struct  instruction ID, struct instruction IF)
{
 if((ID.PC+4==IF.PC))return 1;
  else return 0; 
}

