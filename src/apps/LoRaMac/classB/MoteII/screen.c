/*!
 * \file      screen.c
 *
 * \brief     Screen management
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include "delay.h"
#include "gps.h"
#include "display-board.h"

#include "buttons.h"
#include "screen.h"

/*!
 * \brief Rx slot glyphs definition
 */
const uint8_t RxSlotGlyph[][8] =
{
    { 0x00, 0x08, 0x38, 0x08, 0x08, 0x08, 0x08, 0x3E }, // 1
    { 0x00, 0x18, 0x24, 0x04, 0x0C, 0x18, 0x30, 0x7E }, // 2
    { 0x00, 0x33, 0x4B, 0x08, 0x18, 0x30, 0x60, 0xFC }, // 2+
    { 0x00, 0x7C, 0x22, 0x22, 0x3C, 0x20, 0x20, 0x78 }, // P
};

/*!
 * \brief Device class glyphs definition
 */
const uint8_t ClassGlyph[][8] =
{
    { 0x00, 0x38, 0x1C, 0x14, 0x14, 0x3E, 0x22, 0x63 }, // A
    { 0x00, 0x78, 0x24, 0x24, 0x3C, 0x22, 0x22, 0x7C }, // B
    { 0x00, 0x1C, 0x22, 0x60, 0x40, 0x60, 0x22, 0x1C }  // C
};

/*!
 * \brief ADR symbols glyphs definition
 */
const uint8_t AdrGlyph[][8] =
{
    { 0x00, 0x38, 0x1C, 0x14, 0x14, 0x3E, 0x22, 0x63 }, // A
    { 0x00, 0x7C, 0x22, 0x22, 0x22, 0x22, 0x22, 0x7C }, // D
    { 0x00, 0x7C, 0x22, 0x22, 0x3C, 0x22, 0x22, 0x73 }  // R
};

/*!
 * \brief Timer used to update the GPS icon state
 */
static TimerEvent_t GpsIconUpdateTimer;

/*!
 * \brief Indicates if the GPS timer is started
 */
static bool GpsIconUpdateTimerStarted = false;

/*!
 * \brief Current GPS icon state.
*        [0: Outer box, 1: Inner box, 2: middle pixel]
 */
static uint8_t GpsIconUpdateState = 0;

/*!
 * \brief GPS icon X coordinate
 */
static uint8_t GpsIconUpdateX = 0;

/*!
 * \brief GPS icon Y coordinate
 */
static uint8_t GpsIconUpdateY = 0;

/*!
 * \brief Current join progress state.
 */
static uint8_t JoinProgress = 0;

/*!
 * \brief Indicates if the Screen need to be updated or not
 */
static bool ScreenUpdate = true;

/*!
 * \brief Holds the current active screen
 */
static Screen_t CurrentScreen = SCREEN_BOOT;

/*!
 * \brief Indicates if the Screen is initialized or not
 */
static bool ScreenInitialized = false;

/*!
 * \brief Indicates if the Screen is ON or OFF
 */
static bool IsDisplayOn = true;

/*!
 * \brief Handle the buttons
 */
static void ButtonsProcess( void )
{
    if( ScreenInitialized == false )
    {
        return;
    }
    if( CurrentScreen == SCREEN_BOOT )
    {
        return;
    }

    if( ButtonGetState( BUTTON_LEFT ) == true )
    {
        if( CurrentScreen > SCREEN_1 )
        {
            CurrentScreen--;
        }
        else
        {
            CurrentScreen = SCREEN_LAST;
        }
        ScreenUpdate = true;
    }

    if( ButtonGetState( BUTTON_MIDDLE ) == true )
    {
        if( IsDisplayOn == true )
        {
            IsDisplayOn = false;
            DisplayOff( );
        }
        else
        {
            IsDisplayOn = true;
            DisplayOn( );
        }
    }

    if( ButtonGetState( BUTTON_RIGHT ) == true )
    {
        if( CurrentScreen < SCREEN_LAST )
        {
            CurrentScreen++;
        }
        else
        {
            CurrentScreen = SCREEN_1;
        }
        ScreenUpdate = true;
    }
}

void ScreenInit( void )
{
    DisplayInit( );

    ScreenInitialized = true;
    CurrentScreen = SCREEN_BOOT;
}

Screen_t ScreenGetCurrent( void )
{
    return CurrentScreen;
}

void ScreenSetCurrent( Screen_t current )
{
    CurrentScreen = current;
    ScreenUpdate = true;
}

/*!
 * \brief Draws left and right arrows on the toolbar
 *
 * \param y     Y coordinate for the arrows
 */
void ScreenDrawArrows( uint8_t y )
{
    DisplayFillTriangle( 0, y, 4, y - 4, 4, y + 4, DISPLAY_WHITE );
    DisplayFillTriangle( 127, y, 123, y - 4, 123, y + 4, DISPLAY_WHITE );
}

/*!
 * \brief Draws the battery icon with the given level
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param level Battery level (As defined into the LoRaWAN specification)
 */
void ScreenDrawBattery( uint8_t x, uint8_t y, uint8_t level )
{
    DisplayFillRect( x, y, 18, 8, DISPLAY_BLACK );

    DisplayDrawVerticalLine( x, y + 2, 4, DISPLAY_WHITE );
    DisplayDrawVerticalLine( x + 1, y + 1, 6, DISPLAY_WHITE );
    DisplayDrawRect( x + 2, y, 16, 8, DISPLAY_WHITE );

    if( level == 0 ) // External power source
    {
        // Draw plug
        DisplayDrawHorizontalLine( x + 6, y + 2, 5, DISPLAY_WHITE );
        DisplayDrawHorizontalLine( x + 9, y + 3, 5, DISPLAY_WHITE );
        DisplayDrawHorizontalLine( x + 9, y + 4, 5, DISPLAY_WHITE );
        DisplayDrawHorizontalLine( x + 6, y + 5, 5, DISPLAY_WHITE );
    }
    else if( level == 255 ) // Error
    {
        // Draw cross
        DisplayDrawHorizontalLine( x + 3, y + 1, 3, DISPLAY_WHITE );
        DisplayDrawHorizontalLine( x + 14, y + 1, 3, DISPLAY_WHITE );

        DisplayDrawHorizontalLine( x + 6, y + 2, 2, DISPLAY_WHITE );
        DisplayDrawHorizontalLine( x + 12, y + 2, 2, DISPLAY_WHITE );

        DisplayDrawHorizontalLine( x + 9, y + 3, 4, DISPLAY_WHITE );

        DisplayDrawHorizontalLine( x + 9, y + 4, 4, DISPLAY_WHITE );

        DisplayDrawHorizontalLine( x + 6, y + 5, 2, DISPLAY_WHITE );
        DisplayDrawHorizontalLine( x + 12, y + 5, 2, DISPLAY_WHITE );

        DisplayDrawHorizontalLine( x + 3, y + 6, 3, DISPLAY_WHITE );
        DisplayDrawHorizontalLine( x + 14, y + 6, 3, DISPLAY_WHITE );
    }
    else
    {
        if( level < 32 )
        {
            // Empty
        }
        else if( level < 64 )
        {
            DisplayDrawVerticalLine( x + 16, y + 2, 4, DISPLAY_WHITE );
        }
        else if( level < 96 )
        {
            DisplayDrawVerticalLine( x + 16, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 14, y + 2, 4, DISPLAY_WHITE );
        }
        else if( level < 128 )
        {
            DisplayDrawVerticalLine( x + 16, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 14, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 12, y + 2, 4, DISPLAY_WHITE );
        }
        else if( level < 160 )
        {
            DisplayDrawVerticalLine( x + 16, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 14, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 12, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 10, y + 2, 4, DISPLAY_WHITE );
        }
        else if( level < 192 )
        {
            DisplayDrawVerticalLine( x + 16, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 14, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 12, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 10, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x +  8, y + 2, 4, DISPLAY_WHITE );
        }
        else if( level < 224 )
        {
            DisplayDrawVerticalLine( x + 16, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 14, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 12, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 10, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x +  8, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x +  6, y + 2, 4, DISPLAY_WHITE );
        }
        else
        {
            DisplayDrawVerticalLine( x + 16, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 14, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 12, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x + 10, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x +  8, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x +  6, y + 2, 4, DISPLAY_WHITE );
            DisplayDrawVerticalLine( x +  4, y + 2, 4, DISPLAY_WHITE );
        }
    }
}

/*!
 * \brief Draws the given glyph at coordinates (x,y)
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param glyph Pointer to the glyph data
 * \param color Glyph foreground color
 */
void ScreenDrawGlyph( uint8_t x, uint8_t y, const uint8_t *glyph, DisplayColor_t color )
{
    for( uint8_t i = 0; i < 8; i++ )
    {
        for( uint8_t j = 0; j < 8; j++ )
        {
            if( ( glyph[i] & ( 1 << ( 7 - j ) ) ) != 0 )
            {
                DisplayDrawPixel( x + j, y + i, color );
            }
        }
    }
}

/*!
 * \brief Draws Rx slot number icon
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param slot  Rx window slot number
 */
void ScreenDrawRxSlot( uint8_t x, uint8_t y, uint8_t slot )
{
    DisplayFillRect( x, y, 9, 9, DISPLAY_WHITE );
    ScreenDrawGlyph( x, y, RxSlotGlyph[slot], DISPLAY_BLACK );
}

/*!
 * \brief Draws device class icon
 *
 * \param x        X coordinate
 * \param y        Y coordinate
 * \param devClass Device class
 */
void ScreenDrawDevClass( uint8_t x, uint8_t y, DeviceClass_t devClass )
{
    DisplayFillRect( x, y, 9, 9, DISPLAY_WHITE );
    ScreenDrawGlyph( x, y, ClassGlyph[devClass], DISPLAY_BLACK );
}

/*!
 * \brief Draws the RSSI and SNR status.
 *        RSSI level with steps of 10 dBm
 *        Plus sign indicates that the SNR is positive and minus sign that it is negative
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param rssi  RSSI value
 * \param snr   SNR value
 */
void ScreenDrawRssi( uint8_t x, uint8_t y, int16_t rssi, int8_t snr )
{
    DisplayFillRect( x, y, 13, 10, DISPLAY_BLACK );

    DisplayDrawVerticalLine( x     , y + 8,  2, DISPLAY_WHITE );
    DisplayDrawVerticalLine( x +  3, y + 6,  4, DISPLAY_WHITE );
    DisplayDrawVerticalLine( x +  6, y + 4,  6, DISPLAY_WHITE );
    DisplayDrawVerticalLine( x +  9, y + 2,  8, DISPLAY_WHITE );
    DisplayDrawVerticalLine( x + 12, y    , 10, DISPLAY_WHITE );

    if( snr > 0 )
    {
        DisplayDrawVerticalLine( x + 2, y, 5, DISPLAY_WHITE );
        DisplayDrawHorizontalLine( x, y + 2, 5, DISPLAY_WHITE );
    }
    else if( snr < 0 )
    {
        DisplayDrawHorizontalLine( x, y + 2, 5, DISPLAY_WHITE );
    }

    if( rssi < -120 )
    {
        DisplayDrawVerticalLine( x +  1, y + 8,  2, DISPLAY_WHITE );
    }
    else if( rssi < -110 )
    {
        DisplayDrawVerticalLine( x +  1, y + 8,  2, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x +  4, y + 6,  4, DISPLAY_WHITE );
    }
    else if( rssi < -100 )
    {
        DisplayDrawVerticalLine( x +  1, y + 8,  2, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x +  4, y + 6,  4, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x +  7, y + 4,  6, DISPLAY_WHITE );
    }
    else if( rssi < -90 )
    {
        DisplayDrawVerticalLine( x +  1, y + 8,  2, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x +  5, y + 6,  4, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x +  7, y + 4,  6, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x + 10, y + 2,  8, DISPLAY_WHITE );
    }
    else
    {
        DisplayDrawVerticalLine( x +  1, y + 8,  2, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x +  4, y + 6,  4, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x +  7, y + 4,  6, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x + 10, y + 2,  8, DISPLAY_WHITE );
        DisplayDrawVerticalLine( x + 13, y    , 10, DISPLAY_WHITE );
    }
}

/*!
 * \brief Draws the link status icons.
 *        Up arrow for uplink and down arrow for downlink
 *
 * \param x        X coordinate
 * \param y        Y coordinate
 * \param uplink   uplink state
 * \param downlink downlink state
 */
void ScreenDrawLinkStatus( uint8_t x, uint8_t y, bool uplink, bool downlink )
{
    DisplayFillRect( x, y, 6, 9, DISPLAY_BLACK );
    if( uplink == true )
    {
        DisplayFillTriangle( x + 3, y, x, y + 3, x + 6, y + 3, DISPLAY_WHITE );
    }
    if( downlink == true )
    {
        DisplayFillTriangle( x + 3, y + 9, x, y + 6, x + 6, y + 6, DISPLAY_WHITE );
    }
}

/*!
 * \brief Draws GPS icon outer box
 *
 * \param x        X coordinate
 * \param y        Y coordinate
 */
void ScreenDrawGpsOuterBox( uint8_t x, uint8_t y )
{
    DisplayDrawRect( x, y, 9, 9, DISPLAY_WHITE );
    DisplayDrawPixel( x + 4, y, DISPLAY_BLACK );
    DisplayDrawPixel( x + 8, y + 4, DISPLAY_BLACK );
    DisplayDrawPixel( x + 4, y + 8, DISPLAY_BLACK );
    DisplayDrawPixel( x, y + 4, DISPLAY_BLACK );
}

/*!
 * \brief Draws GPS icon inner box
 *
 * \param x        X coordinate
 * \param y        Y coordinate
 */
void ScreenDrawGpsInnerBox( uint8_t x, uint8_t y )
{
    DisplayDrawRect( x + 2, y + 2, 5, 5, DISPLAY_WHITE );
    DisplayDrawPixel( x + 4, y + 2, DISPLAY_BLACK );
    DisplayDrawPixel( x + 6, y + 4, DISPLAY_BLACK );
    DisplayDrawPixel( x + 4, y + 6, DISPLAY_BLACK );
    DisplayDrawPixel( x + 2, y + 4, DISPLAY_BLACK );
}

/*!
 * \brief GPS icon timer callback function.
 *        Updates the GPS icon animation
 */
void OnGpsIconUpdateTimerEvent( void )
{
    DisplayFillRect( GpsIconUpdateX, GpsIconUpdateY, 24, 9, DISPLAY_BLACK );
    switch( GpsIconUpdateState )
    {
    case 2:
        DisplayDrawPixel( GpsIconUpdateX + 4, GpsIconUpdateY + 4, DISPLAY_WHITE );
        // Intentional fallthrough
    case 1:
        ScreenDrawGpsInnerBox( GpsIconUpdateX, GpsIconUpdateY );
        // Intentional fallthrough
    case 0:
        ScreenDrawGpsOuterBox( GpsIconUpdateX, GpsIconUpdateY );
        break;
    }
    GpsIconUpdateState = ( GpsIconUpdateState + 1 ) % 3;
    // Restart timer
    TimerStart( &GpsIconUpdateTimer );
}

/*!
 * \brief Draws static GPS icon if GPS has a fix or starts an animation if no fix
 *
 * \param x        X coordinate
 * \param y        Y coordinate
 * \param hasFix   GPS has fix
 */
void ScreenDrawGps( uint8_t x, uint8_t y, bool hasFix )
{
    GpsIconUpdateX = x;
    GpsIconUpdateY = y;
    if( hasFix == true )
    {
        TimerStop( &GpsIconUpdateTimer );
        GpsIconUpdateTimerStarted = false;

        ScreenDrawGpsOuterBox( GpsIconUpdateX, GpsIconUpdateY );
        ScreenDrawGpsInnerBox( GpsIconUpdateX, GpsIconUpdateY );
        DisplayDrawPixel( GpsIconUpdateX + 4, GpsIconUpdateY + 4, DISPLAY_WHITE );
    }
    else
    {
        if( GpsIconUpdateTimerStarted == false )
        {
            GpsIconUpdateTimerStarted = true;
            GpsIconUpdateState = 0;
            TimerInit( &GpsIconUpdateTimer, OnGpsIconUpdateTimerEvent );
            TimerSetValue( &GpsIconUpdateTimer, 1000 );
            TimerStart( &GpsIconUpdateTimer );
        }
    }
}

/*!
 * \brief Draws an icon if the ADR is ON and nothing if ADR is OFF
 *
 * \param x   X coordinate
 * \param y   Y coordinate
 * \param adr ADR ON/OFF
 */
void ScreenDrawAdr( uint8_t x, uint8_t y, bool adr )
{
    if( adr == true )
    {
        DisplayFillRect( x, y, 24, 9, DISPLAY_WHITE );
        ScreenDrawGlyph( x     , y, AdrGlyph[0], DISPLAY_BLACK );
        ScreenDrawGlyph( x +  7, y, AdrGlyph[1], DISPLAY_BLACK );
        ScreenDrawGlyph( x + 15, y, AdrGlyph[2], DISPLAY_BLACK );
    }
    else
    {
        DisplayFillRect( x, y, 24, 9, DISPLAY_BLACK );
    }
}

/*!
 * \brief Draws a toolbar on top of the screen
 *
 * \param appSettings Current application settings
 * \param uplink      Current uplink status
 * \param downlink    Current downlink status
 */
void ScreenDrawToolBar( AppSettings_t *appSettings, LoRaMacUplinkStatus_t *uplink, LoRaMacDownlinkStatus_t *downlink )
{
    // Draw tool bar elements
    ScreenDrawArrows( 6 );
    ScreenDrawDevClass( 7, 2, appSettings->DeviceClass );

    ScreenDrawLinkStatus( 20, 1, uplink->UplinkInProgress, downlink->DownlinkInProgress );
    ScreenDrawRxSlot( 29, 2, downlink->RxSlot );

    ScreenDrawRssi( 40, 1, downlink->Rssi, downlink->Snr );

    ScreenDrawAdr( 58, 2, appSettings->AdrOn );
    ScreenDrawGps( 86, 2, uplink->GpsHaxFix );
    ScreenDrawBattery( 98, 2, uplink->BatteryLevel );

    DisplayDrawHorizontalLine( 0, 12, 128, DISPLAY_WHITE );
}

void ScreenProcess( AppSettings_t *appSettings, LoRaMacUplinkStatus_t *uplink, LoRaMacDownlinkStatus_t *downlink )
{
    uint32_t devAddr = 0;

    ButtonsProcess( );

    switch( CurrentScreen )
    {
    case SCREEN_BOOT:
        DisplaySetCursor( 0, 31 );
        DisplayFillRect( 0, 31, 128, 8, DISPLAY_BLACK );
        DisplayPrint( "       Mote II       " );
        DisplayUpdate( );
        DelayMs( 3000 );

        CurrentScreen++;
        // Intentional fallthrough
    case SCREEN_JOIN:
        DisplaySetLine( 1 );
        DisplayPrint( " Joining Network " );
        if( JoinProgress == 0 )
        {
            JoinProgress++;
            DisplaySetLine( 2 );
            DisplayPrint( "." );
        }
        else if( JoinProgress == 1 )
        {
            JoinProgress++;
            DisplaySetLine( 2 );
            DisplayPrint( ".." );
        }
        else if( JoinProgress == 2 )
        {
            JoinProgress++;
            DisplaySetLine( 2 );
            DisplayPrint( ".." );
        }
        else if( JoinProgress == 3 )
        {
            JoinProgress = 0;
            DisplaySetLine( 2 );
            DisplayPrint( "...." );
        }
        DisplaySetLine( 3 );
        DisplayPrint( "" );
        DisplaySetLine( 4 );
        DisplayPrint( "" );
        DisplaySetLine( 5 );
        DisplayPrint( "" );
        DisplaySetLine( 6 );
        DisplayPrint( "" );
        ScreenUpdate = true;
        break;
    case SCREEN_1:
        {
            MibRequestConfirm_t mibReq;

            mibReq.Type = MIB_DEV_ADDR;
            LoRaMacMibGetRequestConfirm( &mibReq );
            devAddr = mibReq.Param.DevAddr;
        }
        DisplaySetLine( 2 );
        DisplayPrintf( "DEUI:%02x%02x%02x%02x%02x%02x%02x%02x", appSettings->DevEui[0],
                                                                appSettings->DevEui[1],
                                                                appSettings->DevEui[2],
                                                                appSettings->DevEui[3],
                                                                appSettings->DevEui[4],
                                                                appSettings->DevEui[5],
                                                                appSettings->DevEui[6],
                                                                appSettings->DevEui[7] );

        if( appSettings->Otaa == true )
        {
            DisplaySetLine( 1 );
            DisplayPrintf( "     LoRaWAN OTAA " );
            DisplaySetLine( 3 );
            DisplayPrintf( "" );
            DisplaySetLine( 4 );
            DisplayPrintf( "AEUI:%02x%02x%02x%02x%02x%02x%02x%02x", appSettings->AppEui[0],
                                                                    appSettings->AppEui[1],
                                                                    appSettings->AppEui[2],
                                                                    appSettings->AppEui[3],
                                                                    appSettings->AppEui[4],
                                                                    appSettings->AppEui[5],
                                                                    appSettings->AppEui[6],
                                                                    appSettings->AppEui[7] );
            DisplaySetLine( 5 );
            DisplayPrintf( "" );
        }
        else
        {
            DisplaySetLine( 1 );
            DisplayPrintf( "    LoRaWAN ABP " );
            DisplaySetLine( 3 );
            DisplayPrintf( "" );
            DisplaySetLine( 4 );
            DisplayPrintf( "" );
            DisplaySetLine( 5 );
            DisplayPrintf( "" );
        }
        DisplaySetLine( 6 );
        DisplayPrintf( "DAddr:%02x-%02x-%02x-%02x", ( uint8_t )( ( devAddr >> 24 ) & 0xFF ),
                                                    ( uint8_t )( ( devAddr >> 16 ) & 0xFF ),
                                                    ( uint8_t )( ( devAddr >> 8 ) & 0xFF ),
                                                    ( uint8_t )( ( devAddr ) & 0xFF ) );
        break;
    case SCREEN_2:
        DisplaySetLine( 1 );
        DisplayPrintf( "     Uplink " );
        DisplaySetLine( 2 );
        if( appSettings->IsTxConfirmed == true )
        {
            DisplayPrintf( "Confirmed Tx" );
        }
        else
        {
            DisplayPrintf( "Unconfirmed Tx" );
        }
        DisplaySetLine( 3 );
        DisplayPrintf( "TxPower:      %d dBm", 14 - ( ( uplink->TxPower - 1 ) * 3 ) );
        DisplaySetLine( 4 );
        DisplayPrintf( "Datarate:     DR%d", uplink->Datarate );
        DisplaySetLine( 5 );
        DisplayPrintf( "# Trials:     %d", uplink->NbTrials );
        DisplaySetLine( 6 );
        DisplayPrintf( "Counter:      %d", uplink->UplinkCounter );
        break;
    case SCREEN_3:
        DisplaySetLine( 1 );
        DisplayPrintf( "     Downlink " );
        DisplaySetLine( 2 );
        DisplayPrintf( "Rssi:         %d", downlink->Rssi );
        DisplaySetLine( 3 );
        if( downlink->Snr < 0 )
        {
            DisplayPrintf( "SNR:         %d", downlink->Snr );
        }
        else
        {
            DisplayPrintf( "SNR:          >0 " );
        }
        DisplaySetLine( 4 );
        if( downlink->RxData == true )
        {
            DisplayPrintf( "Received data " );
        }
        else
        {
            DisplayPrintf( "" );
        }
        DisplaySetLine( 5 );
        if( uplink->Acked == true )
        {
            DisplayPrintf( "Ack Received" );
        }
        DisplaySetLine( 6 );
        DisplayPrintf( "Counter:     %d", downlink->DownlinkCounter );
        break;
    case SCREEN_4:
        DisplaySetLine( 1 );
        DisplayPrintf( "    Rx Message " );
        DisplaySetLine( 2 );
        DisplayPrintf( "Counter:     %d", downlink->DownlinkCounter );

        if( downlink->RxData == true )
        {
            DisplaySetLine( 3 );
            DisplayPrintf( "Port:        %d", downlink->Port );
            DisplaySetLine( 4 );
            if( ( downlink->BufferSize != 0 ) && ( downlink->BufferSize < 22 ) )
            {
                DisplayPrintf( "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", downlink->Buffer[0], downlink->Buffer[1],
                                                                             downlink->Buffer[2], downlink->Buffer[3],
                                                                             downlink->Buffer[4], downlink->Buffer[5],
                                                                             downlink->Buffer[6], downlink->Buffer[7],
                                                                             downlink->Buffer[8], downlink->Buffer[9],
                                                                             downlink->Buffer[10], downlink->Buffer[11],
                                                                             downlink->Buffer[12], downlink->Buffer[13],
                                                                             downlink->Buffer[14], downlink->Buffer[15],
                                                                             downlink->Buffer[16], downlink->Buffer[17],
                                                                             downlink->Buffer[18], downlink->Buffer[19],
                                                                             downlink->Buffer[20], downlink->Buffer[21]);
            }
            else
            {
                DisplayPrintf( "" );
            }
            DisplaySetLine( 5 );
            if( downlink->BufferSize >= 22 )
            {
                DisplayPrintf( "..." );
            }
            else
            {
                DisplayPrintf( "" );
            }
        }
        else
        {
            DisplaySetLine( 3 );
            DisplayPrintf( "" );
            DisplaySetLine( 4 );
            DisplayPrintf( "" );
            DisplaySetLine( 5 );
            DisplayPrintf( "" );
        }
        DisplaySetLine( 6 );
        DisplayPrintf( "" );
        break;
    case SCREEN_5:
        DisplaySetLine( 1 );
        DisplayPrintf( "       Sensors " );
        DisplaySetLine( 2 );
        DisplayPrintf( "Temp       %d.%02d deg", uplink->Temperature / 100, uplink->Temperature % 100 );
        DisplaySetLine( 3 );
        DisplayPrintf( "Pressure   %d.%02d hPa", uplink->Pressure / 10, uplink->Pressure % 10 );
        DisplaySetLine( 4 );
        DisplayPrintf( "Alt Bar    %d.%02d m", uplink->AltitudeBar / 10, uplink->AltitudeBar % 10 );
        DisplaySetLine( 5 );
        if( uplink->BatteryLevel == 0x00 )
        {
            DisplayPrintf( "Bat        USB ");
        }
        else
        {
            DisplayPrintf( "Bat        %d", uplink->BatteryLevel );
        }
        DisplaySetLine( 6 );
#if defined( USE_BAT_DBG )
        DisplayPrintf( "Bat        %d [mV]", BoardGetBatteryVoltage( ) );
#else
        DisplayPrintf( "" );
#endif
        break;
    case SCREEN_GPS:
        DisplaySetLine( 1 );
        DisplayPrintf( "         GPS " );
        DisplaySetLine( 2 );
        DisplayPrintf( "" );
        if( uplink->GpsHaxFix == false )
        {
            DisplaySetLine( 3 );
            DisplayPrintf( "Searching..." );
            DisplaySetLine( 4 );
            DisplayPrintf( "" );
            DisplaySetLine( 5 );
            DisplayPrintf( "" );
        }
        else
        {
            double lati = 0.0, longi = 0.0;
            GpsGetLatestGpsPositionDouble( &lati, &longi );

            DisplaySetLine( 3 );
            DisplayPrintf( "Latitude: %f", lati );
            DisplaySetLine( 4 );
            DisplayPrintf( "Longitude: %f", longi );
            DisplaySetLine( 5 );
            DisplayPrintf( "Altitude: %d", GpsGetLatestGpsAltitude( ) );
        }
        DisplaySetLine( 6 );
        DisplayPrintf( "" );
        break;
    case SCREEN_LAST:
        DisplaySetLine( 1 );
        DisplayPrintf( "      Versions" );
        DisplaySetLine( 2 );
        DisplayPrintf( "" );
        DisplaySetLine( 3 );
        DisplayPrintf( "" );
        DisplaySetLine( 4 );
#if( APP_FIRMWARE_VERSION_REVISION > 0 )
        DisplayPrintf( "Firmware: v%d.%d.RC%d", APP_FIRMWARE_VERSION_MAJOR, APP_FIRMWARE_VERSION_MINOR, APP_FIRMWARE_VERSION_REVISION );
#else
        DisplayPrintf( "Firmware:   v%d.%d.%d", APP_FIRMWARE_VERSION_MAJOR, APP_FIRMWARE_VERSION_MINOR, APP_FIRMWARE_VERSION_REVISION );
#endif
        DisplaySetLine( 5 );
        DisplayPrintf( "" );
        DisplaySetLine( 6 );
        DisplayPrintf( "GitHub  :   v%d.%d.%d", APP_GITHUB_VERSION_MAJOR, APP_GITHUB_VERSION_MINOR, APP_GITHUB_VERSION_REVISION );
        break;
    default: // 0xFF do nothing
        break;
    }

    if( ( ScreenUpdate == true ) ||
        ( uplink->StatusUpdated == true ) ||
        ( downlink->StatusUpdated == true ) )
    {
        ScreenDrawToolBar( appSettings, uplink, downlink );
        DisplayUpdate( );
    }
}
