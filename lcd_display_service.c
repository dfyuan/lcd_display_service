#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>
#include <linux/input.h>

#include "logd.h"
#include "sys_utils.h"
#include "broadcast_utils.h"
#include "global_state.h"
#include "signal_utils.h"
#include "ucgui_utils.h"
#include "lcd_display_service.h"
#include "lcd_display_core.h"
#include "input_devices_man.h"
#include "ui_cfgs/km3100_ui_cfg.h"

int _log_v = LOG_LEVEL;

int service_quit = 0;

extern int service_quit;
static int bc_socket;

int main()
{
	int i = 0;
	int ret;
	fd_set rfds;
	int max_fd;
	struct timeval tv;
	char recv_buf[4096];
	struct string_array event_packets;
	struct br_event_t br_event;
	pthread_t lcd_display_loop_thread;

	ucgui_init();

	os_pthread_create(&lcd_display_loop_thread, lcd_display_loop, 128*1024);

	active_pic = &km3100_pic_main;
	trigger_display();

	bc_socket = connect_broadcast_server();
	if (bc_socket < 0)
		ERR("connect broadcast server err\n");

	register_thread_name(bc_socket, "lcd_display");
	subscribe_event(bc_socket, BROADCAST_KEY_EVENT_ID);
	subscribe_event(bc_socket, BROADCAST_LCD_EVENT_ID);
	subscribe_event(bc_socket, BROADCAST_UPGRADE_EVENT_ID);

	while (!service_quit) {
		FD_ZERO(&rfds);
		FD_SET(bc_socket, &rfds);
		max_fd = bc_socket;

		tv.tv_sec = 0;
		tv.tv_usec = 300 * 1000;

		ret = select(max_fd + 1, &rfds, NULL, NULL, &tv);

		if (ret <= 0) {
			continue;
		}

		/* receive from unix socket */
		if (FD_ISSET(bc_socket, &rfds)) {
			memset(recv_buf, 0, sizeof(recv_buf));
			ret = recv(bc_socket, recv_buf, sizeof(recv_buf) - 1, 0);
			if (ret <= 0) {
				ERR("cann't connect broadcast manage service...\n");
				continue;
			}

			os_strsep(recv_buf, &event_packets, "\n\r");
			for (i = 0; i < event_packets.string_len; i++) {
				parse_broadcast_event(event_packets.string[i], &br_event);
				//MSG("event_id(%d):event_state(%d)\n", br_event.event_id, br_event.event_state);
				lcd_display_event_handler(&br_event);
			}
		}
	}

	MSG("lcd display service quit .... \n");
	ucgui_deinit();
	logd_deinit();

	return 0;
}
