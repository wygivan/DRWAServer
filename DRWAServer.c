#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>

#define BULK_PORT 8001
#define WEB_PORT 8002
#define STREAMING_PORT 8003

#define LISTENQ 1024
#define BUF_SIZE 4096
#define DURATION 15

/*
 * writen() and readn() are both borrowed from UNP by Richard Stevens
 */
ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}

		nleft -= nwritten;
		ptr += nwritten;
	}

	return n;
}

ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		} else if (nread == 0)
			break;

		nleft -= nread;
		ptr += nread;
	}

	return (n - nleft);
}

void *handle_request(void *arg)
{
	int conn_fd;
	char buf[BUF_SIZE];
	ssize_t nread;
	struct timeval tv;
	double t1, t2;

	conn_fd = *((int *)arg);
	free(arg);

	pthread_detach(pthread_self());
	memset(buf, '\0', BUF_SIZE);

	gettimeofday(&tv, NULL);
	t1 = tv.tv_sec + tv.tv_usec / 1000000.0;
	t2 = t1;
	while ((t2 - t1) < DURATION) {
		if (writen(conn_fd, buf, BUF_SIZE) < 0) {
			perror("writen() error");
			break;
		}

		gettimeofday(&tv, NULL);
		t2 = tv.tv_sec + tv.tv_usec / 1000000.0;
	}

	if (shutdown(conn_fd, SHUT_WR) < 0) {
		perror("shutdown() error");
	} 

	if ((nread = readn(conn_fd, buf, BUF_SIZE)) < 0) {
		perror("readn() error");
	} else {
		buf[nread - 1] = '\0';
		printf("Throughput: %sbps\n", buf);
	}

	close(conn_fd);
	return NULL;
}

int main(void)
{
	int listen_fd, *conn_fd;
	int optval = 1;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len = sizeof(client_addr);
	pthread_t tid;
	time_t now;
	struct tm now2;

	signal(SIGPIPE, SIG_IGN);
	setlinebuf(stdout);

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error");
		exit(-1);
	}

	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
		perror("setsockopt() error");
		exit(-1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(BULK_PORT);
	if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("bind() error");
		exit(-1);
	}

	if (listen(listen_fd, LISTENQ) < 0) {
		perror("listen() error");
		exit(-1);
	}

	for ( ; ; ) {
		conn_fd = (int *) malloc(sizeof(int));
		if ((*conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
			perror("accept() error");
			exit(-1);
		}

		now = time(NULL);
		now2 = *localtime(&now);
		printf("%02d/%02d/%d %02d:%02d:%02d Received connection from %s\n", now2.tm_mon + 1, now2.tm_mday, now2.tm_year + 1900, now2.tm_hour, now2.tm_min, now2.tm_sec, inet_ntoa(client_addr.sin_addr));

		if (pthread_create(&tid, NULL, handle_request, (void *)conn_fd) != 0) {
			perror("pthread_create() error");
			exit(-1);
		}
	}

	close(listen_fd);
	return 0;
}
