/**************************************************************/
/* CS/COE 1541				 			
   compile with gcc -o pipeline five_stage.c			
   and execute using							
   ./pipeline  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr	0  
***************************************************************/

#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "CPU.h" 

//int has_two_readRegs(struct instruction IF)
//{
//  return (IF.type == ti_RTYPE || IF.type == ti_STORE || IF.type == ti_BRANCH);
//}

//int has_one_readReg(struct instruction IF)
//{
// return (IF.type == ti_ITYPE || IF.type == ti_JRTYPE || IF.type == ti_LOAD); 
//}

//int has_dReg(struct instruction IF){
// return (IF.type == ti_RTYPE || IF.type == ti_ITYPE || IF.type == ti_LOAD)
//}

//int data_hazard1(struct instruction tr_entry, struct instruction tr_entry2){
//	if (has_dReg(tr_entry)){
//		if (has_two_readRegs(tr_entry2)){
//			if (tr_entry->dReg == tr_entry2->sReg_a || tr_entry->dReg == tr_entry2->sReg_b){
//				return 0;
//			}
//		}
//		else if (has_one_readReg(tr_entry2)){
//			if (tr_entry->dReg == tr_entry2->sReg_a){
//				return 0;
//			}
//		}
//	}
//	return 1;
//}

struct super_instruction move_to_upper_superscalar(struct instruction IF,struct super_instruction IF_S)
{
	  IF_S.type1 = IF.type;
	  IF_S.sReg_a1 = IF.sReg_a;
	  IF_S.sReg_b1 = IF.sReg_b;
	  IF_S.dReg1 = IF.dReg;
	  IF_S.PC1 = IF.PC;
	  IF_S.Addr1 = IF.Addr;
  
  return IF_S;
}

struct super_instruction move_to_lower_superscalar(struct instruction IF,struct super_instruction IF_S)
{
  
    IF_S.type2 = IF.type;
    IF_S.sReg_a2 = IF.sReg_a;
    IF_S.sReg_b2 = IF.sReg_b;
    IF_S.dReg2 = IF.dReg;
    IF_S.PC2 = IF.PC;
    IF_S.Addr2 = IF.Addr; 
   
  return IF_S;
}


int is_diff_pipeline(struct instruction *tr_entry, struct  instruction *tr_entry2)
{
   if(tr_entry->type == ti_STORE || tr_entry->type == ti_LOAD)
   {
   	 if(tr_entry2->type == ti_STORE || tr_entry2->type == ti_LOAD)
   	 	return 0;
   	 else 
   	 	return 1;
   }	
   else 
   {
   	if(tr_entry2->type == ti_STORE || tr_entry2->type == ti_LOAD)
   		return 1;
   	else
   		return 0;
   }
}

struct instruction auto_upper_separate(struct super_instruction IF_S)
{
   struct instruction EX_temp;
   if(IF_S.type2 != ti_LOAD && IF_S.type2 != ti_STORE && IF_S.type2 != ti_NOP)
   {
     EX_temp.type = IF_S.type2;
     EX_temp.sReg_a = IF_S.sReg_a2;
     EX_temp.sReg_b = IF_S.sReg_b2;
     EX_temp.dReg = IF_S.dReg2;
     EX_temp.PC = IF_S.PC2;
     EX_temp.Addr = IF_S.Addr2;
   }
   else
   {
     EX_temp.type = IF_S.type1;
     EX_temp.sReg_a = IF_S.sReg_a1;
     EX_temp.sReg_b = IF_S.sReg_b1;
     EX_temp.dReg = IF_S.dReg1;
     EX_temp.PC = IF_S.PC1;
     EX_temp.Addr = IF_S.Addr1;   
   }
   return EX_temp;
}

struct instruction auto_lower_separate(struct super_instruction IF_S)
{
	struct instruction EX_temp;
   if(IF_S.type2 == ti_LOAD || IF_S.type2 == ti_STORE || IF_S.type2 == ti_NOP)
   {
     EX_temp.type = IF_S.type2;
     EX_temp.sReg_a = IF_S.sReg_a2;
     EX_temp.sReg_b = IF_S.sReg_b2;
     EX_temp.dReg = IF_S.dReg2;
     EX_temp.PC = IF_S.PC2;
     EX_temp.Addr = IF_S.Addr2;
   }
	 else
   {
     EX_temp.type = IF_S.type1;
     EX_temp.sReg_a = IF_S.sReg_a1;
     EX_temp.sReg_b = IF_S.sReg_b1;
     EX_temp.dReg = IF_S.dReg1;
     EX_temp.PC = IF_S.PC1;
     EX_temp.Addr = IF_S.Addr1;   
   }
   return EX_temp;
}



int main(int argc, char **argv)
{
  struct instruction *tr_entry;
  struct instruction *tr_entry2;
  struct instruction IF, ID, EX, MEM, WB, EX_2, MEM_2, WB_2,WB_temp;
  struct super_instruction IF_S, ID_S;
  size_t size;
  char *trace_file_name;
  int trace_view_on = 0;
  int flush_counter = 4; //5 stage pipeline, so we have to move 4 instructions once trace is done
  int switch_buffer = 0;



  unsigned int cycle_number = 0;

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }
    
  trace_file_name = argv[1];
  if (argc == 3) trace_view_on = atoi(argv[2]) ;

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();

  while(1) {
  	if(switch_buffer == 0)
    	size = trace_get_item(&tr_entry); /* put the instruction into a buffer */
    else
    	tr_entry = tr_entry2;
    //put one more instruction in the buffer;
    

 	
    if (!size && flush_counter==0) {       /* no more instructions (instructions) to simulate */
      printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      break;
    }
    else{              /* move the pipeline forward */
      cycle_number++;

      /* move instructions one stage ahead */
      WB = MEM;
      WB_2 = MEM_2;
      MEM = EX;
      MEM_2 = EX_2;
      EX = auto_upper_separate(ID_S);
      EX_2 = auto_lower_separate(ID_S);
      ID_S = IF_S;

      if(!size){    /* if no more instructions in trace, reduce flush_counter */
        flush_counter--;   
      }
      else{   /* copy trace entry into IF stage */
        
        //move two instructions into one super-instruction
		//--data hazard and control hazard
	        memcpy(&IF, tr_entry , sizeof(IF));
	        IF_S = move_to_upper_superscalar(IF,IF_S);
	        size = trace_get_item(&tr_entry2);
	        if(size) 
	        {
	          if(is_diff_pipeline(tr_entry, tr_entry2)  && IF_S.type1 != ti_JTYPE && IF_S.type1 != ti_BRANCH)
		        {	
		          memcpy(&IF, tr_entry2 , sizeof(IF));
		          IF_S = move_to_lower_superscalar(IF, IF_S);
		          switch_buffer=0;
		        }
		        else 
		        {
		      	  IF_S.type2 = ti_NOP;
				  switch_buffer = 1;
	          }
	        }
	       else{	
	          IF_S.type2 = ti_NOP;
	          switch_buffer = 0;
	        }

      }
      // if(IF_S.type1 == ti_STORE || IF_S.type1 == ti_LOAD)
      // {
      //   WB_temp = WB;
      //   WB = WB_2;
      //   WB_2 = WB_temp;
      // }
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

      switch(WB_2.type) {
        case ti_NOP:
          printf("[cycle %d] NOP:\n",cycle_number) ;
          break;
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
          printf("[cycle %d] RTYPE:",cycle_number) ;
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", WB_2.PC, WB_2.sReg_a, WB_2.sReg_b, WB_2.dReg);
          break;
        case ti_ITYPE:
          printf("[cycle %d] ITYPE:",cycle_number) ;
          printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", WB_2.PC, WB_2.sReg_a, WB_2.dReg, WB_2.Addr);
          break;
        case ti_LOAD:
          printf("[cycle %d] LOAD:",cycle_number) ;      
          printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", WB_2.PC, WB_2.sReg_a, WB_2.dReg, WB_2.Addr);
          break;
        case ti_STORE:
          printf("[cycle %d] STORE:",cycle_number) ;      
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", WB_2.PC, WB_2.sReg_a, WB_2.sReg_b, WB_2.Addr);
          break;
        case ti_BRANCH:
          printf("[cycle %d] BRANCH:",cycle_number) ;
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", WB_2.PC, WB_2.sReg_a, WB_2.sReg_b, WB_2.Addr);
          break;
        case ti_JTYPE:
          printf("[cycle %d] JTYPE:",cycle_number) ;
          printf(" (PC: %d)(addr: %d)\n", WB_2.PC,WB_2.Addr);
          break;
        case ti_SPECIAL:
          printf("[cycle %d] SPECIAL:\n",cycle_number) ;      	
          break;
        case ti_JRTYPE:
          printf("[cycle %d] JRTYPE:",cycle_number) ;
          printf(" (PC: %d) (sReg_a: %d)(addr: %d)\n", WB_2.PC, WB_2.dReg, WB_2.Addr);
          break;
      }
    }
  }

  trace_uninit();

  exit(0);
}


