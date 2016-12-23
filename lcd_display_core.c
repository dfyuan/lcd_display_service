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
#include "broadcast_utils.h"
#include "global_state.h"
#include "ucgui_utils.h"
#include "lcd_display_service.h"
#include "lcd_display_core.h"

extern int service_quit;

static pthread_mutex_t active_pic_update_mutex = PTHREAD_MUTEX_INITIALIZER;

struct picture_s *active_pic = NULL;
static int active_pic_update = 0;

static int pic_shift_event_handler(int event_id, struct event_id_s event_id_table, struct picture_s *p_pic)
{
	int ret = -1;

	if (event_id_table.pre_event != -1 && event_id == event_id_table.pre_event) {
		if (p_pic->pre_pic != NULL) {
			printf("pre\n");
			active_pic = p_pic->pre_pic;
			//active_pic->parent_pic = p_pic->parent_pic;
			ret = 0;
		}
	} else if (event_id_table.next_event != -1 && event_id == event_id_table.next_event) {
		if (p_pic->next_pic != NULL) {
			printf("next\n");
			active_pic = p_pic->next_pic;
			//active_pic->parent_pic = p_pic->parent_pic;
			ret = 0;
		}
	} else if (event_id_table.child_event != -1 && event_id == event_id_table.child_event) {
		if (p_pic->child_pic!= NULL) {
			printf("child\n");
			active_pic = p_pic->child_pic;
			active_pic->parent_pic = p_pic;
			ret = 0;
		}
	} else if (event_id_table.parent_event != -1 && event_id == event_id_table.parent_event) {
		if (p_pic->parent_pic != NULL) {
			printf("return\n");
			active_pic = p_pic->parent_pic;
			active_pic->child_pic = p_pic;
			ret = 0;
		}
	}

	return ret;
}

static int lcd_event_handler(int event_id)
{
	int i;
	int j;
	int event_trigger = 0;

	/* handle picture event */
	if (event_id == active_pic->event_id.pre_event ||
			event_id == active_pic->event_id.next_event ||
				event_id == active_pic->event_id.child_event ||
					event_id == active_pic->event_id.parent_event) {
		if (pic_shift_event_handler(event_id, active_pic->event_id, active_pic) == 0) {
			trigger_display();
			/* if picture event is hanlded success, return directly */
			return 0;
		}
	}

	/* handle window event */
	for (i = 0; i < active_pic->windows_num; i++) {
		/* 1. handle knob event */
		if (active_pic->p_win[i].knob_event_handler.knob_pos_event > 0
				&& active_pic->p_win[i].knob_event_handler.knob_pos_event == event_id) {
			active_pic->p_win[i].knob_event_handler.event_handler(KEY_KNOB_POS, &active_pic->p_win[i]);
			event_trigger = 1;
		} else if (active_pic->p_win[i].knob_event_handler.knob_neg_event > 0
				&& active_pic->p_win[i].knob_event_handler.knob_neg_event == event_id) {
			active_pic->p_win[i].knob_event_handler.event_handler(KEY_KNOB_NEG, &active_pic->p_win[i]);
			event_trigger = 1;
		}

		/* 2, handle other misc type event */
		for (j = 0; j < 32; j++) {
			if (active_pic->p_win[i].misc_event_handlers[i].event_id > 0
					&& active_pic->p_win[i].misc_event_handlers[i].event_id == event_id) {
				active_pic->p_win[i].misc_event_handlers[i].event_handler(event_id, &active_pic->p_win[i]);
				event_trigger = 1;
			}
		}
	}

	if (event_trigger == 1) {
		trigger_display();
	}

	return 0;
}

void lcd_display_event_handler(struct br_event_t *p_br_event)
{
	if (p_br_event->event_id != BROADCAST_KEY_EVENT_ID) {
		PRINT("not key event: %d\n", p_br_event->event_id);
		return;
	}

	lcd_event_handler(p_br_event->event_state);
}

void trigger_display()
{
	pthread_mutex_lock(&active_pic_update_mutex);
	active_pic_update = 1;
	pthread_mutex_unlock(&active_pic_update_mutex);
}

void *lcd_display_loop(void *arg)
{
	while (!service_quit) {
		pthread_mutex_lock(&active_pic_update_mutex);
		if (active_pic_update == 1) {
			display_picture(active_pic);
			active_pic_update = 0;
		}
		pthread_mutex_unlock(&active_pic_update_mutex);

		usleep(10*1000);
	}

	return NULL;
}

