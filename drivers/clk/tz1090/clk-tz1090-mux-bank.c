/*
 * Copyright (C) 2013-2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * TZ1090 Clock mux bank
 */

#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <asm/global_lock.h>

#include "clk.h"

/**
 * struct tz1090_clk_mux_priv - tz1090 multiplexer clock
 *
 * @mux:	the parent class
 * @ops:	pointer to clk_ops of parent class
 *
 * Clock with multiple selectable parents. Extends basic mux by using a global
 * exclusive lock when read-modify-writing the mux field so that multiple
 * threads/cores can use different fields in the same register.
 */
struct tz1090_clk_mux_priv {
	struct clk_mux		mux;
	const struct clk_ops	*ops;
};

static inline struct tz1090_clk_mux_priv *to_tz1090_clk_mux(struct clk_hw *hw)
{
	struct clk_mux *mux = container_of(hw, struct clk_mux, hw);

	return container_of(mux, struct tz1090_clk_mux_priv, mux);
}

static u8 tz1090_clk_mux_get_parent(struct clk_hw *hw)
{
	struct tz1090_clk_mux_priv *mux = to_tz1090_clk_mux(hw);

	return mux->ops->get_parent(&mux->mux.hw);
}

/* Acquire exclusive lock since other cores may access the same register */
static int tz1090_clk_mux_set_parent(struct clk_hw *hw, u8 index)
{
	struct tz1090_clk_mux_priv *mux = to_tz1090_clk_mux(hw);
	int ret;
	unsigned long flags;

	__global_lock2(flags);
	ret = mux->ops->set_parent(&mux->mux.hw, index);
	__global_unlock2(flags);

	return ret;
}

static long tz1090_clk_mux_determine_rate(struct clk_hw *hw, unsigned long rate,
					  unsigned long min_rate,
					  unsigned long max_rate,
					  unsigned long *prate,
					  struct clk_hw **best_parent)
{
	struct tz1090_clk_mux_priv *mux = to_tz1090_clk_mux(hw);

	return mux->ops->determine_rate(&mux->mux.hw, rate, min_rate, max_rate,
					prate, best_parent);
}

static const struct clk_ops tz1090_clk_mux_ops = {
	.get_parent = tz1090_clk_mux_get_parent,
	.set_parent = tz1090_clk_mux_set_parent,
	.determine_rate = tz1090_clk_mux_determine_rate,
};

static struct clk *__init __register_mux(const char *name,
					 const char **parent_names,
					 unsigned long flags, void __iomem *reg,
					 u8 shift, u8 clk_mux_flags)
{
	struct tz1090_clk_mux_priv *mux;
	struct clk *clk;
	struct clk_init_data init;

	/* allocate the mux */
	mux = kzalloc(sizeof(struct tz1090_clk_mux_priv), GFP_KERNEL);
	if (!mux)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &tz1090_clk_mux_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = parent_names;
	init.num_parents = 2;

	/* struct clk_mux assignments */
	mux->mux.reg = reg;
	mux->mux.shift = shift;
	mux->mux.mask = 0x1;
	mux->mux.flags = clk_mux_flags;
	mux->mux.hw.init = &init;

	/* struct tz1090_clk_mux_priv assignments */
	if (clk_mux_flags & CLK_MUX_READ_ONLY)
		mux->ops = &clk_mux_ro_ops;
	else
		mux->ops = &clk_mux_ops;

	clk = clk_register(NULL, &mux->mux.hw);

	if (IS_ERR(clk))
		kfree(mux);

	return clk;
}

/**
 * tz1090_clk_register_mux_bank() - Register bank of muxes with a provider.
 * @p:		TZ1090 clock provider.
 * @bank:	Data describing mux bank.
 */
void __init tz1090_clk_register_mux_bank(struct tz1090_clk_provider *p,
					 const struct tz1090_clk_mux_bank *bank)
{
	const struct tz1090_clk_mux *mux;
	struct clk *clk;
	unsigned int id = bank->id_base;
	const char *parents[2];
	unsigned long flags;
	u32 val;

	for (mux = bank->muxes; mux->name; ++mux, ++id) {
		if (mux->default_parent >= 0) {
			__global_lock2(flags);
			val = ioread32(p->base + bank->reg_base);
			if (mux->default_parent)
				val |= BIT(mux->shift);
			else
				val &= ~BIT(mux->shift);
			iowrite32(val, p->base + bank->reg_base);
			__global_unlock2(flags);
		}

		parents[0] = tz1090_clk_xlate(p, mux->parents[0]);
		parents[1] = tz1090_clk_xlate(p, mux->parents[1]);
		clk = __register_mux(tz1090_clk_xlate(p, mux->name), parents,
				     CLK_SET_RATE_PARENT,
				     p->base + bank->reg_base, mux->shift,
				     mux->mux_flags);
		p->clk_data.clks[id] = clk;
	}
}
