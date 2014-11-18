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


/* Clock mux banks */

/**
 * struct tz1090_clk_mux - Describes an individual mux in a bank.
 * @shift:	Shift of bit controlling mux within the bank control register.
 * @name:	Name of muxed clock to provide.
 * @parents:	Name of two parent/source clocks for when the bit is 0 and 1.
 * @default_parent:	Default parent to set the mux to.
 * @mux_flags:	Mux flags.
 */
struct tz1090_clk_mux {
	unsigned int	shift;
	const char	*name;
	const char	*parents[2];
	s8		default_parent;
	u8		mux_flags;
};

/**
 * struct tz1090_clk_mux_bank - Describes a mux bank.
 * @id_base:	Base id of bank in provider.
 *		Individual muxes get the id id_base + index in muxes array.
 * @reg_base:	Offset of mux bank register in the MMIO region.
 * @muxes:	Pointer to array of muxes in the bank, terminated by one with a
 *		NULL name field.
 */
struct tz1090_clk_mux_bank {
	unsigned int			id_base;
	unsigned long			reg_base;
	const struct tz1090_clk_mux	*muxes;
};

#define MUX(_shift, _parent0, _parent1, _name)				\
	{								\
		.shift		= (_shift),				\
		.name		= (_name),				\
		.parents	= {					\
			_parent0,					\
			_parent1					\
		},							\
		.default_parent	= -1,					\
		.mux_flags	= 0,					\
	},

#define MUX_FIXED(_shift, _parent0, _parent1, _name, _default)		\
	{								\
		.shift		= (_shift),				\
		.name		= (_name),				\
		.parents	= {					\
			_parent0,					\
			_parent1					\
		},							\
		.default_parent	= (_default),				\
		.mux_flags	= CLK_MUX_READ_ONLY,			\
	},

#define MUX_BANK(_name, _id, _reg, _muxes)				\
	static const struct tz1090_clk_mux_bank _name __initconst = {	\
		.id_base	= (_id),				\
		.reg_base	= (_reg),				\
		.muxes		= (const struct tz1090_clk_mux[]) {	\
			_muxes						\
			{ .name	= NULL }				\
		},							\
	}

void tz1090_clk_register_mux_bank(struct tz1090_clk_provider *p,
				  const struct tz1090_clk_mux_bank *bank);


/* Deleters */

/**
 * struct tz1090_clk_deleter - Describes a clock deleter.
 * @id:		Id of output clock in provider.
 * @reg:	Offset of deleter register in the MMIO region.
 * @name:	Name of deleted clock to provide.
 * @parent:	Name of parent/source clocks.
 *
 * The deleter is assumed to have a period of 1024.
 */
struct tz1090_clk_deleter {
	unsigned int		id;
	unsigned long		reg;
	const char		*name;
	const char		*parent;
};

#define DEL(_id, _parent, _name, _reg)		\
	{					\
		.id		= (_id),	\
		.reg		= (_reg),	\
		.name		= (_name),	\
		.parent		= (_parent),	\
	}

void tz1090_clk_register_deleters(struct tz1090_clk_provider *p,
				  const struct tz1090_clk_deleter *deleters,
				  unsigned int count);


/* Dividers */

/**
 * struct tz1090_clk_divider - Describes a clock divider.
 * @id:		Id of output clock in provider.
 * @reg:	Offset of divider register in the MMIO region.
 * @flags:	Clock flags.
 * @div_flags:	Divider flags.
 * @shift:	Shift of field controlling divider.
 * @width:	Width of field controlling divider.
 * @shared:	1 if register is shared with other important fields and requires
 *		global locking.
 * @name:	Name of divided clock to provide.
 * @parent:	Name of parent/source clocks.
 */
struct tz1090_clk_divider {
	unsigned int		id;
	unsigned long		reg;
	unsigned long		flags;
	u8			div_flags;
	u8			shift;
	u8			width;
	u8			shared;
	const char		*name;
	const char		*parent;
};

#define DIV(_id, _parent, _name, _reg, _width)	\
	{								\
		.id		= (_id),				\
		.reg		= (_reg),				\
		.width		= (_width),				\
		.name		= (_name),				\
		.parent		= (_parent),				\
	}

#define DIV_FLAGS(_id, _parent, _name, _reg, _width, _flags, _divflags)	\
	{								\
		.id		= (_id),				\
		.reg		= (_reg),				\
		.flags		= (_flags),				\
		.div_flags	= (_divflags),				\
		.width		= (_width),				\
		.name		= (_name),				\
		.parent		= (_parent),				\
	}

#define DIV_SHARED(_id, _parent, _name, _reg, _width, _shift)		\
	{								\
		.id		= (_id),				\
		.reg		= (_reg),				\
		.shift		= (_shift),				\
		.width		= (_width),				\
		.shared		= 1,					\
		.name		= (_name),				\
		.parent		= (_parent),				\
	}

void tz1090_clk_register_dividers(struct tz1090_clk_provider *p,
				  const struct tz1090_clk_divider *dividers,
				  unsigned int count);

#endif
