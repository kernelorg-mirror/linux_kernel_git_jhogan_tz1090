/*
 * Copyright (C) 2012-2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Clock deleter in TZ1090
 */

#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>

#include "clk.h"

/**
 * struct tz1090_clk_deleter_priv - Clock deleter
 *
 * @hw:		handle between common and hardware-specific interfaces
 * @reg:	delete register
 * @period:	cycle period
 * @mask:	bit mask of delete field
 * @shift:	start bit of delete field
 *
 * Deleter in TZ1090, allowing up to period-1 out of each period cycles to be
 * deleted.
 */
struct tz1090_clk_deleter_priv {
	struct clk_hw	hw;
	void __iomem	*reg;
	u32		period;
	u32		mask;
	u8		shift;
};

#define to_tz1090_clk_deleter(_hw) \
	container_of(_hw, struct tz1090_clk_deleter_priv, hw)

static unsigned long tz1090_clk_deleter_recalc_rate(struct clk_hw *hw,
						    unsigned long parent_rate)
{
	struct tz1090_clk_deleter_priv *deleter = to_tz1090_clk_deleter(hw);
	u32 delete;
	u64 rate;

	delete = (readl(deleter->reg) & deleter->mask) >> deleter->shift;
	rate = (u64)parent_rate * (deleter->period - delete);
	do_div(rate, deleter->period);
	return rate;
}

static const struct clk_ops tz1090_clk_deleter_ops = {
	.recalc_rate = tz1090_clk_deleter_recalc_rate,
};

/**
 * __register_deleter() - register a clock deleter
 * @name:		name of this clock
 * @parent_name:	name of clock's parent
 * @flags:		framework-specific flags
 * @reg:		register address to adjust deleter
 * @period:		delete cycle period
 * @mask:		mask of delete field
 * @shift:		start bit of delete field
 *
 * Register a TZ1090 clock deleter with the clock framework.
 */
static struct clk *__init __register_deleter(const char *name,
					     const char *parent_name,
					     unsigned long flags,
					     void __iomem *reg,
					     u32 period,
					     u32 mask,
					     u8 shift)
{
	struct tz1090_clk_deleter_priv *deleter;
	struct clk *clk;
	struct clk_init_data init;

	/* allocate the deleter */
	deleter = kzalloc(sizeof(struct tz1090_clk_deleter_priv), GFP_KERNEL);
	if (!deleter)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &tz1090_clk_deleter_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = (parent_name ? &parent_name : NULL);
	init.num_parents = (parent_name ? 1 : 0);

	/* struct tz1090_clk_deleter_priv assignments */
	deleter->reg = reg;
	deleter->period = period;
	deleter->mask = mask;
	deleter->shift = shift;
	deleter->hw.init = &init;

	/* register the clock */
	clk = clk_register(NULL, &deleter->hw);

	if (IS_ERR(clk))
		kfree(deleter);

	return clk;
}

/**
 * tz1090_clk_register_deleters() - Register set of deleters with a provider.
 * @p:		TZ1090 clock provider.
 * @deleters:	Array of deleter descriptions.
 * @count	Number of deleters described in the array.
 */
void __init tz1090_clk_register_deleters(struct tz1090_clk_provider *p,
				const struct tz1090_clk_deleter *deleters,
				unsigned int count)
{
	const struct tz1090_clk_deleter *del;
	struct clk *clk;
	unsigned int i;

	for (del = deleters, i = 0; i < count; ++del, ++i) {
		clk = __register_deleter(tz1090_clk_xlate(p, del->name),
					 tz1090_clk_xlate(p, del->parent), 0,
					 p->base + del->reg, 1024, 0x3ff, 0);
		p->clk_data.clks[del->id] = clk;
	}
}
