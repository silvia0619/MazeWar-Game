#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>


#include "client_registry.h"
#include "maze.h"
#include "player.h"
#include "debug.h"
#include "server.h"
#include "csapp.h"

static void terminate(int status);
static void handler(int sig);
void *thread(void *vargp);
int listenfd;

static char *default_maze[] = {
  "******************************",
  "***** %%%%%%%%% &&&&&&&&&&& **",
  "***** %%%%%%%%%        $$$$  *",
  "*           $$$$$$ $$$$$$$$$ *",
  "*##########                  *",
  "*########## @@@@@@@@@@@@@@@@@*",
  "*           @@@@@@@@@@@@@@@@@*",
  "******************************",
  NULL
};

CLIENT_REGISTRY *client_registry;
volatile sig_atomic_t term = 0;

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    // int* port_number = 0;
    char* port;
    char* template_file = NULL;
    int opt;
    while((opt = getopt(argc, argv, "p:t:")) != -1){
        switch(opt) {
            case 'p':
                port = optarg;
                if(port == NULL) {
                    fprintf(stderr, "Port number is required!");
                    exit(1);
                }
                break;
            case 't':
                template_file = optarg;
                break;
            case '?':
                fprintf(stderr, "Unexpected argument!");
                exit(1);
                break;
        }
    }
    if(argc > optind) {
        fprintf(stderr, "Unexpected argument!");
        exit(1);
    }

    // Perform required initializations of the client_registry,
    // maze, and player modules.
    client_registry = creg_init();
    if(template_file == NULL) {
        int i = 0;
        while(default_maze[i] != NULL) {
            // debug("%s\n", default_maze[i]);
            i++;
        }
        maze_init(default_maze);
    }
    else {
        ///////////////////////////////////////////////////////////////////////////////////////////////template_file
        FILE *in = fopen(template_file, "r");
        debug("open temp file");
        char *line = NULL;
        ssize_t read;
        size_t len = 0;
        int col = 0;
        int row = 0;
        while ((read = getline(&line, &len, in)) != -1) {
            col = read;
            row++;
        }
        free(line);
        debug("%d, %d", row, col);

        char* template[row];
        for(int i = 0; i < row; i++)
            template[i] = (char*)malloc((col + 1) * sizeof(char));
        rewind(in);
        int i = 0;
        while ((read = getline(&template[i], &len, in)) != -1) {
            template[i][read - 1] = '\0';
            // debug("%s", template[i]);
            i++;
        }
        template[i] = NULL;
        fclose(in);
        maze_init(template);
        for (int i = 0; i < row; i++) {
            free(template[i]);
        }
    }
    // maze_init(default_maze);
    player_init();
    debug_show_maze = 1;  // Show the maze after each packet.

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function mzw_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    struct sigaction act = { 0 };
    act.sa_handler = handler;
    if (sigaction(SIGHUP, &act, 0) == -1) {
        perror(0);
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);

    // set up the server socket
    // int server_socket;
    // server_socket = socket(PF_INET, SOCK_STREAM,0);
    // if(server_socket == -1) {
    //     printf("server_socket error\n");
    // }
    // struct sockaddr_in server_addr;
    // memset(&server_addr, 0, sizeof(server_addr));
    // server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // server_addr.sin_port = htons(port_number);

    // if(bind(server_socket, (void*)&server_addr, sizeof(server_addr)) == -1){
    //     fprintf(stderr, "server_socket bind error");
    // }
    // if(listen(server_socket, 5) == -1) {
    //     fprintf(stderr, "server_socket listen error");
    // }
    // a loop to accept connections on this socket
    // int *client_socket;
    // socklen_t clientlen;
    // struct sockaddr_storage client_addr;
    // pthread_t tid;

    // while(1) {
    //     clientlen = sizeof(struct sockaddr_storage);
    //     connfdp = malloc(sizeof(int));
    //     *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &clientlen);
    //     if(*client_socket == -1){
    //         fprintf(stderr, "client_socket error");
    //     }
    //     pthread_create(&tid, NULL, mzw_client_service, client_socket);
    // }

    // int listenfd;
    int *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    term = 0;
    listenfd = Open_listenfd(port);
    while (term == 0) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Pthread_create(&tid, NULL, mzw_client_service, connfdp);
    }

    fprintf(stderr, "You have to finish implementing main() "
	    "before the MazeWar server will function.\n");

    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    player_fini();
    maze_fini();

    debug("MazeWar server terminating");
    exit(status);
}

static void handler(int sig) {
    debug("HANDLER!!!!\n");
    terminate(EXIT_SUCCESS);
    // term = 1;
}

void *thread(void *vargp) {
    mzw_client_service(vargp);
    return NULL;
}

