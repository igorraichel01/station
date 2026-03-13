#include "menu.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "i18n.h"
#include "w25qxx.h"

// Definições dos pinos dos botões
#define BTN_SET_PIN    GPIO_PIN_0
#define BTN_UP_PIN     GPIO_PIN_1
#define BTN_DOWN_PIN   GPIO_PIN_3
#define BTN_LEFT_PIN   GPIO_PIN_4
#define BTN_RIGHT_PIN  GPIO_PIN_5
#define BTN_PORT       GPIOA

#define DEBOUNCE_TIME  40  // ms

// ✅ DECLARAÇÕES EXTERNAS (ATUALIZADAS)
extern AcquisitionTime_t current_acq_time;
extern uint32_t acquisition_interval_ms;
extern uint8_t current_language;

extern const AcqTimeConfig_t acq_time_options[];
extern void ApplyAcquisitionTime(AcquisitionTime_t time);
extern void SaveAcquisitionTimeToFlash(AcquisitionTime_t time);

extern W25Q_t flash;
extern uint32_t total_samples_saved;


extern void W25Q_TestReset(W25Q_t *flash);
extern void W25Q_TestState_Save(W25Q_t *flash);

// NOVO: precisa existir no driver
extern void W25Q_LogReset(W25Q_t *flash);
extern void W25Q_LogState_Save(W25Q_t *flash);

// ✅ Idioma (você vai definir no main.c)

extern void SaveLanguageToFlash(uint8_t lang);

// Protótipos
static void ShowMainMenu(Menu_t *menu, LCD_I2C_t *lcd);
static void ShowDateTimeEditor(Menu_t *menu, LCD_I2C_t *lcd);
static void ShowAcqEditor(Menu_t *menu, LCD_I2C_t *lcd);
static void ShowLanguageEditor(Menu_t *menu, LCD_I2C_t *lcd);

static void ProcessMainMenu(Menu_t *menu, Button_t button, LCD_I2C_t *lcd, I2C_HandleTypeDef *hi2c, DS3231_Time_t *current_time);
static void ProcessDateTimeEditor(Menu_t *menu, Button_t button, LCD_I2C_t *lcd, I2C_HandleTypeDef *hi2c);
static void ProcessAcqEditor(Menu_t *menu, Button_t button, LCD_I2C_t *lcd);
static void ProcessLanguageEditor(Menu_t *menu, Button_t button, LCD_I2C_t *lcd);

static void ShowClearConfirm(Menu_t *menu, LCD_I2C_t *lcd);
static void ProcessClearConfirm(Menu_t *menu, Button_t button, LCD_I2C_t *lcd);

static void IncrementField(Menu_t *menu);
static void DecrementField(Menu_t *menu);

static const char* LangToStr(uint8_t lang) {
    switch (lang) {
        case LANG_PT: return "PORTUGUES";
        case LANG_ES: return "ESPANHOL";
        case LANG_EN: return "ENGLISH";
        default:      return "PORTUGUES";
    }
}

void Menu_Init(Menu_t *menu) {
    menu->state = MENU_STATE_IDLE;
    menu->menu_selection = 0;
    menu->edit_field = FIELD_YEAR;
    menu->last_button_press = 0;
    menu->button_pressed = false;
    menu->acq_selection = (uint8_t)current_acq_time;
    menu->menu_scroll = 0;
    menu->confirm_selection = 0;

    // ✅ idioma atual
    menu->lang_selection = current_language;
    if (menu->lang_selection >= LANG_TOTAL) menu->lang_selection = LANG_PT;
}

Button_t Menu_ReadButtons(void) {
    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_SET_PIN) == GPIO_PIN_RESET) {
        return BTN_SET;
    }
    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_UP_PIN) == GPIO_PIN_RESET) {
        return BTN_UP;
    }
    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_DOWN_PIN) == GPIO_PIN_RESET) {
        return BTN_DOWN;
    }
    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_LEFT_PIN) == GPIO_PIN_RESET) {
        return BTN_LEFT;
    }
    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_RIGHT_PIN) == GPIO_PIN_RESET) {
        return BTN_RIGHT;
    }
    return BTN_NONE;
}

bool Menu_IsIdle(Menu_t *menu) {
    return (menu->state == MENU_STATE_IDLE);
}

void Menu_Process(Menu_t *menu, LCD_I2C_t *lcd, I2C_HandleTypeDef *hi2c, DS3231_Time_t *current_time) {
    Button_t button = Menu_ReadButtons();

    if (button != BTN_NONE) {
        uint32_t now = HAL_GetTick();
        if ((now - menu->last_button_press) < DEBOUNCE_TIME) {
            return;
        }
        menu->last_button_press = now;
    }

    switch(menu->state) {
        case MENU_STATE_IDLE:
            if (button == BTN_SET) {
                menu->state = MENU_STATE_MAIN;
                menu->menu_selection = 0;
                LCD_Clear(lcd);
                ShowMainMenu(menu, lcd);
            }
            break;

        case MENU_STATE_MAIN:
            ProcessMainMenu(menu, button, lcd, hi2c, current_time);
            break;

        case MENU_STATE_CONFIG_DATETIME:
            ProcessDateTimeEditor(menu, button, lcd, hi2c);
            break;

        case MENU_STATE_CONFIG_ACQ:
            ProcessAcqEditor(menu, button, lcd);
            break;

        case MENU_STATE_CONFIG_LANGUAGE:
            ProcessLanguageEditor(menu, button, lcd);
            break;

        case MENU_STATE_TEST_MODE:
               // Será processado no main.c
               break;

        case MENU_STATE_CLEAR_DATA_CONFIRM:
            ProcessClearConfirm(menu, button, lcd);
            break;
    }
}

static void ProcessAcqEditor(Menu_t *menu, Button_t button, LCD_I2C_t *lcd) {
    switch(button) {
        case BTN_UP:
            if (menu->acq_selection > 0) {
                menu->acq_selection--;
                ShowAcqEditor(menu, lcd);
            }
            break;

        case BTN_DOWN:
            if (menu->acq_selection + 1 < ACQ_TOTAL_OPTIONS) {
                menu->acq_selection++;
                ShowAcqEditor(menu, lcd);
            }
            break;

        case BTN_SET:
            ApplyAcquisitionTime((AcquisitionTime_t)menu->acq_selection);
            SaveAcquisitionTimeToFlash((AcquisitionTime_t)menu->acq_selection);

            LCD_Clear(lcd);
            LCD_SetCursor(lcd, 0, 1);
            LCD_Print(lcd, I18N(TXT_MENU_ACQ_SAVED, current_language));

            HAL_Delay(800);

            menu->state = MENU_STATE_IDLE;
            LCD_Clear(lcd);
            break;

        case BTN_LEFT:
        case BTN_RIGHT:
            menu->state = MENU_STATE_IDLE;
            LCD_Clear(lcd);
            break;

        default:
            break;
    }
}


static const TextId_t kMenuItems[MENU_TOTAL_ITEMS] = {
    TXT_MENU_ITEM_DATETIME,
    TXT_MENU_ITEM_ACQ,
    TXT_MENU_ITEM_LANGUAGE,
    TXT_MENU_ITEM_TEST,
    TXT_MENU_ITEM_CLEAR_DATA
};
static void ShowClearConfirm(Menu_t *menu, LCD_I2C_t *lcd) {
    char line[21];

    LCD_SetCursor(lcd, 0, 0);
    LCD_Print(lcd, I18N(TXT_CLEAR_TITLE, current_language));

    LCD_SetCursor(lcd, 0, 1);
    LCD_Print(lcd, I18N(TXT_CLEAR_QUESTION, current_language));

    // Linha 2: "NAO      SIM" (com seleção)
    const char *no_s  = I18N(TXT_CLEAR_NO, current_language);
    const char *yes_s = I18N(TXT_CLEAR_YES, current_language);

    // monta 20 colunas
    // exemplo: "[NAO]     SIM" ou " NAO    [SIM]"
    if (menu->confirm_selection == 0) {
        snprintf(line, sizeof(line), "[%s]           %s", no_s, yes_s);
    } else {
        snprintf(line, sizeof(line), " %s           [%s]", no_s, yes_s);
    }
    while (strlen(line) < 20) strcat(line, " ");
    line[20] = '\0';

    LCD_SetCursor(lcd, 0, 2);
    LCD_Print(lcd, line);

    LCD_SetCursor(lcd, 0, 3);
    LCD_Print(lcd, I18N(TXT_CLEAR_FOOTER, current_language));
}


static void PrintMenuLine(LCD_I2C_t *lcd, uint8_t row, TextId_t id, uint8_t selected) {
    char line[21];
    const char *s = I18N(id, current_language);

    strncpy(line, s, 20);
    line[20] = '\0';

    if (!selected && line[0] == '>') line[0] = ' ';
    LCD_SetCursor(lcd, 0, row);
    LCD_Print(lcd, line);
}

static void ShowMainMenu(Menu_t *menu, LCD_I2C_t *lcd) {
    // garante que o item selecionado esteja visível nas 4 linhas
    if (menu->menu_selection < menu->menu_scroll) {
        menu->menu_scroll = menu->menu_selection;
    } else if (menu->menu_selection >= (menu->menu_scroll + 4)) {
        menu->menu_scroll = menu->menu_selection - 3;
    }

    for (uint8_t row = 0; row < 4; row++) {
        uint8_t item_index = menu->menu_scroll + row;

        if (item_index < MENU_TOTAL_ITEMS) {
            uint8_t selected = (item_index == menu->menu_selection);
            PrintMenuLine(lcd, row, kMenuItems[item_index], selected);
        } else {
            LCD_SetCursor(lcd, 0, row);
            LCD_Print(lcd, "                    ");
        }
    }
}





static void ProcessClearConfirm(Menu_t *menu, Button_t button, LCD_I2C_t *lcd) {
    switch (button) {
        case BTN_LEFT:
        case BTN_RIGHT:
            menu->confirm_selection = 1 - menu->confirm_selection;
            ShowClearConfirm(menu, lcd);
            break;

        case BTN_SET:
            if (menu->confirm_selection == 0) {
                // NÃO
                menu->state = MENU_STATE_MAIN;
                LCD_Clear(lcd);
                ShowMainMenu(menu, lcd);
                return;
            }

            // SIM: feedback na tela (3 idiomas via I18N)
            LCD_Clear(lcd);
            LCD_SetCursor(lcd, 0, 1);
            LCD_Print(lcd, I18N(TXT_CLEAR_ERASING, current_language));
            HAL_Delay(150);

            // SIM: apaga NORMAL + TESTE (sem mexer em config)
            W25Q_LogReset(&flash);   // deve salvar state preservando config
            W25Q_TestReset(&flash);  // já salva o state de teste no seu driver
            total_samples_saved = 0;

            LCD_Clear(lcd);
            LCD_SetCursor(lcd, 0, 1);
            LCD_Print(lcd, I18N(TXT_CLEAR_DONE, current_language));
            HAL_Delay(1200);

            menu->state = MENU_STATE_IDLE;
            LCD_Clear(lcd);
            break;

        default:
            break;
    }
}



static void ProcessMainMenu(Menu_t *menu, Button_t button, LCD_I2C_t *lcd, I2C_HandleTypeDef *hi2c, DS3231_Time_t *current_time) {
    switch(button) {
        case BTN_UP:
            if (menu->menu_selection > 0) {
                menu->menu_selection--;
                ShowMainMenu(menu, lcd);
            }
            break;

        case BTN_DOWN:
            if (menu->menu_selection + 1 < MENU_TOTAL_ITEMS) {
                menu->menu_selection++;
                ShowMainMenu(menu, lcd);
            }
            break;

        case BTN_SET:
            if (menu->menu_selection == 0) {
                menu->state = MENU_STATE_CONFIG_DATETIME;
                menu->edit_field = FIELD_YEAR;
                memcpy(&menu->temp_time, current_time, sizeof(DS3231_Time_t));
                LCD_Clear(lcd);
                ShowDateTimeEditor(menu, lcd);

            } else if (menu->menu_selection == 1) {
                menu->state = MENU_STATE_CONFIG_ACQ;
                menu->acq_selection = (uint8_t)current_acq_time;
                LCD_Clear(lcd);
                ShowAcqEditor(menu, lcd);

            } else if (menu->menu_selection == 2) {
                menu->state = MENU_STATE_CONFIG_LANGUAGE;
                menu->lang_selection = current_language;
                if (menu->lang_selection >= LANG_TOTAL) menu->lang_selection = LANG_PT;
                LCD_Clear(lcd);
                ShowLanguageEditor(menu, lcd);

            } else if (menu->menu_selection == 3) {
                menu->state = MENU_STATE_TEST_MODE;
                LCD_Clear(lcd);
            }
            else if (menu->menu_selection == 4) {
                menu->state = MENU_STATE_CLEAR_DATA_CONFIRM;
                menu->confirm_selection = 0; // começa em NÃO
                LCD_Clear(lcd);
                ShowClearConfirm(menu, lcd);
            }

            break;

        case BTN_LEFT:
        case BTN_RIGHT:
            menu->state = MENU_STATE_IDLE;
            LCD_Clear(lcd);
            break;

        default:
            break;
    }
}

static void ShowLanguageEditor(Menu_t *menu, LCD_I2C_t *lcd) {
    LCD_SetCursor(lcd, 0, 0);
    LCD_Print(lcd, I18N(TXT_MENU_LANG_TITLE, current_language));

    LCD_SetCursor(lcd, 0, 1);
    LCD_Print(lcd, I18N(TXT_MENU_LANG_LINE1, current_language));

    LCD_SetCursor(lcd, 0, 2);
    char line[21];
    snprintf(line, sizeof(line), "    [ %s ]", LangToStr(menu->lang_selection));
    while (strlen(line) < 20) strcat(line, " ");
    line[20] = '\0';
    LCD_Print(lcd, line);

    LCD_SetCursor(lcd, 0, 3);
    LCD_Print(lcd, I18N(TXT_MENU_LANG_FOOTER, current_language));
}

static void ProcessLanguageEditor(Menu_t *menu, Button_t button, LCD_I2C_t *lcd) {
    switch(button) {
        case BTN_UP:
            if (menu->lang_selection > 0) {
                menu->lang_selection--;
                ShowLanguageEditor(menu, lcd);
            }
            break;

        case BTN_DOWN:
            if (menu->lang_selection + 1 < LANG_TOTAL) {
                menu->lang_selection++;
                ShowLanguageEditor(menu, lcd);
            }
            break;

        case BTN_SET:
            current_language = menu->lang_selection;
            SaveLanguageToFlash(current_language);

            LCD_Clear(lcd);
            LCD_SetCursor(lcd, 0, 1);
            LCD_Print(lcd, I18N(TXT_MENU_LANG_SAVED, current_language));
            HAL_Delay(800);

            menu->state = MENU_STATE_IDLE;
            LCD_Clear(lcd);
            break;

        case BTN_LEFT:
        case BTN_RIGHT:
            menu->state = MENU_STATE_IDLE;
            LCD_Clear(lcd);
            break;

        default:
            break;
    }
}

static void ShowDateTimeEditor(Menu_t *menu, LCD_I2C_t *lcd) {
    char buffer[25];
    char line1[21];
    char line2[21];
    char line3[21];

    LCD_SetCursor(lcd, 0, 0);
   // LCD_Print(lcd, "Config Data/Hora:     ");
    LCD_Print(lcd, I18N(TXT_MENU_DATETIME_TITLE, current_language));

    line1[0] = '\0';

    if (menu->edit_field == FIELD_YEAR) sprintf(buffer, "[%04d]", menu->temp_time.year);
    else                                sprintf(buffer, " %04d ", menu->temp_time.year);
    strcat(line1, buffer); strcat(line1, "/");

    if (menu->edit_field == FIELD_MONTH) sprintf(buffer, "[%02d]", menu->temp_time.month);
    else                                 sprintf(buffer, " %02d ", menu->temp_time.month);
    strcat(line1, buffer); strcat(line1, "/");

    if (menu->edit_field == FIELD_DAY) sprintf(buffer, "[%02d]", menu->temp_time.date);
    else                               sprintf(buffer, " %02d ", menu->temp_time.date);
    strcat(line1, buffer);

    while(strlen(line1) < 20) strcat(line1, " ");
    line1[20] = '\0';

    LCD_SetCursor(lcd, 0, 1);
    LCD_Print(lcd, line1);

    line2[0] = '\0';

    if (menu->edit_field == FIELD_HOUR) sprintf(buffer, "[%02d]", menu->temp_time.hours);
    else                                sprintf(buffer, " %02d ", menu->temp_time.hours);
    strcat(line2, buffer); strcat(line2, ":");

    if (menu->edit_field == FIELD_MINUTE) sprintf(buffer, "[%02d]", menu->temp_time.minutes);
    else                                  sprintf(buffer, " %02d ", menu->temp_time.minutes);
    strcat(line2, buffer); strcat(line2, ":");

    if (menu->edit_field == FIELD_SECOND) sprintf(buffer, "[%02d]", menu->temp_time.seconds);
    else                                  sprintf(buffer, " %02d ", menu->temp_time.seconds);
    strcat(line2, buffer);

    while(strlen(line2) < 20) strcat(line2, " ");
    line2[20] = '\0';

    LCD_SetCursor(lcd, 0, 2);
    LCD_Print(lcd, line2);
    line3[0] = '\0';

    // Monta " SAIR      SALVAR " (com [] dependendo do campo)
    char left[8], right[10];
    snprintf(left, sizeof(left), "%s", I18N(TXT_MENU_DATETIME_CANCEL, current_language));
    snprintf(right, sizeof(right), "%s", I18N(TXT_MENU_DATETIME_SAVE, current_language));

    if (menu->edit_field == FIELD_EXIT) {
        snprintf(line3, sizeof(line3), "[%s]     %s", left + 1, right); // left tem espaço inicial
    } else if (menu->edit_field == FIELD_DONE) {
        snprintf(line3, sizeof(line3), " %s    [%s]", left + 1, right + 1); // right tem espaço inicial
    } else {
        snprintf(line3, sizeof(line3), " %s     %s", left + 1, right);
    }
    while (strlen(line3) < 20) strcat(line3, " ");
    line3[20] = '\0';

    LCD_SetCursor(lcd, 0, 3);
    LCD_Print(lcd, line3);

}

static void ShowAcqEditor(Menu_t *menu, LCD_I2C_t *lcd) {
    char buffer[25];

    if (menu->acq_selection >= ACQ_TOTAL_OPTIONS) {
        menu->acq_selection = 0;
    }

    LCD_SetCursor(lcd, 0, 0);
    LCD_Print(lcd, I18N(TXT_MENU_ACQ_TITLE, current_language));

    LCD_SetCursor(lcd, 0, 2);
    sprintf(buffer, "      [ %s ]         ", acq_time_options[menu->acq_selection].display_text);
    buffer[20] = '\0';
    LCD_Print(lcd, buffer);

    LCD_SetCursor(lcd, 0, 3);
    LCD_Print(lcd, I18N(TXT_MENU_ACQ_FOOTER, current_language));
}

static void ProcessDateTimeEditor(Menu_t *menu, Button_t button, LCD_I2C_t *lcd, I2C_HandleTypeDef *hi2c) {
    switch(button) {
        case BTN_UP:
            if (menu->edit_field < FIELD_EXIT) {
                IncrementField(menu);
                ShowDateTimeEditor(menu, lcd);
            }
            break;

        case BTN_DOWN:
            if (menu->edit_field < FIELD_EXIT) {
                DecrementField(menu);
                ShowDateTimeEditor(menu, lcd);
            }
            break;

        case BTN_RIGHT:
            if (menu->edit_field < FIELD_DONE) {
                menu->edit_field++;
                ShowDateTimeEditor(menu, lcd);
            }
            break;

        case BTN_LEFT:
            if (menu->edit_field > FIELD_YEAR) {
                menu->edit_field--;
                ShowDateTimeEditor(menu, lcd);
            }
            break;

        case BTN_SET:
            if (menu->edit_field == FIELD_EXIT) {
                LCD_Clear(lcd);
                LCD_SetCursor(lcd, 0, 1);
                LCD_Print(lcd, I18N(TXT_DATETIME_CANCELED, current_language));
                HAL_Delay(800);
                menu->state = MENU_STATE_IDLE;
                LCD_Clear(lcd);
            } else if (menu->edit_field == FIELD_DONE) {
                DS3231_SetTime(hi2c, &menu->temp_time);
                LCD_Clear(lcd);
                LCD_SetCursor(lcd, 0, 1);
                LCD_Print(lcd, I18N(TXT_DATETIME_SAVED, current_language));
                HAL_Delay(1000);
                menu->state = MENU_STATE_IDLE;
                LCD_Clear(lcd);
            }
            break;

        default:
            break;
    }
}

static void IncrementField(Menu_t *menu) {
    switch(menu->edit_field) {
        case FIELD_YEAR:
            menu->temp_time.year++;
            if (menu->temp_time.year > 2099) menu->temp_time.year = 2000;
            break;
        case FIELD_MONTH:
            menu->temp_time.month++;
            if (menu->temp_time.month > 12) menu->temp_time.month = 1;
            break;
        case FIELD_DAY:
            menu->temp_time.date++;
            if (menu->temp_time.date > 31) menu->temp_time.date = 1;
            break;
        case FIELD_HOUR:
            menu->temp_time.hours++;
            if (menu->temp_time.hours > 23) menu->temp_time.hours = 0;
            break;
        case FIELD_MINUTE:
            menu->temp_time.minutes++;
            if (menu->temp_time.minutes > 59) menu->temp_time.minutes = 0;
            break;
        case FIELD_SECOND:
            menu->temp_time.seconds++;
            if (menu->temp_time.seconds > 59) menu->temp_time.seconds = 0;
            break;
        default:
            break;
    }
}

static void DecrementField(Menu_t *menu) {
    switch(menu->edit_field) {
        case FIELD_YEAR:
            menu->temp_time.year--;
            if (menu->temp_time.year < 2000) menu->temp_time.year = 2099;
            break;
        case FIELD_MONTH:
            menu->temp_time.month--;
            if (menu->temp_time.month < 1) menu->temp_time.month = 12;
            break;
        case FIELD_DAY:
            menu->temp_time.date--;
            if (menu->temp_time.date < 1) menu->temp_time.date = 31;
            break;
        case FIELD_HOUR:
            menu->temp_time.hours--;
            if (menu->temp_time.hours > 23) menu->temp_time.hours = 23;
            break;
        case FIELD_MINUTE:
            menu->temp_time.minutes--;
            if (menu->temp_time.minutes > 59) menu->temp_time.minutes = 59;
            break;
        case FIELD_SECOND:
            menu->temp_time.seconds--;
            if (menu->temp_time.seconds > 59) menu->temp_time.seconds = 59;
            break;
        default:
            break;
    }
}
