// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.0
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "ui_helpers.h"
#include "ui_comp.h"
#include "ui_comp_hook.h"
#include "ui_events.h"


// SCREEN: ui_wificonnect
void ui_wificonnect_screen_init(void);
extern lv_obj_t * ui_wificonnect;
void ui_event_Panel1(lv_event_t * e);
extern lv_obj_t * ui_Panel1;
void ui_event_password(lv_event_t * e);
extern lv_obj_t * ui_password;
void ui_event_Keyboard1(lv_event_t * e);
extern lv_obj_t * ui_Keyboard1;
void ui_event_wificonne(lv_event_t * e);
extern lv_obj_t * ui_wificonne;
// CUSTOM VARIABLES
extern lv_obj_t * uic_wifi_password;
extern lv_obj_t * uic_wifi;

// EVENTS

extern lv_obj_t * ui____initial_actions0;

// FONTS
LV_FONT_DECLARE(ui_font_ali);

// UI INIT
void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif