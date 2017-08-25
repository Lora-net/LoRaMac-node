#include "board.h"

void TimerInit( TimerEvent_t *obj, void ( *callback )( void ) )
{
	if (obj == NULL)
		return;
	obj->id = &obj->timer;
	ret_code_t err_code = app_timer_create(&obj->id, APP_TIMER_MODE_SINGLE_SHOT, (app_timer_timeout_handler_t)callback);
	APP_ERROR_CHECK(err_code);
}

void TimerStart( TimerEvent_t *obj )
{
	if (obj->id == NULL || obj->timeout == 0)
		return;

	uint32_t ticks = APP_TIMER_TICKS(obj->timeout);
	if (ticks < APP_TIMER_MIN_TIMEOUT_TICKS)
		ticks = APP_TIMER_MIN_TIMEOUT_TICKS;

	ret_code_t err_code = app_timer_start(obj->id, ticks, NULL);
	APP_ERROR_CHECK(err_code);
}

void TimerStop( TimerEvent_t *obj )
{
	if (obj->id == NULL)
		return;

	ret_code_t err_code = app_timer_stop(obj->id);
	APP_ERROR_CHECK(err_code);
}

void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
	if (value < 10)
		value = 10;
	obj->timeout = value;
}

TimerTime_t TimerGetCurrentTime( void )
{
	return rtc_get_timestamp();
}

TimerTime_t TimerGetElapsedTime( TimerTime_t savedTime )
{
	uint32_t ts = rtc_get_timestamp();
	TimerTime_t elapsed = (TimerTime_t)0;
	if (savedTime < ts)
		elapsed = (TimerTime_t)(ts - savedTime);

	return elapsed;
}

void TimerLowPowerHandler( void )
{
}

TimerTime_t TimerTempCompensation( TimerTime_t period, float temperature )
{
	return period;
}
