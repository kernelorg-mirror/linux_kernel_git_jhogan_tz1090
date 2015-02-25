/*
 * Copyright (C) 2013-2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * TZ1090 PDC Clocks.
 */

#include <dt-bindings/clock/tz1090-pdc.h>

#include "clk.h"

/* Register offsets into PDC SoC region */
#define PDC_SOC0		0x00

/*
 *                  SOC_GPIO_CONTROL 0
 *                  ==================
 *           ___________
 * xtal1 ___| xtal1_div |____________________________
 *          |___________|  |    ________   xtal1_div
 *                         `--o| rtc_sw \____________
 * xtal3 ----------------------|________/  32khz
 */

static const struct tz1090_clk_divider tz1090_pdc_dividers[] __initconst = {
	/*         id                  in        out         reg  width shift */
	DIV_SHARED(CLK_PDC_XTAL1_DIV, "@xtal1", "xtal1_div", PDC_SOC0, 11, 16),
};

MUX_BANK(tz1090_pdc_mux, CLK_PDC_32KHZ, PDC_SOC0,
	/*  bit in[0]		 in[1]		out	*/
	MUX(30, "xtal1_div",	 "@xtal3",	"32khz")
);

static void __init tz1090_pdc_cr_init(struct device_node *np)
{
	struct tz1090_clk_provider *p;

	p = tz1090_clk_alloc_provider(np, CLK_PDC_MAX);
	if (!p)
		return;

	tz1090_clk_register_dividers(p, tz1090_pdc_dividers,
				     ARRAY_SIZE(tz1090_pdc_dividers));
	tz1090_clk_register_mux_bank(p, &tz1090_pdc_mux);

	tz1090_clk_register_provider(p);
}
CLK_OF_DECLARE(tz1090_pdc_cr, "img,tz1090-pdc-cr", tz1090_pdc_cr_init);
