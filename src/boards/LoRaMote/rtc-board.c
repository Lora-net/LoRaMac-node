/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: MCU RTC timer and low power modes management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <math.h>
#include "board.h"
#include "rtc-board.h"

/*!
 * RTC Month Coded in Decimal format
 */
#define MONTH_JANUARY              ( ( uint8_t )  1 )
#define MONTH_FEBRUARY             ( ( uint8_t )  2 )
#define MONTH_MARCH                ( ( uint8_t )  3 )
#define MONTH_APRIL                ( ( uint8_t )  4 )
#define MONTH_MAY                  ( ( uint8_t )  5 )
#define MONTH_JUNE                 ( ( uint8_t )  6 )
#define MONTH_JULY                 ( ( uint8_t )  7 )
#define MONTH_AUGUST               ( ( uint8_t )  8 )
#define MONTH_SEPTEMBER            ( ( uint8_t )  9 )
#define MONTH_OCTOBER              ( ( uint8_t ) 10 )
#define MONTH_NOVEMBER             ( ( uint8_t ) 11 )
#define MONTH_DECEMBER             ( ( uint8_t ) 12 )

/*!
 * RTC Time base in ms
 */
#define RTC_ALARM_TICK_DURATION                     0.12207031 // 1 tick every 122us

/*!
 * RTC Time base in ms
 */
#define RTC_ALARM_TICK_PER_MS                       8.192      //  1/8.192 = tick duration in ms

/*!
 * Number of seconds in a minute
 */
static const uint8_t SecondsInMinute = 60;

/*!
 * Number of minutes in a hour
 */
static const uint8_t MinutesInHour = 60;

/*!
 * Number of seconds in an hour
 */
static const uint16_t SecondsInHour = 3600;

/*!
 * Number of seconds in a day
 */
static const uint32_t SecondsInDay = 86400;

/*!
 * Number of hours in a day
 */
static const uint8_t HoursInDay = 24;

/*!
 * \brief Start the Rtc Alarm (timeoutValue is in ms)
 */
static void RtcStartWakeUpAlarm( uint32_t timeoutValue );

/*!
 * \brief Used to calibrate the MCU make-up time at the first timeout
 */
static void RtcComputeWakeUpTime( void );

/*!
 * \brief Used to compute the month overflow in Timeout computation
 */
static uint8_t RtcComputeMonthOverflow( uint8_t timeoutInDays, uint8_t year, uint8_t month );

/*!
 * \brief Flag to indicate if the timestamps until the next event is long enough
 * to set the MCU into low power mode
 */
static bool RtcTimerEventAllowsLowPower = false;

/*!
 * \brief Flag to disable the LowPower Mode even if the timestamps until the
 * next event is long enough to allow Low Power mode
 */
static bool LowPowerDisableDuringTask = false;

/*!
 * \brief RTC Handler
 */
RTC_HandleTypeDef RtcHandle;

/*!
 * \brief Keep the value of the RTC Date when the RTC alarm is set
 */
DateContext_t DateContext;

/*!
 * \brief Keep the value of the RTC Time when the RTC alarm is set
 */
TimeContext_t TimeContext;

/*!
 * \brief Indicates if the RTC is already Initalized or not
 */
static bool RtcInitalized = false;

/*!
 * \brief Indicates if the RTC Wake Up Time is calibrated or not
 */
static bool WakeUpTimeInitialized = false;

/*!
 * \brief Hold the Wake-up time duration in ms
 */
volatile uint32_t McuWakeUpTime = 0;

/*!
 * \brief Hold the cumulated error in micro-second to compensate the timing errors
 */
static uint32_t TimeoutValueError = 0;

void RtcInit( void )
{
    RTC_DateTypeDef dateRtc;
    RTC_TimeTypeDef timeRtc;

    if( RtcInitalized == false )
    {
        __HAL_RCC_RTC_ENABLE( );

        RtcHandle.Instance = RTC;
        RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
        RtcHandle.Init.AsynchPrediv = 1;
        RtcHandle.Init.SynchPrediv = 1;

        RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
        RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
        RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
        HAL_RTC_Init( &RtcHandle );

        /* Set Date: Friday 1st of Janurary 2000 */
        dateRtc.Year = 0x00;
        dateRtc.Month = MONTH_JANUARY;
        dateRtc.Date = 0x01;
        dateRtc.WeekDay = RTC_WEEKDAY_FRIDAY;
        HAL_RTC_SetDate( &RtcHandle, &dateRtc, RTC_FORMAT_BIN );

        /* Set Time: 00:00:00 */
        timeRtc.Hours = 0x00;
        timeRtc.Minutes = 0x00;
        timeRtc.Seconds = 0x01;
        timeRtc.TimeFormat = RTC_HOURFORMAT12_AM;
        timeRtc.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
        timeRtc.StoreOperation = RTC_STOREOPERATION_RESET;
        HAL_RTC_SetTime( &RtcHandle, &timeRtc, RTC_FORMAT_BIN );

        HAL_NVIC_SetPriority( RTC_Alarm_IRQn, 4, 0 );
        HAL_NVIC_EnableIRQ( RTC_Alarm_IRQn );
        RtcInitalized = true;
    }
}

uint32_t RtcGetMinimumTimeout( void )
{
    return 1;
}

void RtcSetTimeout( uint32_t timeout )
{
    if( timeout < 50 ) // 50 ms
    {
        // we don't go in Low Power mode for delay below 50ms (needed for LEDs)
        RtcTimerEventAllowsLowPower = false;
    }
    else
    {
        RtcTimerEventAllowsLowPower = true;
    }

    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {
        timeout = timeout - McuWakeUpTime;
    }

    RtcStartWakeUpAlarm( timeout );
}

TimerTime_t RtcGetElapsedAlarmTime( void )
{
    uint32_t elapsedTime = 0;
    uint32_t currentTimeMs = 0;
    uint32_t savedTimeMs = 0;

    RTC_DateTypeDef dateRtc;
    RTC_TimeTypeDef timeRtc;

    HAL_RTC_WaitForSynchro( &RtcHandle );

    HAL_RTC_GetTime( &RtcHandle, &timeRtc, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &RtcHandle, &dateRtc, RTC_FORMAT_BIN );

    if( DateContext.Year != dateRtc.Year ) // year roll over, DateContext in December and dateRtc in January
    {
        elapsedTime = RtcComputeMonthOverflow( ( dateRtc.Date + DateContext.Date ), DateContext.Year, RTC_MONTH_DECEMBER );
        elapsedTime = elapsedTime * SecondsInDay;
    }
    else if( DateContext.Month != dateRtc.Month )
    {
        elapsedTime = RtcComputeMonthOverflow( ( dateRtc.Date + DateContext.Date ), dateRtc.Year, DateContext.Month );
        elapsedTime = elapsedTime * SecondsInDay;
    }
    else
    {
        elapsedTime = ( dateRtc.Date - DateContext.Date ) * SecondsInDay;
    }

    currentTimeMs =  ( ( ( timeRtc.Hours * SecondsInHour ) +
                         ( timeRtc.Minutes * SecondsInMinute ) +
                           timeRtc.Seconds ) );

    savedTimeMs = ( ( ( TimeContext.Hours * SecondsInHour ) +
                      ( TimeContext.Minutes * SecondsInMinute ) +
                        TimeContext.Seconds ) );

    elapsedTime = ( elapsedTime ) + ( int32_t )( currentTimeMs - savedTimeMs );

    elapsedTime = round( elapsedTime * RTC_ALARM_TICK_DURATION );
    return( elapsedTime );
}

TimerTime_t RtcGetTimerValue( void )
{
    uint32_t currentTimeMs = 0;

    RTC_DateTypeDef dateRtc;
    RTC_TimeTypeDef timeRtc;

    HAL_RTC_WaitForSynchro( &RtcHandle );

    HAL_RTC_GetTime( &RtcHandle, &timeRtc, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &RtcHandle, &dateRtc, RTC_FORMAT_BIN );

    /* Get the Current time in Milisecond */
    currentTimeMs =  ( ( ( dateRtc.Date * SecondsInDay ) +
                         ( timeRtc.Hours * SecondsInHour ) +
                         ( timeRtc.Minutes * SecondsInMinute ) +
                           timeRtc.Seconds ) );

    currentTimeMs = round( currentTimeMs * RTC_ALARM_TICK_DURATION );

    return( currentTimeMs );
}

TimerTime_t RtcComputeFutureEventTime( TimerTime_t futureEventInTime )
{
    return( RtcGetTimerValue( ) + futureEventInTime );
}

TimerTime_t RtcComputeElapsedTime( TimerTime_t eventInTime )
{
    uint32_t currentTimeMs = 0;
    uint32_t elapsedTimeMs = 0;
    uint8_t lastMonth = 0;

    RTC_DateTypeDef dateRtc;
    RTC_TimeTypeDef timeRtc;

    HAL_RTC_WaitForSynchro( &RtcHandle );

    HAL_RTC_GetTime( &RtcHandle, &timeRtc, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &RtcHandle, &dateRtc, RTC_FORMAT_BIN );

    /* Get the Current time in Milisecond */
    currentTimeMs =  ( ( ( dateRtc.Date * SecondsInDay ) +
                         ( timeRtc.Hours * SecondsInHour ) +
                         ( timeRtc.Minutes * SecondsInMinute ) +
                           timeRtc.Seconds ) );

    currentTimeMs = round( currentTimeMs * RTC_ALARM_TICK_DURATION );

    if( eventInTime == 0 ) //Needed at boot, cannot compute with 0 or elapsed time will be equal to current time
    {
        return 0;
    }
    else if( currentTimeMs < eventInTime ) // counter has rolled over
    {
        // if the counter has roll over, eventInTime was last month
        if( dateRtc.Month == MONTH_JANUARY )   // if we are in January, lastMonth was December
        {
            lastMonth = MONTH_DECEMBER;
        }
        else
        {
            lastMonth = dateRtc.Month - 1;
        }

        // compute from eventInTime to roll over and add currentTimeMs
        if( lastMonth != MONTH_FEBRUARY )
        {
            switch( lastMonth )
            {
                case MONTH_APRIL:
                case MONTH_JUNE:
                case MONTH_SEPTEMBER:
                case MONTH_NOVEMBER:
                    elapsedTimeMs = ( 30 * SecondsInDay );
                    break;
                default:
                    elapsedTimeMs = ( 31 * SecondsInDay );
                    break;
            }
        }
        else
        {
            switch( dateRtc.Year )
            {
                case 0:
                case 4:
                case 8:
                case 12:
                case 16:
                case 20:
                case 24:
                case 28:
                case 32:
                case 36:
                case 40:
                    elapsedTimeMs = ( 29 * SecondsInDay );
                    break;
                default:
                    elapsedTimeMs = ( 28 * SecondsInDay );
                    break;
            }
        }
        // roll over is at the end of the day on the last day of the month
        elapsedTimeMs = round( ( elapsedTimeMs + SecondsInDay ) * RTC_ALARM_TICK_DURATION );
        // ( elapsedTimeMs - eventInTime ) => from eventInTime to roll over
        elapsedTimeMs = currentTimeMs  + ( elapsedTimeMs - eventInTime );
    }
    else
    {
        elapsedTimeMs = currentTimeMs - eventInTime;
    }

    return( elapsedTimeMs );
}

static void RtcStartWakeUpAlarm( uint32_t timeoutValue )
{
    uint8_t timeoutValueSeconds = 0;
    uint8_t timeoutValueMinutes = 0;
    uint8_t timeoutValueHours = 0;
    uint8_t timeoutValueDays = 0;
    float timeoutValueTemp = 0.0;

    RTC_AlarmTypeDef AlarmStructure;
    RTC_DateTypeDef dateRtc;
    RTC_TimeTypeDef timeRtc;

    HAL_RTC_DeactivateAlarm( &RtcHandle, RTC_ALARM_A );
    HAL_RTCEx_DeactivateWakeUpTimer( &RtcHandle );

    HAL_RTC_WaitForSynchro( &RtcHandle );

    HAL_RTC_GetTime( &RtcHandle, &timeRtc, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &RtcHandle, &dateRtc, RTC_FORMAT_BIN );

    // Save Context
    DateContext.Date = dateRtc.Date;
    DateContext.Month = dateRtc.Month;
    DateContext.Year = dateRtc.Year;

    TimeContext.Hours = timeRtc.Hours;
    TimeContext.Minutes = timeRtc.Minutes;
    TimeContext.Seconds = timeRtc.Seconds;

    if( timeoutValue < 1 )
    {
        timeoutValue = 1;
    }

    /* timeoutValueTemp used to compensate the cumulating errors in timing far in the futur */
    timeoutValueTemp =  ( float )timeoutValue * RTC_ALARM_TICK_PER_MS;

    /* convert from ms base timeout into tick */
    /* tick is equivalent to 1/2048 sec meaning the calendar move by 34 minutes every sec */
    /* The MCU wake-up time in computed in tick here to simplify the computation */
    timeoutValue = round( timeoutValue * RTC_ALARM_TICK_PER_MS );

    if( timeoutValue < timeoutValueTemp )      // timeoutValue was rounded down
    {
        TimeoutValueError = TimeoutValueError + ( ( timeoutValueTemp - timeoutValue ) * 1000 );     // error in uS
    }
    else                                       // timeoutValue was rounded up
    {
        TimeoutValueError = TimeoutValueError - ( ( timeoutValue - timeoutValueTemp ) * 1000 ) ;    // error in uS
    }

    if( TimeoutValueError >= ( uint32_t )( RTC_ALARM_TICK_DURATION * 1000 ) )
    {
        timeoutValue = timeoutValue + 1;
        TimeoutValueError = TimeoutValueError - ( uint32_t )( RTC_ALARM_TICK_DURATION * 1000 );
    }

    if( timeoutValue > 2160000 )
    {
        // 25 "days" in tick
        // drastically reduce the computation time as we simply add 25 days in tick to current time.
        timeoutValueSeconds = timeRtc.Seconds;
        timeoutValueMinutes = timeRtc.Minutes;
        timeoutValueHours   = timeRtc.Hours;
        // simply add 25 days to current date and time
        timeoutValueDays     = 25 + dateRtc.Date;

        AlarmStructure.AlarmDateWeekDay = RtcComputeMonthOverflow( timeoutValueDays, dateRtc.Year, dateRtc.Month );
    }
    else
    {
        /* convert microsecs to RTC format and add to 'Now'*/
        /* 3x faster than legacy calc*/
        /* calc days */
        timeoutValueDays = dateRtc.Date;
        while( timeoutValue >= SecondsInDay )
        {
            timeoutValue -= SecondsInDay;
            timeoutValueDays++;
        }

        /* calc hours */
        timeoutValueHours = timeRtc.Hours;
        while( timeoutValue >= SecondsInHour )
        {
            timeoutValue -= SecondsInHour;
            timeoutValueHours++;
        }

        /* calc minutes */
        timeoutValueMinutes = timeRtc.Minutes;
        while( timeoutValue >= SecondsInMinute )
        {
            timeoutValue -= SecondsInMinute;
            timeoutValueMinutes++;
        }

        /* calc seconds */
        timeoutValueSeconds =  timeRtc.Seconds + timeoutValue;

        /***** correct for modulo********/
        while( timeoutValueSeconds >= SecondsInMinute )
        {
            timeoutValueSeconds -= SecondsInMinute;
            timeoutValueMinutes++;
        }

        while( timeoutValueMinutes >= MinutesInHour )
        {
            timeoutValueMinutes -= MinutesInHour;
            timeoutValueHours++;
        }

        while( timeoutValueHours >= HoursInDay )
        {
            timeoutValueHours -= HoursInDay;
            timeoutValueDays++;
        }

        if( timeoutValueDays < 28 )   // Timeout is on the same month, avoid Month roll over computation
        {
            AlarmStructure.AlarmDateWeekDay = timeoutValueDays;
        }
        else
        {
            AlarmStructure.AlarmDateWeekDay = RtcComputeMonthOverflow( timeoutValueDays, dateRtc.Year, dateRtc.Month );
        }
    }

    AlarmStructure.Alarm = RTC_ALARM_A;
    AlarmStructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    AlarmStructure.AlarmMask = RTC_ALARMMASK_NONE;
    AlarmStructure.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
    AlarmStructure.AlarmTime.Seconds = timeoutValueSeconds;
    AlarmStructure.AlarmTime.Minutes = timeoutValueMinutes;
    AlarmStructure.AlarmTime.Hours = timeoutValueHours;

    if( HAL_RTC_SetAlarm_IT( &RtcHandle,&AlarmStructure,RTC_FORMAT_BIN ) != HAL_OK )
    {
        assert_param( FAIL );
    }
}

void RtcEnterLowPowerStopMode( void )
{
    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {
        BoardDeInitMcu( );

        /* Disable the Power Voltage Detector */
        HAL_PWR_DisablePVD( );

        SET_BIT( PWR->CR, PWR_CR_CWUF );

        /* Enable Ultra low power mode */
        HAL_PWREx_EnableUltraLowPower( );

        /* Enable the fast wake up from Ultra low power mode */
        HAL_PWREx_EnableFastWakeUp( );

        /* Enter Stop Mode */
        HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
    }
}

void RtcRecoverMcuStatus( void )
{
    if( ( __HAL_RCC_GET_SYSCLK_SOURCE( ) == RCC_SYSCLKSOURCE_STATUS_HSI ) ||
        ( __HAL_RCC_GET_SYSCLK_SOURCE( ) == RCC_SYSCLKSOURCE_STATUS_MSI ) )
    {
        BoardInitMcu( );
    }
}

/*!
 * \brief RTC IRQ Handler on the RTC Alarm
 */
void RTC_Alarm_IRQHandler( void )
{
    HAL_RTC_AlarmIRQHandler( &RtcHandle );
    HAL_RTC_DeactivateAlarm( &RtcHandle, RTC_ALARM_A );
    __HAL_PWR_CLEAR_FLAG( PWR_FLAG_WU );
    RtcRecoverMcuStatus( );
    RtcComputeWakeUpTime( );
    BlockLowPowerDuringTask( false );
    TimerIrqHandler( );
}

void BlockLowPowerDuringTask( bool status )
{
    if( status == true )
    {
        RtcRecoverMcuStatus( );
    }
    LowPowerDisableDuringTask = status;
}

static uint8_t RtcComputeMonthOverflow( uint8_t timeoutInDays, uint8_t year, uint8_t month )
{
    uint8_t timeout = 0;

    if( month != MONTH_FEBRUARY )
    {
        switch( month )
        {
            case MONTH_APRIL:
            case MONTH_JUNE:
            case MONTH_SEPTEMBER:
            case MONTH_NOVEMBER:
                if( timeoutInDays > 30 )    // strictly above the due date or will go to 0
                {
                    timeout = timeoutInDays % 30;
                }
                else
                {
                    timeout = timeoutInDays;
                }
                break;

            default:
                if( timeoutInDays > 31 )    // strictly above the due date or will go to 0
                {
                    timeout = timeoutInDays % 31;
                }
                else
                {
                    timeout = timeoutInDays;
                }
                break;
        }
    }
    else
    {
        switch( year )
        {
            case 0:
            case 4:
            case 8:
            case 12:
            case 16:
            case 20:
            case 24:
            case 28:
            case 32:
            case 36:
            case 40:
                if( timeoutInDays > 29 )    // strickly above the due date or will go to 0
                {
                    timeout = timeoutInDays % 29;// Feb has 29 days every leap year
                }
                else
                {
                    timeout = timeoutInDays;
                }
                break;
            default:
                if( timeoutInDays > 28 )    // strictly above the due date or will go to 0
                {
                    timeout = timeoutInDays % 28;// Feb has 29 days every leap year
                }
                else
                {
                    timeout = timeoutInDays;
                }
                break;
        }
    }

    return( timeout );
}

static void RtcComputeWakeUpTime( void )
{
    uint32_t start = 0;
    uint32_t stop = 0;
    RTC_AlarmTypeDef  AlarmRtc;
    RTC_TimeTypeDef  timeRtc;

    if( WakeUpTimeInitialized == false )
    {
        HAL_RTC_WaitForSynchro( &RtcHandle );

        HAL_RTC_GetTime( &RtcHandle, &timeRtc, RTC_FORMAT_BIN );
        HAL_RTC_GetAlarm( &RtcHandle, &AlarmRtc, RTC_ALARM_A, RTC_FORMAT_BIN );

        start = AlarmRtc.AlarmTime.Seconds + ( SecondsInMinute * AlarmRtc.AlarmTime.Minutes ) + ( SecondsInHour * AlarmRtc.AlarmTime.Hours );
        stop = timeRtc.Seconds + ( SecondsInMinute * timeRtc.Minutes ) + ( SecondsInHour * timeRtc.Hours );

        if( GetBoardPowerSource( ) == USB_POWER )
        {
            McuWakeUpTime = stop - start;
        }
        else    // BATTERY_POWER
        {
            McuWakeUpTime = stop - start + 1;// 1 tick for RTC synchronisation when waking up from stop mode
        }

        McuWakeUpTime = round( McuWakeUpTime * RTC_ALARM_TICK_DURATION );
        WakeUpTimeInitialized = true;
    }
}
