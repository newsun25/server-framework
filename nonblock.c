#include <sys/select.h>
#include <stdio.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_MAX	1024
#define max(a,b)	(((a)>(b))?(a):(b))

int main() {
	int socket_fd;
	if ( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0 ) {
		puts("create socket error!");
	}

	struct sockaddr_in	addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(7);
	if (inet_pton(AF_INET, "10.0.0.2", &addr.sin_addr) <= 0) {
		puts("inet_pton error!");
	}
	
	//puts("debug: before connect");
	if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("error:");
		puts("connect error!");
		return -1;
	}

	//puts("debug: after connect");

	int flag;
	if ( (flag=fcntl(socket_fd, F_GETFL, 0)) < 0) {
		puts("get_socket flag error!");
	}
	if (fcntl(socket_fd, F_SETFL, flag|O_NONBLOCK) < 0) {
		puts("set socket flag error!");
	}
	if ( (flag=fcntl(STDIN_FILENO, F_GETFL, 0)) < 0) {
		puts("get_socket flag error!");
	}
	if (fcntl(STDIN_FILENO, F_SETFL, flag|O_NONBLOCK) < 0) {
		puts("set socket flag error!");
	}
	if ( (flag=fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0) {
		puts("get_socket flag error!");
	}
	if (fcntl(STDOUT_FILENO, F_SETFL, flag|O_NONBLOCK) < 0) {
		puts("set socket flag error!");
	}

	char	buffer_in[BUFFER_MAX];
	char	buffer_out[BUFFER_MAX];
	char	*r_in, *w_in, *r_out, *w_out;
	int	n, maxfd;
	
	r_in = w_in = buffer_in;
	r_out = w_out = buffer_out;

	fd_set	rset, wset;
	//puts("debug: before for");
	for (;;) {
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		if (r_out < &buffer_out[BUFFER_MAX])
			FD_SET(socket_fd, &rset);
		if (r_in < &buffer_in[BUFFER_MAX])
			FD_SET(STDIN_FILENO, &rset);
		
		if (w_in != r_in)
			FD_SET(socket_fd, &wset);
		if (w_out != r_out)
			FD_SET(STDOUT_FILENO, &wset);

		maxfd = max(max(socket_fd, STDIN_FILENO), STDOUT_FILENO);
		//printf("maxfd: %d\n", maxfd);
		if (select(maxfd+1, &rset, &wset, NULL, NULL) > 0) {
			//puts("select occurs");
			if (FD_ISSET(STDIN_FILENO, &rset)) {
				//puts("event read from stdin");
				n = read(STDIN_FILENO, w_in, &buffer_in[BUFFER_MAX]-w_in);
				if (n < 0) {
					puts("read from stdin error!");
				}
				else if (n == 0) {
					puts("read zero from stdin");
					/* need to be handled */
				}
				else {
					w_in += n;

				}
					
			}

			if (FD_ISSET(socket_fd, &rset)) {
				//puts("event read from socket fd");
				n = read(socket_fd, w_out, &buffer_out[BUFFER_MAX]-w_out);
				if (n < 0) {
					puts("read from socket_fd error!");
				}
				else if (n==0) {
					puts("read zero from socket");
					/* need to be handled */
				}
				else {
					w_out += n;
				
				}
			}

			if (FD_ISSET(STDOUT_FILENO, &wset)) {
				//puts("event write to stdout");
				n = write(STDOUT_FILENO, r_out, w_out - r_out);
				if (n <= 0) {
					puts("write to stdout error");
				}
				else {
					r_out += n;
					if (r_out == w_out) {
						r_out = w_out = buffer_out;
					}
				}
			}

			if (FD_ISSET(socket_fd, &wset)) {
				//puts("event write to socket fd");
				n = write(socket_fd, r_in, w_in - r_in);
				if (n <= 0) {
					puts("write to socket_fd error!");
				}
				else {
					r_in += n;
					if (r_in == w_in) {
						r_in = w_in = buffer_in;
					}
				}
			}
		}//select
		else {
			puts("select error!");
		}
	}//for	

}


