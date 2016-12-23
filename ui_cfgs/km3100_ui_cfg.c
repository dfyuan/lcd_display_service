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

#include "../logd.h"
#include "sys_utils.h"
#include "input_devices_man.h"
#include "../lcd_display_service.h"

/**************************** common func APIs ********************************/

int data_indecrease_hanlder(int dir, struct window_des_s *p_win)
{
	int val = 0;
	int ret = 0;

	if (dir == KEY_KNOB_POS) {
		p_win->attr.data_func(DATA_READ, &val);
		val++;
		p_win->attr.data_func(DATA_WRITE, &val);
	}

	if (dir == KEY_KNOB_NEG) {
		p_win->attr.data_func(DATA_READ, &val);
		val--;
		p_win->attr.data_func(DATA_WRITE, &val);
	}

	return ret;
}

int data_func(int flag, void *p_data)
{
	static int val = 10000;

	if (flag == DATA_READ) {
		*(int *)p_data = val;
	} else if (flag == DATA_WRITE) {
		val = *(int *)p_data;
	}

	return 0;
}

extern struct picture_s km3100_pic_main;
extern struct picture_s pic_sys[];
extern struct picture_s pic_effect[];

/**************************** main pics ********************************/

struct window_des_s main_window[] = {
	{ { 1, 0, ATTR_TYPE_NON, "",     ATTR_TYPE_STR,  data_func, {"SWEET", "HEROIC", "FLAME"}, .lim.lim_int = {0, 2, 1} }, {-1, -1, NULL}, {{-1, NULL},} },

	{
		{ 0, 0, ATTR_TYPE_STR, "EFF:", ATTR_TYPE_INT,  data_func, {}, .lim.lim_int = {0, 100, 1} },
		.knob_event_handler = { KNOB_EFFECT_POS, KNOB_EFFECT_NEG, data_indecrease_hanlder },
		.misc_event_handlers = { { -1, NULL }, },
	},

	{ { 1, 0, ATTR_TYPE_NON, "",     ATTR_TYPE_STR,  data_func, {"FB.EX0", "FB.EX1", "FB.EX2", "FB.EX3"}, .lim.lim_int = {0, 3, 1} }, {-1, -1, NULL}, {{-1, NULL},} },

	{
		{ 0, 0, ATTR_TYPE_STR, "MIC:", ATTR_TYPE_INT,  data_func, {}, .lim.lim_int = {0, 100, 1} },
		.knob_event_handler = { KNOB_MIC_POS, KNOB_MIC_NEG, data_indecrease_hanlder },
		.misc_event_handlers = { { -1, NULL }, },
	},

	{ { 1, 0, ATTR_TYPE_NON, "",     ATTR_TYPE_STR,  data_func, {"M.VOD", "M.BGM", "S/PDIF"}, .lim.lim_int = {0, 2, 1} }, {-1, -1, NULL}, {{-1, NULL},} },

	{
		{ 0, 0, ATTR_TYPE_STR, "MUS:", ATTR_TYPE_INT,  data_func, {}, .lim.lim_int = {0, 100, 1} },
		.knob_event_handler = { KNOB_MUSIC_POS, KNOB_MUSIC_NEG, data_indecrease_hanlder },
		.misc_event_handlers = { { -1, NULL }, },
	},
};

struct picture_s km3100_pic_main = {
	.type = SIX_BAR,
	.windows_num = 6,
	.p_win = main_window,
	.pre_pic = NULL,
	.next_pic = NULL,
	.child_pic = &pic_effect[0],
	.parent_pic = NULL,
	.event_id = {
		.pre_event = -1,
		.next_event = -1,
		.child_event = KEY_EFFECT_MENU,
		.parent_event = KEY_RETURN,
	},
};

/**************************** effect pics ********************************/

struct window_des_s effect_window[3][4] = {
	{ /*1*/
		{ { 0, 0, ATTR_TYPE_STR, "EFFECT", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "menu[1/14]", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "Echo filterBP", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{
			{ 1, 0, ATTR_TYPE_STR, "type", ATTR_TYPE_STR, data_func, { "butt", "reily", "bypass" }, .lim.lim_int = {0, 3, 1} },
			.knob_event_handler = { KNOB_MUSIC_POS, KNOB_MUSIC_NEG, data_indecrease_hanlder },
			.misc_event_handlers = { { -1, NULL }, },
		},
	},

	{ /*2*/
		{ { 0, 0, ATTR_TYPE_STR, "EFFECT", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "menu[2/14]", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "Echo filterBP", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{
			{ 1, 0, ATTR_TYPE_STR, "l_freq", ATTR_TYPE_STR, data_func, { "butt1", "reily1", "bypass1" }, .lim.lim_int = {0, 3, 1} },
			.knob_event_handler = { KNOB_MUSIC_POS, KNOB_MUSIC_NEG, data_indecrease_hanlder },
			.misc_event_handlers = { { -1, NULL }, },
		},
	},

	{ /*3*/
		{ { 0, 0, ATTR_TYPE_STR, "EFFECT", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "menu[3/14]", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "Echo filterBP", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{
			{ 1, 0, ATTR_TYPE_STR, "h_freq", ATTR_TYPE_STR, data_func, { "butt2", "reily2", "bypass2" }, .lim.lim_int = {0, 3, 1} },
			.knob_event_handler = { KNOB_MUSIC_POS, KNOB_MUSIC_NEG, data_indecrease_hanlder },
			.misc_event_handlers = { { -1, NULL }, },
		},
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
		.parent_pic = &km3100_pic_main,
		.event_id = {
			.pre_event = KNOB_EFFECT_NEG,
			.next_event = KNOB_EFFECT_POS,
			.child_event = KEY_EFFECT_MENU,
			.parent_event = KEY_RETURN,
		},
	},

	{
		.type = FOUR_BAR,
		.windows_num = 4,
		.p_win = effect_window[1],
		.pre_pic = &pic_effect[0],
		.next_pic = &pic_effect[2],
		.child_pic = NULL,
		.parent_pic = &km3100_pic_main,
		.event_id = {
			.pre_event = KNOB_EFFECT_NEG,
			.next_event = KNOB_EFFECT_POS,
			.child_event = KEY_EFFECT_MENU,
			.parent_event = KEY_RETURN,
		},
	},

	{
		.type = FOUR_BAR,
		.windows_num = 4,
		.p_win = effect_window[2],
		.pre_pic = &pic_effect[1],
		.next_pic = NULL,
		.child_pic = &pic_sys[0],
		.parent_pic = &km3100_pic_main,
		.event_id = {
			.pre_event = KNOB_EFFECT_NEG,
			.next_event = KNOB_EFFECT_POS,
			.child_event = KEY_EFFECT_MENU,
			.parent_event = KEY_RETURN,
		},
	},
};

/**************************** sys pics ********************************/

struct window_des_s sys_window[3][4] = {
	{ /*1*/
		{ { 0, 0, ATTR_TYPE_STR, "SYSTEM", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "menu[1/14]", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "Option", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{
			{ 1, 0, ATTR_TYPE_STR, "mode", ATTR_TYPE_STR, data_func, { "easy", "full", "bypass" }, .lim.lim_int = {0, 3, 1} },
			.knob_event_handler = { KNOB_MUSIC_POS, KNOB_MUSIC_NEG, data_indecrease_hanlder },
			.misc_event_handlers = { { -1, NULL }, },
		},
	},

	{ /*2*/
		{ { 0, 0, ATTR_TYPE_STR, "SYSTEM", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "menu[2/14]", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "TFT", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{
			{ 1, 0, ATTR_TYPE_STR, "contrast", ATTR_TYPE_STR, data_func, { "butt1", "reily1", "bypass1" }, .lim.lim_int = {0, 3, 1} },
			.knob_event_handler = { KNOB_MUSIC_POS, KNOB_MUSIC_NEG, data_indecrease_hanlder },
			.misc_event_handlers = { { -1, NULL }, },
		},
	},

	{ /*3*/
		{ { 0, 0, ATTR_TYPE_STR, "SYSTEM", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "menu[3/14]", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{ { 0, 0, ATTR_TYPE_STR, "Panel", ATTR_TYPE_NON, NULL, {} }, {-1, -1, NULL}, {{-1, NULL},}  },
		{
			{ 1, 0, ATTR_TYPE_STR, "Lock", ATTR_TYPE_STR, data_func, { "on", "off", "on/off" }, .lim.lim_int = {0, 3, 1} },
			.knob_event_handler = { KNOB_MUSIC_POS, KNOB_MUSIC_NEG, data_indecrease_hanlder },
			.misc_event_handlers = { { -1, NULL }, },
		},
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
		.event_id = {
			.pre_event = KNOB_EFFECT_NEG,
			.next_event = KNOB_EFFECT_POS,
			.child_event = KEY_EFFECT_MENU,
			.parent_event = KEY_RETURN,
		},
	},

	{
		.type = FOUR_BAR,
		.windows_num = 4,
		.p_win = sys_window[1],
		.pre_pic = &pic_sys[0],
		.next_pic = &pic_sys[2],
		.child_pic = NULL,
		.parent_pic = &pic_effect[0],
		.event_id = {
			.pre_event = KNOB_EFFECT_NEG,
			.next_event = KNOB_EFFECT_POS,
			.child_event = KEY_EFFECT_MENU,
			.parent_event = KEY_RETURN,
		},
	},

	{
		.type = FOUR_BAR,
		.windows_num = 4,
		.p_win = sys_window[2],
		.pre_pic = &pic_sys[1],
		.next_pic = NULL,
		.child_pic = NULL,
		.parent_pic = &pic_effect[0],
		.event_id = {
			.pre_event = KNOB_EFFECT_NEG,
			.next_event = KNOB_EFFECT_POS,
			.child_event = KEY_EFFECT_MENU,
			.parent_event = KEY_RETURN,
		},
	},
};
