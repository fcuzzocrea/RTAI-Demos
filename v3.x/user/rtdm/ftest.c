/*
 * rt_serial_uprog.c
 *
 * Userspace-Testprogramm für den Echtzeit-Serial-Treiber unter
 * RTAI mit RTDM,
 * 07/2005 by Joerg Langenberg <joergel75@gmx.net>
 * 11/2005 by Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/poll.h>

#include <rtai_lxrt.h>
#include <rtdm/rtserial.h>

#define LOG_PREFIX    "rt_serial_uprog: "
#define WTASK_PREFIX  "write_task: "
#define RTASK_PREFIX  "read_task : "

#define WRITE_FILE    "rtser0"
#define READ_FILE     "rtser1"

int timer_started = 0;
int read_fd  = -1;
int write_fd = -1;

#define STATE_FILE_OPENED         1
#define STATE_TASK_CREATED        2

unsigned int read_state = 0;
unsigned int write_state = 0;

//                           --s-ms-us-ns
RTIME write_task_period_ns =    2500000llu;
RT_TASK *write_task;
RT_TASK *read_task;

#define TIMEOUT  100000000

static const struct rtser_config read_config = {
    0xFFFF,                     /* config_mask */
    115200,                     /* baud_rate */
    RTSER_DEF_PARITY,           /* parity */
    RTSER_DEF_BITS,             /* data_bits */
    RTSER_DEF_STOPB,            /* stop_bits */
    RTSER_DEF_HAND,             /* handshake */
    RTSER_DEF_FIFO_DEPTH,       /* fifo_depth*/
    RTSER_DEF_TIMEOUT,          /* rx_timeout */
    RTSER_DEF_TIMEOUT,          /* tx_timeout */
    TIMEOUT,                    /* event_timeout */
    RTSER_RX_TIMESTAMP_HISTORY, /* timestamp_history */
    RTSER_EVENT_RXPEND          /* event mask */
};

static const struct rtser_config write_config = {
    0xFFFF,                     /* config_mask */
    115200,                     /* baud_rate */
    RTSER_DEF_PARITY,           /* parity */
    RTSER_DEF_BITS,             /* data_bits */
    RTSER_DEF_STOPB,            /* stop_bits */
    RTSER_DEF_HAND,             /* handshake */
    RTSER_DEF_FIFO_DEPTH,       /* fifo_depth*/
    RTSER_DEF_TIMEOUT,          /* rx_timeout */
    RTSER_DEF_TIMEOUT,          /* tx_timeout */
    RTSER_DEF_TIMEOUT,          /* event_timeout */
    RTSER_DEF_TIMESTAMP_HISTORY /* timestamp_history */
};

static int close_file(int fd, char *name)
{
  int ret, i = 0;
  do {
    i++;
    ret = rt_dev_close(fd);
    switch(-ret){
      case EBADF:   printf(LOG_PREFIX "%s -> invalid fd or context\n",name);
                    break;
      case EAGAIN:  printf(LOG_PREFIX "%s -> EAGAIN (%d times)\n",name,i);
                    rt_sleep(nano2count(50000)); // wait 50us
                    break;
      case 0:       printf(LOG_PREFIX "%s -> closed\n",name);
                    break;
      default:      printf(LOG_PREFIX "%s -> ???\n",name);
                    break;
    }
  } while (ret == -EAGAIN && i < 10);
  return ret;
}

void cleanup_all(void)
{
  if (read_state & STATE_FILE_OPENED) {
    close_file( read_fd, READ_FILE" (read)");
    read_state &= ~STATE_FILE_OPENED;
  }

  if (write_state & STATE_FILE_OPENED) {
    close_file( write_fd, WRITE_FILE " (write)");
    write_state &= ~STATE_FILE_OPENED;
  }

  if (write_state & STATE_TASK_CREATED) {
    printf(LOG_PREFIX "delete write_task\n");
//    rt_task_delete(write_task);
    write_state &= ~STATE_TASK_CREATED;
  }

  if (read_state & STATE_TASK_CREATED) {
    printf(LOG_PREFIX "delete read_task\n");
    rt_task_delete(read_task);
    read_state &= ~STATE_TASK_CREATED;
  }

  if (timer_started) {
    printf(LOG_PREFIX "stop timer\n");
    stop_rt_timer();
  }
}

void catch_signal(int sig)
{
  cleanup_all();
  printf(LOG_PREFIX "exit\n");
  return;
}

void write_task_proc(void *arg)
{
  int ret;
  RTIME write_time;
  ssize_t sz = sizeof(RTIME);
  ssize_t written = 0;

  write_task = rt_task_init_schmod(nam2num("WRTSK"), 2, 0, 0, SCHED_FIFO, 0xF);
  poll(0, 0, 10);
  mlockall(MCL_CURRENT | MCL_FUTURE);
  rt_make_hard_real_time();
  rt_task_make_periodic_relative_ns(NULL, 0, write_task_period_ns);

  while (1) {

    ret = rt_task_wait_period();
    if (ret) {
      printf(WTASK_PREFIX "error while rt_task_wait_period, code %d\n",ret);
      goto exit_write_task;
    }

    write_time = rt_get_time_ns();
    written = rt_dev_write(write_fd, &write_time, sz);
    if (written != sz ) {
      if (written < 0 ) {
        printf(WTASK_PREFIX "error while rt_dev_write, code %d\n",written);
      } else {
        printf(WTASK_PREFIX "only %d / %d byte transmitted\n",written, sz);
      }
      goto exit_write_task;
    }
  }

exit_write_task:
  if (write_state & STATE_FILE_OPENED) {
    if (!close_file( write_fd, WRITE_FILE " (write)")) {
      write_state &= ~STATE_FILE_OPENED;
    }
  rt_make_soft_real_time();
  rt_task_delete(write_task);
  }

  printf(WTASK_PREFIX "exit\n");
}

void read_task_proc(void *arg)
{
  int ret;
  int nr = 0;
  RTIME read_time  = 0;
  RTIME write_time = 0;
  RTIME irq_time   = 0;
  ssize_t sz = sizeof(RTIME);
  ssize_t red = 0;
  struct rtser_event rx_event;

  printf(" Nr |   write->irq    |    irq->read    |   write->read   |\n");
  printf("-----------------------------------------------------------\n");

  read_task = rt_task_init_schmod(nam2num("RDTSK"), 1, 0, 0, SCHED_FIFO, 0xF);
  poll(0, 0, 10);
  mlockall(MCL_CURRENT | MCL_FUTURE);
  rt_make_hard_real_time();

  while (1) {

    /* waiting for event */
    ret = rt_dev_ioctl(read_fd, RTSER_RTIOC_WAIT_EVENT, &rx_event );
    if (ret) {
      printf(RTASK_PREFIX "error while RTSER_RTIOC_WAIT_EVENT, code %d\n",ret);
      goto exit_read_task;
    }

    irq_time = rx_event.rxpend_timestamp;
    red = rt_dev_read(read_fd, &write_time, sz);
    if (red == sz ) {
      read_time = rt_get_time_ns();
      printf("%3d |%16llu |%16llu |%16llu\n", nr,
             irq_time  - write_time,
             read_time - irq_time,
             read_time - write_time);
      nr++;
    } else {
      if (red < 0 ) {
        printf(RTASK_PREFIX "error while rt_dev_read, code %d\n",red);
      } else {
        printf(RTASK_PREFIX "only %d / %d byte received \n",red,sz);
      }
      goto exit_read_task;
    }
  }

exit_read_task:
  if (read_state & STATE_FILE_OPENED) {
    if (!close_file( read_fd, READ_FILE " (read)")) {
      read_state &= ~STATE_FILE_OPENED;
    }
  }
  rt_make_soft_real_time();
  rt_task_delete(read_task);

  printf(RTASK_PREFIX "exit (you can let the write task running if testing unconnected)\n");
}

static pthread_t write_thr, read_thr;

int main(int argc, char* argv[])
{
  RT_TASK *maint;
  int ret = 0;

  signal(SIGTERM, catch_signal);
  signal(SIGINT, catch_signal);
  signal(SIGKILL, catch_signal);

  /* no memory-swapping for this programm */

    maint = rt_task_init_schmod(nam2num("MAIN"), 3, 0, 0, SCHED_FIFO, 0xF);
    rt_set_oneshot_mode();
    start_rt_timer(0);

  /* open rtser0 */
  write_fd = rt_dev_open(WRITE_FILE, 0);
  if (write_fd < 0) {
    printf(LOG_PREFIX "can't open %s (write)\n", WRITE_FILE);
    goto error;
  }
  write_state |= STATE_FILE_OPENED;
  printf(LOG_PREFIX "write-file opened\n");

  /* writing write-config */
  ret = rt_dev_ioctl(write_fd, RTSER_RTIOC_SET_CONFIG, &write_config);
  if (ret) {
    printf(LOG_PREFIX "error while RTSER_RTIOC_SET_CONFIG, code %d\n",ret);
    goto error;
  }
  printf(LOG_PREFIX "write-config written\n");

  /* open rtser1 */
  read_fd = rt_dev_open(READ_FILE, 0);
  if (read_fd < 0) {
    printf(LOG_PREFIX "can't open %s (read)\n", READ_FILE);
    goto error;
  }
  read_state |= STATE_FILE_OPENED;
  printf(LOG_PREFIX "read-file opened\n");

  /* writing read-config */
  ret = rt_dev_ioctl(read_fd, RTSER_RTIOC_SET_CONFIG, &read_config);
  if (ret) {
    printf(LOG_PREFIX "error while rt_dev_ioctl, code %d\n",ret);
    goto error;
  }
  printf(LOG_PREFIX "read-config written\n");

  /* create read_task */
  pthread_create(&read_thr, NULL, (void *)read_task_proc, NULL);
  read_state |= STATE_TASK_CREATED;
  printf(LOG_PREFIX "read-task created\n");

  /* create write_task */
  pthread_create(&write_thr, NULL, (void *)write_task_proc, NULL);
  write_state |= STATE_TASK_CREATED;
  printf(LOG_PREFIX "write-task created\n");

  pause();
  return 0;

error:
  cleanup_all();
  rt_task_delete(maint);
  return ret;
}

