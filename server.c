#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#define ADDRESS "127.0.0.1"
#define PORT 8080

#define FILE_PATH "../bootstrap.min.css"
#define RESPONSE_HEADER "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/css\r\nContent-Length: %lld\r\n\r\n"

int main(int argc, const char *argv[]) {
	const struct sockaddr_in address = {
		.sin_family = AF_INET,
		.sin_port = htons(PORT),
		.sin_addr.s_addr = inet_addr(ADDRESS)
	};

	/* Setup TCP connection */
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (bind(server_socket, (const struct sockaddr *)&address, sizeof(address)) == -1) {
		fprintf(stderr, "[FATAL] Cannot bind to the address!");
		return -1;
	}

	listen(server_socket, 65535);
	printf("[INFO] Listening on http://%s:%d...\n", ADDRESS, PORT);

	int client_socket = accept(server_socket, NULL, NULL);

	/* Get file size */
	struct stat info;
	stat(FILE_PATH, &info);

	/* Format HTTP header */
	int size_needed = snprintf(NULL, 0, RESPONSE_HEADER, info.st_size);
	char *header = (char *)malloc(size_needed);
	sprintf(header, RESPONSE_HEADER, info.st_size);

	/* Create iovec */
	struct iovec header_vector = {
		.iov_base = header,
		.iov_len = size_needed
	};

	/* Add header string to sendfile header & trailer struct */
	struct sf_hdtr header_and_trailer = {
		.headers = &header_vector,
		.hdr_cnt = 1,
		.trailers = NULL,
		.trl_cnt = 0
	};

	off_t bytes_sent;
	int file_fd = open(FILE_PATH, O_RDWR);
	int ret = sendfile(file_fd, client_socket, 0, &bytes_sent, &header_and_trailer, 0);
	printf("%lld/%lld bytes sent\nsendfile() returns %d\nerrno: %d\nerror message: %s\n",
		bytes_sent, info.st_size + size_needed, ret, errno, strerror(errno));

	/**
	 * It seems sendfile() returns before the file is fully sent to the socket,
	 * sleep two seconds to allow all the file contents to be fully sent
	 */
	if (argc > 1)
		sleep(2);

	free(header);
	close(file_fd);
	close(client_socket);
	close(server_socket);
	return 0;
}
