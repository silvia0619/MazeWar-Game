#include "server.h"
#include "client_registry.h"
#include "protocol.h"
#include "player.h"
#include "csapp.h"
#include "debug.h"
#include "maze.h"

#include <string.h>

int debug_show_maze;
CLIENT_REGISTRY *client_registry;

void *mzw_client_service(void *arg) {
	int fd = *((int *)arg);
    free(arg);
    Pthread_detach(pthread_self());
    creg_register(client_registry, fd);

    MZW_PACKET* pkt = Malloc(sizeof(MZW_PACKET));
    void **datap = Malloc(sizeof(void*));
    int loged_in = -1;
    PLAYER* new_player = Malloc(sizeof(PLAYER*));
    while(1) {
        if(loged_in != -1) {
            player_check_for_laser_hit(new_player);
        }
    	if(proto_recv_packet(fd, pkt, datap) != 0) {
            if(loged_in == 0) {
                debug("Logging out player %d!!\n", fd);
                player_logout(new_player);
            }
            else {
                free(new_player);
            }
            break;
        }
        // LOGIN
    	if(pkt->type == MZW_LOGIN_PKT && loged_in == -1) {
    		debug("MZW_LOGIN_PKT received username: %s\n", (char*)*datap);
            new_player = player_login(fd, pkt->param1, *datap);
            // successful LOGIN
            if(new_player != NULL) {
                debug("successful LOGIN");
                MZW_PACKET* ready_packet = Malloc(sizeof(MZW_PACKET));
                ready_packet->type = MZW_READY_PKT;
                if(proto_send_packet(fd, ready_packet, datap) != -1) {
                    player_reset(new_player);
                }
                free(ready_packet);
                loged_in = 0;
            }
            // unsuccessful LOGIN
            else {
                debug("unsuccessful LOGIN");
                MZW_PACKET* inuse_packet = Malloc(sizeof(MZW_PACKET));
                inuse_packet->type = MZW_INUSE_PKT;
                if(proto_send_packet(fd, inuse_packet, datap) != -1) {
                    debug("failed to LOGIN\n");
                }
                free(inuse_packet);
            }
    	}
        // MOVE
        else if(pkt->type == MZW_MOVE_PKT && loged_in == 0) {
            debug("MZW_MOVE_PKT received\n");
            if(player_move(new_player, pkt->param1) != 0) {
                debug("failed to MOVE\n");
            }
        }
        // TURN
        else if(pkt->type == MZW_TURN_PKT && loged_in == 0) {
            debug("MZW_TURN_PKT received\n");
            player_rotate(new_player, pkt->param1);
        }
        // FIRE
        else if(pkt->type == MZW_FIRE_PKT && loged_in == 0) {
            debug("[%c] MZW_FIRE_PKT received\n", pkt->param1);
            player_fire_laser(new_player);
        }
        // REFRESH
        else if(pkt->type == MZW_REFRESH_PKT && loged_in == 0) {
            debug("MZW_REFRESH_PKT received\n");
            player_invalidate_view(new_player);
            player_update_view(new_player);
        }
        // SEND
        else if(pkt->type == MZW_SEND_PKT && loged_in == 0) {
            debug("MZW_SEND_PKT received\n");
            player_send_chat(new_player, *datap, pkt->size);
        }
        show_maze();
    }
    debug("DONE while loop\n");

    creg_unregister(client_registry, fd);
    close(fd);
    free(pkt);
    free(datap);

	return arg;
}