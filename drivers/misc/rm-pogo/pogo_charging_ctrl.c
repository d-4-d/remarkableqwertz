// SPDX-License-Identifier: GPL-2.0-only
/*
 * reMarkable OTG Control
 *
 * Copyright (C) 2019 reMarkable AS - http://www.remarkable.com/
 *
 * Author: Steinar Bakkemo <steinar.bakkemo@remarkable.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "pogo_charging_ctrl.h"

#include <linux/errno.h>
#include <linux/power_supply.h>

/* Copy of the structure defined in
 * ../drivers/power/supply/power_supply_sysfs.c
 *
 * As the power supply API does not define any
 * methods to access the defined enum strings for
 * the various properties, a local version is defined
 * here, as this driver also defines corresponding enumerations locally
 */
static const char * const charger_mode_string_list[] = {
	"Charger", "OTG Supply", "Off"
};

int pogo_get_otg_charger_modes(struct rm_pogo_data *pdata,
				     char *prop_buf)
{
	int i;
	int cur_pos = 0;

	for(i = 0;i < ARRAY_SIZE(charger_mode_string_list);i++) {
		sprintf(&prop_buf[cur_pos],
			"%s%s",
			(i > 0) ? " " : "",
			charger_mode_string_list[i]);

		cur_pos = strlen(prop_buf);
	}
	sprintf(&prop_buf[cur_pos], "\n");
	return strlen(prop_buf);
}

int pogo_change_otg_charger_mode_int(struct rm_pogo_data *pdata,
					   int mode)
{
	union power_supply_propval property_val;
	int ret;

	switch(mode)
	{
	case POGO_CHARGERMODE_CHARGE:
		dev_dbg(pdata->dev,
			"%s: Setting POGO chargermode (CHARGE)\n",
			__func__);

		property_val.intval = POWER_SUPPLY_MODE_CHARGER;
		ret = power_supply_set_property(pdata->vbus_supply,
						POWER_SUPPLY_PROP_CHARGER_MODE,
						&property_val);
		if (ret < 0) {
			dev_err(pdata->dev,
				"%s: Failed to set charger mode\n",
				__func__);

			return ret;
		}
		break;

	case POGO_CHARGERMODE_OTG:
		dev_dbg(pdata->dev,
			"%s: Setting POGO chargermode (OTG)\n",
			__func__);
		property_val.intval = POWER_SUPPLY_MODE_OTG_SUPPLY;
		ret = power_supply_set_property(pdata->vbus_supply,
						POWER_SUPPLY_PROP_CHARGER_MODE,
						&property_val);
		if (ret < 0) {
			dev_err(pdata->dev,
				"%s: Failed to set charger mode\n",
				__func__);

			return ret;
		}
		break;

	case POGO_CHARGERMODE_OFF:
		dev_dbg(pdata->dev,
			"%s: Setting POGO chargermode (OFF)\n",
			__func__);
		property_val.intval = POWER_SUPPLY_MODE_ALL_OFF;
		ret = power_supply_set_property(pdata->vbus_supply,
						POWER_SUPPLY_PROP_CHARGER_MODE,
						&property_val);
		if (ret < 0) {
			dev_err(pdata->dev,
				"%s: Failed to set charger mode\n",
				__func__);

			return ret;
		}
		break;

	default:
		dev_err(pdata->dev,
			"%s: Unable to set POGO chargermode (invalid mode %d)\n",
			__func__, mode);
		return -EINVAL;
	}

	pdata->pogo_chargermode = mode;
	return 0;
}

int pogo_change_otg_charger_mode_str(struct rm_pogo_data *pdata,
					   const char *buf)
{
	int ret, mode, i, buf_len, mode_len;

	/* Remove trailing \n */
	if (buf[strlen(buf) - 1] == '\n')
		buf_len = strlen(buf) - 1;
	else
		buf_len = strlen(buf);

	ret = kstrtoint(buf, 10, &mode);
	if (ret < 0) {
		mode = -1;
		for (i = 0;i < ARRAY_SIZE(charger_mode_string_list);i++) {
			mode_len = strlen(charger_mode_string_list[i]);
			if (!strncmp(charger_mode_string_list[i],
				     buf,
				     max(buf_len, mode_len))) {
				mode = i;
				break;
			}
		}
		if (mode < 0) {
			dev_err(pdata->dev,
				"%s: Unable to set POGO chargermode "
				"(invalid mode %s)\n",
				__func__,
				buf);

			return -EINVAL;
		}

	}
	else if (mode >= ARRAY_SIZE(charger_mode_string_list)) {
		dev_err(pdata->dev,
			"%s: Unable to set POGO chargermode (invalid mode %d)\n",
			__func__, mode);

		return -EINVAL;
	}

	return pogo_change_otg_charger_mode_int(pdata, mode);
}
