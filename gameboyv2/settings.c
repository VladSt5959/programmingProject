#include "settings.h"
#include "colors.h"
#include "gfx.h"
#include "ili9341.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/adc.h"
#include "libs/ownLibs/btnReader.h"
#include <stdint.h>


#define SETTINGS_BATTERY_SENSE_ENABLED 0
#define LDR_GPIO                  27
#define LDR_ADC_CHANNEL            1
#define BATTERY_GPIO              26
#define BATTERY_ADC_CHANNEL        0
#define BATTERY_DIVIDER_RATIO    2.0f  
#define ADC_REFERENCE_VOLTAGE    3.3f
#define BRIGHTNESS_MIN             10
#define BRIGHTNESS_MAX            100
#define AUTO_SAMPLE_INTERVAL_MS   150

enum { SETTING_BRIGHTNESS, SETTING_BATTERY, SETTING_UPTIME, SETTING_REBOOT,
       SETTING_COUNT };

static int brightness = 80;
static int batteryPercent = -1;
static float batteryVoltage = 0.0f;
static int selectedSettingIndex;
static bool autoBrightness = true;
static bool adcReady;
static uint32_t lastAutoSampleMs;
static uint32_t lastBatterySampleMs;

static uint32_t uptimeSeconds(void)
{
    return to_ms_since_boot(get_absolute_time()) / 1000u;
}

static void initSettingsAdc(void)
{
    if (adcReady) return;
    adc_init();
    adc_gpio_init(LDR_GPIO);
#if SETTINGS_BATTERY_SENSE_ENABLED
    adc_gpio_init(BATTERY_GPIO);
#endif
    adcReady = true;
}

static uint16_t readAdcAverage(uint channel)
{
    adc_select_input(channel);
    (void)adc_read(); 
    uint32_t total = 0;
    for (int i = 0; i < 8; ++i) total += adc_read();
    return (uint16_t)(total / 8u);
}

static int clampBrightness(int value)
{
    if (value < BRIGHTNESS_MIN) return BRIGHTNESS_MIN;
    if (value > BRIGHTNESS_MAX) return BRIGHTNESS_MAX;
    return value;
}


void settingsService(void)
{
    if (!autoBrightness) return;
    initSettingsAdc();

    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((uint32_t)(now - lastAutoSampleMs) < AUTO_SAMPLE_INTERVAL_MS) return;
    lastAutoSampleMs = now;

    uint16_t raw = readAdcAverage(LDR_ADC_CHANNEL);
    int target = BRIGHTNESS_MIN + (int)((uint32_t)raw *
                 (BRIGHTNESS_MAX - BRIGHTNESS_MIN) / 4095u);
    target = clampBrightness(target);


    if (target > brightness + 2 || target < brightness - 2) {
        brightness = target;
        LCD_setBrightness((uint8_t)brightness);
    }
}

static int voltageToPercent(float v)
{
  
    if (v <= 3.30f) return 0;
    if (v < 3.50f) return 5;
    if (v < 3.60f) return 10;
    if (v < 3.70f) return 20;
    if (v < 3.75f) return 35;
    if (v < 3.80f) return 50;
    if (v < 3.85f) return 65;
    if (v < 3.90f) return 78;
    if (v < 4.00f) return 88;
    if (v < 4.10f) return 95;
    return 100;
}

static void updateBatteryReading(void)
{
#if SETTINGS_BATTERY_SENSE_ENABLED
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((uint32_t)(now - lastBatterySampleMs) < 1000u) return;
    lastBatterySampleMs = now;
    uint16_t raw = readAdcAverage(BATTERY_ADC_CHANNEL);
    batteryVoltage = ((float)raw * ADC_REFERENCE_VOLTAGE / 4095.0f) * BATTERY_DIVIDER_RATIO;
    batteryPercent = voltageToPercent(batteryVoltage);
#else
    batteryPercent = -1;
    batteryVoltage = 0.0f;
#endif
}

void settingsInit(void)
{
    selectedSettingIndex = 0;
    initSettingsAdc();
    LCD_setBrightness((uint8_t)brightness);
    lastAutoSampleMs = 0;
    lastBatterySampleMs = 0;
    updateBatteryReading();
    GFX_clearScreen();
}

int settingsRead(void)
{
    if (btnPressed(BTN_B) || btnPressed(BTN_MENU) || btnPressed(BTN_MENU2)) return 1;

    if (btnPressed(BTN_UP)) {
        selectedSettingIndex = (selectedSettingIndex + SETTING_COUNT - 1) % SETTING_COUNT;
    } else if (btnPressed(BTN_DOWN)) {
        selectedSettingIndex = (selectedSettingIndex + 1) % SETTING_COUNT;
    }

    if (selectedSettingIndex == SETTING_BRIGHTNESS) {
        if (btnPressed(BTN_A)) {
            autoBrightness = !autoBrightness;
            if (autoBrightness) settingsService();
        }
        if (btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT)) {
            autoBrightness = false; /* directional adjustment switches to manual */
            brightness += btnPressed(BTN_RIGHT) ? 10 : -10;
            brightness = clampBrightness(brightness);
            LCD_setBrightness((uint8_t)brightness);
        }
    } else if (selectedSettingIndex == SETTING_REBOOT) {
        if (btnPressed(BTN_A) || btnPressed(BTN_RIGHT)) reset_usb_boot(0, 0);
    }
    return 0;
}

static void drawRow(int y, bool selected)
{
    if (selected) {
        GFX_fillRect(12, y, 296, 34, 0x10A4);
        GFX_drawRect(12, y, 296, 34, 0x07FF);
    } else {
        GFX_fillRect(12, y, 296, 34, 0x0841);
    }
    GFX_setTextColor(selected ? 0x07FF : 0xBDF7);
    GFX_setTextBack(selected ? 0x10A4 : 0x0841);
    GFX_setTextSize(1);
}

void settingsUpdate(void)
{
    settingsService();
    updateBatteryReading();

    uint32_t seconds = uptimeSeconds();
    uint32_t hours = seconds / 3600u;
    uint32_t minutes = (seconds / 60u) % 60u;
    uint32_t secs = seconds % 60u;

    GFX_fillScreen(0x1082);
    GFX_fillRect(0, 0, 320, 42, 0x0000);
    GFX_setTextColor(0x07FF); GFX_setTextBack(0x0000); GFX_setTextSize(2);
    GFX_setCursor(16, 12); GFX_printf("CONSOLE SETTINGS");

    drawRow(50, selectedSettingIndex == SETTING_BRIGHTNESS);
    GFX_setCursor(22, 57); GFX_printf("BRIGHTNESS  %s", autoBrightness ? "AUTO" : "MANUAL");
    GFX_setCursor(225, 57); GFX_printf("%d%%", brightness);
    GFX_fillRect(22, 73, 270, 5, 0x4208);
    GFX_fillRect(22, 73, (brightness * 270) / 100, 5, 0x07FF);

    drawRow(89, selectedSettingIndex == SETTING_BATTERY);
#if SETTINGS_BATTERY_SENSE_ENABLED
    GFX_setCursor(22, 96); GFX_printf("BATTERY  ~%d%%  %.2fV", batteryPercent, batteryVoltage);
    GFX_fillRect(22, 112, 270, 5, 0x4208);
    GFX_fillRect(22, 112, (batteryPercent * 270) / 100, 5, 0x07E0);
#else
    GFX_setCursor(22, 96); GFX_printf("BATTERY  NOT CONNECTED");
    GFX_setTextColor(0x8410); GFX_setCursor(22, 109); GFX_printf("Optional: enable divider in settings source");
#endif

    drawRow(128, selectedSettingIndex == SETTING_UPTIME);
    GFX_setCursor(22, 140); GFX_printf("CONSOLE UPTIME");
    GFX_setTextColor(0xFFFF); GFX_setCursor(202, 140);
    GFX_printf("%02lu:%02lu:%02lu", (unsigned long)hours,
               (unsigned long)minutes, (unsigned long)secs);

    drawRow(167, selectedSettingIndex == SETTING_REBOOT);
    GFX_setCursor(22, 179); GFX_printf("REBOOT TO BOOTSEL");
    GFX_setTextColor(0x8410); GFX_setCursor(22, 192); GFX_printf("Press A or RIGHT to enter USB mode");

    GFX_setTextColor(0xFFFF); GFX_setTextBack(0x1082); GFX_setCursor(22, 218);
    GFX_printf("UP/DOWN: select   LEFT/RIGHT: adjust   B: back");
    GFX_flush();
}
