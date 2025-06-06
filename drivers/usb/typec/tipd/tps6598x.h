/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Driver for TI TPS6598x USB Power Delivery controller family
 *
 * Copyright (C) 2017, Intel Corporation
 * Author: Heikki Krogerus <heikki.krogerus@linux.intel.com>
 */

#include <linux/bits.h>
#include <linux/bitfield.h>

#ifndef __TPS6598X_H__
#define __TPS6598X_H__

#define TPS_FIELD_GET(_mask, _reg) ((typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask)))

/* TPS_REG_STATUS bits */
#define TPS_STATUS_PLUG_PRESENT		BIT(0)
#define TPS_STATUS_PLUG_UPSIDE_DOWN	BIT(4)
#define TPS_STATUS_TO_UPSIDE_DOWN(s)	(!!((s) & TPS_STATUS_PLUG_UPSIDE_DOWN))
#define TPS_STATUS_PORTROLE		BIT(5)
#define TPS_STATUS_TO_TYPEC_PORTROLE(s) (!!((s) & TPS_STATUS_PORTROLE))
#define TPS_STATUS_DATAROLE		BIT(6)
#define TPS_STATUS_TO_TYPEC_DATAROLE(s)	(!!((s) & TPS_STATUS_DATAROLE))
#define TPS_STATUS_VCONN		BIT(7)
#define TPS_STATUS_TO_TYPEC_VCONN(s)	(!!((s) & TPS_STATUS_VCONN))
#define TPS_STATUS_OVERCURRENT		BIT(16)
#define TPS_STATUS_GOTO_MIN_ACTIVE	BIT(26)
#define TPS_STATUS_BIST			BIT(27)
#define TPS_STATUS_HIGH_VOLTAGE_WARNING	BIT(28)
#define TPS_STATUS_HIGH_LOW_VOLTAGE_WARNING BIT(29)

#define TPS_STATUS_CONN_STATE_MASK		GENMASK(3, 1)
#define TPS_STATUS_CONN_STATE(x)		TPS_FIELD_GET(TPS_STATUS_CONN_STATE_MASK, (x))
#define TPS_STATUS_PP_5V0_SWITCH_MASK		GENMASK(9, 8)
#define TPS_STATUS_PP_5V0_SWITCH(x)		TPS_FIELD_GET(TPS_STATUS_PP_5V0_SWITCH_MASK, (x))
#define TPS_STATUS_PP_HV_SWITCH_MASK		GENMASK(11, 10)
#define TPS_STATUS_PP_HV_SWITCH(x)		TPS_FIELD_GET(TPS_STATUS_PP_HV_SWITCH_MASK, (x))
#define TPS_STATUS_PP_EXT_SWITCH_MASK		GENMASK(13, 12)
#define TPS_STATUS_PP_EXT_SWITCH(x)		TPS_FIELD_GET(TPS_STATUS_PP_EXT_SWITCH_MASK, (x))
#define TPS_STATUS_PP_CABLE_SWITCH_MASK		GENMASK(15, 14)
#define TPS_STATUS_PP_CABLE_SWITCH(x)		TPS_FIELD_GET(TPS_STATUS_PP_CABLE_SWITCH_MASK, (x))
#define TPS_STATUS_POWER_SOURCE_MASK		GENMASK(19, 18)
#define TPS_STATUS_POWER_SOURCE(x)		TPS_FIELD_GET(TPS_STATUS_POWER_SOURCE_MASK, (x))
#define TPS_STATUS_VBUS_STATUS_MASK		GENMASK(21, 20)
#define TPS_STATUS_VBUS_STATUS(x)		TPS_FIELD_GET(TPS_STATUS_VBUS_STATUS_MASK, (x))
#define TPS_STATUS_USB_HOST_PRESENT_MASK	GENMASK(23, 22)
#define TPS_STATUS_USB_HOST_PRESENT(x)		TPS_FIELD_GET(TPS_STATUS_USB_HOST_PRESENT_MASK, (x))
#define TPS_STATUS_LEGACY_MASK			GENMASK(25, 24)
#define TPS_STATUS_LEGACY(x)			TPS_FIELD_GET(TPS_STATUS_LEGACY_MASK, (x))

#define TPS_STATUS_CONN_STATE_NO_CONN		0
#define TPS_STATUS_CONN_STATE_DISABLED		1
#define TPS_STATUS_CONN_STATE_AUDIO_CONN	2
#define TPS_STATUS_CONN_STATE_DEBUG_CONN	3
#define TPS_STATUS_CONN_STATE_NO_CONN_R_A	4
#define TPS_STATUS_CONN_STATE_RESERVED		5
#define TPS_STATUS_CONN_STATE_CONN_NO_R_A	6
#define TPS_STATUS_CONN_STATE_CONN_WITH_R_A	7

#define TPS_STATUS_PP_SWITCH_STATE_DISABLED	0
#define TPS_STATUS_PP_SWITCH_STATE_FAULT	1
#define TPS_STATUS_PP_SWITCH_STATE_OUT		2
#define TPS_STATUS_PP_SWITCH_STATE_IN		3

#define TPS_STATUS_POWER_SOURCE_UNKNOWN		0
#define TPS_STATUS_POWER_SOURCE_VIN_3P3		1
#define TPS_STATUS_POWER_SOURCE_DEAD_BAT	2
#define TPS_STATUS_POWER_SOURCE_VBUS		3

#define TPS_STATUS_VBUS_STATUS_VSAFE0V		0
#define TPS_STATUS_VBUS_STATUS_VSAFE5V		1
#define TPS_STATUS_VBUS_STATUS_PD		2
#define TPS_STATUS_VBUS_STATUS_FAULT		3

#define TPS_STATUS_USB_HOST_PRESENT_NO		0
#define TPS_STATUS_USB_HOST_PRESENT_PD_NO_USB	1
#define TPS_STATUS_USB_HOST_PRESENT_NO_PD	2
#define TPS_STATUS_USB_HOST_PRESENT_PD_USB	3

#define TPS_STATUS_LEGACY_NO			0
#define TPS_STATUS_LEGACY_SINK			1
#define TPS_STATUS_LEGACY_SOURCE		2

/* TPS_REG_INT_* bits */
#define TPS_REG_INT_USER_VID_ALT_MODE_OTHER_VDM		BIT_ULL(27+32)
#define TPS_REG_INT_USER_VID_ALT_MODE_ATTN_VDM		BIT_ULL(26+32)
#define TPS_REG_INT_USER_VID_ALT_MODE_EXIT		BIT_ULL(25+32)
#define TPS_REG_INT_USER_VID_ALT_MODE_ENTERED		BIT_ULL(24+32)
#define TPS_REG_INT_EXIT_MODES_COMPLETE			BIT_ULL(20+32)
#define TPS_REG_INT_DISCOVER_MODES_COMPLETE		BIT_ULL(19+32)
#define TPS_REG_INT_VDM_MSG_SENT			BIT_ULL(18+32)
#define TPS_REG_INT_VDM_ENTERED_MODE			BIT_ULL(17+32)
#define TPS_REG_INT_ERROR_UNABLE_TO_SOURCE		BIT_ULL(14+32)
#define TPS_REG_INT_SRC_TRANSITION			BIT_ULL(10+32)
#define TPS_REG_INT_ERROR_DISCHARGE_FAILED		BIT_ULL(9+32)
#define TPS_REG_INT_ERROR_MESSAGE_DATA			BIT_ULL(7+32)
#define TPS_REG_INT_ERROR_PROTOCOL_ERROR		BIT_ULL(6+32)
#define TPS_REG_INT_ERROR_MISSING_GET_CAP_MESSAGE	BIT_ULL(4+32)
#define TPS_REG_INT_ERROR_POWER_EVENT_OCCURRED		BIT_ULL(3+32)
#define TPS_REG_INT_ERROR_CAN_PROVIDE_PWR_LATER		BIT_ULL(2+32)
#define TPS_REG_INT_ERROR_CANNOT_PROVIDE_PWR		BIT_ULL(1+32)
#define TPS_REG_INT_ERROR_DEVICE_INCOMPATIBLE		BIT_ULL(0+32)
#define TPS_REG_INT_CMD2_COMPLETE			BIT(31)
#define TPS_REG_INT_CMD1_COMPLETE			BIT(30)
#define TPS_REG_INT_ADC_HIGH_THRESHOLD			BIT(29)
#define TPS_REG_INT_ADC_LOW_THRESHOLD			BIT(28)
#define TPS_REG_INT_PD_STATUS_UPDATE			BIT(27)
#define TPS_REG_INT_STATUS_UPDATE			BIT(26)
#define TPS_REG_INT_DATA_STATUS_UPDATE			BIT(25)
#define TPS_REG_INT_POWER_STATUS_UPDATE			BIT(24)
#define TPS_REG_INT_PP_SWITCH_CHANGED			BIT(23)
#define TPS_REG_INT_HIGH_VOLTAGE_WARNING		BIT(22)
#define TPS_REG_INT_USB_HOST_PRESENT_NO_LONGER		BIT(21)
#define TPS_REG_INT_USB_HOST_PRESENT			BIT(20)
#define TPS_REG_INT_GOTO_MIN_RECEIVED			BIT(19)
#define TPS_REG_INT_PR_SWAP_REQUESTED			BIT(17)
#define TPS_REG_INT_SINK_CAP_MESSAGE_READY		BIT(15)
#define TPS_REG_INT_SOURCE_CAP_MESSAGE_READY		BIT(14)
#define TPS_REG_INT_NEW_CONTRACT_AS_PROVIDER		BIT(13)
#define TPS_REG_INT_NEW_CONTRACT_AS_CONSUMER		BIT(12)
#define TPS_REG_INT_VDM_RECEIVED			BIT(11)
#define TPS_REG_INT_ATTENTION_RECEIVED			BIT(10)
#define TPS_REG_INT_OVERCURRENT				BIT(9)
#define TPS_REG_INT_BIST				BIT(8)
#define TPS_REG_INT_RDO_RECEIVED_FROM_SINK		BIT(7)
#define TPS_REG_INT_DR_SWAP_COMPLETE			BIT(5)
#define TPS_REG_INT_PR_SWAP_COMPLETE			BIT(4)
#define TPS_REG_INT_PLUG_EVENT				BIT(3)
#define TPS_REG_INT_HARD_RESET				BIT(1)
#define TPS_REG_INT_PD_SOFT_RESET			BIT(0)

/* Apple-specific TPS_REG_INT_* bits */
#define APPLE_CD_REG_INT_DATA_STATUS_UPDATE		BIT(10)
#define APPLE_CD_REG_INT_POWER_STATUS_UPDATE		BIT(9)
#define APPLE_CD_REG_INT_STATUS_UPDATE			BIT(8)
#define APPLE_CD_REG_INT_PLUG_EVENT			BIT(1)

/* TPS_REG_SYSTEM_POWER_STATE states */
#define TPS_SYSTEM_POWER_STATE_S0	0x00
#define TPS_SYSTEM_POWER_STATE_S3	0x03
#define TPS_SYSTEM_POWER_STATE_S4	0x04
#define TPS_SYSTEM_POWER_STATE_S5	0x05

/* TPS_REG_POWER_STATUS bits */
#define TPS_POWER_STATUS_CONNECTION(x)  TPS_FIELD_GET(BIT(0), (x))
#define TPS_POWER_STATUS_SOURCESINK(x)	TPS_FIELD_GET(BIT(1), (x))
#define TPS_POWER_STATUS_BC12_DET(x)	TPS_FIELD_GET(BIT(2), (x))

#define TPS_POWER_STATUS_TYPEC_CURRENT_MASK GENMASK(3, 2)
#define TPS_POWER_STATUS_PWROPMODE(p)	    TPS_FIELD_GET(TPS_POWER_STATUS_TYPEC_CURRENT_MASK, (p))
#define TPS_POWER_STATUS_BC12_STATUS_MASK   GENMASK(6, 5)
#define TPS_POWER_STATUS_BC12_STATUS(p)	    TPS_FIELD_GET(TPS_POWER_STATUS_BC12_STATUS_MASK, (p))

#define TPS_POWER_STATUS_TYPEC_CURRENT_USB     0
#define TPS_POWER_STATUS_TYPEC_CURRENT_1A5     1
#define TPS_POWER_STATUS_TYPEC_CURRENT_3A0     2
#define TPS_POWER_STATUS_TYPEC_CURRENT_PD      3

#define TPS_POWER_STATUS_BC12_STATUS_SDP 0
#define TPS_POWER_STATUS_BC12_STATUS_CDP 2
#define TPS_POWER_STATUS_BC12_STATUS_DCP 3

/* TPS25750_REG_POWER_STATUS bits */
#define TPS25750_POWER_STATUS_CHARGER_DETECT_STATUS_MASK	GENMASK(7, 4)
#define TPS25750_POWER_STATUS_CHARGER_DETECT_STATUS(p) \
	TPS_FIELD_GET(TPS25750_POWER_STATUS_CHARGER_DETECT_STATUS_MASK, (p))
#define TPS25750_POWER_STATUS_CHARGER_ADVERTISE_STATUS_MASK	GENMASK(9, 8)
#define TPS25750_POWER_STATUS_CHARGER_ADVERTISE_STATUS(p) \
	TPS_FIELD_GET(TPS25750_POWER_STATUS_CHARGER_ADVERTISE_STATUS_MASK, (p))

#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_DISABLED	0
#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_IN_PROGRESS	1
#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_NONE		2
#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_SPD		3
#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_BC_1_2_CPD	4
#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_BC_1_2_DPD	5
#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_DIV_1_DCP	6
#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_DIV_2_DCP	7
#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_DIV_3_DCP	8
#define TPS25750_POWER_STATUS_CHARGER_DET_STATUS_1_2V_DCP	9

/* TPS_REG_DATA_STATUS bits */
#define TPS_DATA_STATUS_DATA_CONNECTION	     BIT(0)
#define TPS_DATA_STATUS_UPSIDE_DOWN	     BIT(1)
#define TPS_DATA_STATUS_ACTIVE_CABLE	     BIT(2)
#define TPS_DATA_STATUS_USB2_CONNECTION	     BIT(4)
#define TPS_DATA_STATUS_USB3_CONNECTION	     BIT(5)
#define TPS_DATA_STATUS_USB3_GEN2	     BIT(6)
#define TPS_DATA_STATUS_USB_DATA_ROLE	     BIT(7)
#define TPS_DATA_STATUS_DP_CONNECTION	     BIT(8)
#define TPS_DATA_STATUS_DP_SINK		     BIT(9)
#define TPS_DATA_STATUS_TBT_CONNECTION	     BIT(16)
#define TPS_DATA_STATUS_TBT_TYPE	     BIT(17)
#define TPS_DATA_STATUS_OPTICAL_CABLE	     BIT(18)
#define TPS_DATA_STATUS_ACTIVE_LINK_TRAIN    BIT(20)
#define TPS_DATA_STATUS_FORCE_LSX	     BIT(23)
#define TPS_DATA_STATUS_POWER_MISMATCH	     BIT(24)

#define TPS_DATA_STATUS_DP_PIN_ASSIGNMENT_MASK GENMASK(11, 10)
#define TPS_DATA_STATUS_DP_PIN_ASSIGNMENT(x) \
	TPS_FIELD_GET(TPS_DATA_STATUS_DP_PIN_ASSIGNMENT_MASK, (x))
#define TPS_DATA_STATUS_TBT_CABLE_SPEED_MASK   GENMASK(27, 25)
#define TPS_DATA_STATUS_TBT_CABLE_SPEED \
	TPS_FIELD_GET(TPS_DATA_STATUS_TBT_CABLE_SPEED_MASK, (x))
#define TPS_DATA_STATUS_TBT_CABLE_GEN_MASK     GENMASK(29, 28)
#define TPS_DATA_STATUS_TBT_CABLE_GEN \
	TPS_FIELD_GET(TPS_DATA_STATUS_TBT_CABLE_GEN_MASK, (x))

/* Map data status to DP spec assignments */
#define TPS_DATA_STATUS_DP_SPEC_PIN_ASSIGNMENT(x) \
	((TPS_DATA_STATUS_DP_PIN_ASSIGNMENT(x) << 1) | \
		TPS_FIELD_GET(TPS_DATA_STATUS_USB3_CONNECTION, (x)))
#define TPS_DATA_STATUS_DP_SPEC_PIN_ASSIGNMENT_E    0
#define TPS_DATA_STATUS_DP_SPEC_PIN_ASSIGNMENT_F    BIT(0)
#define TPS_DATA_STATUS_DP_SPEC_PIN_ASSIGNMENT_C    BIT(1)
#define TPS_DATA_STATUS_DP_SPEC_PIN_ASSIGNMENT_D    (BIT(1) | BIT(0))
#define TPS_DATA_STATUS_DP_SPEC_PIN_ASSIGNMENT_A    BIT(2)
#define TPS_DATA_STATUS_DP_SPEC_PIN_ASSIGNMENT_B    (BIT(2) | BIT(1))

/* BOOT STATUS REG*/
#define TPS_BOOT_STATUS_DEAD_BATTERY_FLAG	BIT(2)
#define TPS_BOOT_STATUS_I2C_EEPROM_PRESENT	BIT(3)

/* PD STATUS REG */
#define TPS_REG_PD_STATUS_PORT_TYPE_MASK	GENMASK(5, 4)
#define TPS_PD_STATUS_PORT_TYPE(x) \
	TPS_FIELD_GET(TPS_REG_PD_STATUS_PORT_TYPE_MASK, x)

#define TPS_PD_STATUS_PORT_TYPE_SINK_SOURCE	0
#define TPS_PD_STATUS_PORT_TYPE_SINK		1
#define TPS_PD_STATUS_PORT_TYPE_SOURCE		2
#define TPS_PD_STATUS_PORT_TYPE_SOURCE_SINK	3

/* SLEEP CONF REG */
#define TPS_SLEEP_CONF_SLEEP_MODE_ALLOWED	BIT(0)

/* Start Patch Download Sequence */
#define TPS_PTCS_CONTENT_APP			BIT(0)
#define TPS_PTCS_CONTENT_DEV			BIT(1)
#define TPS_PTCS_OUT_BYTES			4
#define TPS_PTCS_STATUS				1

#define TPS_PTCS_STATUS_FAIL			0x80
/* Patch Download */
#define TPS_PTCD_OUT_BYTES			10
#define TPS_PTCD_TRANSFER_STATUS		1
#define TPS_PTCD_LOADING_STATE			2

#define TPS_PTCD_LOAD_ERR			0x09
/* Patch Download Complete */
#define TPS_PTCC_OUT_BYTES			4
#define TPS_PTCC_DEV				2
#define TPS_PTCC_APP				3

/* Version Register */
#define TPS_VERSION_HW_VERSION_MASK            GENMASK(31, 24)
#define TPS_VERSION_HW_VERSION(x)              TPS_FIELD_GET(TPS_VERSION_HW_VERSION_MASK, (x))
#define TPS_VERSION_HW_65981_2_6               0x00
#define TPS_VERSION_HW_65987_8_DH              0xF7
#define TPS_VERSION_HW_65987_8_DK              0xF9

/* Int Event Register length */
#define TPS_65981_2_6_INTEVENT_LEN             8
#define TPS_65987_8_INTEVENT_LEN               11

#endif /* __TPS6598X_H__ */
