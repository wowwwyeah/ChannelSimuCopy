#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include "fpga_driver.h"
#ifdef  USE_FPGA_TEST
#include <QDebug>
#endif
#ifdef __QT__
#include <QDebug>
#define SO_DEBUG(format, ...) qDebug() << QString(format).arg(__VA_ARGS__)
#else
#define SO_DEBUG(format, ...) \
do { \
        printf("%s,%d->" format "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__); \
} while(0)
#endif

typedef struct {         //写寄存器
    uint32_t fpga_idx;
    uint32_t addr;
    uint32_t value;
} CTL_REG;

#define __QT__ 1

#define SPI_IOC_MAGIC 'A'
#define FPGA_SET_VALUE _IOW(SPI_IOC_MAGIC,0, CTL_REG)
#define FPGA_GET_VALUE _IOWR(SPI_IOC_MAGIC,1, CTL_REG)  //读寄存器，需要先写再读，所以用_IOWR


/****************************************自动增益、PTT********************************************************/
/*
    开关模式：手动、自动
    bit0：0手动控开关，1自动控开关
*/
#define REG_RX_SW_MODE 0x0038

/*
    手动开关
    前提：开关模式变为手动
    bit0-bit3：写入0x1、0x2、0x4、0x8（打开通道1、2、3、4 ）
*/
#define REG_RX_SWITCH 0x002D
#define REG_RX_SWITCH_CHL1 (1 << 0)
#define REG_RX_SWITCH_CHL2 (1 << 1)
#define REG_RX_SWITCH_CHL3 (1 << 2)
#define REG_RX_SWITCH_CHL4 (1 << 3)

/*
    自动增益模式
    bit0   0自动，1手动
*/
#define REG_RX_ATT_MODE 0x0020

/*
    增益手动控制寄存器地址
    前提：1.打开开关控制，2增益模式变为手动
    bit12-bit15：写入0、1、2、3（设置通道1、2、3、4 ）
    bit8：1，上升沿触发
    bit0-5:设置衰减值
    eg:通道1衰减3dbm,先配0x0，再配0x0106
*/
#define REG_RX_ATT_VALUE 0x0021

/*
    自动增益控制，功率统计长度
    需要配置，高低门限需要获取
*/
#define REG_ATT_LEN 0x0025

/*
    高速ADC，设置高低门限，实现自动增益控制
    power = round(10^(0.1*dbfs)*len*2^30)
        power是输入寄存器的值
        dbfs输入的值
        len：自动增益功率统计长度
*/
#define REG_ATT_H_GATE_L 0x0026
#define REG_ATT_H_GATE_H 0x0027
#define REG_ATT_L_GATE_L 0x0028
#define REG_ATT_L_GATE_H 0x0029

/*
    通道选择-读取通道功率
    1bit对应1通道，共4通道
*/
#define REG_ATT_RD_CH_SEL 0x002F

/*
    电台功率查询
*/
#define REG_ATT_POWER_L 0x0041
#define REG_ATT_POWER_H 0x0042

/*
    低速adc
*/
const uint32_t LOW_ADC[4] = { 0X44, 0X45, 0X46, 0X47 };
/*
    PTT状态查询
    输出：1bit检测一个通道
    eg：输出0x0001，通道1发
*/
#define REG_PTT_STATE 0x0048

/*
    PTT检测门限-切换开关、电台收发状态
    如果700mv，直接700转成16进制写入寄存器
*/
#define REG_PTT_GATE 0x002B

/*
    低速ADC的fpga自动读取周期,看电台的收发
*/
#define REG_LADC_TAP 0x002E
/*

当前ATT值，查询
*/
#define REG_CURR_ATT 0x0049

/****************************************解调控制寄存器********************************************************/
/*
    解调衰减触发
    bit0-bit7对应解调1的att1...解调4的att2
    eg：先置零，再将相应衰减器置1.
*/
#define REG_JT_ATT_SEL 0x0031

/*
    解调衰减配置
    bit0-bit5,设置衰减值
*/
#define REG_JT_ATT_DATA 0x0032

/*
    解调开关控制
    bit0-bit3：写入0x1、0x2、0x4、0x8（打开解调通道1、2、3、4 ）
*/
#define REG_JT_ATT_TX_EN 0x0033

/****************************************CHX控制寄存器********************************************************/
/*
    通道衰减触发
    bit0-bit7:通道1的att1...通道2的att2
    eg：先置零，再将相应衰减器置1.
*/
#define REG_CH_ATT_SEL 0x0034
/*
    通道衰减配置
    bit0-bit5,设置衰减值
*/
#define REG_CH_ATT_DATA 0x0035
/*
    通道开关控制
    bit0-bit3：写入0x1、0x2、0x4、0x8（打开解调通道1、2、3、4 ）
*/
#define REG_CH_ATT_TX_EN 0x0037
/*
    通道4选1
    bit0-bit7：每两bit对应1通道，
        bit0-bit1：0、1、2、3：分别通合路器1、2、3、4
*/
#define REG_CH_ATT_V1V2 0x0036



/****************************************信道模拟控制寄存器********************************************************/
/*
    DAC输出数据源选择
    bit0-bit31：每4bit对应一个DAC，共8个dac
        bit0-bit3：0-9，9种输出来源
*/
#define REG_DAC_OUT_SEL 0x003c

/*
    DAC输出测试单音
    计算公式：f=REG/2^31*125MHz
*/
#define REG_DAC_dds 0x003b

/*
    带阻滤波器配置
    1通道对应一个寄存器
    配置step:
        1.START先0后1
        2.data在同一寄存器覆盖形写19个值
        3.ENT先0后1
*/
const uint32_t REG_AXIS_RELOAD1_START[4] = { 0X10C, 0X20C, 0X30C, 0X40C };
const uint32_t REG_AXIS_RELOAD1_END[4] = { 0X10E, 0X20E, 0X30E, 0X40E };
const uint32_t REG_AXIS_RELOAD1_DATA[4] = { 0X10F, 0X20F, 0X30F, 0X40F };

/*
    通道频移-只读取
    1通道对应一个寄存器
*/
const uint32_t REG_CHNL_FREQ[4] = {
    0X110, 0X210, 0X310, 0X410
};

/*
    路径延时
    共4通道，1通道5路径
    1路径对应1个延时寄存器（路径1没有延时）
*/
const uint32_t REG_DELAY[4][5] = {
    {0x0, 0x111, 0x112, 0x113, 0x114},
    {0x0, 0x211, 0x212, 0x213, 0x214},
    {0x0, 0x311, 0x312, 0x313, 0x314},
    {0x0, 0x411, 0x412, 0x413, 0x414},
    };


/*
    路径频扩
    共4通道，1通道5路径
    1路径对应15个扩频寄存器，分别为i0-i6、q0-q7
*/
const uint32_t REG_DPL_FDI[4][5][7] = {
    {
        {0x115, 0x116, 0x117, 0x118, 0x119, 0x11A, 0x11B},
        {0x127, 0x128, 0x129, 0x12a, 0x12b, 0x12c, 0x12d},
        {0x139, 0x13a, 0x13b, 0x13c, 0x13d, 0x13e, 0x13f},
        {0x14b, 0x14c, 0x14d, 0x14e, 0x14f, 0x150, 0x151},
        {0x15d, 0x15e, 0x15f, 0x160, 0x161, 0x162, 0x163}
    },
    {
        {0x215, 0x216, 0x217, 0x218, 0x219, 0x21A, 0x21B},
        {0x227, 0x228, 0x229, 0x22a, 0x22b, 0x22c, 0x22d},
        {0x239, 0x23a, 0x23b, 0x23c, 0x23d, 0x23e, 0x23f},
        {0x24b, 0x24c, 0x24d, 0x24e, 0x24f, 0x250, 0x251},
        {0x25d, 0x25e, 0x25f, 0x260, 0x261, 0x262, 0x263}
    },
    {
        {0x315, 0x316, 0x317, 0x318, 0x319, 0x31A, 0x31B},
        {0x327, 0x328, 0x329, 0x32a, 0x32b, 0x32c, 0x32d},
        {0x339, 0x33a, 0x33b, 0x33c, 0x33d, 0x33e, 0x33f},
        {0x34b, 0x34c, 0x34d, 0x34e, 0x34f, 0x350, 0x351},
        {0x35d, 0x35e, 0x35f, 0x360, 0x361, 0x362, 0x363}
    },
    {
        {0x415, 0x416, 0x417, 0x418, 0x419, 0x41A, 0x41B},
        {0x427, 0x428, 0x429, 0x42a, 0x42b, 0x42c, 0x42d},
        {0x439, 0x43a, 0x43b, 0x43c, 0x43d, 0x43e, 0x43f},
        {0x44b, 0x44c, 0x44d, 0x44e, 0x44f, 0x450, 0x451},
        {0x45d, 0x45e, 0x45f, 0x460, 0x461, 0x462, 0x463}
    },
    };
const uint32_t REG_DPL_FDQ[4][5][8] = {
    {
        {0x11C, 0x11D, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0x123},
        {0x12e, 0x12f, 0x130, 0x131, 0x132, 0x133, 0x134, 0x135},
        {0x140, 0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x147},
        {0x152, 0x153, 0x154, 0x155, 0x156, 0x157, 0x158, 0x159},
        {0x164, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16a, 0x16b}
    },
    {
        {0x21C, 0x21D, 0x21E, 0x21F, 0x220, 0x221, 0x222, 0x223},
        {0x22e, 0x22f, 0x230, 0x231, 0x232, 0x233, 0x234, 0x235},
        {0x240, 0x241, 0x242, 0x243, 0x244, 0x245, 0x246, 0x247},
        {0x252, 0x253, 0x254, 0x255, 0x256, 0x257, 0x258, 0x259},
        {0x264, 0x265, 0x266, 0x267, 0x268, 0x269, 0x26a, 0x26b}
    },
    {
        {0x31C, 0x31D, 0x31E, 0x31F, 0x320, 0x321, 0x322, 0x323},
        {0x32e, 0x32f, 0x330, 0x331, 0x332, 0x333, 0x334, 0x335},
        {0x340, 0x341, 0x342, 0x343, 0x344, 0x345, 0x346, 0x347},
        {0x352, 0x353, 0x354, 0x355, 0x356, 0x357, 0x358, 0x359},
        {0x364, 0x365, 0x366, 0x367, 0x368, 0x369, 0x36a, 0x36b}
    },
    {
        {0x41C, 0x41D, 0x41E, 0x41F, 0x420, 0x421, 0x422, 0x423},
        {0x42e, 0x42f, 0x430, 0x431, 0x432, 0x433, 0x434, 0x435},
        {0x440, 0x441, 0x442, 0x443, 0x444, 0x445, 0x446, 0x447},
        {0x452, 0x453, 0x454, 0x455, 0x456, 0x457, 0x458, 0x459},
        {0x464, 0x465, 0x466, 0x467, 0x468, 0x469, 0x46a, 0x46b}
    },
    };


/*
    多普勒频移
    共4通道，1通道5路径
    1路径对应1个频移寄存器
*/
const uint32_t REG_DPL_DFS[4][5] = {
    {0X124, 0x136, 0x148, 0x15a, 0x16c},  //通道1
    {0X224, 0x236, 0x248, 0x25a, 0x26c},  //通道2
    {0X324, 0x336, 0x348, 0x35a, 0x36c},  //通道3
    {0X424, 0x436, 0x448, 0x45a, 0x46c},  //通道4
};

/*
    扩频因子Ci,n、 Cq,n
    共4通道，1通道5路径
    1路径对应1个扩频因子寄存器
*/
const uint32_t REG_DPL_SD[4][5] = {
    {0X125, 0x137, 0x149, 0x15b, 0x16d},
    {0X225, 0x237, 0x249, 0x25b, 0x26d},
    {0X325, 0x337, 0x349, 0x35b, 0x36d},
    {0X425, 0x437, 0x449, 0x45b, 0x46d},
    };

/*
    信道路径增益控制
    共4通道，1通道5路径
    1路径对应1个增益控制寄存器
*/
const uint32_t REG_gain[4][5] = {
    {0X126, 0x138, 0x14a, 0x15c, 0x16e},
    {0X226, 0x238, 0x24a, 0x25c, 0x26e},
    {0X326, 0x338, 0x34a, 0x35c, 0x36e},
    {0X426, 0x438, 0x44a, 0x45c, 0x46e},
    };

/************************************************/


/*
    旁路开关
    共4通道，1通道对应1个旁路开关
    bit0-bit12
*/
const uint32_t REG_DPL_BYPASS[4] = {
    0X16F, 0X26F, 0X36F, 0X46F
};

/****************************************干扰控制寄存器********************************************************/
//FPGA2
//触发
#define REG_GR_ATT_SEL 0x000C
//设置CH衰减值
#define REG_GR_ATT_DATA 0x000D
//二选一开关
#define REG_GR_ATT_TX_EN 0x0006

/**************************************************************************************************************/


static int g_spi_fd;

//打开设备
int open_device() {
    g_spi_fd = open("/dev/fpga_spi", O_RDWR);
    if (g_spi_fd < 0) { //节点打开失败
        SO_DEBUG("open /dev/fpga_spi error, errno=%d\r\n", errno);
        return -1;
    }
    return 0;
}

void close_device() {
    close(g_spi_fd);
}

// 根据错误码获取错误信息
const char* fpga_strerror(FPGA_ERR err) {
    switch (err) {
    case FPGA_OK: return "Success";
    case FPGA_ERR_RX_SWITCH: return "Failed to turn on the gain switch";
    case FPGA_ERR_RX_ATT_MODE: return "Failed to set the gain mode";
    case FPGA_ERR_RX_ATT_VALUE: return "Failed to set the gain";
    case FPGA_ERR_PTT_LEN: return "Failed to set the PTT power statistics length";
    case FPGA_ERR_PTT_GATE: return "Failed to configure the PTT threshold";
    case FPGA_ERR_PTT_STATE: return "Failed to query the radio status";
    case FPGA_ERR_L_ADC: return "Failed to query the radio power";
    case FPGA_ERR_JT_ATT_SEL: return "Failed to trigger the demodulation attenuation";
    case FPGA_ERR_JT_ATT_DATA: return "Failed to set the demodulation attenuation 1";
    case FPGA_ERR_JT_ATT2_DATA: return "Failed to set the demodulation attenuation 2";
    case FPGA_ERR_JT_ATT_TX_EN: return "Failed to turn on the demodulation switch";
    case FPGA_ERR_CH_ATT_SEL: return "Failed to trigger the channel attenuation";
    case FPGA_ERR_CH_ATT_DATA: return "Failed to set the channel attenuation 1";
    case FPGA_ERR_CH_ATT2_DATA: return "Failed to set the channel attenuatio 2";
    case FPGA_ERR_CH_ATT_TX_EN: return "Failed to turn on the channel 2 switch";
    case FPGA_ERR_CH_ATT_V1V2: return "Failed to turn on the channel 4 switch";
    case FPGA_ERR_AXIS_RELOAD0_DATA: return "Failed to set the low-pass filter";
    case FPGA_ERR_AXIS_RELOAD1_DATA: return "Failed to set the band-stop filter";
    case FPGA_ERR_CHNL_FREQ: return "Failed to set the channel frequency shift";
    case FPGA_ERR_DELAY: return "Failed to set the path delay";
    case FPGA_ERR_DPL_FDI: return "Failed to set the spread spectrum I";
    case FPGA_ERR_DPL_FDQ: return "Failed to set the spread spectrum Q";
    case FPGA_ERR_DPL_DFS: return "Failed to set the Doppler frequency shift";
    case FPGA_ERR_GAIN: return "Failed to set the path gain";
    case FPGA_ERR_DPL_BYPASS: return "Failed to set the bypass switch";
    case FPGA_ERR_GR_ATT_SEL: return "Failed to trigger the demodulation attenuation";
    case FPGA_ERR_GR_ATT_DATA: return "Failed to set the demodulation attenuation 1";
    case FPGA_ERR_GR_ATT2_DATA: return "Failed to set the demodulation attenuation 2";
    case FPGA_ERR_GR_ATT_TX_EN: return "Failed to turn on the demodulation switch";
    default: return "Unknown error";
    }
}

/***************************************寄存器读写函数********************************************************/
int read_reg(FPGA_IDX idx, uint32_t reg_addr, uint32_t* out_value) {
    CTL_REG reg = {idx, reg_addr, 0};
    int ret;

    ret = ioctl(g_spi_fd, FPGA_GET_VALUE, &reg);
    if (ret < 0) {
        perror("ioctl FPGA_GET_VALUE failed");
        return -1;
    }

    *out_value = reg.value;
    //SO_DEBUG("addr:0x%X, value:0x%X", reg_addr, reg.value);
    return 0;
}

int write_reg(FPGA_IDX idx, uint32_t reg_addr, uint32_t value) {
    CTL_REG reg = {idx, reg_addr, value };
    int ret;

    // 写入
    ret = ioctl(g_spi_fd, FPGA_SET_VALUE, &reg);
    if (ret < 0) {
        perror("ioctl FPGA_SET_VALUE failed");
        return -1;
    }
    //SO_DEBUG("addr:0x%X, value:0x%X", reg_addr, value);

    // 立即读回验证
    uint32_t readback;
    if (read_reg(FPGA1, reg_addr, &readback) == 0) {
        if (readback != value) {
            SO_DEBUG("write fail, w_value:0x%x, r_value:0x%x", value, readback);
        }
    }

    return 0;
}

/****************************************增益控制函数********************************************************/
/*
    rf_adc 开关模式
    输入：通道 rs_in: RS_IN_1-RS_IN_4
          模式mode：0手动控制、1自动控制
*/
int set_rx_sw_mode(RS_IN_E rs_in, int mode) {
    if (rs_in >= RS_IN_MAX) {
        SO_DEBUG("invalid chl:%d", rs_in);
        return FPGA_ERR_INVALID_CHL;
    }

    uint32_t current_sw_mode;
    read_reg(FPGA1, REG_RX_SW_MODE, &current_sw_mode);

    uint32_t mask = 1U << rs_in;
    if (mode == 1) {
        current_sw_mode |= mask;
    } else {
        current_sw_mode &= ~mask;
    }

    write_reg(FPGA1, REG_RX_SW_MODE, current_sw_mode);
    return FPGA_OK;
}

/*
    rf-adc增益控制开关
    输入：通道 rs_in:0-3
          开关sw：0关、1开
*/
int set_rx_sw(RS_IN_E rs_in, bool sw) {
    if (rs_in >= RS_IN_MAX) {
        SO_DEBUG("invalid chl:%d", rs_in);
        return FPGA_ERR_INVALID_CHL;
    }

    uint32_t current_sw_value;
    read_reg(FPGA1, REG_RX_SWITCH, &current_sw_value);
    uint32_t mask = 1U << rs_in;
    if (sw == 1) {
        current_sw_value |= mask;
    } else {
        current_sw_value &= ~mask;
    }

    set_rx_sw_mode(rs_in, 0);
    write_reg(FPGA1, REG_RX_SWITCH, current_sw_value);
    return FPGA_OK;
}


/*
    rf_adc 手动增益衰减控制
        输入：通道 rs_in:0-3
            控制 enable: true自动、false手动
*/
int set_rx_att_auto(RS_IN_E rs_in, bool enable) {

    if (rs_in >= RS_IN_MAX) {
        SO_DEBUG("invalid chl:%d", rs_in);
        return FPGA_ERR_INVALID_CHL;
    }

    write_reg(FPGA1, REG_RX_ATT_MODE, !enable);

    return FPGA_OK;
}

/*
    rf_adc 手动增益衰减控制
        输入：通道 rs_in:0-3
              衰减值att:0.0-31.5
*/
int set_rx_att_value(RS_IN_E rs_in, float att) {
    int ret;
    uint32_t mode_value;
    uint32_t att_code;
    uint32_t rx_att_value;

    if (rs_in >= RS_IN_MAX) {
        SO_DEBUG("invalid chl:%d", rs_in);
        return FPGA_ERR_INVALID_CHL;
    }

    set_rx_att_auto(rs_in, false);

    if (att < 0.0f) {
        att = 0.0f;
    }
    else if (att > 31.5f) {
        att = 31.5f;
    }
    att_code = (uint32_t)(att * 2.0f + 0.5f);
    switch (rs_in) {
    case 0: rx_att_value = att_code | 0x0000;
        break;
    case 1: rx_att_value = att_code | 0x1000;
        break;
    case 2: rx_att_value = att_code | 0x2000;
        break;
    case 3: rx_att_value = att_code | 0x3000;
        break;
    }
    write_reg(FPGA1,REG_RX_ATT_VALUE, rx_att_value);

    // 转换为6位控制码 (0.5dB步进)
    att_code = (uint32_t)(att * 2.0f + 0.5f);
    switch (rs_in) {
    case 0: rx_att_value = att_code | 0x0100;
        break;
    case 1: rx_att_value = att_code | 0x1100;
        break;
    case 2: rx_att_value = att_code | 0x2100;
        break;
    case 3: rx_att_value = att_code | 0x3100;
        break;
    }

    //先写0，再写衰减值
    write_reg(FPGA1, REG_RX_ATT_VALUE, rx_att_value);
    return FPGA_OK;
}
/****************************************PTT控制********************************************************/
/*
    rf_adc 自动增益控制，功率统计长度 att_len
*/
int set_att_len(int att_len) {
    uint32_t len_reg = (uint32_t)att_len;
    write_reg(FPGA1, REG_ATT_LEN, len_reg);
    return FPGA_OK;
}

/*
    rf_adc ptt门限设置
    输入：长度len
          功率power
*/
int set_att_h_gate( float dbfs) {
    const double   scale = (1ULL << 23);  // 2^23
    int64_t power;
    int32_t power_l;
    int32_t power_h;

    uint32_t len_reg;
    read_reg(FPGA1, REG_ATT_LEN, &len_reg);

    //门限功率计算
    power = (uint64_t)round(pow(10.0, 0.1 * dbfs) * len_reg * scale);
    power_l = (uint32_t)(power & 0xFFFFFFFFULL);
    power_h = (uint32_t)((power >> 32) & 0xFFFFFFFFULL);

    write_reg(FPGA1, REG_ATT_H_GATE_L, power_l);
    write_reg(FPGA1, REG_ATT_H_GATE_H, power_h);
}

/*
    rf_adc att门限设置
    输入: 功率power
*/
int set_att_l_gate(float dbfs) {
    const double   scale = (1ULL << 23);  // 2^23
    int64_t power;
    int32_t power_l;
    int32_t power_h;

    uint32_t len_reg;
    read_reg(FPGA1, REG_ATT_LEN, &len_reg);

    //门限功率计算
    power = (uint64_t)round(pow(10.0, 0.1 * dbfs) * len_reg * scale);
    power_l = (uint32_t)(power & 0xFFFFFFFFULL);
    power_h = (uint32_t)((power >> 32) & 0xFFFFFFFFULL);

    write_reg(FPGA1, REG_ATT_L_GATE_L, power_l);
    write_reg(FPGA1, REG_ATT_L_GATE_H, power_h);
    return FPGA_OK;
}



/*低速adc查询*/
int get_low_adc(struct low_adc *lowadc) {
    uint32_t lowadc_value[4];
    for (int i = 0; i < 4; i++) {
        read_reg(FPGA1, LOW_ADC[i], &lowadc_value[i]);
        lowadc->low_adc_buf[i] = lowadc_value[i];
    }
}

/*
    rf-adc 状态和功率检测,直接获取4个电台的功率
    输入：无
    输出：电台状态 radio_sta;  //0001 :4321  1发送、4接收
          电台功率 radio_power[4];
*/
int get_ptt_sta_power(struct radios* dt) {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用get_ptt_sta_power()";
    // 测试数组和索引计数器
    static unsigned char test_ptt[15] = { 0x1, 0x3, 0xc,  0x8, 0xa, 0xd, 0x7, 0xe, 0x6, 0x2, 0xc, 0x4, 0xb, 0x5, 0x9 };
    static int index = 0;

    if (dt != nullptr) {
        // 从数组中循环取值
        dt->radio_sta = test_ptt[index];
        qDebug() << "测试模式: radio_sta = 0x" << QString::number(dt->radio_sta, 16);

        // 更新索引，使其循环
        index = (index + 1) % 15;
    }

    return FPGA_OK;
#endif
    int ret;
    uint32_t ptt_state;
    uint32_t power_value_l;
    uint32_t power_value_h;
    uint64_t power_value;
    uint64_t reg_power_value;
    uint32_t len;
    const double   scale = (1ULL << 22);  // 2^22

    read_reg(FPGA1, REG_PTT_STATE, &ptt_state);
    dt->radio_sta = ptt_state & 0XF;
    read_reg(FPGA1, REG_ATT_LEN, &len);
    for (int i = 0; i < 4; i++) {
        write_reg(FPGA1, REG_ATT_RD_CH_SEL,(0x1<<i));
        read_reg(FPGA1, REG_ATT_POWER_L, &power_value_l);
        read_reg(FPGA1, REG_ATT_POWER_H, &power_value_h);
        power_value = ((uint64_t)power_value_h << 32) | power_value_l;
        reg_power_value = 10 * log10(power_value / (len * scale)) ;
        dt->radio_power[i] = reg_power_value;
    }

    return FPGA_OK;
}


int get_rx_att(RS_IN_E rs_in, float* rx_att) {
    uint32_t reg_att;

    if (rs_in >= RS_IN_MAX) {
        SO_DEBUG("invalid chl:%d", rs_in);
        return FPGA_ERR_INVALID_CHL;
    }

    if (rx_att == NULL) {
        SO_DEBUG("null pointer");
        return FPGA_ERR_NULL_P;
    }

    read_reg(FPGA1, REG_CURR_ATT, &reg_att);
    int shift = rs_in * 8;
    int curr_att = (reg_att >> shift) & 0xFF;
    *rx_att = (float)curr_att / 2.0f;
    return FPGA_OK; // 成功
}

int get_all_rx_att(float* rs_in_1_att, float* rs_in_2_att, float* rs_in_3_att, float* rs_in_4_att) {
    uint32_t reg_att;

    if (rs_in_1_att == NULL || rs_in_2_att == NULL || rs_in_3_att == NULL || rs_in_4_att == NULL) {
        SO_DEBUG("null pointer");
        return FPGA_ERR_NULL_P;
    }

    read_reg(FPGA1, REG_CURR_ATT, &reg_att);
    int curr_att = reg_att & 0xFF;
    *rs_in_1_att = (float)curr_att / 2.0f;

    curr_att = (reg_att >> 8) & 0xFF;
    *rs_in_2_att = (float)curr_att / 2.0f;

    curr_att = (reg_att >> 16) & 0xFF;
    *rs_in_3_att = (float)curr_att / 2.0f;

    curr_att = (reg_att >> 24) & 0xFF;
    *rs_in_4_att = (float)curr_att / 2.0f;

    return FPGA_OK; // 成功
}

/*
    PTT检测门限
    输入：700mv
*/
int set_ptt_gate(int v_value) {
    write_reg(FPGA1, REG_PTT_GATE, v_value);
    return FPGA_OK;
}

/*
    低速ADC的fpga自动读取周期,看电台的收发
    参数转换
*/
int set_ladc_tap(int tap_clk) {
    if (tap_clk <= 512) {
        tap_clk = 512 + 1;
    }
    write_reg(FPGA1, REG_LADC_TAP, tap_clk);
    return FPGA_OK;
}

/****************************************解调控制函数********************************************************/
/*
    rf-dac解调开关控制
    输入：通道 rs_jt:0-3
          开关sw：0关、1开
*/
int set_jt_sw(RS_JT_E rs_jt, bool sw) {
    uint32_t current_sw_value;

    if (rs_jt >= RS_JT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_jt);
        return FPGA_ERR_INVALID_CHL;
    }

    read_reg(FPGA1, REG_JT_ATT_TX_EN, &current_sw_value);
    uint32_t mask = 1U << rs_jt;
    if (sw == true) {
        current_sw_value |= mask;
    } else {
        current_sw_value &= ~mask;
    }

    write_reg(FPGA1, REG_JT_ATT_TX_EN, current_sw_value);
    return FPGA_OK;

}

/*
    rf-dac解调衰减控制
        输入：通道 rs_jt:0-3
              衰减值att:0.0-31.5
*/
int set_jt_att_value(RS_JT_E rs_jt, float att) {
    int ret;
    float att2;
    uint32_t jt_att_value;
    uint32_t jt_att2_value;

    uint32_t le_value = 0x00;
    uint32_t le2_value = 0x00;

    if (rs_jt > 3) {
        SO_DEBUG("invalid chl:%d", rs_jt);
        return FPGA_ERR_INVALID_CHL;
    }

    if (att < 0.0f) {
        att = 0.0f;
    } else if (att > 63.0f) {
        att = 63.0f;
    }

    if (0.0f <= att && att <= 31.5f) {
        //衰减器1配置
        jt_att_value = (uint32_t)(att * 2.0f + 0.5f);
        write_reg(FPGA1, REG_JT_ATT_DATA, jt_att_value);

        le_value |= 0x1 << (rs_jt * 2);
        write_reg(FPGA1, REG_JT_ATT_SEL, 0x00);
        write_reg(FPGA1, REG_JT_ATT_SEL, le_value);

        //衰减器2配0
        write_reg(FPGA1, REG_JT_ATT_DATA, 0x00);

        le2_value |= 0x2 << (rs_jt * 2);
        write_reg(FPGA1, REG_JT_ATT_SEL, 0x00);
        write_reg(FPGA1, REG_JT_ATT_SEL, le2_value);
    } else if (31.5f < att && att <= 63.0f) {
        // 第一个衰减器衰减31.5
        write_reg(FPGA1, REG_JT_ATT_DATA, 0x3f);

        le_value |= 0x1 << (rs_jt * 2);
        write_reg(FPGA1, REG_JT_ATT_SEL, 0x00);
        write_reg(FPGA1, REG_JT_ATT_SEL, le_value);

        // 第二个衰减器衰减att-31.5
        att2 = att - 31.5f;
        jt_att2_value = (uint32_t)(att2 * 2.0f + 0.5f);
        write_reg(FPGA1, REG_JT_ATT_DATA, jt_att2_value);

        le2_value |= 0x2 << (rs_jt * 2);
        write_reg(FPGA1, REG_JT_ATT_SEL, 0x00);
        write_reg(FPGA1, REG_JT_ATT_SEL, le2_value);
    }
    return FPGA_OK;
}

/****************************************CHX控制函数********************************************************/

/*
    CHX二选一开关
    输入：DAC的0-3通道，开关
*/
int set_chl_sw(RS_OUT_E rs_out, bool sw) {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用set_chl_sw()"<<"rs_out"<<rs_out<<"sw"<<sw;
    return FPGA_OK;
#endif
    uint32_t current_sw_value;

    if (rs_out > RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    read_reg(FPGA1, REG_CH_ATT_TX_EN, &current_sw_value);
    uint32_t mask = 1U << rs_out;
    if (sw == 1) {
        current_sw_value |= mask;
    } else {
        current_sw_value &= ~mask;
    }
    write_reg(FPGA1, REG_CH_ATT_TX_EN, current_sw_value);
    return FPGA_OK;

}

/*
    CHX四通道选路切换开关
    输入：DAC的0-3通道，开关(1-4)
    0：合路器1，01：合路器2，10：合路器3，11：合路器4
*/
int set_chl_sw4(RS_OUT_E rs_out, int sw) {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用set_chl_sw4()"<<"rs_out"<<rs_out<<"sw"<<sw;
    return FPGA_OK;
#endif
    uint32_t current_val;

    if (rs_out > RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    read_reg(FPGA1, REG_CH_ATT_V1V2, &current_val);

    // 2. 计算该通道的位偏移（每通道占2位）
    int shift = rs_out * 2;

    // 3. 清除该通道的2位
    uint32_t mask = 0x3U << shift;          // 0x3 = 0b11
    current_val &= ~mask;                   // 清零目标2位

    // 4. 设置新值（注意：sw=1→00, sw=2→01, sw=3→10, sw=4→11）
    uint32_t new_field;
    if (sw == 1) {
        new_field = 0x0;  // 00
    } else if (sw == 2) {
        new_field = 0x1;  // 01
    } else if (sw == 3) {
        new_field = 0x2;  // 10
    } else if (sw == 4) {
        new_field = 0x3;  // 11
    } else {
        SO_DEBUG("invalid sw value:%d", sw);
    }

    current_val |= (new_field << shift);
    write_reg(FPGA1, REG_CH_ATT_V1V2, current_val);
    return FPGA_OK;
}

/*
    通道衰减控制
    输入：AD的0-3通道，衰减值(写入衰减的浮点值)0-61
*/
int set_chl_att(RS_OUT_E rs_out, float att) {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用set_chl_att()"<<"rs_out"<<rs_out<<"att"<<att;
    return FPGA_OK;
#endif
    int ret;
    float att2;
    uint32_t ch_att_value;
    uint32_t ch_att2_value;
    uint32_t le_value = 0x00;
    uint32_t le2_value = 0x00;

    if (rs_out > RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    if (att < 0.0f) {
        att = 0.0f;
    } else if (att > 63.0f) {
        att = 63.0f;
    }

    if (0.0f <= att && att <= 31.5f) {
        //衰减器1配置
        ch_att_value = (uint32_t)(att * 2.0f + 0.5f);
        write_reg(FPGA1, REG_CH_ATT_DATA, ch_att_value);

        le_value |= 0x1 << (rs_out * 2);
        write_reg(FPGA1, REG_CH_ATT_SEL, 0x00);
        write_reg(FPGA1, REG_CH_ATT_SEL, le_value);

        //衰减器2配0
        write_reg(FPGA1, REG_CH_ATT_DATA, 0x00);

        le2_value |= 0x2 << (rs_out * 2);
        write_reg(FPGA1, REG_CH_ATT_SEL, 0x00);
        write_reg(FPGA1, REG_CH_ATT_SEL, le2_value);
    } else if (31.5f < att && att <= 63.0f) {
        // 第一个衰减器衰减31.5
        write_reg(FPGA1, REG_CH_ATT_DATA, 0x3f);

        le_value |= 0x1 << (rs_out * 2);
        write_reg(FPGA1, REG_CH_ATT_SEL, 0x00);
        write_reg(FPGA1, REG_CH_ATT_SEL, le_value);

        // 第二个衰减器衰减att-31.5
        att2 = att - 31.5f;
        ch_att2_value = (uint32_t)(att2 * 2.0f + 0.5f);
        write_reg(FPGA1, REG_CH_ATT_DATA, ch_att2_value);

        le2_value |= 0x2 << (rs_out * 2);
        write_reg(FPGA1, REG_CH_ATT_SEL, 0x00);
        write_reg(FPGA1, REG_CH_ATT_SEL, le2_value);
    }
    return FPGA_OK;
}


/**************************************信道模拟寄存器控制*********************************************************************/

/*
DAC输出来源:dac_n为0-3，  channel_sel表示DAC输出来源取值为0-9
*/

int set_chl_out_sel(RS_OUT_E rs_out, DATA_SRC src_sel) {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用set_chl_out_sel()"<<"rs_out"<<rs_out<<"src_sel"<<src_sel;
    return FPGA_OK;
#endif
    int ret;
    uint32_t old_value;
    uint32_t new_value;

    read_reg(FPGA1, REG_DAC_OUT_SEL, &old_value);
    switch (src_sel)
    {
    case DATA_SRC_NONE:
        new_value = old_value & ~(0xFU << (4 * rs_out));
        break;
    case DATA_SRC_ADC1_ALG:
        new_value = (old_value & ~(0xFU << (4 * rs_out))) | (1 << (4 * rs_out));
        break;
    case DATA_SRC_ADC2_ALG:
        new_value = (old_value & ~(0xFU << (4 * rs_out))) | (2 << (4 * rs_out));
        break;
    case DATA_SRC_ADC3_ALG:
        new_value = (old_value & ~(0xFU << (4 * rs_out))) | (3 << (4 * rs_out));
        break;
    case DATA_SRC_ADC4_ALG:
        new_value = (old_value & ~(0xFU << (4 * rs_out))) | (4 << (4 * rs_out));
        break;
    case DATA_SRC_ADC1:
        new_value = (old_value & ~(0xFU << (4 * rs_out))) | (5 << (4 * rs_out));
        break;
    case DATA_SRC_ADC2:
        new_value = (old_value & ~(0xFU << (4 * rs_out))) | (6 << (4 * rs_out));
        break;
    case DATA_SRC_ADC3:
        new_value = (old_value & ~(0xFU << (4 * rs_out))) | (7 << (4 * rs_out));
        break;
    case DATA_SRC_ADC4:
        new_value = (old_value & ~(0xFU << (4 * rs_out))) | (8 << (4 * rs_out));
        break;
    case DATA_SRC_SINE:
        new_value = (old_value & ~(0xFU << (4 * rs_out))) | (9 << (4 * rs_out));
        break;

    default: new_value = old_value & ~(0xFU << (4 * rs_out));
        break;
    }

    write_reg(FPGA1, REG_DAC_OUT_SEL, new_value);
    return FPGA_OK;
}

int set_jt_out_sel(RS_JT_E rs_jt, DATA_SRC src_sel) {
    int ret;
    uint32_t old_value;
    uint32_t new_value;

    uint32_t offset = rs_jt + 4;

    read_reg(FPGA1, REG_DAC_OUT_SEL, &old_value);
    switch (src_sel)
    {
    case DATA_SRC_NONE:
        new_value = old_value & ~(0xFU << (4 * offset));
        break;
    case DATA_SRC_ADC1_ALG:
        new_value = (old_value & ~(0xFU << (4 * offset))) | (1 << (4 * offset));
        break;
    case DATA_SRC_ADC2_ALG:
        new_value = (old_value & ~(0xFU << (4 * offset))) | (2 << (4 * offset));
        break;
    case DATA_SRC_ADC3_ALG:
        new_value = (old_value & ~(0xFU << (4 * offset))) | (3 << (4 * offset));
        break;
    case DATA_SRC_ADC4_ALG:
        new_value = (old_value & ~(0xFU << (4 * offset))) | (4 << (4 * offset));
        break;
    case DATA_SRC_ADC1:
        new_value = (old_value & ~(0xFU << (4 * offset))) | (5 << (4 * offset));
        break;
    case DATA_SRC_ADC2:
        new_value = (old_value & ~(0xFU << (4 * offset))) | (6 << (4 * offset));
        break;
    case DATA_SRC_ADC3:
        new_value = (old_value & ~(0xFU << (4 * offset))) | (7 << (4 * offset));
        break;
    case DATA_SRC_ADC4:
        new_value = (old_value & ~(0xFU << (4 * offset))) | (8 << (4 * offset));
        break;
    case DATA_SRC_SINE:
        new_value = (old_value & ~(0xFU << (4 * offset))) | (9 << (4 * offset));
        break;

    default: new_value = old_value & ~(0xFU << (4 * offset));
        break;
    }

    write_reg(FPGA1, REG_DAC_OUT_SEL, new_value);
    return FPGA_OK;
}

int set_dds(float freq) {
    double dds;
    double rounded_dds;
    uint32_t reg_dds;
    const double  max_freq = 125000000;      // 125000000 Hz
    const double  scale = (1ULL << 31);  // 2^31
    dds = (double)freq * scale / max_freq;
    rounded_dds = round(dds);
    reg_dds = (uint32_t)(int32_t)rounded_dds;

    write_reg(FPGA1, REG_DAC_dds, reg_dds);
    SO_DEBUG("freq:%f, rounded_dds:%lf, reg_dds:%u", freq, rounded_dds, reg_dds);
    return FPGA_OK;
}

int set_axis(RS_OUT_E rs_out, struct bs_axis* bs_axis_value)
{
    int ret;

    if (rs_out >= RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    write_reg(FPGA1, REG_AXIS_RELOAD1_START[rs_out], 0x0);
    write_reg(FPGA1, REG_AXIS_RELOAD1_START[rs_out], 0x1);

    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[0]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[1]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[2]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[3]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[4]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[5]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[6]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[7]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[8]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[9]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[10]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[11]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[12]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[13]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[14]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[15]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[16]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[17]);
    write_reg(FPGA1, REG_AXIS_RELOAD1_DATA[rs_out], (uint32_t)bs_axis_value->coeff[18]);

    write_reg(FPGA1, REG_AXIS_RELOAD1_END[rs_out], 0x0);
    write_reg(FPGA1, REG_AXIS_RELOAD1_END[rs_out], 0x1);
    return FPGA_OK;
}

//路径延时,单位ns
// channel_id 0-3
// path_id 0-4
int set_chl_delay(RS_OUT_E rs_out, ALG_PATH_E path, int delay) {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用set_chl_delay()"<<"rs_out"<<rs_out<<"path"<<path<<"delay"<<delay;
    return FPGA_OK;
#endif
    int ret;
    int delay_clk = delay / 8;

    if (rs_out >= RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    if (path > ALG_PATH_MAX) {
        SO_DEBUG("invalid path:%d", path);
        return FPGA_ERR_INVALID_PATH;
    }

    if (path == 0) {
        return FPGA_OK;
    }

    if (delay_clk > 8192) {
        delay_clk = 8192;
        SO_DEBUG("set_chl_delay delay overflow");
    }
    write_reg(FPGA1, REG_DELAY[rs_out][path], delay_clk);
    return FPGA_OK;
}


//频扩，输入频率值
int set_dpl_df(RS_OUT_E rs_out, ALG_PATH_E path, float freq) {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用set_dpl_df()"<<"rs_out"<<rs_out<<"path"<<path<<"freq"<<freq;
    return FPGA_OK;
#endif
    // 参数有效性检查
    if (path == 0) {
        return FPGA_OK;
    }

    if (rs_out >= RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    if (path >= ALG_PATH_MAX) {
        SO_DEBUG("invalid path:%d", path);
        return FPGA_ERR_INVALID_PATH;
    }

    const double  max_freq = 125000000;      // 125000000 Hz
    const double  scale = (1ULL << 31);  // 2^31
    float freq_value;
    double dpl_fd_i;
    double dpl_fd_q;
    double rounded_dpl_fd;
    double rounded_dpl_fdq;
    uint32_t reg_dpl_fd;
    uint32_t reg_dpl_fdq;
    float df_i[7];
    float df_q[8];
    int a = 0;
    int b = 0;  // 初始化 a 和 b 为 0
    freq_value = freq / 2;
    for (int i = 1; i <= 15; i++) {

        if (i % 2 == 0) {
            df_i[a] = (freq_value / 15) * i;

            dpl_fd_i = (double)df_i[a] * scale / max_freq;
            rounded_dpl_fd = round(dpl_fd_i);
            reg_dpl_fd = (uint32_t)(int32_t)rounded_dpl_fd;  //即使 reg_rounded 为负，也会正确转换为 uint32_t 的补码形式

            write_reg(FPGA1, REG_DPL_FDI[rs_out][path][a], reg_dpl_fd);
            a++;
        }
        else {
            df_q[b] = (freq_value / 15) * i;

            dpl_fd_q = (double)df_q[b] * scale / max_freq;
            rounded_dpl_fdq = round(dpl_fd_q);
            reg_dpl_fdq = (uint32_t)(int32_t)rounded_dpl_fdq;  //即使 reg_rounded 为负，也会正确转换为 uint32_t 的补码形式
            write_reg(FPGA1, REG_DPL_FDQ[rs_out][path][b], reg_dpl_fdq);
            b++;
        }
    }
    return FPGA_OK;
}


//多普勒频移
int set_dpl_dfs(RS_OUT_E rs_out, ALG_PATH_E path, float freq) {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用set_dpl_dfs()"<<"rs_out"<<rs_out<<"path"<<path<<"freq"<<freq;
    return FPGA_OK;
#endif
    int ret;
    uint32_t chl_freq;
    uint32_t dfs_init;
    uint32_t real_freq;
    const double  max_freq = 125000000;      // 125 MHz
    const double   scale = (1ULL << 31);  // 2^31
    double abs_freq = abs(freq);
    double dpl_dfs = (double)abs_freq * scale / max_freq;
    double rounded_dpl_dfs = round(dpl_dfs);
    // 3. 转为 32 位无符号整数（保留补码语义）
    // 即使 reg_rounded 为负，也会正确转换为 uint32_t 的补码形式
    uint32_t reg_dpl_dfs = (uint32_t)(int32_t)rounded_dpl_dfs;

    if (rs_out >= RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    if (path >= ALG_PATH_MAX) {
        SO_DEBUG("invalid path:%d", path);
        return FPGA_ERR_INVALID_PATH;
    }

    read_reg(FPGA1, REG_CHNL_FREQ[rs_out], &chl_freq);
    dfs_init = 0x0 - chl_freq;
    if (freq >= 0) {
        real_freq = dfs_init + reg_dpl_dfs;
    }
    else {
        real_freq = dfs_init - reg_dpl_dfs;

    }

    write_reg(FPGA1, REG_DPL_DFS[rs_out][path], real_freq);
    SO_DEBUG("freq:%f, real_freq:%u", freq, real_freq);
    return FPGA_OK;
}


//增益
int set_gain(RS_OUT_E rs_out, ALG_PATH_E path, float gain) {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用set_gain()"<<"rs_out"<<rs_out<<"path"<<path<<"gain"<<gain;
    return FPGA_OK;
#endif
    int ret;
    unsigned int reg_gain = pow(10, (gain / 10)) * 4096;

    if (rs_out > RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    if (path > ALG_PATH_MAX) {
        SO_DEBUG("invalid path:%d", path);
        return FPGA_ERR_INVALID_PATH;
    }

    write_reg(FPGA1, REG_gain[rs_out][path], reg_gain);
    return FPGA_OK;

}

/************************旁路开关控制*************************************/
/*
    先读再写
    参数：channel_id通道、r_axis带阻滤波
    0开，1关
*/
int set_bypass_raxis(RS_OUT_E rs_out, int r_axis_sw) {
    int ret;
    uint32_t old_value;
    uint32_t new_value;

    if (rs_out > RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    read_reg(FPGA1, REG_DPL_BYPASS[rs_out], &old_value);

    if (r_axis_sw == 1) {
        new_value = old_value | (1U << 12);
    }
    else {
        new_value = old_value & ~(1U << 12);
    }
    write_reg(FPGA1, REG_DPL_BYPASS[rs_out], new_value);

    return FPGA_OK;

}
/*
    先读再写
    参数：channel_id通道、iq_depart：iq分离
    0开，1关
*/
int set_bypass_iq(RS_OUT_E rs_out, int iq_depart_sw) {
    int ret;
    uint32_t old_value;
    uint32_t new_value;

    if (rs_out > RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    read_reg(FPGA1, REG_DPL_BYPASS[rs_out], &old_value);

    if (iq_depart_sw == 1) {
        new_value = old_value | (1U << 0);
    }
    else {
        new_value = old_value & ~(1U << 0);
    }
    write_reg(FPGA1, REG_DPL_BYPASS[rs_out], new_value);

    return FPGA_OK;

}
/*
    先读再写
    参数：rs_out通道、l_axis低通滤波
    0开，1关
*/

int set_bypass_laxis(RS_OUT_E rs_out, int l_axis_sw) {
    int ret;
    uint32_t old_value;
    uint32_t new_value;

    if (rs_out > RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    read_reg(FPGA1, REG_DPL_BYPASS[rs_out], &old_value);

    if (l_axis_sw == 1) {
        new_value = old_value | (1U << 1);
    }
    else {
        new_value = old_value & ~(1U << 1);
    }
    write_reg(FPGA1, REG_DPL_BYPASS[rs_out], new_value);
    return FPGA_OK;

}
/*
    旁路开关控制,.先读再写
    参数：rs_out 通道、path_id路径、dfs频移、fd_sw频扩、
    */
int set_bypass_dpl_iq(RS_OUT_E rs_out, ALG_PATH_E path_id, int dfs_sw, int fd_sw) {
    int ret;
    uint32_t old_value;
    uint32_t new_value;

    if (rs_out > RS_OUT_MAX) {
        SO_DEBUG("invalid chl:%d", rs_out);
        return FPGA_ERR_INVALID_CHL;
    }

    read_reg(FPGA1, REG_DPL_BYPASS[rs_out], &old_value);

    uint32_t new_bits = (dfs_sw << 1) | fd_sw;  // 自动组合成 00, 01, 10, 11
    int shift = path_id * 2;
    uint32_t mask = ~(0x03U << shift);
    new_value = (old_value & mask) | (new_bits << shift);
    write_reg(FPGA1, REG_DPL_BYPASS[rs_out], new_value);
    return FPGA_OK;
}


/**************************************干扰寄存器控制*********************************************************************/
/*
    干扰控制开关
    输入：DA的1-5通道，开关0 1

*/
int set_gr_sw(GR_OUT_E gr_out, bool sw) {
    uint32_t current_sw_value;

    read_reg(FPGA2, REG_GR_ATT_TX_EN, &current_sw_value);
    uint32_t mask = 1U << gr_out;
    if (sw == true) {
        current_sw_value |= mask;
    } else {
        current_sw_value &= ~mask;
    }
    write_reg(FPGA2, REG_GR_ATT_TX_EN, current_sw_value);
    return FPGA_OK;
}
/*
    干扰衰减控制
    输入：DA的1-5通道，衰减值(写入衰减的浮点值)0-61
*/
int set_gr_att(GR_OUT_E gr_out, float att) {
    int ret;
    float att2;
    uint32_t gr_att_value;
    uint32_t gr_att2_value;

    uint32_t le_value = 0x00;
    uint32_t le2_value = 0x00;

    if (att < 0.0f) {
        att = 0.0f;
    }
    else if (att > 63.0f) {
        att = 63.0f;
    }
    if (0.0f <= att && att <= 31.5f) {
        //衰减器1配置
        gr_att_value = (uint32_t)(att * 2.0f + 0.5f);
        write_reg(FPGA2, REG_GR_ATT_DATA, gr_att_value);

        le_value |= 0x1 << (gr_out * 2);
        write_reg(FPGA2, REG_GR_ATT_SEL, 0x00);
        write_reg(FPGA2, REG_GR_ATT_SEL, le_value);

        //衰减器2配0
        write_reg(FPGA2, REG_GR_ATT_DATA, 0x00);

        le2_value |= 0x2 << (gr_out * 2);
        write_reg(FPGA2, REG_GR_ATT_SEL, 0x00);
        write_reg(FPGA2, REG_GR_ATT_SEL, le2_value);
    }
    else if (31.5f < att && att <= 63.0f) {
        // 第一个衰减器衰减31.5
        write_reg(FPGA2, REG_GR_ATT_DATA, 0x3f);

        le_value |= 0x1 << (gr_out * 2);
        write_reg(FPGA2, REG_GR_ATT_SEL, 0x00);
        write_reg(FPGA2, REG_GR_ATT_SEL, le_value);

        // 第二个衰减器衰减att-31.5
        att2 = att - 31.5f;
        gr_att2_value = (uint32_t)(att2 * 2.0f + 0.5f);
        write_reg(FPGA2, REG_GR_ATT_DATA, gr_att2_value);

        le2_value |= 0x2 << (gr_out * 2);
        write_reg(FPGA2, REG_GR_ATT_SEL, 0x00);
        write_reg(FPGA2, REG_GR_ATT_SEL, le2_value);
    }
    return FPGA_OK;
}


/**************************************初始化*********************************************************************/

int fpga_init() {
#ifdef  USE_FPGA_TEST
    qDebug() << "成功调用fpga_init()";
    return FPGA_OK;
#endif
    //打开设备
    int ret = open_device();
    if (ret != FPGA_OK) {
        return -1;
    }

    for (int rs_in = RS_IN_1; rs_in < RS_IN_MAX; rs_in++) {
        //开关模式设置为自动
        ret = set_rx_sw_mode((RS_IN_E)rs_in, 1);
        if (ret != FPGA_OK) {
            return ret;
        }

        //设置ptt门限

        //设置自动增益控制门限（高门限和低门限）

        //通道数据源初始化为空
        set_chl_out_sel((RS_OUT_E)rs_in, DATA_SRC_NONE);

        //
        // ret = set_rx_sw((RS_IN_E)rs_in, 1);
        // if (ret != FPGA_OK) {
        //     return ret;
        // }

        set_rx_att_auto((RS_IN_E)rs_in, true);

        //设置输入增益为0，并关闭自动增益控制
        // ret = set_rx_att_value((RS_IN_E)rs_in, 0.0);
        // if (ret != FPGA_OK) {
        //     return ret;
        // }
    }

    for (int rs_jt = RS_JT_1; rs_jt < RS_JT_MAX; rs_jt++) {
        ret = set_jt_sw((RS_JT_E)rs_jt, 0);
        if (ret != FPGA_OK) {
            return ret;
        }

        ret = set_jt_att_value((RS_JT_E)rs_jt, 0.0);
        if (ret != FPGA_OK) {
            return ret;
        }

        set_jt_out_sel((RS_JT_E)rs_jt, DATA_SRC_NONE);
    }

    for (int rs_out = RS_OUT_1; rs_out < RS_OUT_MAX; rs_out++) {
        ret = set_chl_sw((RS_OUT_E)rs_out, 1);
        if (ret != FPGA_OK) {
            return ret;
        }

        ret = set_chl_sw4((RS_OUT_E)rs_out, rs_out);
        if (ret != FPGA_OK) {
            return ret;
        }

        ret = set_chl_att((RS_OUT_E)rs_out, 0);
        if (ret != FPGA_OK) {
            return ret;
        }

        // for (int path = ALG_PATH_1; path < ALG_PATH_MAX; path++) {
        //     ret = set_dpl_dfs((RS_OUT_E)rs_out, (ALG_PATH_E)path, 0.0);
        //     if (ret != FPGA_OK) {
        //         return ret;
        //     }

        //     ret = set_dpl_df((RS_OUT_E)rs_out, (ALG_PATH_E)path, 0.0);
        //     if (ret != FPGA_OK) {
        //         return ret;
        //     }

        //     ret = set_bypass_dpl_iq((RS_OUT_E)rs_out, (ALG_PATH_E)path, 1, 1);
        //     if (ret != FPGA_OK) {
        //         return ret;
        //     }
        //     usleep(10000);
        // }

        ret = set_bypass_raxis((RS_OUT_E)rs_out, 1);
        if (ret != FPGA_OK) {
            return ret;
        }

        ret = set_bypass_iq((RS_OUT_E)rs_out, 1);
        if (ret != FPGA_OK) {
            return ret;
        }

        ret = set_bypass_laxis((RS_OUT_E)rs_out, 1);
        if (ret != FPGA_OK) {
            return ret;
        }
    }

    for (int gr_out = GR_OUT_1; gr_out < GR_OUT_MAX; gr_out++) {
        ret = set_gr_sw((GR_OUT_E)gr_out, 1);
        if (ret != FPGA_OK) {
            return ret;
        }

        ret = set_gr_att((GR_OUT_E)gr_out, 0.0);
        if (ret != FPGA_OK) {
            return ret;
        }
    }

    ret = set_ptt_gate(700);
    ret = set_ladc_tap(2000);
    return FPGA_OK;
}

int fpga_deinit()
{
    close_device();
}

/**************************************主函数*********************************************************************/


void help(char* p) {
    SO_DEBUG("[help] Usage:");
    SO_DEBUG("[help] read_reg [fpga] [addr]");
    SO_DEBUG("[help] write_reg [fpga] [addr] [value]");
    SO_DEBUG("[help] set_rx_sw_mode [rs_in] [mode]");
    SO_DEBUG("[help] set_rx_sw [rs_in] [sw]");
    SO_DEBUG("[help] set_rx_att_auto [rs_in] [enable]");
    SO_DEBUG("[help] set_rx_att_value [rs_in] [att]");
    SO_DEBUG("[help] set_att_len [len]");
    SO_DEBUG("[help] set_att_h_gata [dbfs]");
    SO_DEBUG("[help] set_att_l_gata [dbfs]");
    SO_DEBUG("[help] get_low_adc");
    SO_DEBUG("[help] get_ptt_sta_power ");
    SO_DEBUG("[help] get_rx_att [rs_in]");
    SO_DEBUG("[help] set_ptt_gate [v_value]");
    SO_DEBUG("[help] set_ladc_tap [tap_clk]");
    SO_DEBUG("[help] set_jt_sw [rs_jt] [sw]");
    SO_DEBUG("[help] set_jt_att_value [rs_jt] [att]");
    SO_DEBUG("[help] set_chl_sw [rs_out] [sw]");
    SO_DEBUG("[help] set_chl_sw4 [rs_out] [sw]");
    SO_DEBUG("[help] set_chl_att [rs_out] [att]");
    SO_DEBUG("[help] set_chl_out_sel [rs_out] [src_sel]");
    SO_DEBUG("[help] set_jt_out_sel [rs_jt] [src_sel]");
    SO_DEBUG("[help] set_dds [freq]");
    SO_DEBUG("[help] set_axis [rs_out]");
    SO_DEBUG("[help] set_chl_delay [rs_out] [path] [delay]");
    SO_DEBUG("[help] set_dpl_df [rs_out] [path] [freq]");
    SO_DEBUG("[help] set_dpl_dfs [rs_out] [path] [freq]");
    SO_DEBUG("[help] set_gain [rs_out] [path] [gain]");
    SO_DEBUG("[help] set_bypass_raxis [rs_out] [r_axis_sw]");
    SO_DEBUG("[help] set_bypass_iq [rs_out] [iq_depart_sw]");
    SO_DEBUG("[help] set_bypass_laxis [rs_out] [l_axis_sw]");
    SO_DEBUG("[help] set_bypass_dpl_iq [rs_out] [path] [dfs_sw] [fd_sw]");
}

// #define _TEST_
#ifdef _TEST_

int main(int argc, char *argv[]) {
    if (open_device() < 0) {
        printf("Device open failed");
        return 1;
    }

    struct bs_axis cfg;
    cfg.coeff[0] = 0xff01;
    cfg.coeff[1] = 0xff24;
    cfg.coeff[2] = 0x0052;
    cfg.coeff[3] = 0xfe42;
    cfg.coeff[4] =  0x0086;
    cfg.coeff[5] = 0x01e0;
    cfg.coeff[6] =  0x0176;
    cfg.coeff[7] = 0x0127;
    cfg.coeff[8] = 0x0258;
    cfg.coeff[9] = 0xfee5;
    cfg.coeff[10] =  0xfa38;
    cfg.coeff[11] = 0xfccf;
    cfg.coeff[12] = 0xff37;
    cfg.coeff[13] = 0xf9fe;
    cfg.coeff[14] = 0xffe8;
    cfg.coeff[15] = 0x174d;
    cfg.coeff[16] = 0x18ba;
    cfg.coeff[17] = 0xf10f;
    cfg.coeff[18] = 0x5592;

    uint32_t read_out;
    int32_t iv1;
    int32_t iv2;
    int32_t iv3;
    int32_t iv4;
    int32_t iv5;
    int32_t iv6;

    unsigned long uiv1;
    unsigned long uiv2;
    unsigned long uiv3;
    unsigned long uiv4;
    unsigned long uiv5;
    unsigned long uiv6;

    float fv1;
    float fv2;
    float fv3;
    float fv4;
    float fv5;
    float fv6;

    printf("argc:%d\r\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]:%s\r\n", i, argv[i]);
    }

    struct radios dt = { 0 };
    struct low_adc lowadc;
    float att;
    char *cmd = argv[1];

    if (strcmp(cmd, "read_reg") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = strtol(argv[3], NULL, 16);
        read_reg(iv1 == 1 ? FPGA1 : FPGA2, iv2,&read_out);
        printf("read_reg fpga:%d addr:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "write_reg") == 0 && argc == 5) {
        iv1 = atoi(argv[2]);
        iv2 = strtol(argv[3], NULL, 16);
        iv3 = strtol(argv[4], NULL, 16);
        write_reg(iv1 == 1 ? FPGA1 : FPGA2, iv2, iv3);
        printf("write_reg fpga:%d addr:%d value:%d \r\n", iv1, iv2, iv3);
    } else if (strcmp(cmd, "set_rx_sw_mode") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_rx_sw_mode((RS_IN_E)iv1, iv2);
        printf("set_rx_sw_mode chl:%d mode:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_rx_sw") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_rx_sw((RS_IN_E)iv1, iv2);
        printf("set_rx_sw chl:%d sw:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_rx_att_auto") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_rx_att_auto((RS_IN_E)iv1, (bool)iv2);
        printf("set_rx_att_value chl:%d att:%f\r\n", iv1, fv2);
    } else if (strcmp(cmd, "set_rx_att_value") == 0 && argc == 4) {
        iv1 = (int32_t)atoi(argv[2]);
        fv2 = atof(argv[3]);
        set_rx_att_value((RS_IN_E)iv1, fv2);
        printf("set_rx_att_value chl:%d att:%f\r\n", iv1, fv2);
    } else if (strcmp(cmd, "set_att_len") == 0 && argc == 3) {
        iv1 = atof(argv[2]);
        set_att_len(iv1);
        printf("set_att_len");
    } else if (strcmp(cmd, "set_att_h_gate") == 0 && argc == 3) {
        fv1 = atof(argv[2]);
        set_att_h_gate(fv1);
        printf("set_att_h_gate");
    } else if (strcmp(cmd, "set_att_l_gate") == 0 && argc == 3) {
        fv1 = atof(argv[2]);
        set_att_l_gate(fv1);
        printf("set_att_l_gata" );
    } else if (strcmp(cmd, "get_low_adc") == 0 && argc == 2) {
        get_low_adc(&lowadc);
        printf("get_low_adc\r\n");
    } else if (strcmp(cmd, "get_ptt_sta_power") == 0 && argc == 2) {
        get_ptt_sta_power(&dt);
        printf("get_ptt_sta_power\r\n");
    } else if (strcmp(cmd, "get_rx_att") == 0 && argc == 3) {
        iv1 = atoi(argv[3]);
        get_rx_att((RS_IN_E)iv1, &att);
        printf("get_rx_att att:%d\r\n", iv1);
    } else if (strcmp(cmd, "set_ptt_gate") == 0 && argc == 3) {
        iv1 = strtol(argv[2], NULL, 16);
        set_ptt_gate(iv1);
        printf("set_ptt_gata v_value:%d\r\n", iv1);
    } else if (strcmp(cmd, "set_ladc_tap") == 0 && argc == 3) {
        iv1 = strtol(argv[2], NULL, 16);
        set_ladc_tap(iv1);
        printf("set_ladc_tap clk:%d\r\n", iv1);
    } else if (strcmp(cmd, "set_jt_sw") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_jt_sw((RS_JT_E)iv1, iv2);
        printf("set_jt_sw chl:%d sw:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_jt_att_value") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        fv2 = atof(argv[3]);
        set_jt_att_value((RS_JT_E)iv1, fv2);
        printf("set_jt_att_value chl:%d att:%f\r\n", iv1, fv2);
    } else if (strcmp(cmd, "set_chl_sw") == 0 && argc == 4) {
        iv1 = (int32_t)atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_chl_sw((RS_OUT_E)iv1, iv2);
        printf("set_chl_sw chl:%d sw:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_chl_sw4") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_chl_sw4((RS_OUT_E)iv1, iv2);
        printf("set_ch_sw4 chl:%d sw4:%d \r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_chl_att") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        fv2 = atof(argv[3]);
        set_chl_att((RS_OUT_E)iv1, fv2);
        printf("set_chl_att chl:%d att:%f\r\n", iv1, fv2);
    } else if (strcmp(cmd, "set_chl_out_sel") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_chl_out_sel((RS_OUT_E)iv1, (DATA_SRC)iv2);
        printf("set_chl_out_sel DAC:%d SEL:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_jt_out_sel") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_jt_out_sel((RS_OUT_E)iv1, (DATA_SRC)iv2);
        printf("set_jt_out_sel DAC:%d SEL:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_dds") == 0 && argc == 3) {
        fv1 = atof(argv[2]);
        set_dds(fv1);
        printf("set_dds freq:%f\r\n", fv1);
    } else if (strcmp(cmd, "set_axis") == 0 && argc == 3) {
        iv1 = atoi(argv[2]);
        set_axis((RS_OUT_E)iv1, &cfg);
        printf("set_axis success");
    } else if (strcmp(cmd, "set_chl_delay") == 0 && argc == 5) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        iv3 = atoi(argv[4]);
        set_chl_delay((RS_OUT_E)iv1, (ALG_PATH_E)iv2, iv3);
        printf("set_chl_delay chl:%d path:%d delay:%d\r\n", iv1, iv2, iv3);
    } else if (strcmp(cmd, "set_dpl_df") == 0 && argc == 5) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        fv2 = atof(argv[4]);
        set_dpl_df((RS_OUT_E)iv1, (ALG_PATH_E)iv2, fv2);
        printf("set_dpl_df chl:%d path:%d freq:%f\r\n", iv1, iv2, fv3);
    } else if (strcmp(cmd, "set_dpl_dfs") == 0 && argc == 5) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        fv2 = atof(argv[4]);
        set_dpl_dfs((RS_OUT_E)iv1, (ALG_PATH_E)iv2, fv2);
        printf("set_dpl_dfs chl:%d path:%d freq:%f\r\n", iv1, iv2, fv3);
    } else if (strcmp(cmd, "set_gain") == 0 && argc == 5) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        fv2 = atof(argv[4]);
        set_gain((RS_OUT_E)iv1, (ALG_PATH_E)iv2, fv2);
        printf("set_gain chl:%d path:%d gain:%f\r\n", iv1, iv2, fv3);
    } else if (strcmp(cmd, "set_bypass_raxis") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_bypass_raxis((RS_OUT_E)iv1, iv2);
        printf("set_bypass_raxis chl:%d sw:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_bypass_iq") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_bypass_iq((RS_OUT_E)iv1, iv2);
        printf("set_bypass_iq chl:%d sw:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_bypass_laxis") == 0 && argc == 4) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        set_bypass_laxis((RS_OUT_E)iv1, iv2);
        printf("set_bypass_laxis chl:%d sw:%d\r\n", iv1, iv2);
    } else if (strcmp(cmd, "set_bypass_dpl_iq") == 0 && argc == 6) {
        iv1 = atoi(argv[2]);
        iv2 = atoi(argv[3]);
        iv3 = atoi(argv[4]);
        iv4 = atoi(argv[5]);
        set_bypass_dpl_iq((RS_OUT_E)iv1, iv2, iv3, iv4);
        printf("set_bypass chnl:%d path:%d dfs_sw:%d fd_sw:%d\r\n", iv1, iv2, iv3,iv4);
    } else if (strcmp(cmd, "fpga_init") == 0 && argc == 2) {
        fpga_init();
        printf("fpga_init\r\n");
    } else {
        fprintf(stderr, "Unknown command: %s\n", cmd);
        help(argv[0]);
        close_device();
        return 1;
    }

    close_device();
    return 0;

}
#endif
