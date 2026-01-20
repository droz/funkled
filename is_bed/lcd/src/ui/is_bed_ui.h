#ifndef IS_BED_UI_H
#define IS_BED_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../lvgl.h"

#if LV_USE_GRID == 0
#error "LV_USE_GRID needs to be enabled"
#endif

#if LV_USE_FLEX == 0
#error "LV_USE_FLEX needs to be enabled"
#endif

// Create a complete UI for the IS Bed
void is_bed_ui(void);

// Unhide the widgets once we have at least received one pattern from the controller
void unhide_widgets();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*IS_BED_UI_H*/
