//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include "portfunc.h"
#include "clock.h"

struct color {
    unsigned int red;
    unsigned int green;
    unsigned int blue;
};

struct time {
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;
};

struct color colors[] =
{
                     {1, 1, 1},         // Off
                     {255, 1, 2},       // Red
                     {1, 255, 2},       // Green
                     {200, 201, 202},   // White
                     {1, 2, 255},       // Blue
                     {254, 255, 1},     // Yellow
                     {1, 144, 35},      // Ocean
                     {254, 1, 255},     // Violet
                     {1, 254, 255},     // Cyan
                     {156, 20, 30},     // Pink
                     {156, 25, 1},      // Orange
                     {64, 5, 75},       // Purple
};

struct time currentTime = {12, 0, 0};


void InitializeLEDs(void)
{
    P1DIR |= BIT0;
    SelectPortFunction(1,0,0,0);
    P1OUT |= BIT1;

    P2DIR |= (BIT0|BIT1|BIT2);
    SelectPortFunction(2,0,0,0);
    SelectPortFunction(2,1,0,0);
    SelectPortFunction(2,2,0,0);
}

void InitializePushButtons(void)
{
    P1DIR &= ~(BIT1|BIT4);
    P1REN |= (BIT1|BIT4);

    SelectPortFunction(1,1,0,0);
    SelectPortFunction(1,4,0,0);
}

void ConfigureTimerMode0(void)
{
    TA0CTL = 0x0100; // Disable/Establish the Timer

    TA0CCTL0 = 0x2010;
    TA0CCR0 = 256;

    TA0CCTL1 = 0x2010;
    TA0CCTL2 = 0x2010;
    TA0CCTL3 = 0x2010;

    TA0CTL = 0x0156; // Configure/Reset the Timer. Currently set to 64kHz
}

void setColor(unsigned int i)
{
    struct color c = colors[i % 12];
    TA0CCR1 = c.red;
    TA0CCR2 = c.green;
    TA0CCR3 = c.blue;
}

void PortOneInterrupt(void)
{
    unsigned short iflag = P1IV;

    P1OUT ^= BIT0;
}

void updateTime(void)
{
    static int cycleCounter = 0;

    ++cycleCounter;
    if (cycleCounter >= 1000)
    {
        currentTime.seconds += 1;
        cycleCounter = 0;
        //setColor(currentTime.seconds);
    }

    if (currentTime.seconds >= 60)
    {
        currentTime.minutes++;
        currentTime.seconds = 0;
    }

    if (currentTime.minutes >= 60)
    {
        currentTime.hours++;
        currentTime.minutes = 0;
    }

    if (currentTime.hours > 12)
    {
        currentTime.hours = 1;
    }
}

void TimerA0Interrupt(void)
{
    unsigned int intv = TA0IV;

    updateTime();

    switch (intv)
    {
    case 0x0E: // Overflow
        P2OUT |= (BIT2|BIT1|BIT0);
        break;
    case 0x02: // Red
        P2OUT ^= BIT0;
        break;
    case 0x04: // Green
        P2OUT ^= BIT1;
        break;
    case 0x06: // Blue
        P2OUT ^= BIT2;
        break;
    }
}

void main(void)
{

    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    InitializePushButtons();
    InitializeLEDs();
    SetClockFrequency();
    ConfigureTimerMode0();
    setColor(7);

    // Interrupt Stuff
    P1IE = (BIT1|BIT4);
    P1IES = (BIT1|BIT4);
    NVIC_EnableIRQ(PORT1_IRQn);
    NVIC_EnableIRQ(TA0_N_IRQn);

    for(;;);
}
