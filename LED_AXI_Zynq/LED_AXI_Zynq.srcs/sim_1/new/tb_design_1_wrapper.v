`timescale 1ns/1ps

module tb_design_1_wrapper;
    reg ps_clk = 1'b0;
    reg ps_porb = 1'b0;
    reg ps_srstb = 1'b0;
    reg [1:0] buttons = 2'b00;
    reg [1:0] switches = 2'b00;
    wire [3:0] leds;

    tri fixed_io_ddr_vrn, fixed_io_ddr_vrp;
    tri [53:0] fixed_io_mio;
    tri fixed_io_ps_clk, fixed_io_ps_porb, fixed_io_ps_srstb;

    assign fixed_io_ps_clk = ps_clk;
    assign fixed_io_ps_porb = ps_porb;
    assign fixed_io_ps_srstb = ps_srstb;

    always #15 ps_clk = ~ps_clk; // 33.333 MHz reference clock

    design_1_wrapper dut (
        .DDR_addr(), .DDR_ba(), .DDR_cas_n(), .DDR_ck_n(), .DDR_ck_p(),
        .DDR_cke(), .DDR_cs_n(), .DDR_dm(), .DDR_dq(), .DDR_dqs_n(),
        .DDR_dqs_p(), .DDR_odt(), .DDR_ras_n(), .DDR_reset_n(), .DDR_we_n(),
        .FIXED_IO_ddr_vrn(fixed_io_ddr_vrn),
        .FIXED_IO_ddr_vrp(fixed_io_ddr_vrp),
        .FIXED_IO_mio(fixed_io_mio),
        .FIXED_IO_ps_clk(fixed_io_ps_clk),
        .FIXED_IO_ps_porb(fixed_io_ps_porb),
        .FIXED_IO_ps_srstb(fixed_io_ps_srstb),
        .btn_tri_i(buttons), .led_tri_o(leds), .sw_tri_i(switches)
    );

    initial begin
        $dumpfile("zynq_led_buttons.vcd");
        $dumpvars(0, tb_design_1_wrapper);
        $monitor("%0t ns: BTN=%b SW=%b LED=%b",
                 $time, buttons, switches, leds);

        #1000;
        ps_porb = 1'b1;
        ps_srstb = 1'b1;

        // Automatic movement, forward direction.
        switches[0] = 1'b1;
        #5_000_000;

        // BTN1 changes speed.
        buttons[1] = 1'b1;
        #100_000;
        buttons[1] = 1'b0;
        #3_000_000;

        // BTN0 pauses, then resumes the LED ring.
        buttons[0] = 1'b1;
        #100_000;
        buttons[0] = 1'b0;
        #2_000_000;
        buttons[0] = 1'b1;
        #100_000;
        buttons[0] = 1'b0;
        #3_000_000;

        // SW0 selects reverse direction.
        switches[0] = 1'b0;
        #3_000_000;

        $display("Stimulus finished. Waveform: zynq_led_buttons.vcd");
        $finish;
    end
endmodule
