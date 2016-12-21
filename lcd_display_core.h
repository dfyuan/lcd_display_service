#ifndef LCD_DISPLAY_CORE_H_
#define LCD_DISPLAY_CORE_H_

extern struct picture_s *active_pic;
extern void trigger_display();
extern void *lcd_display_loop(void *arg);
extern void lcd_display_event_handler(struct br_event_t *p_br_event);

#endif
