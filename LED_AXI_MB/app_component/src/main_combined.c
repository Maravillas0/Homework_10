#include "xparameters.h"
#include "xgpio.h"
#include "xtmrctr.h"
#include "xil_printf.h"
#include "xstatus.h"

/* AXI peripherals present in the MicroBlaze hardware design. */
#define LED_GPIO_BASEADDR  XPAR_AXI_GPIO_0_BASEADDR /* 4 LEDs */
#define BTN_GPIO_BASEADDR  XPAR_AXI_GPIO_1_BASEADDR /* buttons */
#define SW_GPIO_BASEADDR   XPAR_AXI_GPIO_2_BASEADDR /* switches */
#define TIMER_BASEADDR     XPAR_AXI_TIMER_0_BASEADDR

#define TIMER_CHANNEL      0U

/* Timer clock in this design is 100 MHz. Values are timer clock ticks. */
#ifdef FAST_SIMULATION
#define SPEED_SLOW         25000U
#define SPEED_MEDIUM       12500U
#define SPEED_FAST         5000U
#define BUTTON_DEBOUNCE_ITERATIONS 200U
#else
#define SPEED_SLOW         25000000U
#define SPEED_MEDIUM       12500000U
#define SPEED_FAST         5000000U
#define BUTTON_DEBOUNCE_ITERATIONS 200000U
#endif

static XGpio Gpio_Leds;
static XGpio Gpio_Btns;
static XGpio Gpio_Sws;
static XTmrCtr Timer;

static u8 led_pos = 0U;
static u8 is_running = 1U;
static u32 current_period = SPEED_MEDIUM;

static void AdvanceLed(void)
{
    u32 switch_data = XGpio_DiscreteRead(&Gpio_Sws, 1U);

    if ((switch_data & 0x01U) != 0U) {
        led_pos = (u8)((led_pos + 1U) % 4U);
    } else {
        led_pos = (led_pos == 0U) ? 3U : (u8)(led_pos - 1U);
    }

    XGpio_DiscreteWrite(&Gpio_Leds, 1U, 1U << led_pos);
}

static void SetSpeed(u32 period)
{
    current_period = period;
    XTmrCtr_SetResetValue(&Timer, TIMER_CHANNEL, current_period);
    /* Loading the reset value also clears a previously latched expiry. */
    XTmrCtr_Reset(&Timer, TIMER_CHANNEL);
}

int main(void)
{
    int status;
    u32 previous_buttons = 0U;

    xil_printf("--- MicroBlaze AXI Timer LED Controller ---\r\n");

    /* Under the Vitis SDT build, Initialize takes the peripheral base address. */
    status = XGpio_Initialize(&Gpio_Leds, LED_GPIO_BASEADDR);
    if (status != XST_SUCCESS) return XST_FAILURE;
    XGpio_SetDataDirection(&Gpio_Leds, 1U, 0x0U);

    status = XGpio_Initialize(&Gpio_Btns, BTN_GPIO_BASEADDR);
    if (status != XST_SUCCESS) return XST_FAILURE;
    XGpio_SetDataDirection(&Gpio_Btns, 1U, 0x3U);

    status = XGpio_Initialize(&Gpio_Sws, SW_GPIO_BASEADDR);
    if (status != XST_SUCCESS) return XST_FAILURE;
    XGpio_SetDataDirection(&Gpio_Sws, 1U, 0x3U);

    XGpio_DiscreteWrite(&Gpio_Leds, 1U, 1U << led_pos);

    status = XTmrCtr_Initialize(&Timer, TIMER_BASEADDR);
    if (status != XST_SUCCESS) return XST_FAILURE;

    /* Poll the AXI Timer overflow flag; this design has no interrupt controller. */
    XTmrCtr_SetOptions(&Timer, TIMER_CHANNEL,
                       XTC_AUTO_RELOAD_OPTION | XTC_DOWN_COUNT_OPTION);
    XTmrCtr_SetResetValue(&Timer, TIMER_CHANNEL, current_period);
    XTmrCtr_Start(&Timer, TIMER_CHANNEL);

    while (1) {
        u32 buttons = XGpio_DiscreteRead(&Gpio_Btns, 1U) & 0x3U;
        u8 btn0 = (u8)(buttons & 0x1U);        /* Start / pause */
        u8 btn1 = (u8)((buttons >> 1) & 0x1U); /* Change speed */

        if ((btn0 != 0U) && ((previous_buttons & 0x1U) == 0U)) {
            is_running = (u8)!is_running;
            xil_printf("Motion State: %s\r\n",
                       is_running ? "RUNNING" : "PAUSED");
        }

        if ((btn1 != 0U) && ((previous_buttons & 0x2U) == 0U)) {
            if (current_period == SPEED_SLOW) {
                SetSpeed(SPEED_MEDIUM);
                xil_printf("Speed: MEDIUM\r\n");
            } else if (current_period == SPEED_MEDIUM) {
                SetSpeed(SPEED_FAST);
                xil_printf("Speed: FAST\r\n");
            } else {
                SetSpeed(SPEED_SLOW);
                xil_printf("Speed: SLOW\r\n");
            }
        }
        previous_buttons = buttons;

        if (XTmrCtr_IsExpired(&Timer, TIMER_CHANNEL)) {
            XTmrCtr_Reset(&Timer, TIMER_CHANNEL);
            if (is_running != 0U) {
                AdvanceLed();
            }
        }

        for (volatile u32 i = 0U; i < BUTTON_DEBOUNCE_ITERATIONS; ++i) {
            /* Simple polling debounce delay. */
        }
    }

    /* Unreachable; keeps some toolchains happy about main's return type. */
    return XST_SUCCESS;
}
