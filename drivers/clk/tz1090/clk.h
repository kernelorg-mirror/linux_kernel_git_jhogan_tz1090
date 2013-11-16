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


/* Clock gate banks */

/**
 * struct tz1090_clk_gate - Describes an individual gate in a bank.
 * @shift:	Shift of bit controlling gate within the bank control register.
 * @name:	Name of gated clock to provide.
 * @parent:	Name of parent/source clock.
 */
struct tz1090_clk_gate {
	unsigned int	shift;
	const char	*name;
	const char	*parent;
};

/**
 * struct tz1090_clk_gate_bank - Describes a gate bank.
 * @id_base:	Base id of bank in provider.
 *		Individual gates get the id id_base + index in gates array.
 * @reg_base:	Offset of gate bank register in the MMIO region.
 * @gates:	Pointer to array of gates in the bank, terminated by one with a
 *		NULL name field.
 */
struct tz1090_clk_gate_bank {
	unsigned int			id_base;
	unsigned long			reg_base;
	const struct tz1090_clk_gate	*gates;
};

#define GATE(_shift, _parent, _name)					\
	{								\
		.shift		= (_shift),				\
		.name		= (_name),				\
		.parent		= (_parent),				\
	},

#define GATE_BANK(_name, _id, _reg, _gates)				\
	static const struct tz1090_clk_gate_bank _name __initconst = {	\
		.id_base	= (_id),				\
		.reg_base	= (_reg),				\
		.gates		= (const struct tz1090_clk_gate[]) {	\
			_gates						\
			{ .name	= NULL }				\
		},							\
	}

void tz1090_clk_register_gate_bank(struct tz1090_clk_provider *p,
				   const struct tz1090_clk_gate_bank *bank);

#endif
