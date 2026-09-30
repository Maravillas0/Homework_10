`timescale 1ns / 1ps

module tb_design_1_wrapper;
    reg         clk_p = 1'b0;
    reg         reset_n = 1'b0;
    reg  [2:0]  buttons = 3'b000;
    reg  [1:0]  switches = 2'b00;
    wire [3:0]  leds;

    // The design uses a 100 MHz differential input clock.
    always #5 clk_p = ~clk_p;

    design_1_wrapper dut (
        .btn_tri_i(buttons),
        .diff_clock_rtl_0_clk_p(clk_p),
        .diff_clock_rtl_0_clk_n(~clk_p),
        .led_tri_o(leds),
        .reset_rtl_0(reset_n),
        .sw_tri_i(switches)
    );

    // Hold a button long enough to pass the firmware's polling/debounce loop,
    // then release it so the application's rising-edge detection can re-arm.
    task press_button;
        input integer button_index;
        begin
            buttons[button_index] = 1'b1;
            $display("%0t: press BTN%0d", $time, button_index);
            #50_000; // 50 us
            buttons[button_index] = 1'b0;
            $display("%0t: release BTN%0d", $time, button_index);
            #50_000; // 50 us between presses
        end
    endtask

    always @(leds) begin
        $display("%0t: LEDs = %b", $time, leds);
    end

    initial begin
        // reset_rtl_0 is active-low in the block design.
        #200 reset_n = 1'b1;
        $display("%0t: reset released", $time);
        #50_000; // Let MicroBlaze boot and begin the LED sequence.

        // Leave all buttons untouched first: LEDs must circulate automatically.
        switches[0] = 1'b1; // Select forward direction.
        #500_000;

        // BTN1 changes speed while the sequence keeps running.
        press_button(1);
        #250_000;

        // BTN0 pauses the sequence. It must resume from the same LED position.
        press_button(0);
        #200_000;
        press_button(1); // Change speed while paused; LEDs should stay fixed.
        press_button(0); // Resume.
        #250_000;

        // SW0 selects reverse direction without stopping the sequence.
        switches[0] = 1'b0;
        #250_000;

        $display("%0t: simulation finished, LEDs = %b", $time, leds);
        $finish;
    end
endmodule
