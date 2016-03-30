/*A simple server framework to handle client using select
 *Author: newsun
 *Date: 2016.3.25
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace std;

#define MAX_FD		2048
#define MAX_BUFFER	2048

typedef struct {
	int	flag;
	int	fd;
} t_connfd;

int main()
{
	int 		listenFd;

	if ( (listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		cout << "create socket error!" << endl;
	}

	sockaddr_in	addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if ( bind(listenFd, (sockaddr *)&addr, (socklen_t)sizeof(addr)) < 0 ) {
		cout << "bind error!" << endl;
	}

	if ( listen(listenFd, 5) < 0 ) {
		cout << "listen error!" << endl;
	}

	fd_set		rset;
	int		maxfd = listenFd;
	int		retnfd;
	t_connfd	fdslot[MAX_FD]; //store connFd
	bzero(fdslot, MAX_FD*sizeof(t_connfd));
	char		buffer[MAX_BUFFER];
	FD_ZERO(&rset);
	
	for (; ;) {
start:
		FD_SET(listenFd, &rset);
		for (int i = 0; i < MAX_FD; i++) {
			if(fdslot[i].flag) {
				FD_SET(fdslot[i].fd, &rset);
				if (fdslot[i].fd > maxfd) {
					maxfd = fdslot[i].fd;
				}
			}
		}

		if ( (retnfd=select(maxfd + 1, &rset, NULL, NULL, NULL)) > 0 ) {
			if (FD_ISSET(listenFd, &rset)) {
				int connFd;
				if ( (connFd = accept(listenFd, NULL, NULL)) < 0 ) {
					cout << "accept error!" << endl;
				}
				
				/*find the empty slot*/
				int ii;
				for (ii = 0; ii != MAX_FD; ii++) {
					if (fdslot[ii].flag == 0) {
						fdslot[ii].fd = connFd;
						fdslot[ii].flag = 1;
						break;
					}
				}	
				if (ii==MAX_FD) {
					cout << "Error: exceeds max fd!" << endl;
					return -1;
				}
				if (--retnfd == 0)
					goto start;
			}

			for (int i = 0; i < MAX_FD; i++) { //define variable max_index_used to improve performance
				if (fdslot[i].flag && FD_ISSET(fdslot[i].fd, &rset)) {
					int nr;
					if ( (nr=read(fdslot[i].fd, buffer, MAX_BUFFER)) > 0 ) {//need to be modified for read from network
						cout << "Read from connection: " << buffer << endl;
					}
					if (nr == 0) { //fin
						close(fdslot[i].fd);
						fdslot[i].flag = 0;
						FD_CLR(fdslot[i].fd, &rset);
					}
					if (--retnfd == 0)
						break;	 
				}
			}
		}
	} //for(;;)

	return 0;			
}

