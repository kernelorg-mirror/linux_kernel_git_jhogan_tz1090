/*
 * Copyright (C) 2014 Google, Inc.
 * Copyright (C) 2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * TZ1090 Clocks
 */

#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>

#include "clk.h"

struct tz1090_clk_provider *tz1090_clk_alloc_provider(struct device_node *node,
						      unsigned int num_clks)
{
	struct tz1090_clk_provider *p;

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!p)
		return p;

	p->clk_data.clks = kcalloc(num_clks, sizeof(struct clk *), GFP_KERNEL);
	if (!p->clk_data.clks)
		goto free_provider;
	p->clk_data.clk_num = num_clks;
	p->node = node;
	p->base = of_iomap(node, 0);
	if (!p->base) {
		pr_err("%s: Failed to map clock provider registers\n",
		       node->full_name);
		goto free_clks;
	}

	return p;

free_clks:
	kfree(p->clk_data.clks);
free_provider:
	kfree(p);
	return NULL;
}

const char *tz1090_clk_xlate(struct tz1090_clk_provider *p,
			     const char *clk_name)
{
	/*
	 * If clock name begins with @, the rest refers to an external clock.
	 *
	 * Look for the index of the parent clock with a matching label in
	 * clock-names. If found, find the name of the specified parent clock.
	 *
	 * If not found, we leave it unchanged. The @ at the beginning should
	 * ensure it doesn't accidentally match a real clock.
	 */
	if (*clk_name == '@') {
		const char *clk_label = clk_name + 1;
		int idx = of_property_match_string(p->node, "clock-names",
						   clk_label);
		if (idx >= 0) {
			clk_name = of_clk_get_parent_name(p->node, idx);
			pr_debug("%s: Parent clock '%s' found as '%s'\n",
				 p->node->full_name, clk_label, clk_name);
		} else {
			pr_err("%s: No parent clock '%s' found\n",
			       p->node->full_name, clk_label);
		}
	}

	return clk_name;
}

void tz1090_clk_register_provider(struct tz1090_clk_provider *p)
{
	unsigned int i;

	for (i = 0; i < p->clk_data.clk_num; i++)
		if (IS_ERR(p->clk_data.clks[i]))
			pr_warn("%s: Failed to register clock %d: %ld\n",
				p->node->full_name, i,
				PTR_ERR(p->clk_data.clks[i]));

	of_clk_add_provider(p->node, of_clk_src_onecell_get, &p->clk_data);
}
