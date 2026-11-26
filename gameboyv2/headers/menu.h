#ifndef MENU_TEMP_H
#define MENU_TEMP_H

void menuInit(void);
int menuRead(void);
void menuUpdate(void);

// Set this to a measured value from 0 to 100. Use -1 for unknown.
void menuTempSetBatteryPercent(int percent);

#endif