/*
 * Copyright (C) 2013-2014 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * TZ1090 Clock gate bank
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
 * struct tz1090_clk_gate_priv - tz1090 gating clock
 *
 * @mux:	the parent class
 * @ops:	pointer to clk_ops of parent class
 *
 * Clock which can gate its output. Extends basic mux by using a global
 * exclusive lock when read-modify-writing the mux field so that multiple
 * threads/cores can use different fields in the same register.
 */
struct tz1090_clk_gate_priv {
	struct clk_gate		gate;
	const struct clk_ops	*ops;
};

static inline struct tz1090_clk_gate_priv *to_tz1090_clk_gate(struct clk_hw *hw)
{
	struct clk_gate *gate = container_of(hw, struct clk_gate, hw);

	return container_of(gate, struct tz1090_clk_gate_priv, gate);
}

/* Acquire exclusive lock since other cores may access the same register */
static int tz1090_clk_gate_enable(struct clk_hw *hw)
{
	struct tz1090_clk_gate_priv *gate = to_tz1090_clk_gate(hw);
	int ret;
	unsigned long flags;

	__global_lock2(flags);
	ret = gate->ops->enable(&gate->gate.hw);
	__global_unlock2(flags);

	return ret;
}

/* Acquire exclusive lock since other cores may access the same register */
static void tz1090_clk_gate_disable(struct clk_hw *hw)
{
	struct tz1090_clk_gate_priv *gate = to_tz1090_clk_gate(hw);
	unsigned long flags;

	__global_lock2(flags);
	gate->ops->disable(&gate->gate.hw);
	__global_unlock2(flags);
}

static int tz1090_clk_gate_is_enabled(struct clk_hw *hw)
{
	struct tz1090_clk_gate_priv *gate = to_tz1090_clk_gate(hw);

	return gate->ops->is_enabled(&gate->gate.hw);
}

static const struct clk_ops tz1090_clk_gate_ops = {
	.enable = tz1090_clk_gate_enable,
	.disable = tz1090_clk_gate_disable,
	.is_enabled = tz1090_clk_gate_is_enabled,
};

/**
 * __register_gate() - register a TZ1090 gate clock with clock framework.
 * @name: name of this clock
 * @parent_name: name of this clock's parent
 * @flags: framework-specific flags for this clock
 * @reg: register address to control gating of this clock
 * @bit_idx: which bit in the register controls gating of this clock
 * @clk_gate_flags: gate-specific flags for this clock
 */
static struct clk *__init __register_gate(const char *name,
					  const char *parent_name,
					  unsigned long flags,
					  void __iomem *reg,
					  u8 bit_idx,
					  u8 clk_gate_flags)
{
	struct tz1090_clk_gate_priv *gate;
	struct clk *clk;
	struct clk_init_data init;

	/* allocate the gate */
	gate = kzalloc(sizeof(struct tz1090_clk_gate_priv), GFP_KERNEL);
	if (!gate)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &tz1090_clk_gate_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = (parent_name ? &parent_name : NULL);
	init.num_parents = (parent_name ? 1 : 0);

	/* struct clk_gate assignments */
	gate->gate.reg = reg;
	gate->gate.bit_idx = bit_idx;
	gate->gate.flags = clk_gate_flags;
	gate->gate.hw.init = &init;

	/* struct tz1090_clk_gate_priv assignments */
	gate->ops = &clk_gate_ops;

	clk = clk_register(NULL, &gate->gate.hw);

	if (IS_ERR(clk))
		kfree(gate);

	return clk;
}

/**
 * tz1090_clk_register_gate_bank() - Register bank of gates with a provider.
 * @p:		TZ1090 clock provider.
 * @bank:	Data describing gate bank.
 */
void __init tz1090_clk_register_gate_bank(struct tz1090_clk_provider *p,
					const struct tz1090_clk_gate_bank *bank)
{
	const struct tz1090_clk_gate *gate;
	struct clk *clk;
	unsigned int id = bank->id_base;

	for (gate = bank->gates; gate->name; ++gate, ++id) {
		clk = __register_gate(tz1090_clk_xlate(p, gate->name),
				      tz1090_clk_xlate(p, gate->parent),
				      CLK_IGNORE_UNUSED | CLK_SET_RATE_PARENT,
				      p->base + bank->reg_base, gate->shift, 0);
		p->clk_data.clks[id] = clk;
	}
}
