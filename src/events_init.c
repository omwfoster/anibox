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


static lv_timer_t * task_chart;
static void record_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_SCREEN_LOADED:
	{
	//	task_chart = lv_timer_create(record_chart_timer_cb, 100, guider_ui.record_chart_board);
		break;
	}
	case LV_EVENT_SCREEN_UNLOAD_START:
	{
		lv_timer_del(task_chart);
		break;
	}
	default:
		break;
	}
}
void events_init_record(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->record, record_event_handler, LV_EVENT_ALL, NULL);
}

void events_init(lv_ui *ui)
{

}
