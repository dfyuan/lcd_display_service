#ifndef _GUI_API_H
#define _GUI_API_H

#include "GUI.h"


#define MAX_EDITOR_NUM 6
#define MAX_SEPARATOR_NUM 6

typedef struct {
	int x;
	int y;
	int width;
	int height;
	const GUI_BITMAP* pic;
	int	bg_flag; // if pic==null and bg_flag==1, draw background by bg_color
	unsigned int bg_color;
	unsigned int fg_color;
	int string_x;
	int string_y;
	char string[32];
	char value[32];
} tGUI_EDITOR;

typedef struct {
	int x;
	int y;
	int width;
	int height;
	const GUI_BITMAP* pic;  //if pic not null, following color no sense;
	unsigned int color;
} tGUI_SEPARATOR;

typedef struct {
	const GUI_BITMAP* pic;  //if pic not null, following color no sense;
	unsigned int color;
} tGUI_BACKGROUND;
/*
  *******************************************************************
  *
  *              API
  *
  *******************************************************************
*/

//mode:0-左对齐， 1-居中
void gui_set_stringMode(int mode);
void gui_start(void);
int gui_create_editor(tGUI_EDITOR* editor);
int gui_destroy_editor(int handle);
int gui_destroy_editor_all(void);
int gui_update_editor(int handle, tGUI_EDITOR* editor);
int gui_update_editor_value(int handle , char*string);
int gui_create_separator(tGUI_SEPARATOR* separator);
int gui_destroy_separator(int handle);
int gui_destroy_separator_all(void);
int gui_update_separator(int handle, tGUI_SEPARATOR* separator);
int gui_set_background(tGUI_BACKGROUND* background);
int gui_update_page(tGUI_BACKGROUND* background, tGUI_EDITOR* editor[],
		tGUI_SEPARATOR* separator[], int editor_handle[], int separator_handle[]);

void gui_light_on(void);
void gui_light_off(void);
#endif
