#include "client_registry.h"
#include "csapp.h"
#include "debug.h"

struct client_registry {
	int num_of_clients;
	struct fd *registered_fds;
	sem_t mutex;
	sem_t mutex_e;
};

struct fd {
	int curr_fd;
	struct fd* next;
};


CLIENT_REGISTRY *creg_init() {
	debug("creg_init\n");
	CLIENT_REGISTRY *client;

	client = Malloc(sizeof(CLIENT_REGISTRY));
	client->num_of_clients = 0;
	// client->registered_fds = (struct fd*)Malloc(sizeof(struct fd));
	client->registered_fds = NULL;

	Sem_init(&client->mutex, 0, 1);
	Sem_init(&client->mutex_e, 0, 1);
	return client;
}

void creg_fini(CLIENT_REGISTRY *cr) {
	debug("creg_fini\n");
	// struct fd* curr_fd = cr->registered_fds;
	// while(curr_fd != NULL) {
	// 	struct fd* prev = curr_fd;
	// 	curr_fd = curr_fd->next;
	// 	free(prev);
	// }
	free(cr);
}

void creg_register(CLIENT_REGISTRY *cr, int fd) {
	P(&cr->mutex);
	struct fd* the_fd = (struct fd*)Malloc(sizeof(struct fd));
	the_fd->curr_fd = fd;
	if(cr->num_of_clients == 0) {
		the_fd->next = NULL;
		cr->registered_fds = the_fd;
	}
	else {
		the_fd->next = cr->registered_fds;
		cr->registered_fds = the_fd;
	}
	cr->num_of_clients++;
	debug("creg_register %d, total %d\n", fd, cr->num_of_clients);
	V(&cr->mutex);
}

void creg_unregister(CLIENT_REGISTRY *cr, int fd) {
	debug("creg_unregister\n");
	P(&cr->mutex);
	int last_index = cr->num_of_clients;
	if(last_index == 1) {
		debug("only one");
		free(cr->registered_fds);
		cr->num_of_clients--;
		debug("creg_unregister %d, total %d\n", fd, cr->num_of_clients);
		V(&cr->mutex);
		return;
	}
	int i = 0;
	struct fd* temp_fd = cr->registered_fds;
	struct fd* prev_fd = cr->registered_fds;
	while(temp_fd != NULL) {
		debug("go find the temp_fd");
		if(temp_fd->curr_fd == fd) {
			if(temp_fd == cr->registered_fds) {
				cr->registered_fds = temp_fd->next;
			}

			else if(i == last_index - 1) {
				prev_fd->next = NULL;
			}

			else {
				prev_fd->next = temp_fd->next;
			}
			free(temp_fd);
			cr->num_of_clients--;
			V(&cr->mutex);
			return;
		}
		prev_fd = temp_fd;
		temp_fd = temp_fd->next;
	}
	debug("unregister error");
	V(&cr->mutex);
}

void creg_wait_for_empty(CLIENT_REGISTRY *cr) {
	if(cr->num_of_clients != 0){
		debug("WAIT creg_wait_for_empty\n");
		P(&cr->mutex_e);
	}
	else {
		debug("DONE creg_wait_for_empty\n");
		V(&cr->mutex_e);
	}
}

void creg_shutdown_all(CLIENT_REGISTRY *cr) {
	debug("creg_shutdown_all\n");
	struct fd* temp_fd = cr->registered_fds;
	for(int i = 0; i < cr->num_of_clients; i++) {
        close(temp_fd->curr_fd);
        temp_fd = temp_fd->next;
    }
    cr->registered_fds = NULL;
    cr->num_of_clients = 0;
}
