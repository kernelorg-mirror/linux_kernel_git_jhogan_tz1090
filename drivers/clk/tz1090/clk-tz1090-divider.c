/*
 * Copyright (C) 2013-2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * TZ1090 Divider Clock.
 */

#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <asm/global_lock.h>

#include "clk.h"

/**
 * struct tz1090_clk_div_priv - tz1090 divider clock
 *
 * @div:	the parent class
 * @ops:	pointer to clk_ops of parent class
 *
 * Divider clock whose field shares a register with other fields which may be
 * used by multiple threads/cores and other drivers.
 */
struct tz1090_clk_div_priv {
	struct clk_divider	div;
	const struct clk_ops	*ops;
};

static inline struct tz1090_clk_div_priv *to_tz1090_clk_div(struct clk_hw *hw)
{
	struct clk_divider *div = container_of(hw, struct clk_divider, hw);

	return container_of(div, struct tz1090_clk_div_priv, div);
}

static unsigned long tz1090_clk_divider_recalc_rate(struct clk_hw *hw,
						    unsigned long parent_rate)
{
	struct tz1090_clk_div_priv *div = to_tz1090_clk_div(hw);

	return div->ops->recalc_rate(&div->div.hw, parent_rate);
}

static long tz1090_clk_divider_round_rate(struct clk_hw *hw, unsigned long rate,
					  unsigned long *prate)
{
	struct tz1090_clk_div_priv *div = to_tz1090_clk_div(hw);

	return div->ops->round_rate(&div->div.hw, rate, prate);
}

/* Acquire exclusive lock since other cores may access the same register */
static int tz1090_clk_divider_set_rate(struct clk_hw *hw, unsigned long rate,
				       unsigned long parent_rate)
{
	struct tz1090_clk_div_priv *div = to_tz1090_clk_div(hw);
	int ret;
	unsigned long flags;

	__global_lock2(flags);
	ret = div->ops->set_rate(&div->div.hw, rate, parent_rate);
	__global_unlock2(flags);

	return ret;
}

static const struct clk_ops tz1090_clk_div_ops = {
	.recalc_rate	= tz1090_clk_divider_recalc_rate,
	.round_rate	= tz1090_clk_divider_round_rate,
	.set_rate	= tz1090_clk_divider_set_rate,
};

static struct clk *__init __register_divider(const char *name,
					     const char *parent_name,
					     unsigned long flags,
					     void __iomem *reg, u8 shift,
					     u8 width, u8 clk_divider_flags)
{
	struct tz1090_clk_div_priv *div;
	struct clk *clk;
	struct clk_init_data init;

	/* allocate the divider */
	div = kzalloc(sizeof(struct tz1090_clk_div_priv), GFP_KERNEL);
	if (!div)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &tz1090_clk_div_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = (parent_name ? &parent_name : NULL);
	init.num_parents = (parent_name ? 1 : 0);

	/* struct clk_divider assignments */
	div->div.reg = reg;
	div->div.shift = shift;
	div->div.width = width;
	div->div.flags = clk_divider_flags;
	div->div.hw.init = &init;

	/* struct tz1090_clk_div_priv assignments */
	div->ops = &clk_divider_ops;

	/* register the clock */
	clk = clk_register(NULL, &div->div.hw);

	if (IS_ERR(clk))
		kfree(div);

	return clk;
}

/**
 * tz1090_clk_register_dividers() - Register set of dividers with a provider.
 * @p:		TZ1090 clock provider.
 * @dividers:	Array of divider descriptions.
 * @count	Number of dividers described in the array.
 */
void __init tz1090_clk_register_dividers(struct tz1090_clk_provider *p,
				const struct tz1090_clk_divider *dividers,
				unsigned int count)
{
	const struct tz1090_clk_divider *div;
	struct clk *clk;
	unsigned int i;

	for (div = dividers, i = 0; i < count; ++div, ++i) {
		/*
		 * Dividers in registers shared between OSes must protect the
		 * register with a global lock. Others with dedicated registers
		 * can just use a normal divider.
		 */
		if (div->shared)
			clk = __register_divider(tz1090_clk_xlate(p, div->name),
					tz1090_clk_xlate(p, div->parent),
					div->flags, p->base + div->reg,
					div->shift, div->width, div->div_flags);
		else
			clk = clk_register_divider(NULL,
					tz1090_clk_xlate(p, div->name),
					tz1090_clk_xlate(p, div->parent),
					div->flags, p->base + div->reg,
					div->shift, div->width, div->div_flags,
					NULL);
		p->clk_data.clks[div->id] = clk;
	}
}
