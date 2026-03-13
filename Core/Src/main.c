/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           :  main.c
  * @brief          : Weather Station - 30m/1h + Modo Teste + Bateria
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "usb_host.h"
#include "i18n.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "data_buffer.h"
#include "bmp180.h"
#include "dht11.h"
#include "ds3231.h"
#include "lcd_i2c.h"
#include "usb_datalogger.h"
#include "w25qxx.h"
#include <stdio.h>
#include <math.h>
#include "menu.h"
#include "usbh_core.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LOCAL_ALTITUDE_M  11.0f

// ✅ Config setor (4KB) - NÃO usar erase+1byte
#define CONFIG_SECTOR_ADDRESS   0x3FF000u
//#define W25Q_LOG_STATE_ADDR (W25Q32_CAPACITY - 1*W25Q32_SECTOR_SIZE) // 0x3FF000
#define CONFIG_ACQ_OFFSET       0u
#define CONFIG_LANG_OFFSET      4u

#define BATTERY_ADC_CHANNEL ADC_CHANNEL_7    // Ajuste para PA7, ADC1_IN7
#define BATTERY_VREF 3.3f
#define BATTERY_ADC_MAX 4095.0f
#define BATTERY_VOLTAGE_DIVIDER 2.0f   // Divisor 10k/10k = metade
#define BATTERY_FULL    4.2f
#define BATTERY_EMPTY   3.0f
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi2;
TIM_HandleTypeDef htim2;
/* USER CODE BEGIN PV */
BMP180_t bmp180;
DHT11_t dht11;
DS3231_Time_t rtc_time;
LCD_I2C_t lcd;
USB_DataLogger_t datalogger;
W25Q_t flash;

uint32_t acquisition_interval_ms = 3600000;
uint32_t total_samples_saved = 0;
static uint32_t last_dht_read = 0;
static uint8_t last_humidity = 0;
uint32_t last_display_update = 0;
uint32_t display_update_interval = 1000;
float last_temp = 0;
float last_press_abs = 0;
float last_press_sea = 0;
uint8_t last_battery_percent = 0;
Menu_t menu;
uint8_t usb_menu_active = 0;
uint8_t usb_menu_selection = 0;
uint8_t usb_was_connected = 0;

const AcqTimeConfig_t acq_time_options[ACQ_TOTAL_OPTIONS] = {
    {ACQ_30M,  1800000,  "30m"},
    {ACQ_1H,   3600000,  "1h"}
};
AcquisitionTime_t current_acq_time = ACQ_1H;
extern ApplicationTypeDef Appli_state;

// ✅ idioma atual (persistido na flash)
uint8_t current_language = LANG_PT;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_SPI2_Init(void);
static void MX_ADC1_Init(void);
void MX_USB_HOST_Process(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void float_to_string(char *str, float value, uint8_t decimal_places) {
    int int_part = (int)value;
    int dec_part = (int)((value - int_part) * 100);
    if (dec_part < 0) dec_part = -dec_part;
    if (decimal_places == 1) {
        dec_part /= 10;
        sprintf(str, "%d.%d", int_part, dec_part);
    } else {
        sprintf(str, "%d.%02d", int_part, dec_part);
    }
}

float CalculateSeaLevelPressure(float pressure_hpa, float altitude_m) {
    return pressure_hpa + (altitude_m / 8.3f);
}

void UpdateDisplay(DS3231_Time_t *time, float temp, float press_abs, float press_sea, uint8_t humidity, uint8_t battery_percent, USB_State_t usb_state) {
    char buffer[25];
    char value_str[12];
    LCD_SetCursor(&lcd, 0, 0);
    sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
            time->date, time->month, time->year,
            time->hours, time->minutes, time->seconds);
    buffer[20] = '\0';
    LCD_Print(&lcd, buffer);

    // ===== Linha 2 =====
    LCD_SetCursor(&lcd, 0, 1);
    float_to_string(value_str, temp, 1);
    snprintf(buffer, sizeof(buffer), I18N(TXT_MAIN_LINE2_FMT, current_language),
             value_str, humidity, battery_percent);
    while (strlen(buffer) < 20) strcat(buffer, " ");
    buffer[20] = '\0';
    LCD_Print(&lcd, buffer);

    // ===== Linha 3 =====
    LCD_SetCursor(&lcd, 0, 2);
    char sea_str[12];
    char abs_str[12];
    float_to_string(sea_str, press_sea, 1);
    float_to_string(abs_str, press_abs, 1);
    snprintf(buffer, sizeof(buffer), I18N(TXT_MAIN_LINE3_FMT, current_language),
             sea_str, abs_str);
    while (strlen(buffer) < 20) strcat(buffer, " ");
    buffer[20] = '\0';
    LCD_Print(&lcd, buffer);

    // ===== Linha 4 =====
    LCD_SetCursor(&lcd, 0, 3);
    snprintf(buffer, sizeof(buffer), I18N(TXT_MAIN_LINE4_FMT, current_language),
             (unsigned long)total_samples_saved,
             acq_time_options[current_acq_time].display_text);
    while (strlen(buffer) < 20) strcat(buffer, " ");
    buffer[20] = '\0';
    LCD_Print(&lcd, buffer);

}

// =========================
// ✅ Config Sector Helpers
// =========================
static void ConfigSector_Read(uint8_t *buf4096) {
    W25Q_ReadData(&flash, CONFIG_SECTOR_ADDRESS, buf4096, W25Q32_SECTOR_SIZE);
}

static void ConfigSector_Write(const uint8_t *buf4096) {
    W25Q_EraseSector(&flash, CONFIG_SECTOR_ADDRESS);

    for (uint32_t off = 0; off < W25Q32_SECTOR_SIZE; off += W25Q32_PAGE_SIZE) {
        W25Q_WritePage(&flash, CONFIG_SECTOR_ADDRESS + off, (uint8_t*)(buf4096 + off), W25Q32_PAGE_SIZE);
    }
}

// =========================
// ✅ Idioma na flash
// =========================
void SaveLanguageToFlash(uint8_t lang) {
    if (lang >= LANG_TOTAL) lang = LANG_PT;

    uint8_t buf[W25Q32_SECTOR_SIZE];
    ConfigSector_Read(buf);
    buf[CONFIG_LANG_OFFSET] = lang;
    ConfigSector_Write(buf);
}

uint8_t LoadLanguageFromFlash(void) {
    uint8_t buf[W25Q32_SECTOR_SIZE];
    ConfigSector_Read(buf);

    uint8_t lang = buf[CONFIG_LANG_OFFSET];
    if (lang == 0xFF || lang >= LANG_TOTAL) {
        return LANG_PT;
    }
    return lang;
}

// =========================
// ✅ Aquisição na flash (sem apagar idioma)
// =========================
void SaveAcquisitionTimeToFlash(AcquisitionTime_t time) {
    uint8_t buf[W25Q32_SECTOR_SIZE];
    ConfigSector_Read(buf);

    buf[CONFIG_ACQ_OFFSET] = (uint8_t)time;

    ConfigSector_Write(buf);
}

AcquisitionTime_t LoadAcquisitionTimeFromFlash(void) {
    uint8_t buf[W25Q32_SECTOR_SIZE];
    ConfigSector_Read(buf);

    uint8_t data = buf[CONFIG_ACQ_OFFSET];
    if (data == 0xFF || data >= ACQ_TOTAL_OPTIONS) {
        return ACQ_1H;
    }
    return (AcquisitionTime_t)data;
}

void ApplyAcquisitionTime(AcquisitionTime_t time) {
    current_acq_time = time;
    acquisition_interval_ms = acq_time_options[time].milliseconds;
}

// FUNÇÕES DE BATERIA
float ReadBatteryVoltage(void) {
    uint32_t adc_value = 0;
    for (uint8_t i = 0; i < 10; i++) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 100);
        adc_value += HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
    }
    adc_value /= 10;
    float voltage = ((float)adc_value / BATTERY_ADC_MAX) * BATTERY_VREF * BATTERY_VOLTAGE_DIVIDER;
    return voltage;
}

uint8_t CalculateBatteryPercent(float voltage) {
    if (voltage >= BATTERY_FULL) return 100;
    if (voltage <= BATTERY_EMPTY) return 0;
    float percent = ((voltage - BATTERY_EMPTY) / (BATTERY_FULL - BATTERY_EMPTY)) * 100.0f;
    return (uint8_t)percent;
}

void RunTestMode(void) {
    LCD_Clear(&lcd);
    LCD_SetCursor(&lcd, 0, 0);
    LCD_Print(&lcd, I18N(TXT_TEST_MODE_TITLE, current_language));
    LCD_SetCursor(&lcd, 0, 1);
    LCD_Print(&lcd, I18N(TXT_TEST_MODE_SUBTITLE, current_language));
    LCD_SetCursor(&lcd, 0, 2);
    LCD_Print(&lcd, I18N(TXT_STARTING, current_language));
    HAL_Delay(2000);

    LCD_Clear(&lcd);
    LCD_SetCursor(&lcd, 0, 0);
    LCD_Print(&lcd, I18N(TXT_TEST_MODE_TITLE, current_language));
    uint32_t last_sample_time = HAL_GetTick();

    // sempre sobrescreve o teste antigo
    W25Q_TestReset(&flash);

    for (uint8_t i = 0; i < 10; i++) {
        if (i > 0) {
            while ((HAL_GetTick() - last_sample_time) < 5000) {
                HAL_Delay(10);
            }
        }
        last_sample_time = HAL_GetTick();

        float temp = BMP180_ReadTemperature(&bmp180);
        float pressure = BMP180_ReadPressure(&bmp180);

        uint32_t now = HAL_GetTick();
        uint8_t humidity = last_humidity;
        if (now - last_dht_read >= 5000) {
            if (DHT11_Read(&dht11)) {
                humidity = dht11.humidity;
                last_humidity = humidity;
            }
            last_dht_read = now;
        }

        float battery_voltage = ReadBatteryVoltage();
        uint8_t battery_percent = CalculateBatteryPercent(battery_voltage);
        last_battery_percent = battery_percent;

        DS3231_Time_t current_time;
        DS3231_GetTime(&hi2c1, &current_time);

        DataSample_t sample;
        sample.temperature = temp;
        sample.pressure = pressure;
        sample.humidity = humidity;
        sample.battery_percent = battery_percent;
        memcpy(&sample.timestamp, &current_time, sizeof(DS3231_Time_t));

        W25Q_WriteTestSample(&flash, (uint8_t*)&sample, sizeof(DataSample_t));
        W25Q_TestState_Save(&flash);

        char msg[25];

        LCD_SetCursor(&lcd, 0, 1);
        snprintf(msg, sizeof(msg), I18N(TXT_SAMPLE_FMT, current_language), (int)(i + 1));
        while (strlen(msg) < 20) strcat(msg, " ");
        msg[20] = '\0';
        LCD_Print(&lcd, msg);

        LCD_SetCursor(&lcd, 0, 2);
        snprintf(msg, sizeof(msg), "T: %.1fC P:%.1fhPa", temp, pressure);
        while (strlen(msg) < 20) strcat(msg, " ");
        msg[20] = '\0';
        LCD_Print(&lcd, msg);

        LCD_SetCursor(&lcd, 0, 3);
        snprintf(msg, sizeof(msg), "H:%d%% B:%d%% ", humidity, battery_percent);
        while (strlen(msg) < 20) strcat(msg, " ");
        msg[20] = '\0';
        LCD_Print(&lcd, msg);
    }

    LCD_Clear(&lcd);
    LCD_SetCursor(&lcd, 0, 1);
    LCD_Print(&lcd, I18N(TXT_TEST_DONE, current_language));
    LCD_SetCursor(&lcd, 0, 2);
    LCD_Print(&lcd, I18N(TXT_TEST_OK_10, current_language));

    HAL_Delay(3000);
    menu.state = MENU_STATE_IDLE;
    LCD_Clear(&lcd);
}

/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_FATFS_Init();
  MX_USB_HOST_Init();
  MX_TIM2_Init();
  MX_SPI2_Init();
  MX_ADC1_Init();

  /* USER CODE BEGIN 2 */

  // LCD / RTC init inicial
  LCD_Init(&lcd, &hi2c1, 20, 4); LCD_Clear(&lcd); LCD_Backlight(&lcd, 1);
  DS3231_Init(&hi2c1);

  LCD_SetCursor(&lcd, 0, 0); LCD_Print(&lcd, "    <  CIDEPE  >  ");
  LCD_SetCursor(&lcd, 0, 1); LCD_Print(&lcd, "       EQ410A   ");
  LCD_SetCursor(&lcd, 0, 2); LCD_Print(&lcd, "       v1.0  ");
  HAL_Delay(2000);

  LCD_Clear(&lcd); LCD_SetCursor(&lcd, 0, 0); LCD_Print(&lcd, "Init Flash W25Q32.. .");
  W25Q_Init(&flash, &hspi2, GPIOB, GPIO_PIN_12); HAL_Delay(50);

  // ✅ carrega idioma primeiro (usa setor config)
  current_language = LoadLanguageFromFlash();

  // ✅ agora sim inicializa menu (pra pegar idioma atual)
  Menu_Init(&menu); HAL_Delay(1000);

  // carregar estado normal e teste (se não existir, zera)
  if (!W25Q_LogState_Load(&flash)) {
      flash.current_address = 0;
      flash.total_samples = 0;
  }
  if (!W25Q_TestState_Load(&flash)) {
      flash.test_current_address = W25Q_TEST_DATA_ADDR;
      flash.test_total_samples = 0;
  }

  total_samples_saved = W25Q_GetTotalSamples(&flash);

  uint8_t cmd_reset1 = 0x66; uint8_t cmd_reset2 = 0x99;
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi2, &cmd_reset1, 1, 100);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi2, &cmd_reset2, 1, 100);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); HAL_Delay(100);

  uint8_t jedec_cmd = 0x9F;
  uint8_t jedec_data[3] = {0};
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi2, &jedec_cmd, 1, 100);
  HAL_SPI_Receive(&hspi2, jedec_data, 3, 100);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

  LCD_SetCursor(&lcd, 0, 1);
  if (jedec_data[0] == 0xEF && jedec_data[2] == 0x16) {
      LCD_Print(&lcd, "W25Q32 OK!   (4MB)");
  } else if (jedec_data[0] == 0xEF && jedec_data[2] == 0x15) {
      LCD_Print(&lcd, "W25Q16 OK!  (2MB)");
  } else if (jedec_data[0] == 0xEF) {
      char msg[25]; sprintf(msg, "Winbond 0x%02X OK!   ", jedec_data[2]); LCD_Print(&lcd, msg);
  } else {
      char msg[25]; sprintf(msg, "Flash: %02X %02X %02X", jedec_data[0], jedec_data[1], jedec_data[2]); LCD_Print(&lcd, msg);
  }
  HAL_Delay(2000);

  LCD_SetCursor(&lcd, 0, 2); LCD_Print(&lcd, I18N(TXT_BOOT_LOADING_CONFIG, current_language));
  current_acq_time = LoadAcquisitionTimeFromFlash();
  ApplyAcquisitionTime(current_acq_time);
  HAL_Delay(100);
  char config_msg[25];

 // sprintf(config_msg, "Modo: %s        ", acq_time_options[current_acq_time].display_text);

  snprintf(config_msg, sizeof(config_msg),
           I18N(TXT_BOOT_MODE_FMT, current_language),
           acq_time_options[current_acq_time].display_text);
    config_msg[20] = '\0';

  LCD_SetCursor(&lcd, 0, 2); LCD_Print(&lcd, config_msg);
  HAL_Delay(2000);

  LCD_Clear(&lcd); LCD_SetCursor(&lcd, 0, 0); LCD_Print(&lcd, I18N(TXT_BOOT_INIT_RTC, current_language));
  if (! DS3231_Init(&hi2c1)) {
	  LCD_SetCursor(&lcd, 0, 1); LCD_Print(&lcd, I18N(TXT_BOOT_RTC_ERR, current_language));
    HAL_Delay(1000);
  } else {
	  LCD_SetCursor(&lcd, 0, 1); LCD_Print(&lcd, I18N(TXT_BOOT_RTC_OK, current_language));
    HAL_Delay(1000);
  }

  LCD_SetCursor(&lcd, 0, 2); LCD_Print(&lcd, I18N(TXT_BOOT_INIT_BMP, current_language));
  if (!BMP180_Init(&bmp180, &hi2c1, 3)) {
      LCD_Clear(&lcd); LCD_SetCursor(&lcd, 0, 1); LCD_Print(&lcd, I18N(TXT_BOOT_BMP_ERR, current_language));
      while(1) { HAL_Delay(100); }

  }
  LCD_SetCursor(&lcd, 0, 2); LCD_Print(&lcd, I18N(TXT_BOOT_BMP_OK, current_language)); HAL_Delay(1000);

  LCD_SetCursor(&lcd, 0, 3); LCD_Print(&lcd, I18N(TXT_BOOT_INIT_DHT, current_language));
  DHT11_Init(&dht11, GPIOA, GPIO_PIN_2);
  LCD_SetCursor(&lcd, 0, 3); LCD_Print(&lcd, I18N(TXT_BOOT_DHT_OK, current_language)); HAL_Delay(1000);

  USB_DataLogger_Init(&datalogger, 5000);

  LCD_Clear(&lcd);
  LCD_SetCursor(&lcd, 0, 1);
  LCD_Print(&lcd, I18N(TXT_SYSTEM_READY, current_language));
  HAL_Delay(1500);
  LCD_Clear(&lcd);

  LCD_SetCursor(&lcd, 0, 1); LCD_Print(&lcd, I18N(TXT_FIRST_ACQ, current_language));
  LCD_SetCursor(&lcd, 0, 2); LCD_Print(&lcd, I18N(TXT_WAIT, current_language));

  float temp = BMP180_ReadTemperature(&bmp180);
  float pressure = BMP180_ReadPressure(&bmp180);
  uint8_t humidity = 0;
  if (DHT11_Read(&dht11)) {
      humidity = dht11.humidity;
      last_humidity = humidity;
  }
  last_dht_read = HAL_GetTick();
  float battery_voltage = ReadBatteryVoltage();
  last_battery_percent = CalculateBatteryPercent(battery_voltage);

  DS3231_Time_t current_time;
  DS3231_GetTime(&hi2c1, &current_time);

  last_temp = temp;
  last_press_abs = pressure;
  last_press_sea = CalculateSeaLevelPressure(pressure, LOCAL_ALTITUDE_M);

  DataSample_t sample;
  sample.temperature = temp;
  sample.pressure = pressure;
  sample.humidity = humidity;
  sample.battery_percent = last_battery_percent;
  memcpy(&sample.timestamp, &current_time, sizeof(DS3231_Time_t));

  W25Q_WriteDataSample(&flash, (uint8_t*)&sample, sizeof(DataSample_t));
  total_samples_saved++;
  if ((total_samples_saved % 10) == 0) {
      W25Q_LogState_Save(&flash);
  }

  LCD_SetCursor(&lcd, 0, 2); LCD_Print(&lcd, "      OK!           ");
  HAL_Delay(1000); LCD_Clear(&lcd);

  static uint32_t last_acquisition_time = 0;
  static uint8_t acquisition_in_progress = 0;
  static uint32_t last_display_sensor_read = 0;
  last_acquisition_time = HAL_GetTick();

  while (1)
  {
    MX_USB_HOST_Process();
    uint32_t now = HAL_GetTick();

    // Aquisição periódica
    if (!acquisition_in_progress && (now - last_acquisition_time) >= acquisition_interval_ms) {
      acquisition_in_progress = 1; last_acquisition_time = now;

      float temp = BMP180_ReadTemperature(&bmp180);
      float pressure = BMP180_ReadPressure(&bmp180);

      uint8_t humidity = last_humidity;
      if (now - last_dht_read >= 5000) {
        uint8_t dht_ok = DHT11_Read(&dht11);
        if (dht_ok) {
            humidity = dht11.humidity;
            last_humidity = humidity;
        }
        last_dht_read = now;
      }

      float battery_voltage = ReadBatteryVoltage();
      uint8_t battery_percent = CalculateBatteryPercent(battery_voltage);
      last_battery_percent = battery_percent;

      DS3231_Time_t current_time;
      DS3231_GetTime(&hi2c1, &current_time);

      last_temp = temp;
      last_press_abs = pressure;
      last_press_sea = CalculateSeaLevelPressure(pressure, LOCAL_ALTITUDE_M);

      DataSample_t sample;
      sample.temperature = temp;
      sample.pressure = pressure;
      sample.humidity = humidity;
      sample.battery_percent = battery_percent;
      memcpy(&sample.timestamp, &current_time, sizeof(DS3231_Time_t));

      W25Q_WriteDataSample(&flash, (uint8_t*)&sample, sizeof(DataSample_t));
      total_samples_saved++;

      if ((total_samples_saved % 10) == 0) {
          W25Q_LogState_Save(&flash);
      }

      acquisition_in_progress = 0;
    }

    // Atualização dos sensores para o display (2s)
    if (Menu_IsIdle(&menu) && (now - last_display_sensor_read) >= 2000) {
      last_display_sensor_read = now;
      float temp = BMP180_ReadTemperature(&bmp180);
      float pressure = BMP180_ReadPressure(&bmp180);

      uint8_t humidity = last_humidity;
      if (now - last_dht_read >= 5000) {
        uint8_t dht_ok = DHT11_Read(&dht11);
        if (dht_ok) {
            humidity = dht11.humidity;
            last_humidity = humidity;
        }
        last_dht_read = now;
      }

      float battery_voltage = ReadBatteryVoltage();
      last_battery_percent = CalculateBatteryPercent(battery_voltage);

      last_temp = temp;
      last_press_abs = pressure;
      last_press_sea = CalculateSeaLevelPressure(pressure, LOCAL_ALTITUDE_M);
    }

    USB_DataLogger_Process(&datalogger);

    if (Appli_state == APPLICATION_READY && !usb_was_connected && !usb_menu_active) {
      usb_was_connected = 1;
      usb_menu_active = 1;
      usb_menu_selection = 0;
      LCD_Clear(&lcd); LCD_SetCursor(&lcd, 0, 0);
      LCD_Print(&lcd, I18N(TXT_USB_DETECTED, current_language));
    }

    if (Appli_state != APPLICATION_READY) {
      usb_was_connected = 0;
      if (usb_menu_active) {
        usb_menu_active = 0; LCD_Clear(&lcd);
      }
    }

    if (usb_menu_active) {
      LCD_SetCursor(&lcd, 0, 2);
      if (usb_menu_selection == 0) LCD_Print(&lcd, I18N(TXT_USB_MENU_TRANSFER_SEL, current_language));
      else                         LCD_Print(&lcd, I18N(TXT_USB_MENU_TRANSFER, current_language));
      LCD_SetCursor(&lcd, 0, 3);
      if (usb_menu_selection == 1) LCD_Print(&lcd, I18N(TXT_USB_MENU_CANCEL_SEL, current_language));
      else                         LCD_Print(&lcd, I18N(TXT_USB_MENU_CANCEL, current_language));
      static uint8_t btn_up_pressed = 0;

      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET && !btn_up_pressed) {
        btn_up_pressed = 1;
        if (usb_menu_selection > 0) usb_menu_selection--;
      }
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET) btn_up_pressed = 0;

      static uint8_t btn_down_pressed = 0;
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET && !btn_down_pressed) {
        btn_down_pressed = 1;
        if (usb_menu_selection < 1) usb_menu_selection++;
      }
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET) btn_down_pressed = 0;

      static uint8_t btn_set_pressed = 0;
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET && !btn_set_pressed) {
        btn_set_pressed = 1;

        if (usb_menu_selection == 0) {
          uint32_t total = W25Q_GetTotalSamples(&flash);
          uint32_t total_test = W25Q_GetTestTotalSamples(&flash);

          if (total == 0 && total_test == 0) {
            LCD_Clear(&lcd); LCD_SetCursor(&lcd, 0, 1);
            LCD_Print(&lcd, I18N(TXT_NO_DATA, current_language));
            HAL_Delay(2000);
          } else {
            LCD_Clear(&lcd);
            LCD_SetCursor(&lcd, 0, 0);
            LCD_Print(&lcd, I18N(TXT_EXPORTING, current_language));
            LCD_SetCursor(&lcd, 0, 1);
            char export_msg[30];
            sprintf(export_msg, " Normal:%lu Test:%lu", total, total_test);
            LCD_Print(&lcd, export_msg);
            LCD_SetCursor(&lcd, 0, 2);
            LCD_Print(&lcd, I18N(TXT_PLEASE_WAIT, current_language));
            uint8_t ok1 = 1;
            uint8_t ok2 = 1;

            if (total > 0) ok1 = USB_DataLogger_ExportFlashToCSV(&datalogger);
            if (total_test > 0) ok2 = USB_DataLogger_ExportTestFlashToCSV(&datalogger);

            LCD_Clear(&lcd); LCD_SetCursor(&lcd, 0, 1);

            if (ok1 && ok2) LCD_Print(&lcd, I18N(TXT_SUCCESS, current_language));
            else LCD_Print(&lcd, I18N(TXT_USB_ERROR, current_language));

            HAL_Delay(3000);
          }
        }

        usb_menu_active = 0; LCD_Clear(&lcd);
      }
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) btn_set_pressed = 0;
    } else {
      Menu_Process(&menu, &lcd, &hi2c1, &rtc_time);
      if (menu.state == MENU_STATE_TEST_MODE) { RunTestMode(); }

      if (Menu_IsIdle(&menu)) {
        if ((now - last_display_update) >= display_update_interval) {
          last_display_update = now;
          DS3231_GetTime(&hi2c1, &rtc_time);
          UpdateDisplay(&rtc_time, last_temp, last_press_abs, last_press_sea,
                        last_humidity, last_battery_percent, datalogger.state);
        }
      }
    }

    HAL_Delay(10);
  }
  /* USER CODE END 2 */
}

/* (restante MX_*_Init e Error_Handler igual ao seu arquivo atual) */

/* Insira aqui suas funções MX_*_Init exatamente igual ao CubeMX gerou ou ao seu arquivo anterior! */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_SPI2_Init(void)
{
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 41999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = SET_Pin|UP_Pin|DOWN_Pin|LEFT_Pin|RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DHT_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DHT_DATA_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = BATTERY_ADC_CHANNEL;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
