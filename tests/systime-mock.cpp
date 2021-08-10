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

SysTime_t SysTimeAdd( SysTime_t a, SysTime_t b )
{
    SysTime_t c =  { .Seconds = 0, .SubSeconds = 0 };

    c.Seconds = a.Seconds + b.Seconds;
    c.SubSeconds = a.SubSeconds + b.SubSeconds;
    if( c.SubSeconds >= 1000 )
    {
        c.Seconds++;
        c.SubSeconds -= 1000;
    }
    return c;
}


uint32_t SysTimeToMs( SysTime_t sysTime )
{
    uint32_t seconds;
    uint32_t subSeconds;

    // RtcBkupRead( &seconds, &subSeconds );

    SysTime_t deltaTime = { .Seconds = seconds, .SubSeconds = ( int16_t )subSeconds };

    SysTime_t calendarTime = SysTimeSub( sysTime, deltaTime );

    return calendarTime.Seconds * 1000 + calendarTime.SubSeconds;
}


SysTime_t SysTimeFromMs( uint32_t timeMs )
{
    uint32_t seconds = timeMs / 1000;
    uint32_t subSeconds = timeMs - seconds * 1000;
    SysTime_t sysTime = { .Seconds = seconds, .SubSeconds = ( int16_t )subSeconds };

    // RtcBkupRead( &seconds, &subSeconds );

    SysTime_t deltaTime = { .Seconds = seconds, .SubSeconds = ( int16_t )subSeconds };

    return SysTimeAdd( sysTime, deltaTime );
}



void SysTimeSet( SysTime_t sysTime )
{
    SysTime_t deltaTime;
  
    SysTime_t calendarTime = { .Seconds = 0, .SubSeconds = 0 };

    // calendarTime.Seconds = RtcGetCalendarTime( ( uint16_t* )&calendarTime.SubSeconds );

    // sysTime is epoch
    deltaTime = SysTimeSub( sysTime, calendarTime );

    // RtcBkupWrite( deltaTime.Seconds, ( uint32_t )deltaTime.SubSeconds );
}
