/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (c) 2014, The Linux Foundation. All rights reserved. */

#ifndef __QCOM_CLK_REGMAP_H__
#define __QCOM_CLK_REGMAP_H__

#include <linux/clk-provider.h>

struct regmap;

struct clk_vdd_class {
	int num_regulators;
	u32 *vdd_uv;
	const char **regulator_names;
};

struct clk_vdd_class_data {
	unsigned int num_vdd_classes;
	struct clk_vdd_class **vdd_classes;
	struct clk_vdd_class *vdd_class;
	unsigned int num_rate_max;
	unsigned long *rate_max;
};

/**
 * struct clk_regmap - regmap supporting clock
 * @hw:		handle between common and hardware-specific interfaces
 * @regmap:	regmap to use for regmap helpers and/or by providers
 * @enable_reg: register when using regmap enable/disable ops
 * @enable_mask: mask when using regmap enable/disable ops
 * @enable_is_inverted: flag to indicate set enable_mask bits to disable
 *                      when using clock_enable_regmap and friends APIs.
 */
struct clk_regmap {
	struct clk_hw hw;
	struct regmap *regmap;
	unsigned int enable_reg;
	unsigned int enable_mask;
	bool enable_is_inverted;
	struct clk_vdd_class_data vdd_data;
};

static inline struct clk_regmap *to_clk_regmap(struct clk_hw *hw)
{
	return container_of(hw, struct clk_regmap, hw);
}

int clk_is_enabled_regmap(struct clk_hw *hw);
int clk_enable_regmap(struct clk_hw *hw);
void clk_disable_regmap(struct clk_hw *hw);
int devm_clk_register_regmap(struct device *dev, struct clk_regmap *rclk);
bool clk_is_regmap_clk(struct clk_hw *hw);
int clk_runtime_get_regmap(struct clk_regmap *rclk);
void clk_runtime_put_regmap(struct clk_regmap *rclk);
int clk_find_vdd_level(struct clk_hw *hw, struct clk_vdd_class_data *vdd_data,
		       unsigned long rate);

#endif
