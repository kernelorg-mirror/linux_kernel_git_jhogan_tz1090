/*
 * Copyright (C) 2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * TZ1090 Clocks
 */

#ifndef CLK_TZ1090_CLK_H
#define CLK_TZ1090_CLK_H

#include <linux/clk-provider.h>
#include <linux/init.h>

/* Generic TZ1090 clock provider */

/**
 * struct tz1090_clk_provider - Clock provider data.
 * @node:	Device tree node for the clock provider.
 * @base:	IO remapped base address.
 * @clk_data:	Standard onecell clock data including list of clocks.
 */
struct tz1090_clk_provider {
	struct device_node		*node;
	void __iomem			*base;
	struct clk_onecell_data		clk_data;
};

struct tz1090_clk_provider *tz1090_clk_alloc_provider(struct device_node *node,
						      unsigned int num_clks);
const char *tz1090_clk_xlate(struct tz1090_clk_provider *p,
			     const char *clk_name);
void tz1090_clk_register_provider(struct tz1090_clk_provider *p);

#endif
