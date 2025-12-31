#ifndef CHANNEL_UTILS_H
#define CHANNEL_UTILS_H
#include "fpga_driver.h"
/**
 * @brief 通道工具宏函数定义
 *
 * 包含各种编号范围检查的宏函数，返回bool类型
 */

// 判断电台编号是否在有效范围 [0, 4]
#define IS_VALID_RADIO_NUM(radio_num) ((radio_num) >= 0 && (radio_num) <= 4)

// 判断DAC通道号是否在有效范围 [0, 3]
#define IS_VALID_DAC_CHANNEL(dac_channel) ((dac_channel) >= 0 && (dac_channel) <= 3)

// 判断动态信道编号是否在有效范围 [-6, 6]
#define IS_VALID_DYNAMIC_CHANNEL(dynamic_channel) ((dynamic_channel) >= 0 && (dynamic_channel) <= 6)

// 判断侦察设备信道编号是否在有效范围 [7, 10]
#define IS_VALID_RECON_CHANNEL(recon_channel) ((recon_channel) >= 7 && (recon_channel) <= 10)

// 判断干扰器编号是否在有效范围 [11, 15]
#define IS_VALID_JAMMER(jammer_num) ((jammer_num) >= 11 && (jammer_num) <= 15)

// 判断算法PATH ID是否在有效范围 [0, 4]
#define IS_VALID_PATH(path_num) ((path_num) >= ALG_PATH_1 && (path_num) <= 4)

// 判断信道编号是否在有效范围 [-6, 15]
#define IS_VALID_CHANNEL(channel) ((channel) >= -6 && (channel) <= 15)

//判断ptt是否有效
#define IS_VALID_PTT(ptt) ((ptt) >= 0x0 && (ptt) <= 0xf)

#endif // CHANNEL_UTILS_H
