#include "systime.h"

SysTime_t SysTimeGet( void )
{
    SysTime_t calendarTime = { .Seconds = 0, .SubSeconds = 0 };
    SysTime_t sysTime = { .Seconds = 0, .SubSeconds = 0 };
    uint32_t seconds;
    uint32_t subSeconds;

    // calendarTime.Seconds = RtcGetCalendarTime( ( uint16_t* )&calendarTime.SubSeconds );

    // RtcBkupRead( &seconds, &subSeconds );

    SysTime_t deltaTime = { .Seconds = seconds, .SubSeconds = ( int16_t )subSeconds };

    // sysTime = SysTimeAdd( deltaTime, calendarTime );

    return sysTime;
}

SysTime_t SysTimeGetMcuTime( void )
{
    SysTime_t calendarTime = { .Seconds = 0, .SubSeconds = 0 };

    // calendarTime.Seconds = RtcGetCalendarTime( ( uint16_t* )&calendarTime.SubSeconds );

    return calendarTime;
}


SysTime_t SysTimeSub( SysTime_t a, SysTime_t b )
{
    SysTime_t c = { .Seconds = 0, .SubSeconds = 0 };

    c.Seconds = a.Seconds - b.Seconds;
    c.SubSeconds = a.SubSeconds - b.SubSeconds;
    if( c.SubSeconds < 0 )
    {
        c.Seconds--;
        c.SubSeconds += 1000;
    }
    return c;
}