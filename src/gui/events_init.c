/*
* Copyright 2023 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"



static void button1_event_handler(lv_event_t *e)
{
//	button_1_event_callback(true);
}
void events_init_screen(lv_ui *ui)
{	
	lv_obj_add_event_cb(ui->screen_roller_1, &screen_roller_1_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_btn_1,button1_event_handler, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(ui->screen_cb_2,button1_event_handler, LV_EVENT_CLICKED, NULL);

}

void events_init(lv_ui *ui)
{

}

void set_ui_callback()
{
//	set_speed_callback = &screen_roller_1_event_callback;
}
