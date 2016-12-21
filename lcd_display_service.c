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
#include "gpio_utils.h"
#include "sys_utils.h"
#include "broadcast_utils.h"
#include "global_state.h"
#include "signal_utils.h"
#include "device_info.h"
#include "input_devices_man.h"
#include "board_info_utils.h"
#include "gpio_utils.h"
#include "ucgui_utils.h"
#include "ucgui_utils/gui_api.h"
#include "lcd_display_service.h"

int _log_v = LOG_LEVEL;

int service_quit = 0;

extern int service_quit;
static int bc_socket;

struct picture_s *active_pic = NULL;
int active_pic_update = 0;

static pthread_mutex_t active_pic_update_mutex = PTHREAD_MUTEX_INITIALIZER;

/**************************** common func APIs ********************************/
int pic_event_handler(int event_id, struct picture_s *p_pic)
{
	int ret = -1;

	switch (event_id) {
		case KNOB_EFFECT_POS:
			if (p_pic->next_pic != NULL) {
				printf("next\n");
				active_pic = p_pic->next_pic;
				ret = 0;
			}
			break;

		case KNOB_EFFECT_NEG:
			if (p_pic->pre_pic != NULL) {
				active_pic = p_pic->pre_pic;
				printf("pre\n");
				ret = 0;
			}
			break;

		case KEY_EFFECT_MENU:
			if (p_pic->child_pic!= NULL) {
				printf("enter\n");
				active_pic = p_pic->child_pic;
				ret = 0;
			}
			break;

		case KEY_RETURN:
			if (p_pic->parent_pic != NULL) {
				active_pic = p_pic->parent_pic;
				printf("return\n");
				ret = 0;
			}
			break;

		default:
			break;
	}

	return ret;
}

int data_indecrease_hanlder(int event_id, int match_event_id[32], struct window_des_s *p_win)
{
	int val = 0;
	int ret = 0;

	if (match_event_id[0] != -1 && event_id == match_event_id[0]) {
		p_win->attr.data_func(DATA_READ, &val);
		val++;
		p_win->attr.data_func(DATA_WRITE, &val);
	} else if (match_event_id[1] != -1 && event_id == match_event_id[1]) {
		p_win->attr.data_func(DATA_READ, &val);
		val--;
		p_win->attr.data_func(DATA_WRITE, &val);
	} else {
		printf("no match event id\n");
		ret = -1;
	}

	return ret;
}

int data_func(int flag, void *p_data)
{
	static int val = 1;

	if (flag == DATA_READ) {
		*(int *)p_data = val;
	} else if (flag == DATA_WRITE) {
		val = *(int *)p_data;
	}

	return 0;
}

extern struct picture_s pic_main;
extern struct picture_s pic_sys[];
extern struct picture_s pic_effect[];

/**************************** main pics ********************************/

struct window_des_s main_window[] = {
	{ { 1, 0, ATTR_TYPE_NON, "",     ATTR_TYPE_STR,  data_func, {"SWEET", "HEROIC", "FLAME"}, .lim.lim_int = {0, 2, 1} }, {-1},  NULL },
	{ { 0, 0, ATTR_TYPE_STR, "EFF:", ATTR_TYPE_INT,  data_func, {}, .lim.lim_int = {0, 100, 1} }, 	{KNOB_EFFECT_POS, KNOB_EFFECT_NEG}, data_indecrease_hanlder},
	{ { 1, 0, ATTR_TYPE_NON, "",     ATTR_TYPE_STR,  data_func, {"FB.EX0", "FB.EX1", "FB.EX2", "FB.EX3"}, .lim.lim_int = {0, 3, 1} }, 	{-1}, NULL},
	{ { 0, 0, ATTR_TYPE_STR, "MIC:", ATTR_TYPE_INT,  data_func, {}, .lim.lim_int = {0, 100, 1} }, {KNOB_MIC_POS, KNOB_MIC_NEG}, data_indecrease_hanlder},
	{ { 1, 0, ATTR_TYPE_NON, "",     ATTR_TYPE_STR,  data_func, {"M.VOD", "M.BGM", "S/PDIF"}, .lim.lim_int = {0, 2, 1} }, {-1}, NULL},
	{ { 0, 0, ATTR_TYPE_STR, "MUS:", ATTR_TYPE_INT,  data_func, {}, .lim.lim_int = {0, 100, 1} }, {KNOB_MUSIC_POS, KNOB_MUSIC_NEG}, data_indecrease_hanlder},
};

struct picture_s pic_main = {
	 .type = SIX_BAR,
	 .windows_num = 6,
	 .p_win = main_window,
	 .pre_pic = NULL,
	 .next_pic = NULL,
	 .child_pic = &pic_effect[1],
	 .parent_pic = NULL,
	 .event_handler = pic_event_handler,
};

/**************************** effect pics ********************************/

struct window_des_s effect_window[3][4] = {
	{ /*1*/
		{ { 0, 0, ATTR_TYPE_STR, "EFFECT", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "menu[1/14]", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "Echo filterBP", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 1, 0, ATTR_TYPE_STR, "type", ATTR_TYPE_STR, data_func, { "butt", "reily", "bypass" }, .lim.lim_int = {0, 3, 1} }, {KNOB_MIC_POS, KNOB_MIC_NEG}, data_indecrease_hanlder },
	},

	{ /*2*/
		{ { 0, 0, ATTR_TYPE_STR, "EFFECT", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "menu[2/14]", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "Echo filterBP", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 1, 0, ATTR_TYPE_STR, "l_freq", ATTR_TYPE_STR, data_func, { "butt1", "reily1", "bypass1" }, .lim.lim_int = {0, 3, 1} }, {KNOB_MIC_POS, KNOB_MIC_NEG}, data_indecrease_hanlder },
	},

	{ /*3*/
		{ { 0, 0, ATTR_TYPE_STR, "EFFECT", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "menu[3/14]", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "Echo filterBP", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 1, 0, ATTR_TYPE_STR, "h_freq", ATTR_TYPE_STR, data_func, { "butt2", "reily2", "bypass2" }, .lim.lim_int = {0, 3, 1} }, {KNOB_MIC_POS, KNOB_MIC_NEG}, data_indecrease_hanlder },
	},

};

struct picture_s pic_effect[] = {
	{
	 .type = FOUR_BAR,
	 .windows_num = 4,
	 .p_win = effect_window[0],
	 .pre_pic = NULL,
	 .next_pic = &pic_effect[1],
	 .child_pic = &pic_sys[0],
	 .parent_pic = &pic_main,
	 .event_handler = pic_event_handler,
	},

	{
	 .type = FOUR_BAR,
	 .windows_num = 4,
	 .p_win = effect_window[1],
	 .pre_pic = &pic_effect[0],
	 .next_pic = &pic_effect[2],
	 .child_pic = &pic_sys[0],
	 .parent_pic = &pic_main,
	 .event_handler = pic_event_handler,
	},

	{
	 .type = FOUR_BAR,
	 .windows_num = 4,
	 .p_win = effect_window[2],
	 .pre_pic = &pic_effect[1],
	 .next_pic = NULL,
	 .child_pic = &pic_sys[0],
	 .parent_pic = &pic_main,
	 .event_handler = pic_event_handler,
	},
};

/**************************** sys pics ********************************/

struct window_des_s sys_window[3][4] = {
	{ /*1*/
		{ { 0, 0, ATTR_TYPE_STR, "SYSTEM", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "menu[1/14]", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "Option", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 1, 0, ATTR_TYPE_STR, "mode", ATTR_TYPE_STR, data_func, { "easy", "full", "bypass" }, .lim.lim_int = {0, 3, 1} }, {KNOB_MIC_POS, KNOB_MIC_NEG}, data_indecrease_hanlder },
	},

	{ /*2*/
		{ { 0, 0, ATTR_TYPE_STR, "SYSTEM", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "SYSTEM", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "TFT", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 1, 0, ATTR_TYPE_STR, "contrast", ATTR_TYPE_STR, data_func, { "butt1", "reily1", "bypass1" }, .lim.lim_int = {0, 3, 1} }, {KNOB_MIC_POS, KNOB_MIC_NEG}, data_indecrease_hanlder },
	},

	{ /*3*/
		{ { 0, 0, ATTR_TYPE_STR, "SYSTEM", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "menu[3/14]", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 0, 0, ATTR_TYPE_STR, "Panel", ATTR_TYPE_NON, NULL, {} }, {-1}, NULL },
		{ { 1, 0, ATTR_TYPE_STR, "Lock", ATTR_TYPE_STR, data_func, { "on", "off", "on/off" }, .lim.lim_int = {0, 3, 1} }, {KNOB_MIC_POS, KNOB_MIC_NEG}, data_indecrease_hanlder },
	},

};

struct picture_s pic_sys[] = {
	{
	 .type = FOUR_BAR,
	 .windows_num = 4,
	 .p_win = sys_window[0],
	 .pre_pic = NULL,
	 .next_pic = &pic_sys[1],
	 .child_pic = NULL,
	 .parent_pic = &pic_effect[0],
	 .event_handler = pic_event_handler,
	},

	{
	 .type = FOUR_BAR,
	 .windows_num = 4,
	 .p_win = sys_window[1],
	 .pre_pic = &pic_sys[0],
	 .next_pic = &pic_sys[2],
	 .child_pic = NULL,
	 .parent_pic = &pic_effect[0],
	 .event_handler = pic_event_handler,
	},

	{
	 .type = FOUR_BAR,
	 .windows_num = 4,
	 .p_win = sys_window[2],
	 .pre_pic = &pic_sys[1],
	 .next_pic = NULL,
	 .child_pic = NULL,
	 .parent_pic = &pic_effect[0],
	 .event_handler = pic_event_handler,
	},
};

void trigger_display()
{
	pthread_mutex_lock(&active_pic_update_mutex);
	active_pic_update = 1;
	pthread_mutex_unlock(&active_pic_update_mutex);
}

int lcd_event_handle(int event_id)
{
	int i;

	if (active_pic->event_handler != NULL) {
		printf("pic event handler\n");
		if (active_pic->event_handler(event_id, active_pic) == 0) {
			trigger_display();
			return 0;
		}
	}

	for (i = 0; i < active_pic->windows_num; i++) {
		printf("window event handler\n");
		if (active_pic->p_win[i].event_handler != NULL) {
			active_pic->p_win[i].event_handler(event_id, active_pic->p_win[i].event_id, &active_pic->p_win[i]);
		}
	}

	trigger_display();

	return 0;
}

void *lcd_display_loop(void *arg)
{
	while (1) {
		pthread_mutex_lock(&active_pic_update_mutex);
		if (active_pic_update == 1) {
			display_picture(active_pic);
			active_pic_update = 0;
		}
		pthread_mutex_unlock(&active_pic_update_mutex);

		usleep(10*1000);
	}
}

void lcd_display_event_handle(struct br_event_t *p_br_event)
{
	if (p_br_event->event_id != BROADCAST_KEY_EVENT_ID) {
		PRINT("not key event: %d\n", p_br_event->event_id);
		return;
	}

	lcd_event_handle(p_br_event->event_state);
}

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

	active_pic = &pic_main;
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
				lcd_display_event_handle(&br_event);
			}
		}
	}

	MSG("lcd display service quit .... \n");
	ucgui_deinit();
	logd_deinit();

	return 0;
}
