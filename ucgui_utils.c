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
#include "ucgui_utils.h"
#include "ucgui_utils/gui_api.h"
#include "lcd_display_service.h"

int fd_lcd;

extern const GUI_BITMAP bmeffect;
extern const GUI_BITMAP bmmic;
extern const GUI_BITMAP bmmusic;
extern const GUI_BITMAP bmsystem;
extern const GUI_BITMAP bmbmp_separator;
extern const GUI_BITMAP bmbmp_rectangle_0;
extern const GUI_BITMAP bmbmp_rectangle_1;

int separator_handle[6];
int editor_handle[6];
tGUI_SEPARATOR separator = {0, 31, 160, 4, &bmbmp_separator, 0xA0A0A0};
tGUI_SEPARATOR *separator_group[] = {&separator, NULL};
tGUI_BACKGROUND background_effect = {&bmeffect, 0x0000ff};
tGUI_BACKGROUND background_mic = {&bmmic, 0x0000ff};
tGUI_BACKGROUND background_music = {&bmmusic, 0x0000ff};
tGUI_BACKGROUND background_system = {&bmsystem, 0x0000ff};
tGUI_BACKGROUND *background = &background_effect;

tGUI_EDITOR editor_1bar[1] = {
	{40, 54, EDITOR0_W, EDITOR0_H, NULL, 0, 0x0, 0xFFFFFF, 80, 20, "", ""},
};

tGUI_EDITOR editor_4bar[4] = {
	{0, 0,  152, 32, NULL,               0, 0x0, 0xFFFFFF, 80, 10, "", ""},
	{0, 35, 152, 32, NULL,               0, 0x0, 0xFFFFFF, 80, 39, "", ""},
	{3, 63, 152, 32, &bmbmp_rectangle_1, 0, 0x0, 0xFFFFFF, 80, 67, "", ""},
	{3, 95, 152, 32, &bmbmp_rectangle_1, 0, 0x0, 0xFFFFFF, 80, 99, "", ""},
};

tGUI_EDITOR editor_6bar[6] = {
	{START_X,                                START_Y,                                        EDITOR0_W, EDITOR0_H, &bmbmp_rectangle_0, 0, 0x0, 0xFFFFFF, 38,  START_Y + 11,  "", ""},
	{START_X + EDITOR0_W + SEPARATOR_X_SIZE, START_Y,                                        EDITOR0_W, EDITOR0_H, &bmbmp_rectangle_0, 0, 0x0, 0xFFFFFF, 118,  START_Y + 11, "", ""},
	{START_X,                                START_Y + EDITOR0_H + SEPARATOR_Y_SIZE,         EDITOR0_W, EDITOR0_H, &bmbmp_rectangle_0, 0, 0x0, 0xFFFFFF, 38,  START_Y + EDITOR0_H + SEPARATOR_Y_SIZE + 11, "", ""},
	{START_X + EDITOR0_W + SEPARATOR_X_SIZE, START_Y + EDITOR0_H + SEPARATOR_Y_SIZE,         EDITOR0_W, EDITOR0_H, &bmbmp_rectangle_0, 0, 0x0, 0xFFFFFF, 118, START_Y + EDITOR0_H + SEPARATOR_Y_SIZE + 11, "", ""},
	{START_X,                                START_Y + EDITOR0_H * 2 + SEPARATOR_Y_SIZE * 2, EDITOR0_W, EDITOR0_H, &bmbmp_rectangle_0, 0, 0x0, 0xFFFFFF, 38, START_Y + EDITOR0_H * 2 + SEPARATOR_Y_SIZE * 2 + 11, "", ""},
	{START_X + EDITOR0_W + SEPARATOR_X_SIZE, START_Y + EDITOR0_H * 2 + SEPARATOR_Y_SIZE * 2, EDITOR0_W, EDITOR0_H, &bmbmp_rectangle_0, 0, 0x0, 0xFFFFFF, 118, START_Y + EDITOR0_H * 2 + SEPARATOR_Y_SIZE * 2 + 11, "", ""},
};

void window_init(tGUI_EDITOR *p_editor, struct window_des_s *p_win)
{
	int float_flag, index_flag;
	int ret;
	int data;

	index_flag = p_win->attr.data_type;
	float_flag = p_win->attr.float_flag;

	if (p_win->attr.key_type == ATTR_TYPE_STR) {
		sprintf(p_editor->string, "%s", p_win->attr.key_des);
	} else if (p_win->attr.key_type == ATTR_TYPE_INT) {
		WARN("Key not support ATTR_TYPE_INI type\n");
	} else if (p_win->attr.key_type == ATTR_TYPE_NON) {
		/* No key, nothing to do */;
	} else {
		WARN("Cann't recognize Key type\n");
	}

	if (index_flag) {
		int index;
		if (p_win->attr.data_func != NULL) {
			ret = p_win->attr.data_func(DATA_READ, &index);
			if (p_win->attr.val_type == ATTR_TYPE_STR) {
				index %= p_win->attr.lim.lim_int.max;
				index = index > (p_win->attr.lim.lim_int.max) ? (p_win->attr.lim.lim_int.max) : index;
				index = index < p_win->attr.lim.lim_int.min ? (p_win->attr.lim.lim_int.min) : index;
				printf("index = %d\n", index);
				sprintf(p_editor->value, "%s", p_win->attr.val_des[index]);
			} else {
				WARN("index flag not support other type beside ATTR_TYPE_STR\n");
			}
		}
	} else {
		if (float_flag) {
			//sprintf(p_editor[i].value, p_win[i].attr.fmt_display, (p_win[i].handle_data(DATA_READ, NULL)));
			printf("TBD");
		} else {
			if (p_win->attr.data_func != NULL) {
				ret = p_win->attr.data_func(DATA_READ, &data);
				sprintf(p_editor->value, "%d", data);
			}
		}
	}

	//strcpy(str[i], p_editor[i].string);
	strcat(p_editor->string, p_editor->value);
	strcpy(p_editor->value, p_editor->string);
	memset(p_editor->string, 0, strlen(p_editor->string));
}

void update_all_value(int handle, tGUI_EDITOR *editor[], int num)
{
	int i;

	for (i = 0; i < num; i++)
		gui_update_editor_value(editor_handle[i], editor[i]->value);
}

void layout_center_horizontal(tGUI_EDITOR *editor[], int handle[], int flag, int num, struct picture_s *p_pic)
{
	if (flag == UPDATE_WINDOW) {
		gui_set_background(background);
		update_all_value(*handle, editor, num);
	} else if (flag == SHOW_FRAME) {
		if (p_pic->type == FOUR_BAR)
			gui_update_page(background, editor, separator_group, handle, separator_handle);
		else
			gui_update_page(background, editor, NULL, handle, separator_handle);
	}
}

char str[6][32];

void clear_single_window(int i, tGUI_EDITOR *p_editor)
{
	memset(p_editor[i].value, 0, strlen(p_editor[i].value));
	strcpy(p_editor[i].string, str[i]);
}


void init_editor(int pic_type, tGUI_EDITOR **p_editor, tGUI_EDITOR *editor[])
{
	switch (pic_type) {
	case ONE_BAR:
		*p_editor = editor_1bar;
		editor[0] = &editor_1bar[0];
		editor[1] = NULL;
		break;

	case FOUR_BAR:
		*p_editor = editor_4bar;
		editor[0] = &editor_4bar[0];
		editor[1] = &editor_4bar[1];
		editor[2] = &editor_4bar[2];
		editor[3] = &editor_4bar[3];
		editor[4] = NULL;
		break;

	case SIX_BAR:
		*p_editor = editor_6bar;
		editor[0] = &editor_6bar[0];
		editor[1] = &editor_6bar[1];
		editor[2] = &editor_6bar[2];
		editor[3] = &editor_6bar[3];
		editor[4] = &editor_6bar[4];
		editor[5] = &editor_6bar[5];
		editor[6] = NULL;
		break;

	default:
		break;
	}
}

void display_picture(struct picture_s *p_pic)
{
	int i;
	int clear_all = 0;
	tGUI_EDITOR *p_editor;
	static int last_type;

	if (last_type != p_pic->type)
		clear_all = 1;
	last_type = p_pic->type;

	tGUI_EDITOR *editor[7];

	init_editor(p_pic->type, &p_editor, editor);

	for (i = 0; i < p_pic->windows_num; i++) {
		//clear_single_window(i, p_editor);
		window_init(&p_editor[i], &(p_pic->p_win[i]));
	}

	if (clear_all == 1)
		layout_center_horizontal(editor, editor_handle, SHOW_FRAME, p_pic->windows_num, p_pic);
	else
		layout_center_horizontal(editor, editor_handle, UPDATE_WINDOW, p_pic->windows_num, p_pic);

	for (i = 0; i < p_pic->windows_num; i++)
		clear_single_window(i, p_editor);

}

int ucgui_init()
{
	gui_start();
	gui_set_stringMode(1);
	gui_set_background(&background_effect);

	fd_lcd = open("/dev/6138_st7735_lcd", O_RDWR);
	if (fd_lcd == -1) {
		ERR("open tft err\n");
		return -1;
	}

	return 0;
}

int ucgui_deinit()
{
	close(fd_lcd);
	return 0;
}

#if 0
void dump_attr(struct attribute *p_attr)
{
	int i;

	printf("%d, %d, \"%s\", \"%s\", \"%s\"", attr.index_flag, attr.float_flag, attr.intro, attr.fmt_intro, attr.fmt_display);
	printf("{ ");

	for (i = 0; i < 5; i++) {
		if (strcmp(attr.index_str[i], "")) {
			printf("\"%s\" ", attr.index_str[i]);
		}
	}

	printf("}");
}
#endif

void dump_window(struct window_des_s *p_win)
{
	int i;

	printf(".event_id = { ");
	for (i = 0; i < 32; i++) {
		if (p_win->event_id[i] > 0) {
			printf("%d, ", p_win->event_id[i]);
		}
	}
	printf("},");

	printf(" .attr = { ");
	//dump_attr(p_win->p_attr);
	printf("}");

	printf("\n");
}

void dump_pic(struct picture_s *p_pic)
{
	int i;

	if (p_pic->type == ONE_BAR) {
		printf(".type = %s\n", "1BAR");
	} else if (p_pic->type == FOUR_BAR) {
		printf(".type = %s\n", "4BAR");
	} else if (p_pic->type == SIX_BAR) {
		printf(".type = %s\n", "6BAR");
	}

	return;
	for (i = 0; i < p_pic->windows_num; i++) {
		dump_window(p_pic->p_win + i);
	}
}

