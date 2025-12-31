#ifndef FPGA_DRIVER_H
#define FPGA_DRIVER_H

// fpga_driver.h - FPGA驱动头文件
// 日期：2025/12/30 18:13
// C++兼容性声明

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/****************************************错误码********************************************************/

typedef unsigned char uint8_t;


typedef enum {
    FPGA1 = 0,
    FPGA2,
} FPGA_IDX;

// 电台输入接口编号
typedef enum {
    RS_IN_1 = 0,
    RS_IN_2,
    RS_IN_3,
    RS_IN_4,
    RS_IN_MAX,
}RS_IN_E;

// 输入到电台接口编号
typedef enum {
    RS_OUT_1 = 0,
    RS_OUT_2,
    RS_OUT_3,
    RS_OUT_4,
    RS_OUT_MAX,
}RS_OUT_E;

//输出到频谱侦察的接口
typedef enum {
    RS_JT_1 = 0,
    RS_JT_2,
    RS_JT_3,
    RS_JT_4,
    RS_JT_MAX,
}RS_JT_E;

//FPGA2 干扰输出
typedef enum {
    GR_OUT_1 = 0,
    GR_OUT_2,
    GR_OUT_3,
    GR_OUT_4,
    GR_OUT_5,
    GR_OUT_MAX,
}GR_OUT_E;

//算法PATH ID
typedef enum {
    ALG_PATH_1 = 0,
    ALG_PATH_2,
    ALG_PATH_3,
    ALG_PATH_4,
    ALG_PATH_5,
    ALG_PATH_MAX,
} ALG_PATH_E;

typedef enum {
    DATA_SRC_NONE = 0,
    DATA_SRC_ADC1_ALG,
    DATA_SRC_ADC2_ALG,
    DATA_SRC_ADC3_ALG,
    DATA_SRC_ADC4_ALG,
    DATA_SRC_ADC1,
    DATA_SRC_ADC2,
    DATA_SRC_ADC3,
    DATA_SRC_ADC4,
    DATA_SRC_SINE,
} DATA_SRC;

typedef enum {
    FPGA_OK = 0,
    FPGA_ERR_RX_SWITCH,
    FPGA_ERR_RX_ATT_MODE,
    FPGA_ERR_RX_ATT_VALUE,
    FPGA_ERR_PTT_LEN,
    FPGA_ERR_PTT_GATE,
    FPGA_ERR_PTT_STATE,
    FPGA_ERR_L_ADC,
    FPGA_ERR_JT_ATT_SEL,
    FPGA_ERR_JT_ATT_DATA,
    FPGA_ERR_JT_ATT2_DATA,
    FPGA_ERR_JT_ATT_TX_EN,
    FPGA_ERR_CH_ATT_SEL,
    FPGA_ERR_CH_ATT_DATA,
    FPGA_ERR_CH_ATT2_DATA,
    FPGA_ERR_CH_ATT_TX_EN,
    FPGA_ERR_CH_ATT_V1V2,
    FPGA_ERR_AXIS_RELOAD0_DATA,
    FPGA_ERR_AXIS_RELOAD1_DATA,
    FPGA_ERR_CHNL_FREQ,
    FPGA_ERR_DELAY,
    FPGA_ERR_DPL_FDI,
    FPGA_ERR_DPL_FDQ,
    FPGA_ERR_DPL_DFS,
    FPGA_ERR_DPL_SD,
    FPGA_ERR_GAIN,
    FPGA_ERR_DPL_BYPASS,
    FPGA_ERR_GR_ATT_SEL,
    FPGA_ERR_GR_ATT_DATA,
    FPGA_ERR_GR_ATT2_DATA,
    FPGA_ERR_GR_ATT_TX_EN,
    FPGA_ERR_INVALID_CHL,
    FPGA_ERR_INVALID_PATH,
    FPGA_ERR_NULL_P,
} FPGA_ERR;

//电台状态和功率
struct radios {
    uint8_t radio_sta;  //0001 :4321  1发送、4接收
    uint64_t radio_power[4];
};
struct low_adc {
    uint32_t low_adc_buf[4];
};
//带阻滤波
struct bs_axis {
    int32_t coeff[19];  // raxis[0] 对应 raxis1, ..., raxis[18] 对应 raxis19
};

//打开设备
int open_device();
void close_device();

int read_reg(FPGA_IDX idx, uint32_t reg_addr, uint32_t *out_value);
int write_reg(FPGA_IDX idx, uint32_t reg_addr, uint32_t value);
int set_rx_sw_mode(RS_IN_E rs_in, int mode);
int set_rx_sw(RS_IN_E rs_in, bool sw);
int set_rx_att_auto(RS_IN_E rs_in, bool enable);
int set_rx_att_value(RS_IN_E rs_in, float att);
int set_att_len(int att_len);
int set_att_h_gate(float dbfs);
int set_att_l_gate(float dbfs);
int get_low_adc(struct low_adc *lowadc);
int get_ptt_sta_power(struct radios *dt);
int get_rx_att(RS_IN_E rs_in, float* rx_att);
int get_all_rx_att(float* rs_in_1_att, float* rs_in_2_att, float* rs_in_3_att, float* rs_in_4_att);
int set_ptt_gate(int v_value);
int set_ladc_tap(int tap_clk);
int set_jt_sw(RS_JT_E rs_jt, bool sw);
int set_jt_att_value(RS_JT_E rs_jt, float att);
int set_chl_sw(RS_OUT_E rs_out, bool sw);
int set_chl_sw4(RS_OUT_E rs_out, int sw);
int set_chl_att(RS_OUT_E rs_out, float att);
int set_chl_out_sel(RS_OUT_E rs_out, DATA_SRC src_sel);
int set_jt_out_sel(RS_JT_E rs_jt, DATA_SRC src_sel);
int set_dds(float freq);
int set_axis(RS_OUT_E rs_out, struct bs_axis *bs_axis_value);
int set_chl_delay(RS_OUT_E rs_out, ALG_PATH_E path, int delay);
int set_dpl_df(RS_OUT_E rs_out, ALG_PATH_E path, float freq);
int set_dpl_dfs(RS_OUT_E rs_out, ALG_PATH_E path, float freq);
int set_gain(RS_OUT_E rs_out, ALG_PATH_E path, float gain);
int set_bypass_raxis(RS_OUT_E rs_out, int r_axis_sw);
int set_bypass_iq(RS_OUT_E rs_out, int iq_depart_sw);
int set_bypass_laxis(RS_OUT_E rs_out, int l_axis_sw);
int set_bypass_dpl_iq(RS_OUT_E rs_out, ALG_PATH_E path, int dfs_sw, int fd_sw);
//干扰
int set_gr_sw(GR_OUT_E gr_out, bool sw);
int set_gr_att(GR_OUT_E gr_out, float att);

//初始化
int fpga_init();
int fpga_deinit();

// C++兼容性声明结束
#ifdef __cplusplus
}
#endif



#endif
