#include "clock.h"

#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"
#include "gfx.h"
#include "colors.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CLOCK_W 320
#define CLOCK_H 240

static const char *const monthNames[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static const char *const weekdayNames[7] = {
    "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY",
    "THURSDAY", "FRIDAY", "SATURDAY"};

static int64_t buildEpochSeconds;
static uint64_t bootTimeUs;
static int64_t lastRenderedSecond = -1;
static bool started;
static bool use12Hour;

static int monthNumber(const char *month)
{
    for (int i = 0; i < 12; i++)
    {
        if (strncmp(month, monthNames[i], 3) == 0)
        {
            return i + 1;
        }
    }
    return 1;
}

// Gregorian calendar date to days since 1970-01-01.
static int64_t daysFromCivil(int year, unsigned month, unsigned day)
{
    year -= month <= 2;
    int era = (year >= 0 ? year : year - 399) / 400;
    unsigned yearOfEra = (unsigned)(year - era * 400);
    int adjustedMonth = (int)month + (month > 2 ? -3 : 9);
    unsigned dayOfYear = (153u * (unsigned)adjustedMonth + 2u) / 5u + day - 1u;
    unsigned dayOfEra = yearOfEra * 365u + yearOfEra / 4u -
                        yearOfEra / 100u + dayOfYear;
    return (int64_t)era * 146097 + (int64_t)dayOfEra - 719468;
}

// Days since 1970-01-01 back to a Gregorian date.
static void civilFromDays(int64_t days, int *year, unsigned *month, unsigned *day)
{
    days += 719468;
    int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    unsigned dayOfEra = (unsigned)(days - era * 146097);
    unsigned yearOfEra = (dayOfEra - dayOfEra / 1460u + dayOfEra / 36524u -
                          dayOfEra / 146096u) /
                         365u;
    int y = (int)yearOfEra + (int)(era * 400);
    unsigned dayOfYear = dayOfEra -
                         (365u * yearOfEra + yearOfEra / 4u - yearOfEra / 100u);
    unsigned monthPrime = (5u * dayOfYear + 2u) / 153u;
    unsigned d = dayOfYear - (153u * monthPrime + 2u) / 5u + 1u;
    unsigned m = monthPrime < 10u ? monthPrime + 3u : monthPrime - 9u;
    y += m <= 2u;

    *year = y;
    *month = m;
    *day = d;
}

void clockAppStartup(void)
{
    if (started)
    {
        return;
    }

    char monthText[4] = {0};
    int day = 1, year = 2024;
    int hour = 0, minute = 0, second = 0;
    sscanf(__DATE__, "%3s %d %d", monthText, &day, &year);
    sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);

    int month = monthNumber(monthText);
    buildEpochSeconds = daysFromCivil(year, (unsigned)month, (unsigned)day) * 86400 +
                        hour * 3600 + minute * 60 + second;
    bootTimeUs = time_us_64();
    started = true;
}
static void renderClock(int64_t nowSeconds)
{
    int64_t days = nowSeconds / 86400;
    int secondsOfDay = (int)(nowSeconds % 86400);
    if (secondsOfDay < 0)
    {
        secondsOfDay += 86400;
        days--;
    }

    int year;
    unsigned month, day;
    civilFromDays(days, &year, &month, &day);

    int hour24 = secondsOfDay / 3600;
    int minute = (secondsOfDay / 60) % 60;
    int second = secondsOfDay % 60;
    int weekday = (int)((days + 4) % 7); // 1970-01-01 was Thursday.
    if (weekday < 0)
        weekday += 7;

    int displayHour = hour24;
    const char *ampm = "";
    if (use12Hour)
    {
        ampm = hour24 < 12 ? "AM" : "PM";
        displayHour = hour24 % 12;
        if (displayHour == 0)
            displayHour = 12;
    }

    GFX_fillScreen(DARKBLUE);
    GFX_fillRect(0, 0, CLOCK_W, 32, PURPLE);
    GFX_setTextColor(CYAN);
    GFX_setTextBack(PURPLE);
    GFX_setTextSize(2);
    GFX_setCursor(12, 8);
    GFX_printf("PIXEL DESK CLOCK");

    GFX_drawRect(12, 42, 296, 144, MAGENTA);
    GFX_drawRect(16, 46, 288, 136, VIOLET);

    GFX_setTextColor(WHITE);
    GFX_setTextBack(DARKBLUE);
    GFX_setTextSize(5);
    GFX_setCursor(30, 78);
    GFX_printf("%02d%c%02d", displayHour, (second % 2) ? ' ' : ':', minute);

    GFX_setTextColor(CYAN);
    GFX_setTextSize(2);
    GFX_setCursor(218, 99);
    GFX_printf("%02d", second);
    if (use12Hour)
    {
        GFX_setCursor(218, 119);
        GFX_printf("%s", ampm);
    }

    GFX_setTextColor(YELLOW);
    GFX_setTextSize(2);
    GFX_setCursor(60, 145);
    GFX_printf("%s  %02u %s %04d", weekdayNames[weekday], day,
               monthNames[month - 1], year);

    GFX_fillRect(32, 196, 256, 5, 0x4208);
    GFX_fillRect(32, 196, (second * 256) / 59, 5, CYAN);

    GFX_setTextColor(WHITE);
    GFX_setTextSize(1);
    GFX_setCursor(78, 218);
    GFX_printf("A: 12/24 HOUR     B: MENU");
    GFX_flush();
}

void clockAppInit(void)
{
    clockAppStartup();
    lastRenderedSecond = -1;
    clockAppUpdate();
}

int clockAppRead(void)
{
    if (btnPressed(BTN_B))
    {
        return 1;
    }
    if (btnPressed(BTN_A))
    {
        use12Hour = !use12Hour;
        lastRenderedSecond = -1;
    }
    return 0;
}

void clockAppUpdate(void)
{
    if (!started)
    {
        clockAppStartup();
    }

    uint64_t elapsedSeconds = (time_us_64() - bootTimeUs) / 1000000u;
    int64_t nowSeconds = buildEpochSeconds + (int64_t)elapsedSeconds;
    if (nowSeconds != lastRenderedSecond)
    {
        renderClock(nowSeconds);
        lastRenderedSecond = nowSeconds;
    }
}