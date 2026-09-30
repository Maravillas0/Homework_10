#include "xparameters.h"
#include "xgpio.h"
#include "xtmrctr.h"
#include "xstatus.h"
#include "xil_printf.h"

/* BTN0: pause/resume; BTN1: speed; SW0: direction (1 forward, 0 reverse). */
#define LED_GPIO_BASEADDR  XPAR_AXI_GPIO_0_BASEADDR
#define BTN_GPIO_BASEADDR  XPAR_AXI_GPIO_1_BASEADDR
#define SW_GPIO_BASEADDR   XPAR_AXI_GPIO_2_BASEADDR
#define TIMER_BASEADDR     XPAR_AXI_TIMER_0_BASEADDR
#define TIMER_CHANNEL      0U

#ifdef FAST_SIMULATION
#define PERIOD_SLOW        25000U
#define PERIOD_MEDIUM      12500U
#define PERIOD_FAST        5000U
#else
/* The generated platform reports a 50 MHz AXI Timer clock. */
#define PERIOD_SLOW        25000000U
#define PERIOD_MEDIUM      12500000U
#define PERIOD_FAST        5000000U
#endif

#define BUTTON_PAUSE       0x01U
#define BUTTON_SPEED       0x02U
#define USED_BUTTONS       (BUTTON_PAUSE | BUTTON_SPEED)

static XGpio GpioLeds;
static XGpio GpioButtons;
static XGpio GpioSwitches;
static XTmrCtr Timer;

static const u32 periods[3] = {
    PERIOD_SLOW, PERIOD_MEDIUM, PERIOD_FAST
};
static const char * const speed_names[3] = {
    "slow", "medium", "fast"
};

static u8 led_position = 0U;
static u8 speed_index = 0U;
static u8 running = 1U;

static void WriteLeds(void)
{
    XGpio_DiscreteWrite(&GpioLeds, 1U, (u32)1U << led_position);
}

static void RestartTimer(void)
{
    XTmrCtr_Stop(&Timer, TIMER_CHANNEL);
    XTmrCtr_SetResetValue(&Timer, TIMER_CHANNEL, periods[speed_index] - 1U);
    XTmrCtr_Reset(&Timer, TIMER_CHANNEL);
    XTmrCtr_Start(&Timer, TIMER_CHANNEL);
}

static void AdvanceLed(void)
{
    u32 direction = XGpio_DiscreteRead(&GpioSwitches, 1U) & 1U;

    if (direction != 0U) {
        led_position = (u8)((led_position + 1U) & 0x03U);
    } else {
        led_position = (led_position == 0U) ? 3U : (u8)(led_position - 1U);
    }
    WriteLeds();
}

int main(void)
{
    int status;
    u32 previous_buttons = 0U;

    status = XGpio_Initialize(&GpioLeds, LED_GPIO_BASEADDR);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: LED GPIO init failed\r\n");
        return XST_FAILURE;
    }
    status = XGpio_Initialize(&GpioButtons, BTN_GPIO_BASEADDR);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: button GPIO init failed\r\n");
        return XST_FAILURE;
    }
    status = XGpio_Initialize(&GpioSwitches, SW_GPIO_BASEADDR);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: switch GPIO init failed\r\n");
        return XST_FAILURE;
    }
    status = XTmrCtr_Initialize(&Timer, TIMER_BASEADDR);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: AXI Timer init failed\r\n");
        return XST_FAILURE;
    }

    XGpio_SetDataDirection(&GpioLeds, 1U, 0x00000000U);
    XGpio_SetDataDirection(&GpioButtons, 1U, 0xFFFFFFFFU);
    XGpio_SetDataDirection(&GpioSwitches, 1U, 0xFFFFFFFFU);

    /* LED speed comes from the hardware AXI Timer expiration flag. */
    XTmrCtr_SetOptions(&Timer, TIMER_CHANNEL,
                       XTC_AUTO_RELOAD_OPTION |
                       XTC_DOWN_COUNT_OPTION |
                       XTC_INT_MODE_OPTION);

    led_position = 0U;
    running = 1U;
    WriteLeds();
    RestartTimer();

    xil_printf("LED ring started: BTN0 pause, BTN1 speed, SW0 direction\r\n");
    xil_printf("Speed: %s\r\n", speed_names[speed_index]);

    for (;;) {
        u32 buttons = XGpio_DiscreteRead(&GpioButtons, 1U) & USED_BUTTONS;
        u32 pressed = buttons & ~previous_buttons;

        if ((pressed & BUTTON_PAUSE) != 0U) {
            running = (u8)!running;
            if (running != 0U) {
                RestartTimer();
                xil_printf("LED ring resumed\r\n");
            } else {
                XTmrCtr_Stop(&Timer, TIMER_CHANNEL);
                xil_printf("LED ring paused\r\n");
            }
        }

        if ((pressed & BUTTON_SPEED) != 0U) {
            speed_index = (u8)((speed_index + 1U) % 3U);
            xil_printf("Speed: %s\r\n", speed_names[speed_index]);
            if (running != 0U) {
                RestartTimer();
            }
        }

        previous_buttons = buttons;

        if ((running != 0U) && XTmrCtr_IsExpired(&Timer, TIMER_CHANNEL)) {
            /* Clear the sticky expiration flag and begin the next period. */
            XTmrCtr_Reset(&Timer, TIMER_CHANNEL);
            AdvanceLed();
        }
    }
}
