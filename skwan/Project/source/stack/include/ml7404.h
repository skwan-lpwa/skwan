/**
    Author:       Skyley Networks, Inc.
    Version:
    Description: ML7404 register/address defs
    
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    3. Neither the name of the Institute nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#ifndef	__ML7404_H__
#define	__ML7404_H__

#define	BANK_SEL	0x00

enum{
	INT_STATUS_GRP1			= 0x000000ff,
	INT_STATUS_GRP2			= 0x0000ff00,
	INT_STATUS_GRP3			= 0x00ff0000,

	INT_GRP1_00				= 0x00000001,
	INT_GRP1_01				= 0x00000002,
	INT_GRP1_02				= 0x00000004,
	INT_GRP1_03				= 0x00000008,
	INT_GRP1_04				= 0x00000010,
	INT_GRP1_05				= 0x00000020,
	INT_GRP1_06				= 0x00000040,
	INT_GRP1_07				= 0x00000080,
	INT_GRP2_08				= 0x00000001,
	INT_GRP2_09				= 0x00000002,
	INT_GRP2_10				= 0x00000004,
	INT_GRP2_11				= 0x00000008,
	INT_GRP2_12				= 0x00000010,
	INT_GRP2_13				= 0x00000020,
	INT_GRP2_14				= 0x00000040,
	INT_GRP2_15				= 0x00000080,
	INT_GRP3_16				= 0x00000001,
	INT_GRP3_17				= 0x00000002,
	INT_GRP3_18				= 0x00000004,
	INT_GRP3_19				= 0x00000008,
	INT_GRP3_20				= 0x00000010,
	INT_GRP3_21				= 0x00000020,
	INT_GRP3_22				= 0x00000040,
	INT_GRP3_23				= 0x00000080,

	INT_CLOCK_STABLE			= INT_GRP1_00 << 0,			// 0x00000001
	INT_COMPLETE_VCO			= INT_GRP1_01 << 0,			// 0x00000002
	INT_UNLOCK_PLL				= INT_GRP1_02 << 0,			// 0x00000004
	INT_STATUS_CHANGED			= INT_GRP1_03 << 0,			// 0x00000008
	INT_FIFO_EMPTY				= INT_GRP1_04 << 0,			// 0x00000010
	INT_FIFO_FULL				= INT_GRP1_05 << 0,			// 0x00000020
	INT_WAKEUP					= INT_GRP1_06 << 0,			// 0x00000040
	INT_COMPLETE_CLOCK_CALIB	= INT_GRP1_07 << 0,			// 0x00000080
	INT_RX_COMPLETE				= INT_GRP2_08 << 8,			// 0x00000100
	INT_CRC_ERROR				= INT_GRP2_09 << 8,			// 0x00000200
	INT_FOUND_DIVERSITY			= INT_GRP2_10 << 8,			// 0x00000400
	INT_RX_LENGTH_ERROR			= INT_GRP2_11 << 8,			// 0x00000800
	INT_RX_FIFO_ERROR			= INT_GRP2_12 << 8,			// 0x00001000
	INT_FOUND_SFD				= INT_GRP2_13 << 8,			// 0x00002000
	INT_CHECK_FIELD				= INT_GRP2_14 << 8,			// 0x00004000
	INT_SYNC_ERROR				= INT_GRP2_15 << 8,			// 0x00008000
	INT_TX_COMPLETE				= INT_GRP3_16 << 16,		// 0x00010000
	INT_TX_COMPLETE_REQUEST		= INT_GRP3_17 << 16,		// 0x00020000
	INT_CCA_COMPLETE			= INT_GRP3_18 << 16,		// 0x00040000
	INT_TX_LENGTH_ERROR			= INT_GRP3_19 << 16,		// 0x00080000
	INT_TX_FIFO_ERROR			= INT_GRP3_20 << 16,		// 0x00100000
	reserved					= INT_GRP3_21 << 16,		// 0x00200000
	INT_TIMER_1					= INT_GRP3_22 << 16,		// 0x00400000
	INT_TIMER_2					= INT_GRP3_23 << 16,		// 0x00800000
};

typedef	enum{
	b0_RST_SET = 0x0101,	// 0x01
	b0_CLK_SET1,			// 0x02
	b0_CLK_SET2,			// 0x03
	b0_PKT_CTRL1,			// 0x04
	b0_PKT_CTRL2,			// 0x05
	b0_DRATE_SET,			// 0x06
	b0_DATA_SET1,			// 0x07
	b0_DATA_SET2,			// 0x08
	b0_CH_SET,				// 0x09
	b0_RF_STATUS_CTRL,		// 0x0a
	b0_RF_STATUS,			// 0x0b
	b0_DIO_SET,				// 0x0c
	b0_INT_SOURCE_GRP1,		// 0x0d
	b0_INT_SOURCE_GRP2,		// 0x0e
	b0_INT_SOURCE_GRP3,		// 0x0f
	b0_INT_EN_GRP1,			// 0x10
	b0_INT_EN_GRP2,			// 0x11
	b0_INT_EN_GRP3,			// 0x12
	b0_CRC_ERR_H,			// 0x13
	b0_CRC_ERR_M,			// 0x14
	b0_CRC_ERR_L,			// 0x15
	b0_STATE_CLR,			// 0x16
	b0_TXFIFO_THRH,			// 0x17
	b0_TXFIFO_THRL,			// 0x18
	b0_RXFIFO_THRH,			// 0x19
	b0_RXFIFO_THRL,			// 0x1a
	b0_C_CHECK_CTRL,		// 0x1b
	b0_M_CHECK_CTRL,		// 0x1c
	b0_A_CHECK_CTRL,		// 0x1d
	b0_C_FIELD_CODE1,		// 0x1e
	b0_C_FIELD_CODE2,		// 0x1f
	b0_C_FIELD_CODE3,		// 0x20
	b0_C_FIELD_CODE4,		// 0x21
	b0_C_FIELD_CODE5,		// 0x22
	b0_M_FIELD_CODE1,		// 0x23
	b0_M_FIELD_CODE2,		// 0x24
	b0_M_FIELD_CODE3,		// 0x25
	b0_M_FIELD_CODE4,		// 0x26
	b0_A_FIELD_CODE1,		// 0x27
	b0_A_FIELD_CODE2,		// 0x28
	b0_A_FIELD_CODE3,		// 0x29
	b0_A_FIELD_CODE4,		// 0x2a
	b0_A_FIELD_CODE5,		// 0x2b
	b0_A_FIELD_CODE6,		// 0x2c
	b0_SLEEP_WU_SET,		// 0x2d
	b0_WUT_CLK_SET,			// 0x2e
	b0_WUT_INTERVAL_H,		// 0x2f
	b0_WUT_INTERVAL_L,		// 0x30
	b0_WU_DURATION,			// 0x31
	b0_GT_SET,				// 0x32
	b0_GT_CLK_SET,			// 0x33
	b0_GT1_TIMER,			// 0x34
	b0_GT2_TIMER,			// 0x35
	b0_CCA_IGNORE_LVL,		// 0x36
	b0_CCA_LVL,				// 0x37
	b0_CCA_ABORT,			// 0x38
	b0_CCA_CTRL,			// 0x39
	b0_ED_RSLT,				// 0x3a
	b0_IDLE_WAIT_H,			// 0x3b
	b0_IDLE_WAIT_L,			// 0x3c
	b0_CCA_PROG_H,			// 0x3d
	b0_CCA_PROG_L,			// 0x3e
	b0_PREAMBLE_SET,		// 0x3f
	b0_VCO_VTRSLT,			// 0x40
	b0_ED_CTRL,				// 0x41
	b0_TXPR_LEN_H,			// 0x42
	b0_TXPR_LEN_L,			// 0x43
	b0_POSTAMBLE_SET,		// 0x44
	b0_SYNC_CONDITION1,		// 0x45
	b0_SYNC_CONDITION2,		// 0x46
	b0_SYNC_CONDITION3,		// 0x47
	b0_2DIV_CTRL,			// 0x48
	b0_2DIV_RSLT,			// 0x49
	b0_ANT1_ED,				// 0x4a
	b0_ANT2_ED,				// 0x4b
	b0_ANT_CTRL,			// 0x4c
	b0_MON_CTRL,			// 0x4d
	b0_GPIO0_CTRL,			// 0x4e
	b0_GPIO1_CTRL,			// 0x4f
	b0_GPIO2_CTRL,			// 0x50
	b0_GPIO3_CTRL,			// 0x51
	b0_EXTCLK_CTRL,			// 0x52
	b0_SPI_EXT_PA_CTRL,		// 0x53
	b0_CHFIL_BW,			// 0x54
	b0_DC_I_ADJ_H,			// 0x55
	b0_DC_I_ADJ_L,			// 0x56
	b0_DC_Q_ADJ_H,			// 0x57
	b0_DC_Q_ADJ_L,			// 0x58
	b0_DC_FIL_ADJ,			// 0x59
	b0_IQ_MAG_ADJ_H,		// 0x5a
	b0_IQ_MAG_ADJ_L,		// 0x5b
	b0_IQ_PHASE_ADJ_H,		// 0x5c
	b0_IQ_PHASE_ADJ_L,		// 0x5d
	b0_IQ_ADJ_WAIT,			// 0x5e
	b0_IQ_ADJ_TARGET,		// 0x5f
	b0_DEC_GAIN,			// 0x60
	b0_IF_FREQ,				// 0x61
	b0_OSC_ADJ1,			// 0x62
	b0_OSC_ADJ2,			// 0x63
	b0_RESERVED_64,			// 0x64
	b0_OSC_ADJ4,			// 0x65
	b0_RSSI_ADJ,			// 0x66
	b0_PA_MODE,				// 0x67
	b0_PA_REG_FINE_ADJ,		// 0x68
	b0_RESERVED_69,			// 0x69
	b0_CHFIL_BW_CCA,		// 0x6a
	b0_RESERVED_6B,			// 0x6b
	b0_RESERVED_6C,			// 0x6c
	b0_RESERVED_6D,			// 0x6d
	b0_VCO_CAL,				// 0x6e
	b0_VCO_CAL_START,		// 0x6f
	b0_CLK_CAL_SET,			// 0x70
	b0_CLK_CAL_TIME,		// 0x71
	b0_CLK_CAL_H,			// 0x72
	b0_CLK_CAL_L,			// 0x73
	b0_RESERVED_74,			// 0x74
	b0_SLEEP_INT_CLR,		// 0x75
	b0_RF_TEST_MODE,		// 0x76
	b0_STM_STATE,			// 0x77
	b0_FIFO_SET,			// 0x78
	b0_RX_FIFO_LAST,		// 0x79
	b0_TX_PKT_LEN_H,		// 0x7a
	b0_TX_PKT_LEN_L,		// 0x7b
	b0_WR_TX_FIFO,			// 0x7c
	b0_RX_PKT_LEN_H,		// 0x7d
	b0_RX_PKT_LEN_L,		// 0x7e
	b0_RD_FIFO,				// 0x7f

	b1_CLK_OUT = 0x0201,	// 0x01
	b1_TX_RATE_H,			// 0x02
	b1_TX_RATE_L,			// 0x03
	b1_RX_RATE1_H,			// 0x04
	b1_RX_RATE1_L,			// 0x05
	b1_RX_RATE2,			// 0x06
	b1_RESERVED_07,			// 0x07
	b1_OSC_W_SEL,			// 0x08
	b1_RESERVED_09,			// 0x09
	b1_RESERVED_0A,			// 0x0A
	b1_PLL_LOCK_DETECT,		// 0x0B
	b1_RESERVED_0C,			// 0x0C
	b1_RESERVED_0D,			// 0x0D
	b1_GAIN_HOLD,			// 0x0E
	b1_RSSI_STABLE_RES,		// 0x0F
	
	b1_GC_MODE_DIV,			// 0x10
	b1_RESERVED_11,			// 0x11
	b1_RSSI_STABLE_TIME,	// 0x12
	b1_RSSI_MAG_ADJ,		// 0x13
	b1_RESERVED_14,			// 0x14
	b1_AFC_GC_CTRL,			// 0x15
	b1_CRC_POLY3,			// 0x16
	b1_CRC_POLY2,			// 0x17
	b1_CRC_POLY1,			// 0x18
	b1_CRC_POLY0,			// 0x19
	b1_PLL_DIV_SET,			// 0x1A
	b1_TXFREQ_I,			// 0x1B
	b1_TXFREQ_FH,			// 0x1C
	b1_TXFREQ_FM,			// 0x1D
	b1_TXFREQ_FL,			// 0x1E
	b1_RXFREQ_I,			// 0x1F
	
	b1_RXFREQ_FH,			// 0x20
	b1_RXFREQ_FM,			// 0x21
	b1_RXFREQ_FL,			// 0x22
	b1_CH_SPACE_H,			// 0x23
	b1_CH_SPACE_L,			// 0x24
	b1_SYNC_WORD_LEN,		// 0x25
	b1_SYNC_WORD_EN,		// 0x26
	b1_SYNCWORD1_SET0,		// 0x27
	b1_SYNCWORD1_SET1,		// 0x28
	b1_SYNCWORD1_SET2,		// 0x29
	b1_SYNCWORD1_SET3,		// 0x2A
	b1_SYNCWORD2_SET0,		// 0x2B
	b1_SYNCWORD2_SET1,		// 0x2C
	b1_SYNCWORD2_SET2,		// 0x2D
	b1_SYNCWORD2_SET3,		// 0x2E
	b1_FSK_CTRL,			// 0x2F
	
	b1_GFSK_DEV_H,			// 0x30
	b1_GFSK_DEV_L,			// 0x31
	b1_FSK_DEV0_H_GFIL0,	// 0x32
	b1_FSK_DEV0_L_GFIL1,	// 0x33
	b1_FSK_DEV1_H_GFIL2,	// 0x34
	b1_FSK_DEV1_L_GFIL3,	// 0x35
	b1_FSK_DEV2_H_GFIL4,	// 0x36
	b1_FSK_DEV2_L_GFIL5,	// 0x37
	b1_FSK_DEV3_H_GFIL6,	// 0x38
	b1_FSK_DEV3_L,			// 0x39
	b1_FSK_DEV4_H,			// 0x3A
	b1_FSK_DEV4_L,			// 0x3B
	b1_FSK_TIM_ADJ0,		// 0x3C
	b1_FSK_TIM_ADJ1,		// 0x3D
	b1_FSK_TIM_ADJ2,		// 0x3E
	b1_FSK_TIM_ADJ3,		// 0x3F
	
	b1_FSK_TIM_ADJ4,		// 0x40
	b1_4FSK_DATA_MAP,		// 0x41
	b1_FREQ_ADJ_H,			// 0x42
	b1_FREQ_ADJ_L,			// 0x43
	b1_Reserved_44,			// 0x44
	b1_Reserved_45,			// 0x45
	b1_Reserved_46,			// 0x46
	b1_Reserved_47,			// 0x47
	b1_2DIV_MODE,			// 0x48
	b1_2DIV_SEARCH1,		// 0x49
	b1_2DIV_SEARCH2,		// 0x4A
	b1_2DIV_FAST_LVL,		// 0x4B
	b1_Reserved_4C,			// 0x4C
	b1_VCO_CAL_MIN_I,		// 0x4D
	b1_VCO_CAL_MIN_FH,		// 0x4E
	b1_VCO_CAL_MIN_FM,		// 0x4F
	
	b1_VCO_CAL_MIN_FL,		// 0x50
	b1_VCO_CAL_MAX_N,		// 0x51
	b1_VCAL_MIN,			// 0x52
	b1_VCAL_MAX,			// 0x53
	b1_Reserved_54,			// 0x54
	b1_Reserved_55,			// 0x55
	b1_DEMOD_SET0,			// 0x56
	b1_DEMOD_SET1,			// 0x57
	b1_DEMOD_SET2,			// 0x58
	b1_DEMOD_SET3,			// 0x59
	b1_Reserved_5A,			// 0x5A
	b1_Reserved_5B,			// 0x5B
	b1_DEMOD_SET6,			// 0x5C
	b1_DEMOD_SET7,			// 0x5D
	b1_DEMOD_SET8,			// 0x5E
	b1_DEMOD_SET9,			// 0x5F

	b1_DEMOD_SET10,			// 0x60
	b1_DEMOD_SET11,			// 0x61
	b1_ADDR_CHK_CTR_H,		// 0x62
	b1_ADDR_CHK_CTR_L,		// 0x63
	b1_WHT_INIT_H,			// 0x64
	b1_WHT_INIT_L,			// 0x65
	b1_WHT_CFG,				// 0x66
	b1_Reserved_67,			// 0x67
	b1_Reserved_68,			// 0x68
	b1_Reserved_69,			// 0x69
	b1_Reserved_6A,			// 0x6A
	b1_Reserved_6B,			// 0x6B
	b1_Reserved_6C,			// 0x6C
	b1_Reserved_6D,			// 0x6D
	b1_Reserved_6E,			// 0x6E
	b1_Reserved_6F,			// 0x6F
	
	b1_Reserved_70,			// 0x70
	b1_Reserved_71,			// 0x71
	b1_Reserved_72,			// 0x72
	b1_Reserved_73,			// 0x73
	b1_Reserved_74,			// 0x74
	b1_Reserved_75,			// 0x75
	b1_Reserved_76,			// 0x76
	b1_Reserved_77,			// 0x77
	b1_Reserved_78,			// 0x78
	b1_Reserved_79,			// 0x79
	b1_Reserved_7A,			// 0x7A
	b1_TX_RATE2_EN,			// 0x7B
	b1_TX_RATE2_H,			// 0x7C
	b1_TX_RATE2_L,			// 0x7D
	b1_Reserved,			// 0x7E
	b1_ID_CODE,				// 0x7F
	
	b2_GAIN_HHTOH = 0x0376,	// 0x76
	b2_GAIN_HTOHH,			// 0x77
	b2_GAIN_HTOM,			// 0x78
	b2_GAIN_MTOH,			// 0x79
	b2_GAIN_MTOL,			// 0x7A
	b2_GAIN_LTOM,			// 0x7B
	b2_RSSI_ADJ_H,			// 0x7C
	b2_RSSI_ADJ_M,			// 0x7D
	b2_RSSI_ADJ_L,			// 0x7E
	b2_Reserved_7F,			// 0x7F

	b3_2MODE_DET = 0x0423,	// 0x23
	b3_RAMP_CTRL1 = 0x0441, // 0x41
	b3_RAMP_CTRL2,			// 0x42
	b3_RAMP_CTRL3,			// 0x43
	b3_EXT_WU_CTRL=0x0450, 	// 0x50
	b3_EXT_WU_INTERVAL, 	// 0x51
		
	b6_MOD_CTRL = 0x0701,	// 0x01
	b6_Reserved_02,			// 0x02
	b6_ASK_RSSI_TH,			// 0x03
	b6_Reserved_04,			// 0x04
	b6_ASK_PDET_STEP1,		// 0x05
	b6_Reserved_06,			// 0x06
	b6_ASK_PDET_STEP2,		// 0x07
	b6_ASK_NOISE_TH1,		// 0x08
	b6_ASK_NOISE_TH2,		// 0x09
	b6_ASK_PDET_FAST,		// 0x0A

	b6_4FSK_DATA_MAP=0x072C,// 0x2C
	b6_Reserved_2D,			// 0x2D
	b6_Reserved_2E,			// 0x2E
	b6_FSK_CTRL,			// 0x2F
	b6_GFSK_DEV_H,			// 0x30
	b6_GFSK_DEV_L,			// 0x31
	b6_FSK_DEV0_H_GFIL0,	// 0x32
	b6_FSK_DEV0_L_GFIL1,	// 0x33
	b6_FSK_DEV1_H_GFIL2,	// 0x34
	b6_FSK_DEV1_L_GFIL3,	// 0x35
	b6_FSK_DEV2_H_GFIL4,	// 0x36
	b6_FSK_DEV2_L_GFIL5,	// 0x37
	b6_FSK_DEV3_H_GFIL6,	// 0x38
	b6_FSK_DEV3_L,			// 0x39
	b6_FSK_DEV4_H,			// 0x3A
	b6_FSK_DEV4_L,			// 0x3B
	b6_PA_AMP5_H,			// 0x3C
	b6_PA_AMP5_L,			// 0x3D
	b6_PA_AMP6_H,			// 0x3E
	b6_PA_AMP6_L,			// 0x3F

	b6_PA_AMP7_H,			// 0x40
	b6_PA_AMP7_L,			// 0x41
	b6_PA_AMP8_H,			// 0x42
	b6_PA_AMP8_L,			// 0x43
	b6_PA_AMP9_H,			// 0x44
	b6_PA_AMP9_L,			// 0x45
	b6_PA_AMP10_H,			// 0x46
	b6_PA_AMP10_L,			// 0x47
	b6_PA_AMP11_H,			// 0x48
	b6_PA_AMP11_L,			// 0x49
	b6_PA_AMP12_H,			// 0x4A
	b6_PA_AMP12_L,			// 0x4B
	b6_PA_AMP13_H,			// 0x4C
	b6_PA_AMP13_L,			// 0x4D
	b6_PA_AMP14_H,			// 0x4E
	b6_PA_AMP14_L,			// 0x4F
	
	b6_PA_AMP15_H,			// 0x50
	b6_PA_AMP15_L,			// 0x51

	b6_FSK_TIM_ADJ0=0x0762,	// 0x62
	b6_FSK_TIM_ADJ1,		// 0x63
	b6_FSK_TIM_ADJ2,		// 0x64
	b6_FSK_TIM_ADJ3,		// 0x65
	b6_FSK_TIM_ADJ4,		// 0x66
	b6_FSK_TIM_ADJ5,		// 0x67
	b6_FSK_TIM_ADJ6,		// 0x68
	b6_FSK_TIM_ADJ7,		// 0x69
	b6_FSK_TIM_ADJ8,		// 0x6A
	b6_FSK_TIM_ADJ9,		// 0x6B
	b6_FSK_TIM_ADJ10,		// 0x6C
	b6_FSK_TIM_ADJ11,		// 0x6D
	b6_FSK_TIM_ADJ12,		// 0x6E
	b6_FSK_TIM_ADJ13,		// 0x6F
	b6_FSK_TIM_ADJ14,		// 0x70
	
	b7_DSSS_CTRL = 0x0801,	// 0x01
	b7_DSSS_MODE,			// 0x02
	b7_FEC_ENC_CTRL,		// 0x03
	b7_Reserved_04,			// 0x04
	b7_FEC_DEC_CTRL,		// 0x05
	b7_SF_CTRL,				// 0x06
	b7_SHR_GOLD_SEED3,		// 0x07
	b7_SHR_GOLD_SEED2,		// 0x08
	b7_SHR_GOLD_SEED1,		// 0x09
	b7_SHR_GOLD_SEED0,		// 0x0A
	b7_PSDU_GOLD_SEED3,		// 0x0B
	b7_PSDU_GOLD_SEED2,		// 0x0C
	b7_PSDU_GOLD_SEED1,		// 0x0D
	b7_PSDU_GOLD_SEED0,		// 0x0E
	b7_DSSS_PREAMBLE3,		// 0x0F
	
	b7_DSSS_PREAMBLE2,		// 0x10
	b7_DSSS_PREAMBLE1,		// 0x11
	b7_DSSS_PREAMBLE0,		// 0x12
	b7_SS_DOWN_SIZE,		// 0x13
	b7_SS_AFC_RANGE_SYNC,	// 0x14
	b7_SS_AFC_RANGE,		// 0x15
	b7_Reserved_16,			// 0x16
	b7_DSSS_RATE_SYNC_H,	// 0x17
	b7_DSSS_RATE_SYNC_L,	// 0x18
	b7_DSSS_RATE_H,			// 0x19
	b7_DSSS_RATE_L,			// 0x1A
	b7_SS_SYNC_BIT8_GATE_H,		// 0x1B
	b7_SS_SYNC_BIT8_GATE_L,		// 0x1C
	b7_SS_SYNC_BIT_GATE_SHR_H,	// 0x1D
	b7_SS_SYNC_BIT_GATE_SHR_L,	// 0x1E
	b7_SS_SYNC_BIT_GATE_PSDU_H,	// 0x1F
	b7_SS_SYNC_BIT_GATE_PSDU_L,	// 0x20
	
	b10_BPSK_STEP_CTRL = 0x0B01, // 0x01
	b10_BPSK_STEP_CLK_SET, 		// 0x02
	b10_Reserved_03, 			// 0x03
	b10_BPSK_STEP_SET0,			// 0x04
	b10_BPSK_STEP_SET1, 		// 0x05
	b10_BPSK_STEP_SET2, 		// 0x06
	b10_BPSK_STEP_SET3, 		// 0x07
	b10_BPSK_STEP_SET4, 		// 0x08
	b10_BPSK_STEP_SET5, 		// 0x09
	b10_BPSK_STEP_SET6, 		// 0x0A
	b10_BPSK_STEP_SET7, 		// 0x0B
	b10_BPSK_STEP_SET8, 		// 0x0C
	b10_BPSK_STEP_SET9, 		// 0x0D
	
	b10_BPSK_STEP_SET10, 		// 0x0E
	b10_BPSK_STEP_SET11, 		// 0x0F
	b10_BPSK_STEP_SET12, 		// 0x10
	b10_BPSK_STEP_SET13, 		// 0x11
	b10_BPSK_STEP_SET14, 		// 0x12
	b10_BPSK_STEP_SET15, 		// 0x13
	b10_BPSK_STEP_SET16, 		// 0x14
	b10_BPSK_STEP_SET17, 		// 0x15
	b10_BPSK_STEP_SET18, 		// 0x16
	b10_BPSK_STEP_SET19, 		// 0x17

	b10_BPSK_STEP_SET20, 		// 0x18
	b10_BPSK_STEP_SET21, 		// 0x19
	b10_BPSK_STEP_SET22, 		// 0x1A
	b10_BPSK_STEP_SET23, 		// 0x1B
	b10_BPSK_STEP_SET24, 		// 0x1C
	b10_BPSK_STEP_SET25, 		// 0x1D
	b10_BPSK_STEP_SET26, 		// 0x1E
	b10_BPSK_STEP_SET27, 		// 0x1F
	b10_BPSK_STEP_SET28, 		// 0x20
	b10_BPSK_STEP_SET29, 		// 0x21

	b10_BPSK_STEP_SET30, 		// 0x22
	b10_BPSK_STEP_SET31, 		// 0x23
	b10_BPSK_STEP_SET32, 		// 0x24
	b10_BPSK_STEP_SET33, 		// 0x25
	b10_BPSK_STEP_SET34, 		// 0x26
	b10_BPSK_STEP_SET35, 		// 0x27
	b10_BPSK_STEP_SET36, 		// 0x28
	b10_BPSK_STEP_SET37, 		// 0x29
	b10_BPSK_STEP_SET38, 		// 0x2A
	b10_BPSK_STEP_SET39, 		// 0x2B
	
	b10_BPSK_STEP_SET40, 		// 0x2C
	b10_BPSK_STEP_SET41, 		// 0x2D
	b10_BPSK_STEP_SET42, 		// 0x2E
	b10_BPSK_STEP_SET43, 		// 0x2F
	b10_BPSK_STEP_SET44, 		// 0x30
	b10_BPSK_STEP_SET45, 		// 0x31
	b10_BPSK_STEP_SET46, 		// 0x32
	b10_BPSK_STEP_SET47, 		// 0x33
	b10_BPSK_STEP_SET48, 		// 0x34
	b10_BPSK_STEP_SET49, 		// 0x35
	
	b10_BPSK_STEP_SET50, 		// 0x36
	b10_BPSK_STEP_SET51, 		// 0x37
	b10_BPSK_STEP_SET52, 		// 0x38
	b10_BPSK_STEP_SET53, 		// 0x39
	b10_BPSK_STEP_SET54, 		// 0x3A
	b10_BPSK_STEP_SET55, 		// 0x3B
	b10_BPSK_STEP_SET56, 		// 0x3C
	b10_BPSK_STEP_SET57, 		// 0x3D
	b10_BPSK_STEP_SET58, 		// 0x3E
	b10_BPSK_STEP_SET59, 		// 0x3F
	
	b10_PADRV_CTRL, 			// 0x40
	b10_PADRV_ADJ1, 			// 0x41
	b10_PADRV_ADJ2_H, 			// 0x42
	b10_PADRV_ADJ2_L, 			// 0x43
	b10_PADRV_CLK_SET_H, 		// 0x44
	b10_PADRV_CLK_SET_L, 		// 0x45
	b10_PADRV_UP_ADJ, 			// 0x46
} ML7404_Register;

#endif
