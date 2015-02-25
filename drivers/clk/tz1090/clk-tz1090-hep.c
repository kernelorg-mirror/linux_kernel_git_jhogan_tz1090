/*
 * Copyright (C) 2013-2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * TZ1090 High End Peripheral (HEP) Clocks
 */

#include <dt-bindings/clock/tz1090-hep.h>

#include "clk.h"

/* Register offsets into high end peripheral memory region */
#define HEP_CLK_EN		0x04

/*
 *                      CR_HEP_CLK_EN
 *                      =============
 *
 * sys              -+--[ 2d_en      ]--- 0 sys_2d
 * sys_x2_undeleted -|--[ ddr_en     ]--- 1 ddr_en
 * sys               `--[ pdp_pdi_en ]--- 2 sys_pdp
 */
GATE_BANK(tz1090_hep_clken, 0, HEP_CLK_EN,
	/*   bit in			out	*/
	GATE( 0, "@sys",		"sys_2d")
	GATE( 1, "@sys_x2_undeleted",	"ddr_en")
	GATE( 2, "@sys",		"sys_pdp")
	/* bits 3..31 unused */
);

static void __init tz1090_hep_cr_init(struct device_node *np)
{
	struct tz1090_clk_provider *p;

	p = tz1090_clk_alloc_provider(np, CLK_HEP_MAX);
	if (!p)
		return;

	tz1090_clk_register_gate_bank(p, &tz1090_hep_clken);

	tz1090_clk_register_provider(p);
}
CLK_OF_DECLARE(tz1090_hep_cr, "img,tz1090-hep-cr", tz1090_hep_cr_init);
