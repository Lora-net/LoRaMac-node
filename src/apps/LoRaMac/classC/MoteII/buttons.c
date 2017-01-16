/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Buttons management.

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "buttons.h"

/*!
 * \brief Structure containing a button context
 */
typedef struct Button_s
{
    bool IsPressed;
    bool IsPressedPrev;
    uint8_t Value;
    uint8_t ZerosCounter;
    uint8_t OnesCounter;
    bool IsDebounceRunning;
}Button_t;

/*!
 * \brief Array containing the buttons context
 */
Button_t Buttons[] =
{
    { false, false, 0, 0, 0, false },
    { false, false, 0, 0, 0, false },
    { false, false, 0, 0, 0, false }
};

/*!
 * GPIO objects
 */
Gpio_t Switch1;
Gpio_t Switch2;
Gpio_t Switch3;

/*!
 * Timer used to perform the debounce
 */
static TimerEvent_t DebounceTimer;

/*!
 * \brief Callback for the DebounceTimer
 */
void OnDebounceTimerEvent( void );

/*!
 * \brief Callback when Switch 1 is pressed
 */
void OnButton1Irq( void );

/*!
 * \brief Callback when Switch 2 is pressed
 */
void OnButton2Irq( void );

/*!
 * \brief Callback when Switch 3 is pressed
 */
void OnButton3Irq( void );

/*!
 * \brief Buttons debounce handling
 *
 * \param id id Button ID
 * \param value Button GPIO current value
 */
static void ButtonsDebounce( ButtonsId_t id, uint8_t value )
{
    Buttons[id].Value = value;
    if( Buttons[id].Value == 0 )
    {
        Buttons[id].ZerosCounter++;
        Buttons[id].OnesCounter = 0;
        if( Buttons[id].ZerosCounter >= 10 )
        {
            Buttons[id].ZerosCounter = 10;
            Buttons[id].IsPressed = true;
        }
    }
    else
    {
        Buttons[id].ZerosCounter = 0;
        Buttons[id].OnesCounter++;
        if( Buttons[id].OnesCounter >= 10 )
        {
            Buttons[id].OnesCounter = 10;
            Buttons[id].IsPressed = false;
            Buttons[id].IsDebounceRunning = false;
        }
   }
}

void ButtonsInit( void )
{
    // IRQ Push Buttons (active low)
    GpioInit( &Switch1, NSWITCH_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioSetInterrupt( &Switch1, IRQ_FALLING_EDGE, IRQ_MEDIUM_PRIORITY, ( GpioIrqHandler * )OnButton1Irq );

    GpioInit( &Switch2, NSWITCH_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioSetInterrupt( &Switch2, IRQ_FALLING_EDGE, IRQ_MEDIUM_PRIORITY, ( GpioIrqHandler * )OnButton2Irq );

    GpioInit( &Switch3, NSWITCH_3, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioSetInterrupt( &Switch3, IRQ_FALLING_EDGE, IRQ_MEDIUM_PRIORITY, ( GpioIrqHandler * )OnButton3Irq );

    TimerInit( &DebounceTimer, OnDebounceTimerEvent );
    TimerSetValue( &DebounceTimer, 10 );
}

bool ButtonGetState( ButtonsId_t id )
{
    bool state = false;

    if( ( Buttons[id].IsPressed == true  ) && ( Buttons[id].IsPressedPrev == false ) )
    {
        state = true;
    }

    Buttons[id].IsPressedPrev = Buttons[id].IsPressed;

    return state;
}

void OnDebounceTimerEvent( void )
{
    TimerStart( &DebounceTimer );

    ButtonsDebounce( BUTTON_LEFT, GpioRead( &Switch1 ) );
    ButtonsDebounce( BUTTON_MIDDLE, GpioRead( &Switch2 ) );
    ButtonsDebounce( BUTTON_RIGHT, GpioRead( &Switch3 ) );

    if( ( Buttons[BUTTON_LEFT].IsDebounceRunning == false ) &&
        ( Buttons[BUTTON_MIDDLE].IsDebounceRunning == false ) &&
        ( Buttons[BUTTON_RIGHT].IsDebounceRunning == false ) )
    {
        TimerStop( &DebounceTimer );
    }
}

void OnButton1Irq( void )
{
    Buttons[BUTTON_LEFT].IsDebounceRunning = true;
    TimerStart( &DebounceTimer );
}

void OnButton2Irq( void )
{
    Buttons[BUTTON_MIDDLE].IsDebounceRunning = true;
    TimerStart( &DebounceTimer );
}

void OnButton3Irq( void )
{
    Buttons[BUTTON_RIGHT].IsDebounceRunning = true;
    TimerStart( &DebounceTimer );
}
