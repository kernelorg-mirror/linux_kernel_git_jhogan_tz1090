/*
 * Copyright (C) 2013-2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * TZ1090 Peripheral Clocks
 */

#include <dt-bindings/clock/tz1090-perip.h>

#include "clk.h"

/* Register offsets into peripheral memory region */
#define PERIP_SRST		0x00
#define PERIP_CLKEN		0x10
#define PERIP_CLKSTATUS		0x14

/*
 *            CR_PERIP_CLKEN
 *            ==============
 *
 * sys ---[CR_PERIP_*_SYS_CLK_EN]--- sys_*
 */
GATE_BANK(tz1090_perip_clken, 0, PERIP_CLKEN,
	/*   bit in      out */
	GATE( 0, "@sys", "sys_scb0")
	GATE( 1, "@sys", "sys_scb1")
	GATE( 2, "@sys", "sys_scb2")
	GATE( 3, "@sys", "sys_sdio")
	GATE( 4, "@sys", "sys_uart0")
	GATE( 5, "@sys", "sys_uart1")
	GATE( 6, "@sys", "sys_spim")
	GATE( 7, "@sys", "sys_spis")
	GATE( 8, "@sys", "sys_spim1")
	GATE( 9, "@sys", "sys_i2sout")
	GATE(10, "@sys", "sys_i2sin")
	GATE(11, "@sys", "sys_lcd")
	GATE(12, "@sys", "sys_sdhost")
	GATE(13, "@sys", "sys_usb")
	/* bits 14..31 unused */
);

static void __init tz1090_perip_cr_init(struct device_node *np)
{
	struct tz1090_clk_provider *p;

	p = tz1090_clk_alloc_provider(np, CLK_PERIP_MAX);
	if (!p)
		return;

	tz1090_clk_register_gate_bank(p, &tz1090_perip_clken);

	tz1090_clk_register_provider(p);
}
CLK_OF_DECLARE(tz1090_perip_cr, "img,tz1090-perip-cr", tz1090_perip_cr_init);
