#ifndef LCD_H
#define LCD_H

#define LCD_WIDTH 800
#define LCD_HEIGHT 480

typedef enum {
	LCD_OK,
	LCD_ERROR,
} LCD_Status;

LCD_Status lcd_init();

#endif // LCD_H
