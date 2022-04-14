#include "protocol.h"
#include <unistd.h>
#include <stdlib.h>
#include <debug.h>
#include <errno.h>

int proto_send_packet(int fd, MZW_PACKET *pkt, void *data) {
	uint16_t temp_size = pkt->size;
	pkt->size = htons(pkt->size);
	pkt->timestamp_sec = htonl(pkt->timestamp_sec);
	pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);
	ssize_t write_size = write(fd, pkt, sizeof(MZW_PACKET));
	if(write_size != sizeof(MZW_PACKET)) {
		return -1;
	}
	if(data != NULL) {
		ssize_t payload_write_size = write(fd, data, temp_size);
		if(payload_write_size != pkt->size) {
			return -1;
		}
		return 0;
	}
	else {
		return 0;
	}
}

int proto_recv_packet(int fd, MZW_PACKET *pkt, void **datap) {
	ssize_t read_size = read(fd, pkt, sizeof(MZW_PACKET));
	pkt->size = ntohs(pkt->size);
	pkt->timestamp_sec = ntohl(pkt->timestamp_sec);
	pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);
	if(read_size != sizeof(MZW_PACKET)) {
		debug("EOF on fd: %d", fd);
		if(errno == 4) {
			errno = 0;
			return 0;
		}
		return -1;
	}
	if(pkt->size != 0) {
		char* data = malloc(pkt->size);
		ssize_t payload_read_size = read(fd, data, pkt->size);
		data[payload_read_size] = '\0';
		if(payload_read_size != pkt->size) {
			return -1;
		}
		else {
			*datap = (void *)data;
			return 0;
		}
	}
	else {
		return 0;
	}
}