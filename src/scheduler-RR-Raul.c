#include <stdio.h>
#include "utlist.h"
#include "utils.h"

#include "memory_controller.h"
#include "params.h"

extern long long int CYCLE_VAL;

void init_scheduler_vars()
{
	// initialize all scheduler variables here

	return;
}

// write queue high water mark; begin draining writes if write queue exceeds this value
#define HI_WM 40

// end write queue drain once write queue has this many writes in it
#define LO_WM 20

// 1 means we are in write-drain mode for that channel
int drain_writes[MAX_NUM_CHANNELS];

/* Each cycle it is possible to issue a valid command from the read or write queues
   OR
   a valid precharge command to any bank (issue_precharge_command())
   OR
   a valid precharge_all bank command to a rank (issue_all_bank_precharge_command())
   OR
   a power_down command (issue_powerdown_command()), programmed either for fast or slow exit mode
   OR
   a refresh command (issue_refresh_command())
   OR
   a power_up command (issue_powerup_command())
   OR
   an activate to a specific row (issue_activate_command()).

   If a COL-RD or COL-WR is picked for issue, the scheduler also has the
   option to issue an auto-precharge in this cycle (issue_autoprecharge()).

   Before issuing a command it is important to check if it is issuable. For the RD/WR queue resident commands, checking the "command_issuable" flag is necessary. To check if the other commands (mentioned above) can be issued, it is important to check one of the following functions: is_precharge_allowed, is_all_bank_precharge_allowed, is_powerdown_fast_allowed, is_powerdown_slow_allowed, is_powerup_allowed, is_refresh_allowed, is_autoprecharge_allowed, is_activate_allowed.
   */

int quantum=5;
int ProcWrite=0;
int i=0,j=0;
int startflag=0;
void schedule(int channel)
{



	//printf("Channel %d \n",i++ );
	request_t * rd_ptr = NULL;
	request_t * wr_ptr = NULL;
	if (startflag==0 && write_queue_length[channel] <1)
	{
		ProcWrite=1;
		startflag=1;
	}


	if (ProcWrite==0)
	{

		if (write_queue_length[channel] >0)
		{
			LL_FOREACH(write_queue_head[channel],wr_ptr)
			{

				if (wr_ptr->command_issuable==1)
				{
					printf("WRITE address to process %lx \n", wr_ptr->dram_addr.actual_address);
					if (wr_ptr->next_command==COL_WRITE_CMD)
					{
						/*count number of reads issued*/
						j++;
					}
					if (j<=quantum)
					{
						issue_request_command(wr_ptr);
						printf("\nwrite queue count: %d \n", write_queue_length[channel]);
						if  (j==quantum)
						{
							printf("restarting count %d\n", j);
							j=0;
							ProcWrite=1;
						}
						break;
					}
				}
			}
		}
		else
			ProcWrite=1;
	}
	if (ProcWrite==1)
	{
		if (read_queue_length[channel] > 0)
		{
			LL_FOREACH(read_queue_head[channel],rd_ptr)
			{

				if (rd_ptr->command_issuable==1)
				{
					printf("READ address to process %lx \n", rd_ptr->dram_addr.actual_address);
					if (rd_ptr->next_command==COL_READ_CMD)
					{
						/*count number of reads issued*/
						i++;
					}

					if (i<=quantum)
					{
						issue_request_command(rd_ptr);
						printf("\nread queue count: %d \n", read_queue_length[channel]);

						if (i==quantum)
						{
							printf("restarting count %d\n", i);
							i=0;
							ProcWrite=0;

						}
						break;
					}

				}
			}
		}
		else
			ProcWrite=0; //switch to read when no pending actions
	}

		return;



}

void scheduler_stats()
{
  /* Nothing to print for now. */
}

