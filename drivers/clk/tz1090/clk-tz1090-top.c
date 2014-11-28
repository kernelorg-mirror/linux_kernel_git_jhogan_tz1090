/*
 * Copyright (C) 2013-2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * TZ1090 TOP Clocks
 */

#include <dt-bindings/clock/tz1090-top.h>

#include "clk.h"

/* Register offsets into top level memory region */
#define TOP_CLKEN		0x00
#define TOP_CLKSTATUS		0x04
#define TOP_CLKSWITCH		0x08
#define TOP_CLKENAB		0x0c
#define TOP_CLKDELETE		0x10
#define TOP_SYSCLK_DIV		0x14
#define TOP_META_CLKDIV		0x18
#define TOP_META_CLKDELETE	0x1c
#define TOP_AFE_DIV		0x20
#define TOP_ADCPLL_DIV		0x24
#define TOP_UARTCLK_DIV		0x28
#define TOP_PDMCK_CTL		0x30
#define TOP_SPICLK_DIV		0x34
#define TOP_SPI1CLK_DIV		0x38
#define TOP_I2SCLK_DIV		0x3c
#define TOP_USB_PLLDIV		0x40
#define TOP_SDHOSTCLK_DIV	0x44
#define TOP_RING_OP_DIV		0x48
#define TOP_SYSPLL_CTL0		0x50
#define TOP_ADCPLL_CTL0		0x58
#define TOP_META_CLK		0x80
#define TOP_CLKSWITCH2		0x88
#define TOP_CLKENAB2		0x8c
#define TOP_I2S_DIV2		0x90
#define TOP_META_TRACE_CLK_DIV	0x94
#define TOP_PIXEL_CLK_DIV	0x98
#define TOP_CLKOUT0_DIV		0x9c
#define TOP_CLKOUT1_DIV		0xa0
#define TOP_UCC0_CLKDELETE	0xa4
#define TOP_UCC1_CLKDELETE	0xa8
#define TOP_DDR_CLKDIV		0xac

/*
 *                    CR_TOP_CLKSWITCH
 *                    ================
 *                       ___________        _________   _____________
 * xtal1         ------o| sys_sw    \______| sys_pll |_| sys_clk_div |__
 * xtal2         -------|___________/  0   |_________| |_____________|  |
 *                    __________________________________________________|
 *                   |   ___________
 * xtal1         ----|-o|sysclk1_sw \____________________________
 * sys_clk_div       `--|___________/  1 sys_clk_x2_undeleted
 * xtal1         ------o|clkout0_sw0\______
 * afe_progdiv1  -------|___________/  2   |
 * sys_undeleted ------o|clkout0_sw1\____  |
 * if0_sw        -------|___________/  3 | |   CR_TOP_CLKENAB
 *                 ,-;==================='-'   ==============
 *                 | |   ___________            ___________
 * clkout0_sw0     | `-o|clkout0_sw2\____     _| out0_inv  \_____
 * xtal2         --|----|___________/  4 |   | |___________/  4
 *                 |  ___________________|   |__________________
 *                 | |   ___________            ___________     |
 * clkout0_sw2     | `-o|clkout0_sw3\__________| out0_en   \____|
 * clkout0_sw1     `----|___________/  5       |___________/  5
 * xtal1         ------o|clkout1_sw0\______     .
 * xtal2         -------|___________/  6   |    .
 * sys_undeleted ------o|clkout1_sw1\____  |    .
 * if1_sw        -------|___________/  7 | |    .
 *                 ,-;==================='-'    .
 *                 | |   ___________            ___________
 * adcpll_clk    --|-|-o|clkout1_sw2\____     _| out1_inv  \_____
 * clkout1_sw1     `-|--|___________/  8 |   | |___________/  8
 *                  _|___________________|   |__________________
 *                 | |   ___________            ___________     |
 * clkout1_sw0     | `-o|clkout1_sw3\__________| out1_en   \____|
 * clkout1_sw2     `----|___________/  9       |___________/  9
 * xtal1         ------o| i2s_sw2   \______     .
 * sys_undeleted -------|___________/ 10   |    .
 * xtal2         ------o| i2s_sw0   \____  |    .
 * adcpll_clk    -------|___________/ 11 | |    .
 *                 ,-;==================='-'    .
 *                 | |   ___________            ___________
 * i2s_sw2         | `-o| i2s_sw1   \__________| i2s_en    \_____
 * i2s_sw0         `----|___________/ 12       |___________/ 12
 * xtal1         ------o| scb_sw    \__________| scb_en    \_____
 * sys_undeleted -------|___________/ 13       |___________/ 13
 * xtal1         ------o| uart_sw   \__________| uart_en   \_____
 * sys_undeleted -------|___________/ 14       |___________/ 14
 * xtal1         ------o|ext_stc0_sw\__________|ext_stc0_en\_____
 * xtal2         -------|___________/ 16       |___________/ 16
 * xtal1         ------o|ext_stc1_sw\__________|ext_stc1_en\_____
 * xtal2         -------|___________/ 17       |___________/ 17
 * adcpll_clk    ------o|  usb_sw0  \____       .
 * afe_progdiv3  -------|___________/ 18 |      .
 *                    ___________________|      .
 *                   |   ___________            ___________
 * usb_sw3         ,-|-o|  usb_sw1  \__________| usb_en    \_____
 * usb_sw0         | `--|___________/ 19       |___________/ 19
 * xtal1         --|---o|  afe_sw0  \________________
 * afe_sw1       ,-|----|___________/ 20        .    |    _____________
 *               `-`=====================:-.    .    |___| afe_clk_div |________
 *                       ___________     | |    .        |_____________| afe_clk
 * adcpll_en       ,---o|  afe_sw1  \____| |    .
 * xtal2         --|----|___________/ 21   |    .         ________
 * xtal1         --|---o|adcpll_sw0 \______|_____________| adcpll |_____________
 * xtal2         --|----|___________/ 22   |    .        |________| adcpll_clk
 * xtal1         --|---o|adcpll_sw1 \____  |    .
 * xtal2         --|----|___________/ 23 | |    .
 *                 |  ___________________| |    .
 *                 | |   ___________       |    .         ________________
 * adcpll_en       +-|-o|adcpll_sw2 \______|_____________| adcpll_clk_div |_____
 * adcpll_sw1      | `--|___________/ 24   |    .        |________________|
 *                 |_______________________|_____________________
 *                       ___________       |    ___________      |
 * sys_undeleted -------|adcpll_sw3 \______|___| adcpll_en \_____|
 * adcpll_clk    -------|___________/ 25   |   |___________/ 25
 * xtal1         -------|  usb_sw2  \____  |
 * xtal2         -------|___________/ 28 | |
 *                    ___________________| |
 *                   |   ___________       |
 * usb_sw2           `--|  usb_sw3  \______|
 * sys_undeleted -------|___________/ 29
 */
MUX_BANK(tz1090_top_clkswitch, CLK_TOP_CLKSWITCH_BASE, TOP_CLKSWITCH,
	/*  bit in[0]		 in[1]			out	*/
	MUX( 0, "@xtal1",	 "@xtal2",		"sys_sw")
	MUX( 1, "@xtal1",	 "sys_div",		"sys_x2_undeleted")
	MUX( 2, "@xtal1",	 "@afe_progdiv1",	"out0_sw0")
	MUX( 3, "sys_undeleted", "if0_sw",		"out0_sw1")
	MUX( 4, "out0_sw0",	 "@xtal2",		"out0_sw2")
	MUX( 5, "out0_sw2",	 "out0_sw1",		"out0_sw3")
	MUX( 6, "@xtal1",	 "@xtal2",		"out1_sw0")
	MUX( 7, "sys_undeleted", "if1_sw",		"out1_sw1")
	MUX( 8, "adcpll",	 "out1_sw1",		"out1_sw2")
	MUX( 9, "out1_sw0",	 "out1_sw2",		"out1_sw3")
	MUX(10, "@xtal1",	 "sys_undeleted",	"i2s_sw2")
	MUX(11, "@xtal2",	 "adcpll",		"i2s_sw0")
	MUX(12, "i2s_sw2",	 "i2s_sw0",		"i2s_sw1")
	/*
	 * SCB clock must be derived from system clock to workaround clock
	 * domain crossing problems in SCB automatic mode.
	 */
	MUX_FIXED(13, "@xtal1",	 "sys_undeleted",	"scb_sw",	1)
	MUX(14, "@xtal1",	 "sys_undeleted",	"uart_sw")
	/* bit 15 unused */
	MUX(16, "@xtal1",	 "@xtal2",		"ext_stc0_sw")
	MUX(17, "@xtal1",	 "@xtal2",		"ext_stc1_sw")
	MUX(18, "adcpll",	 "@afe_progdiv3",	"usb_sw0")
	MUX(19, "usb_sw3",	 "usb_sw0",		"usb_sw1")
	MUX(20, "@xtal1",	 "afe_sw1",		"afe_sw0")
	MUX(21, "adcpll_en",	 "@xtal2",		"afe_sw1")
	MUX(22, "@xtal1",	 "@xtal2",		"adcpll_sw0")
	MUX(23, "@xtal1",	 "@xtal2",		"adcpll_sw1")
	MUX(24, "adcpll_en",	 "adcpll_sw1",		"adcpll_sw2")
	MUX(25, "sys_undeleted", "adcpll",		"adcpll_sw3")
	/* bits 26..27 unused */
	MUX(28, "@xtal1",	 "@xtal2",		"usb_sw2")
	MUX(29, "usb_sw2",	 "sys_undeleted",	"usb_sw3")
	/* bits 30..31 unused */
);

GATE_BANK(tz1090_top_clkenab, CLK_TOP_CLKENAB_BASE, TOP_CLKENAB,
	/*   bit in			out	*/
	/* bits 0..4 unused */
	GATE( 5, "out0_sw3",		"out0_en")
	/* bits 6..8 unused */
	GATE( 9, "out1_sw3",		"out1_en")
	/* bits 10..11 unused */
	GATE(12, "i2s_sw1",		"i2s_en")
	GATE(13, "scb_sw",		"scb")
	GATE(14, "uart_sw",		"uart_en")
	/* bit 15 unused */
	GATE(16, "ext_stc0_sw",		"ext_stc0")
	GATE(17, "ext_stc1_sw",		"ext_stc1")
	/* bit 18 unused */
	GATE(19, "usb_sw1",		"usb_en")
	/* bits 20..24 unused */
	GATE(25, "adcpll_sw3",		"adcpll_en")
	/* bits 26..31 unused */
);

/*
 *                    CR_TOP_CLKSWITCH2
 *                    =================
 *                       ___________
 * xtal1         ------o| pixel_sw0 \______
 * pixel_sw3       ,----|___________/  0   |
 * sys_undeleted --+---o| pixel_sw1 \____  |
 * pixel_sw4       | ,--|___________/  1 | |
 *                 `-`===================|=|=:-.  CR_TOP_CLKENAB2
 *                 ,-;==================='-' | |  ===============
 *                 | |   ___________         | |    __________
 * pixel_sw0       | `-o| pixel_sw2 \________|_|___| pixel_en \_____
 * pixel_sw1       `----|___________/  2     | |   |__________/  2
 * adcpll_clk    ------o| pixel_sw3 \________| |    .
 * afe_progdiv3  -------|___________/  3       |    .
 * usb_phy_clk   ------o| pixel_sw4 \__________|    .
 * xtal2         -------|___________/  4            __________
 * iqadc_sync    ------o| if1_sw    \______________| if1_en   \_____
 * ext_adc_dac   --+----|___________/  5           |__________/  5
 * afe_rxsync    --|---o| if0_sw    \______________| if0_en   \_____
 * ext_adc_dac     |`---|___________/  6      _____|__________/  6
 *                 |.________________________| ext_adc_dac_en \_____
 *                 |     ___________         |________________/  7
 * afe_txsync    --|---o| dac0_sw   \______________| dac0_en  \_____
 * ext_adc_dac     `----|___________/  8           |__________/  8
 * ucc1_clk_del  ------o| ucc1_sw   \______________| ucc1_en  \_____
 * ucc0_clk_del  --+----|___________/  9           |__________/  9
 * ucc0_clk_del    `---o| ucc0_sw   \______________| ucc0_en  \_____
 * sys_clk       -------|___________/ 10           |__________/ 10
 */
MUX_BANK(tz1090_top_clkswitch2, CLK_TOP_CLKSWITCH2_BASE, TOP_CLKSWITCH2,
	/*  bit in[0]			in[1]			out	*/
	MUX( 0, "@xtal1",		"pixel_sw3",		"pixel_sw0")
	MUX( 1, "sys_undeleted",	"pixel_sw4",		"pixel_sw1")
	MUX( 2, "pixel_sw0",		"pixel_sw1",		"pixel_sw2")
	MUX( 3, "adcpll",		"@afe_progdiv3",	"pixel_sw3")
	MUX( 4, "usb_phy",		"@xtal2",		"pixel_sw4")
	MUX( 5, "@iqadc_sync",		"@ext_adc_dac",		"if1_sw")
	MUX( 6, "@afe_rxsync",		"@ext_adc_dac",		"if0_sw")
	/* bit 7 unused */
	MUX( 8, "@afe_txsync",		"@ext_adc_dac",		"dac0_sw")
	MUX( 9, "ucc1_del",		"ucc0",			"ucc1_sw")
	MUX(10, "ucc0",			"sys",			"ucc0_sw")
	/* bits 11..31 unused */
);

GATE_BANK(tz1090_top_clkenab2, CLK_TOP_CLKENAB2_BASE, TOP_CLKENAB2,
	/*   bit in			out	*/
	/* bits 0..1 unused */
	GATE( 2, "pixel_sw2",		"pixel_en")
	/* bits 3..4 unused */
	GATE( 5, "if1_sw",		"if1")
	GATE( 6, "if0_sw",		"if0")
	GATE( 7, "@ext_adc_dac",	"ext_adc_dac_en")
	GATE( 8, "dac0_sw",		"dac0")
	GATE( 9, "ucc1_sw",		"sys_ucc1")
	GATE(10, "ucc0_sw",		"sys_mtx")
	/* bits 11..31 unused */
);

/*
 *                       Deleters
 *                       ========
 *
 * sys_undeleted    ---[ clkdelete      ]--- sys_clk
 * sys_x2_undeleted ---[ meta_clkdelete ]--- meta_core_clk
 * sys_undeleted    ---[ clkdelete      ]--- ucc0_clk_del
 * sys_undeleted    ---[ clkdelete      ]--- ucc1_clk_del
 */

static const struct tz1090_clk_deleter tz1090_top_deleters[] __initconst = {
	DEL(CLK_TOP_SYS,      "sys_undeleted",    "sys",      TOP_CLKDELETE),
	DEL(CLK_TOP_META,     "sys_x2_undeleted", "meta",     TOP_META_CLKDELETE),
	DEL(CLK_TOP_UCC0,     "sys_undeleted",    "ucc0",     TOP_UCC0_CLKDELETE),
	DEL(CLK_TOP_UCC1_DEL, "sys_undeleted",    "ucc1_del", TOP_UCC1_CLKDELETE),
};

/*
 *                       Dividers
 *                       ========
 *
 * sys_pll          ---[ sys_clk_div        ]--- sys_div
 * sys_x2_undeleted ---[ meta_clk_div       ]--- sys_undeleted
 * afe_sw0          ---[ afe_clk_div        ]--- afe_clk
 * adcpll_sw2       ---[ adcpll_clk_div     ]--- adcpll_div
 * uart_en          ---[ uart_clk_div       ]--- uart_clk
 * sys_undeleted    ---[ pdm_clk_div        ]--- pdm_clk
 * sys_undeleted    ---[ spi0_clk_div       ]--- spi0_clk
 * sys_undeleted    ---[ spi1_clk_div       ]--- spi1_clk
 * i2s_en           ---[ i2sm_clk_div       ]--- i2sm
 * usb_en           ---[ usbpll_clk_div     ]--- usb_phy_clk
 * sys_undeleted    ---[ sdhost_clk_div     ]--- sdhost_clk
 * sys_undeleted    ---[ ring_osc_clk_div   ]--- ring_osc_clk
 * i2sm_clk         ---[ i2s_clk_div2       ]--- i2s_clk
 * sys_undeleted    ---[ meta_trace_clk_div ]--- meta_trace_clk
 * pixel_en         ---[ pixel_clk_div      ]--- pixel_clk
 * clkout0_en       ---[ clkout0_clk_div    ]--- clkout0
 * clkout1_en       ---[ clkout1_clk_div    ]--- clkout1
 * ddr_en           ---[ ddr_clk_div        ]--- ddr_clk
 */

static const struct tz1090_clk_divider tz1090_top_dividers[] __initconst = {
	DIV(CLK_TOP_SYS_DIV,    "sys_pll",       "sys_div",    TOP_SYSCLK_DIV,         8),
	/*
	 * CLK_DIVIDER_READ_ONLY: sys_undeleted is set up by the bootloader
	 * along with sys_pll, and has a whole bunch of derivative peripheral
	 * clocks. It would be really bad for it to change on the fly.
	 */
	DIV_FLAGS(CLK_TOP_SYS_UNDELETED, "sys_x2_undeleted", "sys_undeleted", TOP_META_CLKDIV, 2,
		  0, CLK_DIVIDER_READ_ONLY),
	DIV(CLK_TOP_AFE,        "afe_sw0",       "afe",        TOP_AFE_DIV,            8),
	DIV(CLK_TOP_ADCPLL_DIV, "adcpll_sw2",    "adcpll_div", TOP_ADCPLL_DIV,         8),
	/*
	 * CLK_SET_RATE_PARENT: UART clock changes must propagate up to uart_sw,
	 * which muxes between XTAL1 and sys_undeleted, in order to get enough
	 * precision.
	 */
	DIV_FLAGS(CLK_TOP_UART, "uart_en",       "uart",       TOP_UARTCLK_DIV,        8,
		  CLK_SET_RATE_PARENT, CLK_DIVIDER_ROUND_CLOSEST),
	DIV(CLK_TOP_PDM,        "sys_undeleted", "pdm",        TOP_PDMCK_CTL,          3),
	DIV(CLK_TOP_SPI0,       "sys_undeleted", "spi0",       TOP_SPICLK_DIV,         8),
	DIV(CLK_TOP_SPI1,       "sys_undeleted", "spi1",       TOP_SPI1CLK_DIV,        8),
	DIV(CLK_TOP_I2SM,       "i2s_en",        "i2sm",       TOP_I2SCLK_DIV,         8),
	DIV(CLK_TOP_USB_PHY,    "usb_en",        "usb_phy",    TOP_USB_PLLDIV,         8),
	DIV(CLK_TOP_SDHOST,     "sys_undeleted", "sdhost",     TOP_SDHOSTCLK_DIV,      8),
	DIV(CLK_TOP_RING_OSC,   "sys_undeleted", "ring_osc",   TOP_RING_OP_DIV,        4),
	DIV(CLK_TOP_I2S,        "i2sm",          "i2s",        TOP_I2S_DIV2,           8),
	DIV(CLK_TOP_META_TRACE, "sys_undeleted", "meta_trace", TOP_META_TRACE_CLK_DIV, 8),
	DIV(CLK_TOP_PIXEL,      "pixel_en",      "pixel",      TOP_PIXEL_CLK_DIV,      8),
	DIV(CLK_TOP_OUT0,       "out0_en",       "out0",       TOP_CLKOUT0_DIV,        8),
	DIV(CLK_TOP_OUT1,       "out1_en",       "out1",       TOP_CLKOUT1_DIV,        8),
	DIV(CLK_TOP_DDR,        "@ddr_en",       "ddr",        TOP_DDR_CLKDIV,         8),
};

/*
 *                 PLLs
 *                 ====
 *
 * sys_sw     ---[ sys_pll ]---
 * adcpll_sw0 ---[ adcpll  ]---
 */

static const struct tz1090_clk_pll tz1090_top_plls[] __initconst = {
	PLL(CLK_TOP_SYSPLL, "sys_sw",     "sys_pll", TOP_SYSPLL_CTL0),
	PLL(CLK_TOP_ADCPLL, "adcpll_sw0", "adcpll",  TOP_ADCPLL_CTL0),
};

/*
 *          CR_TOP_CLKEN
 *          ============
 *
 * sys ----[ pdc_sys_en ]--- 0 sys_pdc
 */
GATE_BANK(tz1090_top_clken, CLK_TOP_CLKEN_BASE, TOP_CLKEN,
	/*   bit in	out	*/
	GATE( 0, "sys",	"sys_pdc")
	/* bits 1..31 unused */
);

static void __init tz1090_top_cr_init(struct device_node *np)
{
	struct tz1090_clk_provider *p;

	p = tz1090_clk_alloc_provider(np, CLK_TOP_MAX);
	if (!p)
		return;

	tz1090_clk_register_mux_bank(p, &tz1090_top_clkswitch);
	tz1090_clk_register_mux_bank(p, &tz1090_top_clkswitch2);
	tz1090_clk_register_gate_bank(p, &tz1090_top_clkenab);
	tz1090_clk_register_gate_bank(p, &tz1090_top_clkenab2);
	tz1090_clk_register_gate_bank(p, &tz1090_top_clken);
	tz1090_clk_register_deleters(p, tz1090_top_deleters,
				     ARRAY_SIZE(tz1090_top_deleters));
	tz1090_clk_register_dividers(p, tz1090_top_dividers,
				     ARRAY_SIZE(tz1090_top_dividers));
	tz1090_clk_register_plls(p, tz1090_top_plls,
				 ARRAY_SIZE(tz1090_top_plls));

	tz1090_clk_register_provider(p);
}
CLK_OF_DECLARE(tz1090_top_cr, "img,tz1090-top-cr", tz1090_top_cr_init);
