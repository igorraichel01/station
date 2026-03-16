#ifndef MENU_H
#define MENU_H

#include "stm32f4xx_hal.h"
#include "lcd_i2c.h"
#include "ds3231.h"
#include <stdbool.h>
#define MENU_TOTAL_ITEMS 6
typedef enum {
    MENU_STATE_IDLE = 0,
    MENU_STATE_MAIN,
    MENU_STATE_CONFIG_DATETIME,
    MENU_STATE_CONFIG_ACQ,
    MENU_STATE_TEST_MODE,
    MENU_STATE_CONFIG_LANGUAGE,     // ✅ novo
	MENU_STATE_CLEAR_DATA_CONFIRM,
	MENU_STATE_CONFIG_ALTITUDE


} MenuState_t;

typedef enum {
    BTN_NONE,
    BTN_SET,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT
} Button_t;

typedef enum {
    FIELD_YEAR,
    FIELD_MONTH,
    FIELD_DAY,
    FIELD_HOUR,
    FIELD_MINUTE,
    FIELD_SECOND,
    FIELD_EXIT,
    FIELD_DONE
} EditField_t;

// ✅ Idiomas
typedef enum {
    LANG_PT = 0,
    LANG_ES,
    LANG_EN,
    LANG_TOTAL
} Language_t;

typedef struct {
    MenuState_t state;
    uint8_t menu_selection;
    EditField_t edit_field;
    uint32_t last_button_press;
    bool button_pressed;
    DS3231_Time_t temp_time;
    uint8_t acq_selection;
    uint8_t lang_selection;          // ✅ novo
    uint8_t clear_selection;
    uint8_t confirm_selection;
    uint8_t menu_scroll;
    int16_t altitude_selection;
} Menu_t;

void Menu_Init(Menu_t *menu);
void Menu_Process(Menu_t *menu, LCD_I2C_t *lcd, I2C_HandleTypeDef *hi2c, DS3231_Time_t *current_time);
bool Menu_IsIdle(Menu_t *menu);
Button_t Menu_ReadButtons(void);

#endif
