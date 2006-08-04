/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <rtai_posix.h>

static void
remove_sem (int status, void *arg)
{
  sem_unlink (arg);
}


int
main (void)
{
  sem_t *s;
  sem_t *s2;
  sem_t *s3;

  pthread_setschedparam_np(0, SCHED_FIFO, 0, 0xF, PTHREAD_HARD_REAL_TIME_NP);
//  pthread_init_real_time_np("MAIN", 0, SCHED_FIFO, 0xF, PTHREAD_HARD_REAL_TIME);
  s = sem_open ("tstsm7", O_CREAT, 0600, 1);
  if (s == SEM_FAILED)
    {
      if (errno == ENOSYS)
	{
	  puts ("sem_open not supported.  Oh well.");
	  return 0;
	}

      /* Maybe the shm filesystem has strict permissions.  */
      if (errno == EACCES)
	{
	  puts ("sem_open not allowed.  Oh well.");
	  return 0;
	}

      printf ("sem_open: %m\n");
      return 1;
    }

  on_exit (remove_sem, (void *) "tstsm7");

  /* We have the semaphore object.  Now try again.  We should get the
     same address.  */
  s2 = sem_open ("tstsm7", O_CREAT, 0600, 1);
  if (s2 == SEM_FAILED)
    {
      puts ("2nd sem_open failed");
      return 1;
    }
  if (s != s2)
    {
      puts ("2nd sem_open didn't return the same address");
      return 1;
    }

  /* And again, this time without O_CREAT.  */
  s3 = sem_open ("tstsm7", 0);
  if (s3 == SEM_FAILED)
    {
      puts ("3rd sem_open failed");
      return 1;
    }
  if (s != s3)
    {
      puts ("3rd sem_open didn't return the same address");
      return 1;
    }

  /* Now close the handle.  Three times.  */
  if (sem_close (s2) != 0)
    {
      puts ("1st sem_close failed");
      return 1;
    }
  if (sem_close (s) != 0)
    {
      puts ("2nd sem_close failed");
      return 1;
    }
  if (sem_close (s3) != 0)
    {
      puts ("3rd sem_close failed");
      return 1;
    }

  return 0;
}
