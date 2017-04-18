/*
 * tasksend.c
 *
 *  Created on: 7 avr. 2010
 *      Author: hemichel
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/io.h>
#include <sys/mman.h>

#include "task.h"
#undef rt_printf
#define rt_printf printf

#define STACK_SIZE 8192
#define STD_PRIO 1

#define MAXRECVTASK 0
#define ANYSRC 0
#define MAX_MESSAGE_LENGTH 4

RT_TASK test_task_ptr, test_task2_ptr, test_task3_ptr;

int int_count = 0;
int end = 0;

RTIME task_period_ns = 100000;
#define SEND_TIMEOT 1000000000
#define RECV_TIMEOT 1000000000

void testtask(void *cookie)
{
        RT_TASK_MCB mcb_send, mcb_reply;
        long long datasend = 1234567890;
        long long datareply;
	int rv;
        int count = 0;

	rt_task_sleep(rt_timer_ns2tsc(100000000));

        mcb_send.opcode = 0x03;
        datasend        = 1234567890;
        mcb_send.data   = &datasend;
        mcb_send.size   = sizeof(datasend);

        mcb_reply.data = &datareply;
        mcb_reply.size = sizeof(datareply);

        while (!end) {
                count++;
		if (count & 1) {
	                rv = rt_task_send(&test_task3_ptr, &mcb_send, &mcb_reply, SEND_TIMEOT);
		} else {
	                rv = rt_task_send(&test_task2_ptr, &mcb_send, &mcb_reply, SEND_TIMEOT);
		}
                if (rv < 0) {
			rt_printf("rt_task_send error %d.\n", rv);
		} else {
			rt_printf("rt_task_send got the reply = %lld.\n", datareply);
		}
		rt_task_sleep(rt_timer_ns2tsc(task_period_ns));
        }
	end = 1;
}

void testtask2(void *cookie)
{
        RT_TASK_MCB mcb_rcv, mcb_reply;
	int rcvret;
	long long datareply;

        while (!end) {
                mcb_rcv.data = &datareply;
                mcb_rcv.size = sizeof(datareply);

                rcvret = rt_task_receive(&mcb_rcv, RECV_TIMEOT);
                rt_printf("%s recret = %d data = %lld.\n", __FUNCTION__, rcvret, datareply);
                if(rcvret >= 0) {
			datareply = 22222222;
			mcb_reply.opcode = 1;
			mcb_reply.data = &datareply;
			mcb_reply.size = sizeof(datareply);
			rt_task_reply(mcb_rcv.flowid, &mcb_reply);
			rt_printf("task2 replied to Sender.\n");
                } else {
                        rt_printf("err task2 : rcvret < 0.\n");
                }
        }
}


void testtask3(void *cookie) {
        RT_TASK_MCB mcb_rcv, mcb_reply;
	int rcvret;
	long long  datareply;

        while(!end){
                mcb_rcv.data = &datareply;
                mcb_rcv.size = sizeof(datareply);

                rcvret = rt_task_receive(&mcb_rcv, RECV_TIMEOT);
                rt_printf("%s rcvret = %d data = %lld.\n", __FUNCTION__, rcvret, datareply);
                if(rcvret >= 0) {
			datareply = 33333333;
			mcb_reply.opcode = 1;
			mcb_reply.data = &datareply;
			mcb_reply.size = sizeof(datareply);
			rt_task_reply(mcb_rcv.flowid, &mcb_reply);
			rt_printf("task3 replied to Sender.\n");
                } else {
                        rt_printf("err task3 : rcvret < 0.\n");
                }
        }
}

void clean_exit(int dummy)
{
        rt_printf("cleanup\n");
        end = 1;
        rt_task_delete(&test_task_ptr);
        rt_printf("end\n");
}

int main(int argc, char *argv[])
{
        signal(SIGTERM, clean_exit);
        signal(SIGINT, clean_exit);

	rt_timer_start(0);
        mlockall(MCL_CURRENT | MCL_FUTURE);

        rt_print_auto_init(1);

        if (rt_task_spawn(&test_task_ptr, "Tmr", STACK_SIZE, STD_PRIO, 0, testtask, NULL)) {
                rt_printf("error rt_task_spawn\n");
                return 0;
        }
        if (rt_task_spawn(&test_task2_ptr, "Tmr2", STACK_SIZE, STD_PRIO, 0, testtask2, NULL)) {
                rt_printf("error rt_task_spawn\n");
                return 0;
        }
        if (rt_task_spawn(&test_task3_ptr, "Tmr3", STACK_SIZE, STD_PRIO, 0, testtask3, NULL)) {
                rt_printf("error rt_task_spawn\n");
                return 0;
        }
        pause();
        return 0;
}
