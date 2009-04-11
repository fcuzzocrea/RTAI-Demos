/*
COPYRIGHT (C) 2008  Paolo Mantegazza (mantegazza@aero.polimi.it)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main(void)
{
	int sock, msgd, fd;
	struct sockaddr_in SPRT_ADDR;
	char buf[200];

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&SPRT_ADDR, sizeof(struct sockaddr_in));
        SPRT_ADDR.sin_family = AF_INET;
	SPRT_ADDR.sin_port = htons(5000);
        SPRT_ADDR.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sock, (struct sockaddr *)&SPRT_ADDR, sizeof(struct sockaddr_in));

	msgd = msgget(0xcacca, 0x666);

	fd = open("echo", O_RDWR);

	while (1) {
		recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
		printf("SOK: %s", buf);
		msgrcv(msgd, buf, sizeof(buf), 1, 0);
		printf("IPC: %s", buf);
		read(fd, buf, sizeof(buf));
		printf("RED: %s", buf);
	}
	close(sock);
	return 0;
}
