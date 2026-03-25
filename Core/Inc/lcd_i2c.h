#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "stm32f4xx_hal.h"

// Endereço I2C padrão do LCD (pode ser 0x27 ou 0x3F)
#define LCD_I2C_ADDR 0x27 << 1

// Comandos
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// Flags
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04  // Enable bit
#define Rw 0x02  // Read/Write bit
#define Rs 0x01  // Register select bit

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
    uint8_t backlight;
    uint8_t displayfunction;
    uint8_t displaycontrol;
    uint8_t displaymode;
    uint8_t cols;
    uint8_t rows;
} LCD_I2C_t;

// Funções públicas
void LCD_Init(LCD_I2C_t *lcd, I2C_HandleTypeDef *hi2c, uint8_t cols, uint8_t rows);
void LCD_Clear(LCD_I2C_t *lcd);
void LCD_SetCursor(LCD_I2C_t *lcd, uint8_t col, uint8_t row);
void LCD_Print(LCD_I2C_t *lcd, char *str);
void LCD_Printf(LCD_I2C_t *lcd, const char *format, ...);
void LCD_Backlight(LCD_I2C_t *lcd, uint8_t state);

// ===  ADICIONAR ESTAS LINHAS ===
void LCD_Write4Bits(LCD_I2C_t *lcd, uint8_t value);
void LCD_SendCommand(LCD_I2C_t *lcd, uint8_t cmd);
void LCD_SendData(LCD_I2C_t *lcd, uint8_t data);
void LCD_ExpanderWrite(LCD_I2C_t *lcd, uint8_t data);
void LCD_PulseEnable(LCD_I2C_t *lcd, uint8_t data);
void LCD_CreateChar(LCD_I2C_t *lcd, uint8_t location, const uint8_t charmap[8]);
void LCD_WriteChar(LCD_I2C_t *lcd, uint8_t ch);

#endif
