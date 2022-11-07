/**
 *  banker.c
 *
 *  Full Name: Kaamil Parmar
 *  Description of the program: Does FIFO and bankers algo on tasks
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef struct Instruction {  //struct for instructions that are given from input
    char name[10];  //name of instruction
    int  id;        //the task id for that instruction
    int  rtype;     //the resource for that instruction (for compute this is the compute time)
    int  amount;    //the amount of units requested/claimed/released
} Instruction;

struct node {   //struct node used for linked list/queues for instructions
    Instruction *instr;
    struct node *next;
};

typedef struct process {  //struct for task/processes
    int  number;        //the # of the process
    int  isBlocked;     //boolean flag to check if the process is blocked
    int needToUnblock;  //flag to see if the process needs to unblock by the end of a cycle
    int  time;          //the time a task spends running
    int  waitTime;      //the time a task spends waiting
    int comptime;       //the time a task spends computing value is used for when computing
    int pendingReq;     //if blocked, keep track of the # of units it is trying to request
    int pendingRes;     //if blocked, keep track of the resource it wants to use
    int aborted;        // boolean flag to check if the process is aborted
    int *maxClaim;      //used for bankers, array of initial claims process makes for each resource. Index of the array = numbered resource
    int queueIsEmpty;   //boolean flag to see if the instruction queue of the process is empty
		// struct node * tasks;
 	 	struct node ** head;    //head of instruction queue
    struct node * empty;    //helper node to construct instruction queue
    int done;               //boolean flag to see if a process has finished/terminated
    int *using;             //array of allocated units process has from each resource. Index of the array = numbered resource


} Process;

struct Procnode {   //struct node used for linked list/queues for blocked processes
    Process *proc;
    struct Procnode *next;
};

void traverse(struct node *head) {    //helper func used for debugging/ from a1 starter kit
    struct node *temp;
    temp = head;

    while (temp != NULL) {
        printf("[%s] [%d] [%d] [%d] \n", temp->instr->name, temp->instr->id, temp->instr->rtype, temp->instr->amount);
        temp = temp->next;
    }
}

void insertAtEnd(struct node **head, Instruction * newInstr){     //insert an instruction to the instruction queue of a task
    //create a node for the instruction we want to add to the queue.
    struct node *newNode = malloc(sizeof(struct node));
      newNode->instr = newInstr;
      newNode->next = NULL;

    if(*head == NULL){
      *head = newNode;
    }
    else{ //if it is not empty, traverse until the end of the list and put the node there.


      struct node *curr = *head;

      for(*curr; curr->next != NULL; curr= curr->next);

      curr->next = newNode;
    }

 }

 void insertAtStart(struct node **head, Instruction * newInstr){  //insert an instruction to the queue at the very start
     //create a node for the process we want to add to the queue.
       (*head)->instr = newInstr;
       (*head)->next = NULL;
  }

void deleteHead(struct node **head) { //delete current head, make whatever is next to it the new head / borrowed from my a1
    *head = (*head)->next;
}

void insertAtEndBlocked(struct Procnode **head, Process *newProcess){ //blocked process queue insertion
     //create a node for the process we want to add to the queue.
        struct Procnode *newNode = malloc(sizeof(struct Procnode));
       newNode->proc = newProcess;
       newNode->next = NULL;

     //if list is empty
     if(*head == NULL){
       *head = newNode;
     }
     else{ //if it is not empty, traverse until the end of the list and put the node there.
       struct Procnode *curr = *head;

       for(*curr; curr->next != NULL; curr= curr->next);

       curr->next = newNode;
     }

}

void deleteBlocked(struct Procnode **head, int number) {    //for blocked processes queue deletion / from a1 starter kit
    struct Procnode *temp;
    struct Procnode *prev;

    temp = *head;
    // special case - beginning of list
    if (number == temp->proc->number) {
        *head = (*head)->next;
    }
    else {
        // interior or last element in the list
        prev = *head;
        temp = temp->next;
        while ((number != temp->proc->number) ) {
            prev = temp;
            temp = temp->next;
        }

        prev->next = temp->next;

    }
}

int main(int argc, char *argv[])
{
	FILE *fp;

	int num_of_res, res_value;
  int num_of_processes = 0;

	fp  = fopen(argv[1],"r");

	fscanf(fp, "%d %d ", &num_of_processes, &num_of_res);

  //two arrays one for fifo and banker Updateresources used for release instructions as the released unitsonly get usable after the cycle
  int resources[num_of_res+1];
  int resourcesBank[num_of_res+1];

  int Updateresources[num_of_res+1];
  int UpdateresourcesBank[num_of_res+1];

  int i = 0;
  //read the units for each resource and store them in the resources array
  for(i=1; i<=num_of_res; i++){
    fscanf(fp, "%d",  &res_value);
    resources[i] = res_value;
    resourcesBank[i] = res_value;
    Updateresources[i] = res_value;
    UpdateresourcesBank[i] = res_value;
  }


	if (num_of_processes > 0){
		Process *processes = malloc((num_of_processes+1) * sizeof(Process));      //make an array of processes (FIFO)
    Process *processesBank = malloc((num_of_processes+1) * sizeof(Process));  //same for bankers

		for (i=1; i<=num_of_processes; i++){
          //initialize each process in each FIFO and bankers array.
          processes[i].empty = malloc(sizeof(struct node));
					processes[i].number = i;
					processes[i].isBlocked = 0,
					processes[i].time = 0;
          processes[i].comptime = 0;
          processes[i].waitTime = 0;
					processes[i].head = &processes[i].empty;
          processes[i].done = 0;
          processes[i].aborted = 0;
          processes[i].needToUnblock = 0;
          processes[i].queueIsEmpty = 1;
          processes[i].using = malloc((num_of_res+1) * sizeof(int));
          int k;
          for(k=1; k<=num_of_res; k++){
            processes[i].using[k] = 0;
          }

          processesBank[i].empty = malloc(sizeof(struct node));
          processesBank[i].number = i;
          processesBank[i].isBlocked = 0,
          processesBank[i].time = 0;
          processesBank[i].comptime = 0;
          processesBank[i].waitTime = 0;
          processesBank[i].head = &processesBank[i].empty;
          processesBank[i].done = 0;
          processesBank[i].aborted = 0;
          processesBank[i].needToUnblock = 0;
          processesBank[i].queueIsEmpty = 1;
          processesBank[i].using = malloc((num_of_res+1) * sizeof(int));
          processesBank[i].maxClaim = malloc((num_of_res+1) * sizeof(int));
          for(k=1; k<=num_of_res; k++){
            processesBank[i].using[k] = 0;
            processesBank[i].maxClaim[k] = 0;     //bankers uses the initial claims so initialize the array for it.
          }

		}

			int currProc = 0;
      char name[10]; int id; int rtype; int amount;

      //read the instructions from the file, enqueue them based on process id
			while(fscanf(fp, "%s %d %d %d", name, &id, &rtype, &amount) != EOF){
          Instruction *newInstr = malloc(sizeof(Instruction));
          strcpy(newInstr->name, name);
          newInstr->id = id;
          newInstr->rtype = rtype;
          newInstr->amount = amount;
            //insert for both FIFO and BANKER processes
            currProc = id;
						if(processes[currProc].queueIsEmpty == 1){

              insertAtStart(processes[currProc].head, newInstr);

              insertAtStart(processesBank[currProc].head, newInstr);
              processes[currProc].queueIsEmpty = 0;
              processesBank[currProc].queueIsEmpty = 0;
						}else{

              insertAtEnd(processes[currProc].head, newInstr);

              insertAtEnd(processesBank[currProc].head, newInstr);

            }

					}


    fclose(fp);

    int runningProc = num_of_processes;
    //keep track of active nodes and blocked used for deadlock detection
    int num_of_active = 0;
    int num_of_blocked = 0;

    //initialize the blocked queue for FIFO
    struct Procnode * headNode = NULL;
    struct Procnode ** blockedHead = &headNode;
    struct Procnode *curr;

    //FIFO algo
    while(runningProc > 0){
        //check blocked processes first
        curr = *blockedHead;
       //only check blocked processes if the queue isn't empty
          for(*curr; curr != NULL; curr= curr->next){
              curr->proc->time++;
              if(resources[curr->proc->pendingRes] - curr->proc->pendingReq >= 0){  //if the request can be satisfiyed allow the request
                resources[curr->proc->pendingRes] -= curr->proc->pendingReq;
                Updateresources[curr->proc->pendingRes] -= curr->proc->pendingReq;   //subtract resources by amount
                curr->proc->using[curr->proc->pendingRes] += curr->proc->pendingReq;;
                curr->proc->needToUnblock = 1;      //flag the process so it is unblocked next cycle.
                num_of_blocked--;
                deleteHead(curr->proc->head);     //dequeue the instruction
                deleteBlocked(blockedHead, curr->proc->number);   //delete process from blocked queue.

              }
          }


        i = 1;
        for(i=1; i<=num_of_processes; i++){
            if(processes[i].done != 1 && processes[i].isBlocked != 1){      //now check for processes that are not blocked and not finished

              if(strcmp((*processes[i].head)->instr->name, "initiate") == 0){
                //for initiate start incrementing the time for the process and pop the instruction.
                if(processes[i].time == 0){num_of_active++;}
                processes[i].time++;
    						deleteHead(processes[i].head);

    					}
              else if(strcmp((*processes[i].head)->instr->name, "request") == 0){
                processes[i].time++;
                if(resources[(*processes[i].head)->instr->rtype] - (*processes[i].head)->instr->amount >= 0){ //check if request is feasible for given resource
                  resources[(*processes[i].head)->instr->rtype] -= (*processes[i].head)->instr->amount;
                  Updateresources[(*processes[i].head)->instr->rtype] -= (*processes[i].head)->instr->amount;   //subtract resources by amount
                  processes[i].using[(*processes[i].head)->instr->rtype] += (*processes[i].head)->instr->amount;
                  deleteHead(processes[i].head);  //pop this instruction
                } else{ //if task is asking for too much than possible from resource block it for now.
                    processes[i].isBlocked = 1; //block the process
                    processes[i].pendingRes = (*processes[i].head)->instr->rtype;
                    processes[i].pendingReq = (*processes[i].head)->instr->amount;
                    insertAtEndBlocked(blockedHead, &processes[i]);   //add to blocked queue
                    num_of_blocked++;
                }

    					}
              else if(strcmp((*processes[i].head)->instr->name, "release") == 0){

                  processes[i].time++;
                  Updateresources[(*processes[i].head)->instr->rtype] += (*processes[i].head)->instr->amount;   //subtract resources by amount
                  processes[i].using[(*processes[i].head)->instr->rtype] -= (*processes[i].head)->instr->amount;  //relase the units it was using for that resource
                  deleteHead(processes[i].head);  //pop this instruction

    					}
              else if(strcmp((*processes[i].head)->instr->name, "compute") == 0){

                  processes[i].time++;
                  processes[i].comptime++;  //start the comp time
                  if(processes[i].comptime >= (*processes[i].head)->instr->rtype){  //once comptime equals or exceeds the comptime in instruction, pop the instruction else do nothing
                    processes[i].comptime = 0;      //reset comptime in case of future computes
                    deleteHead(processes[i].head);  //pop this instruction
                  }

    					}

            }

            if(processes[i].needToUnblock == 1){    //unblock processes that need to be unblocked
              processes[i].needToUnblock = 0;
              processes[i].isBlocked = 0;
            }

            if(processes[i].isBlocked == 1){    //if a process is blocked increment its wait time.
              processes[i].waitTime++;
            }

        }
        // detect deadlock = when all active tasks have outstanding requests/i.e. are blocked.
        if(num_of_active == num_of_blocked && num_of_active != 0){
          for(i=1; i<=num_of_processes; i++){
            if(processes[i].done != 1 && processes[i].isBlocked == 1){
                processes[i].done = 1;
                processes[i].aborted = 1; //abort the proc
                runningProc--;
                num_of_active--;
                num_of_blocked--;
                int j;
                for(j =1; j<=num_of_res; j++){ //return allocated units back to resources.
                    resources[j] += processes[i].using[j];
                    Updateresources[j] += processes[i].using[j];
                }

                deleteBlocked(blockedHead, processes[i].number); //remove this task/process from the blocked queue
                if((*blockedHead) == NULL){
                  break;
                }

                if(resources[(*blockedHead)->proc->pendingRes] - (*blockedHead)->proc->pendingReq >= 0){ //if after aborting process the blocked head can satisy req then stop aborting processes.
                    num_of_active++;
                    break;
                }
            }
          }
        }

        for(i=1; i<=num_of_processes; i++){
          if((strcmp((*processes[i].head)->instr->name, "terminate") == 0) && processes[i].done != 1){  //if a process terminated flag it as done.
            num_of_active--;
            processes[i].done = 1;
            runningProc--;
          }
        }

        for(i=1; i<=num_of_res; i++){     //since released units only become available next cycle, update our resources array so it has those units next cycle.
          resources[i] = Updateresources[i];
        }


	}



  int runningProcBank = num_of_processes;
  int num_of_activeBank = 0;
  int num_of_blockedBank = 0;
  //initialize blockedqueue for bankers
  struct Procnode * headNodeBank = NULL;
  struct Procnode ** blockedHeadBank = &headNodeBank;
  struct Procnode *currBank;
  //Banker algo
  while(runningProcBank > 0){
      //check blocked processes first
      currBank = *blockedHeadBank;
     //only check blocked processes if the queue isn't empty
        for(*currBank; currBank != NULL; currBank= currBank->next){
            currBank->proc->time++;
            //check if state is safe, if so allow the request.
            if(currBank->proc->maxClaim[currBank->proc->pendingRes] - currBank->proc->using[currBank->proc->pendingRes] <= resourcesBank[currBank->proc->pendingRes]){
              if(resourcesBank[currBank->proc->pendingRes] - currBank->proc->pendingReq >= 0){
                resourcesBank[currBank->proc->pendingRes] -= currBank->proc->pendingReq;
                UpdateresourcesBank[currBank->proc->pendingRes] -= currBank->proc->pendingReq;   //subtract resources by amount
                currBank->proc->using[currBank->proc->pendingRes] += currBank->proc->pendingReq;
                currBank->proc->needToUnblock = 1;
                num_of_blockedBank--;
                deleteHead(currBank->proc->head);
                deleteBlocked(blockedHeadBank, currBank->proc->number);
              }
            }

        }


      i = 1;
      for(i=1; i<=num_of_processes; i++){
        if(processesBank[i].done != 1 && processesBank[i].isBlocked != 1){

          if(strcmp((*processesBank[i].head)->instr->name, "initiate") == 0){
            if(resourcesBank[(*processesBank[i].head)->instr->rtype] - (*processesBank[i].head)->instr->amount < 0){  //if he initial claim exceeds the # of units in resource abort the process
              processesBank[i].done = 1;
              processesBank[i].aborted = 1;
              runningProcBank--;
            }else{    //else activate the process increment its time pop the instruction.

              if(processesBank[i].time == 0){num_of_activeBank++;}
              processesBank[i].time++;
              processesBank[i].maxClaim[(*processesBank[i].head)->instr->rtype] = (*processesBank[i].head)->instr->amount;
              deleteHead(processesBank[i].head);
            }

          }
          else if(strcmp((*processesBank[i].head)->instr->name, "request") == 0){
            processesBank[i].time++;

            if( (*processesBank[i].head)->instr->amount > processesBank[i].maxClaim[(*processesBank[i].head)->instr->rtype] ){ //if it requests more than its initial claim for that resource, abort
              processesBank[i].done = 1;
              processesBank[i].aborted = 1;
              runningProcBank--;
              continue;
            }
            if(processesBank[i].maxClaim[(*processesBank[i].head)->instr->rtype] - processesBank[i].using[(*processesBank[i].head)->instr->rtype] > resourcesBank[(*processesBank[i].head)->instr->rtype]){
              processesBank[i].isBlocked = 1; //block the process
              processesBank[i].pendingRes = (*processesBank[i].head)->instr->rtype;
              processesBank[i].pendingReq = (*processesBank[i].head)->instr->amount;
              insertAtEndBlocked(blockedHeadBank, &processesBank[i]);
              num_of_blockedBank++;
            }
            else if(resourcesBank[(*processesBank[i].head)->instr->rtype] - (*processesBank[i].head)->instr->amount >= 0){ //check if request is feasible for given resource
              resourcesBank[(*processesBank[i].head)->instr->rtype] -= (*processesBank[i].head)->instr->amount;
              UpdateresourcesBank[(*processesBank[i].head)->instr->rtype] -= (*processesBank[i].head)->instr->amount;   //subtract resourcesBank by amount
              processesBank[i].using[(*processesBank[i].head)->instr->rtype] += (*processesBank[i].head)->instr->amount;
              deleteHead(processesBank[i].head);  //pop this instruction
            }

          }
          else if(strcmp((*processesBank[i].head)->instr->name, "release") == 0){

              processesBank[i].time++;
              UpdateresourcesBank[(*processesBank[i].head)->instr->rtype] += (*processesBank[i].head)->instr->amount;   //subtract resourcesBank by amount
              processesBank[i].using[(*processesBank[i].head)->instr->rtype] -= (*processesBank[i].head)->instr->amount;
              deleteHead(processesBank[i].head);  //pop this instruction

          }
          else if(strcmp((*processesBank[i].head)->instr->name, "compute") == 0){

              processesBank[i].time++;
              processesBank[i].comptime++;  //add comptime
              if(processesBank[i].comptime >= (*processesBank[i].head)->instr->rtype){
                processesBank[i].comptime = 0;      //reset comptime
                deleteHead(processesBank[i].head);  //pop this instruction
              }

          }

        }

        if(processesBank[i].needToUnblock == 1){    //unblock processes that need to be unblocked
          processesBank[i].needToUnblock = 0;
          processesBank[i].isBlocked = 0;
        }

        if(processesBank[i].isBlocked == 1){    //increment waittime if process is blocked.
          processesBank[i].waitTime++;
        }

      }


      for(i=1; i<=num_of_processes; i++){
        if((strcmp((*processesBank[i].head)->instr->name, "terminate") == 0) && processesBank[i].done != 1){    //if process terminated flag it as done
          num_of_activeBank--;
          processesBank[i].done = 1;
          runningProcBank--;
        }
      }

      for(i=1; i<=num_of_res; i++){   //update resources for next cycle
        resourcesBank[i] = UpdateresourcesBank[i];
      }

    }


        //print results.
      int totalTime = 0; int totalWait = 0;    //used to calculate total of fifo
      int totalTimeBank = 0; int totalWaitBank = 0;   //used to calculate total of BANKER
      printf("              FIFO                             BANKER'S\n");
      for(i = 1; i<=num_of_processes; i++){
        printf("     Task %d      ", processes[i].number);
        if(processes[i].aborted == 1){
          printf("aborted              ");
        }else{
          printf("%d   %d  %d%%           ", processes[i].time, processes[i].waitTime , (int)round(((double)processes[i].waitTime / (double)processes[i].time) * 100) );   //last %d calculates percentage of wait and running time
          totalTime += processes[i].time; totalWait += processes[i].waitTime;
        }

        printf("Task %d        ", processesBank[i].number);
        if(processesBank[i].aborted == 1){
          printf("aborted\n");
        }else{
          printf("%d   %d  %d%% \n", processesBank[i].time, processesBank[i].waitTime , (int)round(((double)processesBank[i].waitTime / (double)processesBank[i].time) * 100) ); //last %d calculates percentage of wait and running time
          totalTimeBank += processesBank[i].time; totalWaitBank += processesBank[i].waitTime;
        }

      }
      printf("     total       %d   %d  %d%%           ", totalTime, totalWait , (int)round(((double)totalWait / (double)totalTime) * 100) ); //last %d calculates percentage of wait and running time
      printf("total         %d   %d  %d%%\n", totalTimeBank, totalWaitBank, (int)round(((double)totalWaitBank / (double)totalTimeBank) * 100) ); //last %d calculates percentage of wait and running time
  }




	return 0;
}
