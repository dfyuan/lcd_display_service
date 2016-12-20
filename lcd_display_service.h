#ifndef LCD_DISPLAY_SERVICE_H_
#define LCD_DISPLAY_SERVICE_H_

#define TFT_IOCTL_MAGIC  'M'
#define TFT_IOCTL_DISPLAY_COLOUR       1
#define TFT_IOCTL_DISPLAY_8_16         9
#define TFT_IOCTL_BACK_LIGHT_WHITE     4
#define TFT_IOCTL_BACK_LIGHT_BLACK     5
#define TFT_IOCTL_BACK_LIGHT_BLUE      6
#define TFT_IOCTL_BACK_LIGHT_RED       7
#define TFT_IOCTL_BACK_LIGHT           7
#define TFT_IOCTL_DISPLAY_ENABLE       8
#define TFT_IOCTL_WORD_LIGHT           13
#define TFT_IOCTL_DISPLAY_CONNECT      10

#define B_BITS 5
#define G_BITS 6
#define R_BITS 5

#define R_MASK ((1 << R_BITS) -1)
#define G_MASK ((1 << G_BITS) -1)
#define B_MASK ((1 << B_BITS) -1)

#define  gray(color)  (((color >> (8 - R_BITS)) & R_MASK) + \
						(((color >> (16 - G_BITS)) & G_MASK) << R_BITS) + \
						(((color >> (24 - B_BITS)) & B_MASK) << (G_BITS + R_BITS)))

//背景色：以下是各种颜色的数值，如果想定义别的数值颜色，上网搜下16位rgb颜色对应表，可以对应着定义
//#define TFT_LCD_BACK_LIGHT_WHITE      0xFFFF //白色
#define TFT_LCD_BACK_LIGHT_WHITE      gray(0xa0a0a0)
#define TFT_LCD_BACK_LIGHT_BLACK      0x0000 //黑色
#define TFT_LCD_BACK_LIGHT_BLUE       0xF800 //蓝色
#define TFT_LCD_BACK_LIGHT_RED        0x001F //红色
#define TFT_LCD_BACK_LIGHT_GREEN      0x07E0 //绿色

//字体颜色：以下是各种颜色的数值，如果想定义别的数值颜色，上网搜下16位rgb颜色对应表，可以对应着定义
#define TFT_LCD_WORD_LIGHT_WHITE      TFT_LCD_BACK_LIGHT_WHITE //白色
#define TFT_LCD_WORD_LIGHT_BLACK      TFT_LCD_BACK_LIGHT_BLACK //黑色
#define TFT_LCD_WORD_LIGHT_BLUE       TFT_LCD_BACK_LIGHT_BLUE  //蓝色
#define TFT_LCD_WORD_LIGHT_RED        TFT_LCD_BACK_LIGHT_RED   //红色
#define TFT_LCD_WORD_LIGHT_GREEN      TFT_LCD_BACK_LIGHT_GREEN //绿色

#define SEPARATOR_X_SIZE 4
#define SEPARATOR_Y_SIZE 3
#define START_X  3
#define START_Y 3
#define EDITOR0_W 74
#define EDITOR0_H 38
#define EDITOR1_W 152
#define EDITOR1_H 28

#define DATA_READ	 1
#define DATA_WRITE	 2

#define UPDATE_WINDOW    0
#define SHOW_FRAME       1

#define ONE_BAR     1
#define FOUR_BAR    2
#define SIX_BAR     3

struct limit_int {
	int min;
	int max;
	int interval;
};

struct limit_float {
	float min;
	float max;
	float interval;
};

#define ATTR_DATA_TPYE_INDEX	1
#define ATTR_DATA_TYPE_DATA		2

#define ATTR_TYPE_NON		0
#define ATTR_TYPE_STR		1
#define ATTR_TYPE_INT		2

struct attr_des_s {
	int data_type; //标记*p_data是索引还是数值
	int float_flag; //注意，还有DWORD类型
	int key_type;
	char key_des[32];
	int val_type;
	int (*data_func)(int flag, void *p_data);
	char val_des[5][32];
	union {
		struct limit_int lim_int;
		struct limit_float lim_flt;
	} lim;
};

struct window_des_s {
	struct attr_des_s attr;
	int event_id[32];
	void (*event_handler)(int event_id, struct window_des_s *p_win);
};

struct picture_s {
	int type;
	int panel_lock;
	int active;
	int windows_num;
	struct window_des_s *p_win;
	struct picture_s *pre_pic;
	struct picture_s *next_pic;
	struct picture_s *child_pic;
	struct picture_s *parent_pic;
	int (*event_handler)(int event_id, struct picture_s *p_pic);
};

#endif
