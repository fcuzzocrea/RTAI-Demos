/* Copyright (C) 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/mman.h>

#include <rtai_posix.h>


#ifndef INIT
# define INIT PTHREAD_MUTEX_INITIALIZER
#endif


static pthread_mutex_t lock;


#define STACK_SIZE (1 * 1024 * 1024)
#define ROUNDS 1000
#define N 100


static void *
tf (void *arg)
{
  int nr = (long int) arg;
  int cnt;
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000 };

  char name[8];

  sprintf(name, "TSK%d", nr);
  pthread_setschedparam_np(nr, SCHED_FIFO, 0, 0xF, PTHREAD_HARD_REAL_TIME_NP);
  rt_grow_and_lock_stack(STACK_SIZE/2);

  for (cnt = 0; cnt < ROUNDS; ++cnt)
    {
      if (pthread_mutex_lock (&lock) != 0)
	{
	  printf ("thread %d: failed to get the lock\n", nr);
	  return (void *) 1l;
	}

      if (pthread_mutex_unlock (&lock) != 0)
	{
	  printf ("thread %d: failed to release the lock\n", nr);
	  return (void *) 1l;
	}

// needed to avoid frozing Linux
      pthread_soft_real_time_np();
      pthread_hard_real_time_np();
    }

  return NULL;
}


static int
do_test (void)
{
  pthread_attr_t at;
  pthread_t th[N];
  int cnt;

  if (pthread_attr_init (&at) != 0)
    {
      puts ("attr_init failed");
      return 1;
    }

  if (pthread_attr_setstacksize (&at, STACK_SIZE) != 0)
    {
      puts ("attr_setstacksize failed");
      return 1;
    }

// RTAI
  pthread_mutex_init(&lock, NULL);

  if (pthread_mutex_lock (&lock) != 0)
    {
      puts ("locking in parent failed");
      return 1;
    }

  for (cnt = 0; cnt < N; ++cnt) {
    if (pthread_create (&th[cnt], &at, tf, (void *) (long int) cnt) != 0)
      {
	printf ("creating thread %d failed\n", cnt);
	return 1;
      }
      nanosleep (&(struct timespec){ 0, 1000000000/N }, NULL);
  }

  if (pthread_attr_destroy (&at) != 0)
    {
      puts ("attr_destroy failed");
      return 1;
    }

RTIME t = rt_get_cpu_time_ns();
  if (pthread_mutex_unlock (&lock) != 0)
    {
      puts ("unlocking in parent failed");
      return 1;
    }

  for (cnt = 0; cnt < N; ++cnt)
    if (pthread_join (th[cnt], NULL) != 0)
      {
	printf ("joining thread %d failed\n", cnt);
	return 1;
      }
printf(">>>>> %lld\n", rt_get_cpu_time_ns() - t);
  return 0;
}

#define TIMEOUT 60
int main(void)
{
	pthread_setschedparam_np(0, SCHED_FIFO, 0, 0xF, PTHREAD_HARD_REAL_TIME_NP);
	rt_grow_and_lock_stack(STACK_SIZE/2);
	start_rt_timer(0);
        do_test();
        return 0;
}
