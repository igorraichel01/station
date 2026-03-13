#include "lcd_i2c.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// ===  REMOVER 'static' DESTAS DECLARAÇÕES ===
void LCD_Write4Bits(LCD_I2C_t *lcd, uint8_t value);
void LCD_SendCommand(LCD_I2C_t *lcd, uint8_t cmd);
void LCD_SendData(LCD_I2C_t *lcd, uint8_t data);
void LCD_ExpanderWrite(LCD_I2C_t *lcd, uint8_t data);
void LCD_PulseEnable(LCD_I2C_t *lcd, uint8_t data);

void LCD_Init(LCD_I2C_t *lcd, I2C_HandleTypeDef *hi2c, uint8_t cols, uint8_t rows) {
    lcd->hi2c = hi2c;
    lcd->address = LCD_I2C_ADDR;
    lcd->cols = cols;
    lcd->rows = rows;
    lcd->backlight = LCD_BACKLIGHT;

    lcd->displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;

    HAL_Delay(50);

    LCD_ExpanderWrite(lcd, lcd->backlight);
    HAL_Delay(1000);

    // Sequência de inicialização 4-bit
    LCD_Write4Bits(lcd, 0x03 << 4);
    HAL_Delay(5);
    LCD_Write4Bits(lcd, 0x03 << 4);
    HAL_Delay(5);
    LCD_Write4Bits(lcd, 0x03 << 4);
    HAL_Delay(1);
    LCD_Write4Bits(lcd, 0x02 << 4);

    // Configurar display
    LCD_SendCommand(lcd, LCD_FUNCTIONSET | lcd->displayfunction);

    lcd->displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    LCD_SendCommand(lcd, LCD_DISPLAYCONTROL | lcd->displaycontrol);

    LCD_Clear(lcd);

    lcd->displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    LCD_SendCommand(lcd, LCD_ENTRYMODESET | lcd->displaymode);

    HAL_Delay(200);
}

void LCD_Clear(LCD_I2C_t *lcd) {
    LCD_SendCommand(lcd, LCD_CLEARDISPLAY);
    HAL_Delay(2);
}

void LCD_SetCursor(LCD_I2C_t *lcd, uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row >= lcd->rows) {
        row = lcd->rows - 1;
    }
    LCD_SendCommand(lcd, LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LCD_Print(LCD_I2C_t *lcd, char *str) {
    while (*str) {
        LCD_SendData(lcd, *str++);
    }
}

void LCD_Printf(LCD_I2C_t *lcd, const char *format, ...) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    LCD_Print(lcd, buffer);
}

void LCD_Backlight(LCD_I2C_t *lcd, uint8_t state) {
    if (state) {
        lcd->backlight = LCD_BACKLIGHT;
    } else {
        lcd->backlight = LCD_NOBACKLIGHT;
    }
    LCD_ExpanderWrite(lcd, 0);
}

// ===  REMOVER 'static' DE TODAS ESTAS FUNÇÕES ===
void LCD_Write4Bits(LCD_I2C_t *lcd, uint8_t value) {
    LCD_ExpanderWrite(lcd, value);
    LCD_PulseEnable(lcd, value);
}

void LCD_SendCommand(LCD_I2C_t *lcd, uint8_t cmd) {
    uint8_t highnib = cmd & 0xF0;
    uint8_t lownib = (cmd << 4) & 0xF0;
    LCD_Write4Bits(lcd, highnib);
    LCD_Write4Bits(lcd, lownib);
}

void LCD_SendData(LCD_I2C_t *lcd, uint8_t data) {
    uint8_t highnib = data & 0xF0;
    uint8_t lownib = (data << 4) & 0xF0;
    LCD_Write4Bits(lcd, highnib | Rs);
    LCD_Write4Bits(lcd, lownib | Rs);
}

void LCD_ExpanderWrite(LCD_I2C_t *lcd, uint8_t data) {
    uint8_t buffer = data | lcd->backlight;
    HAL_I2C_Master_Transmit(lcd->hi2c, lcd->address, &buffer, 1, 100);
}

void LCD_PulseEnable(LCD_I2C_t *lcd, uint8_t data) {
    LCD_ExpanderWrite(lcd, data | En);
    HAL_Delay(1);
    LCD_ExpanderWrite(lcd, data & ~En);
    HAL_Delay(1);
}
