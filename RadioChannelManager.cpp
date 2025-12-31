// RadioChannelManager.cpp
#include "RadioChannelManager.h"
#include <QDebug>
#include "fpga_driver.h"
#include "channel_utils.h"
#include "channelparaconifg.h"
#include "configmanager.h"
// 静态查找表定义
const INT8 RadioChannelManager::ptt2chls[0x10][4] = {
    {0,  0,  0,  0},  // ptt=0
    {1,  2,  3,  0},  // ptt=1 ，电台1发送，信道编号1、2、3
    {static_cast<INT8>(-1), 4,  5,  0},  // ptt=2
    {2,  3,  4,  5},  // ptt=3
    {static_cast<INT8>(-2), static_cast<INT8>(-4), 6,  0},  // ptt=4
    {1,  3,  static_cast<INT8>(-4), 6},  // ptt=5
    {static_cast<INT8>(-1), 5,  static_cast<INT8>(-2), 6},  // ptt=6
    {3,  5,  6,  0},  // ptt=7
    {static_cast<INT8>(-3), static_cast<INT8>(-5), static_cast<INT8>(-6), 0},  // ptt=8
    {1,  2,  static_cast<INT8>(-5), static_cast<INT8>(-6)}, // ptt=9
    {static_cast<INT8>(-1), 4,  static_cast<INT8>(-3), static_cast<INT8>(-6)}, // ptt=a
    {2,  4,  static_cast<INT8>(-6), 0},  // ptt=b
    {static_cast<INT8>(-2), static_cast<INT8>(-4), static_cast<INT8>(-3), static_cast<INT8>(-5)}, // ptt=c
    {1,  static_cast<INT8>(-4), static_cast<INT8>(-5), 0},  // ptt=d
    {static_cast<INT8>(-1), static_cast<INT8>(-2), static_cast<INT8>(-3), 0},  // ptt=e
    {0,  0,  0,  0},  // ptt=f
};

const INT8 RadioChannelManager::chl_send_tab[4][3] = {
    {1,  2,  3},       // 源为电台1的信道号列表
    {static_cast<INT8>(-1), 4,  5},       // 源为电台2的信道号列表
    {static_cast<INT8>(-2), static_cast<INT8>(-4), 6},       // 源为电台3的信道号列表
    {static_cast<INT8>(-3), static_cast<INT8>(-5), static_cast<INT8>(-6)},      // 源为电台4的信道号列表
};

const INT8 RadioChannelManager::chl_recv_tab[4][3] = {
    {static_cast<INT8>(-1), static_cast<INT8>(-2), static_cast<INT8>(-3)},      // 目的为电台1的信道号列表
    {1,  static_cast<INT8>(-4), static_cast<INT8>(-5)},      // 目的为电台2的信道号列表
    {2,  4,  static_cast<INT8>(-6)},      // 目的为电台3的信道号列表
    {3,  5,  6},       // 目的为电台4的信道号列表
};

const char* RadioChannelManager::chl_dsp_p[7] = {
    "NONE",            // chl = 0
    "(D1->D2)",        // chl = 1
    "(D1->D3)",        // chl = 2
    "(D1->D4)",        // chl = 3
    "(D2->D3)",        // chl = 4
    "(D2->D4)",        // chl = 5
    "(D3->D4)",        // chl = 6
};

const char* RadioChannelManager::chl_dsp_n[7] = {
    "NONE",            // chl = 0
    "(D2->D1)",        // chl = -1
    "(D3->D1)",        // chl = -2
    "(D4->D1)",        // chl = -3
    "(D3->D2)",        // chl = -4
    "(D4->D2)",        // chl = -5
    "(D4->D3)",        // chl = -6
};

const INT8 RadioChannelManager::chl_sel_p[7] = {
    0,     // chl = 0
    2,     // chl = 1 信道1 送到电台2
    3,     // chl = 2
    4,     // chl = 3
    3,     // chl = 4
    4,     // chl = 5
    4,     // chl = 6
};

const INT8 RadioChannelManager::chl_sel_n[7] = {
    0,     // chl = 0
    1,     // chl = -1 信道-1 送到电台1
    1,     // chl = -2 信道-2 送到电台1
    1,     // chl = -3
    2,     // chl = -4
    2,     // chl = -5
    3,     // chl = -6
};

RadioChannelManager::RadioChannelManager(ConfigManager* configManager, QObject *parent)
    : QObject(parent)
    , ptt_val_old(0)
    , ptt_val_current(0)
    , m_configManager(configManager)
{
    initialize();
}

RadioChannelManager::~RadioChannelManager()
{
}

void RadioChannelManager::initialize()
{
    // 初始化DAC信道和选择器
    for (int i = 0; i < 4; i++) {
        dac_chl[i] = 0;
        dac_sel[i] = 0;
    }
    ptt_val_old = 0;
    ptt_val_current = 0;
}

QVector<INT8> RadioChannelManager::getDacChannels() const
{
    QVector<INT8> channels;
    for (int i = 0; i < 4; i++) {
        channels.append(dac_chl[i]);
    }
    return channels;
}

QVector<INT8> RadioChannelManager::getDacSelections() const
{
    QVector<INT8> selections;
    for (int i = 0; i < 4; i++) {
        selections.append(dac_sel[i]);
    }
    return selections;
}

QString RadioChannelManager::getChannelDescription(INT8 channel) const
{
    if (channel >= 0 && channel < 7) {
        return QString(chl_dsp_p[channel]);
    } else if (channel < 0 && channel > -7) {
        return QString(chl_dsp_n[channel * -1]);
    }
    return "INVALID";
}

void RadioChannelManager::processPttChange(UINT8 newPtt)
{
    if(!IS_VALID_PTT(newPtt)){
        qDebug() << "PTT值错误 - PTT:" << newPtt;
        return;
    }

    // 保存旧的PTT值用于比较
    UINT8 oldPtt = ptt_val_current;
    ptt_val_current = newPtt;

    // 输出释放前的状态
    QString beforeStatus = QString("before free: dac1=%1, dac2=%2, dac3=%3, dac4=%4, sel1=%5, sel2=%6, sel3=%7, sel4=%8")
                               .arg(getChannelDescription(dac_chl[0]))
                               .arg(getChannelDescription(dac_chl[1]))
                               .arg(getChannelDescription(dac_chl[2]))
                               .arg(getChannelDescription(dac_chl[3]))
                               .arg(static_cast<int>(dac_sel[0]))
                               .arg(static_cast<int>(dac_sel[1]))
                               .arg(static_cast<int>(dac_sel[2]))
                               .arg(static_cast<int>(dac_sel[3]));
    qDebug() << beforeStatus;

    // 获取从接收变为发送的电台列表
    UINT8 rs_bits = (ptt_val_current ^ oldPtt) & ptt_val_current;
    recvFreeDacChl(rs_bits);

    // 获取从发送变为接收的电台列表
    rs_bits = (ptt_val_current ^ oldPtt) & oldPtt;
    sendFreeDacChl(rs_bits);

    // 输出释放后的状态
    QString afterFreeStatus = QString("after free: dac1=%1, dac2=%2, dac3=%3, dac4=%4")
                                  .arg(getChannelDescription(dac_chl[0]))
                                  .arg(getChannelDescription(dac_chl[1]))
                                  .arg(getChannelDescription(dac_chl[2]))
                                  .arg(getChannelDescription(dac_chl[3]));
    qDebug() << afterFreeStatus;

    // 分配新信道
    allocateDacChl(ptt_val_current);

    // 输出分配后的状态
    QString afterAllocStatus = QString("after alloc: dac1=%1, dac2=%2, dac3=%3, dac4=%4, sel1=%5, sel2=%6, sel3=%7, sel4=%8")
                                   .arg(getChannelDescription(dac_chl[0]))
                                   .arg(getChannelDescription(dac_chl[1]))
                                   .arg(getChannelDescription(dac_chl[2]))
                                   .arg(getChannelDescription(dac_chl[3]))
                                   .arg(static_cast<int>(dac_sel[0]))
                                   .arg(static_cast<int>(dac_sel[1]))
                                   .arg(static_cast<int>(dac_sel[2]))
                                   .arg(static_cast<int>(dac_sel[3]));
    qDebug() << afterAllocStatus;

    // 更新旧PTT值
    ptt_val_old = ptt_val_current;

    // 同步更新电台状态到globalStatusMap
    for (int radioIdx = 1; radioIdx <= 4; radioIdx++) {
        // 检查当前电台的PTT位是否被设置
        bool isTransmit = (ptt_val_current & (1 << (radioIdx - 1))) != 0;

        // 创建RadioStatus对象
        RadioStatus status;
        status.radioState = isTransmit ? RADIO_TRANSMIT : RADIO_RECEIVE;

        // 更新globalStatusMap
        QMutexLocker locker(&globalMutex);
        globalStatusMap[radioIdx] = status;

        // 输出调试信息
        qDebug() << "电台" << radioIdx << "状态更新为:" << (isTransmit ? "发送" : "接收");
    }
}


void RadioChannelManager::sendToHardware(int dacIndex, const ModelParaSetting& params)
{
#if 0
    // 打印基本参数信息
    qDebug() << "[信道参数设置] 将参数设置到信道" << dacIndex;
    qDebug() << "[信道参数设置] 通道号:" << params.channelNum;
    qDebug() << "[信道参数设置] 模型类型:" << params.modelType;
    qDebug() << "[信道参数设置] 模型名称:" << params.modelName;
    qDebug() << "[信道参数设置] 噪声功率:" << params.noisePower;
    qDebug() << "[信道参数设置] 信号衰减:" << params.signalAnt;
    qDebug() << "[信道参数设置] 通信距离:" << params.comDistance;
    qDebug() << "[信道参数设置] 多径数量:" << params.multipathNum;
    qDebug() << "[信道参数设置] 滤波器编号:" << params.filterNum;
    qDebug() << "[信道参数设置] 信道开关：" << params.switchFlag;
#endif
    if(!IS_VALID_DAC_CHANNEL(dacIndex)){
        qDebug() << "[信道参数设置] 通道号错误 -dac 通道:" << dacIndex;
        return;
    }
    if(!IS_VALID_DYNAMIC_CHANNEL(qAbs(params.channelNum))){
        qDebug() << "[信道参数设置] 缓存中的信道号错误 - 信道号:" << params.channelNum;
        return;
    }
    qDebug() << "------------------------------信道参数设置-------------------------------------";

    //1、1/4选路
    int objRadioNumber=-1;//目标电台号

    int chl=dac_chl[dacIndex] ;

    if(chl<0){
        objRadioNumber=chl_sel_n[chl];
    }else{
        objRadioNumber=chl_sel_p[chl];
    }

    if(objRadioNumber<0){
        qDebug() << "[信道参数设置] 1、1/4选路设置失败 - 目标电台号错误";
        return;
    }

    qDebug() << "[信道参数设置] 1、设置1/4选路 - 通道:" << dacIndex << " 值: "<<objRadioNumber;
    int retsw4 = set_chl_sw4(static_cast<RS_OUT_E>(dacIndex), objRadioNumber);
    if (retsw4 != FPGA_OK) {
        qDebug() << "[信道参数设置] 1、1/4选路设置失败 - 错误码:" << retsw4;
    } else {
        qDebug() << "[信道参数设置] 1、1/4选路设置成功";
    }

    //2、衰减 —— 对应信道参数21
    qDebug() << "[信道参数设置] 2、设置信号衰减 - 通道:" << dacIndex << " 值:" << params.signalAnt;
    int retatt = set_chl_att(static_cast<RS_OUT_E>(dacIndex), static_cast<float>(params.signalAnt));
    if (retatt != FPGA_OK) {
        qDebug() << "[信道参数设置] 2、信号衰减设置失败 - 错误码:" << retatt;
    } else {
        qDebug() << "[信道参数设置] 2、信号衰减设置成功";
    }

    //3、算法参数 —— 对应信道参数0-19
    // 多径参数
    qDebug() << "[信道参数设置] 3、设置多径参数 - 通道:" << dacIndex;
    for (const auto& path : params.multipathType) {
#if 0
        qDebug() << "  [路径" << path.pathNum << "] 路径编号:" << path.pathNum;
        qDebug() << "  [路径" << path.pathNum << "] 相对时延(ns):" << path.relativDelay;
        qDebug() << "  [路径" << path.pathNum << "] 衰减功率:" << path.antPower;
        qDebug() << "  [路径" << path.pathNum << "] 路径频移:" << path.freShift;
        qDebug() << "  [路径" << path.pathNum << "] 路径频扩:" << path.freSpread;
        qDebug() << "  [路径" << path.pathNum << "] 多普勒谱类型:" << path.dopplerType;
        qDebug() << "  [路径" << path.pathNum << "] ------------------------------";
#endif
        //时延
        if(!IS_VALID_PATH(path.pathNum)){
            qDebug() << "  [路径" << path.pathNum << "] 路径编号错误 - 路径:" << path.pathNum;
            continue;
        }

        qDebug() << "  [路径" << path.pathNum << "] 设置相对时延 - 通道:" << dacIndex << " 路径:" << path.pathNum << " 值:" << path.relativDelay << "ns";
        int retdelay = set_chl_delay(static_cast<RS_OUT_E>(dacIndex),static_cast<ALG_PATH_E>(path.pathNum-1), path.relativDelay);
        if (retdelay != FPGA_OK) {
            qDebug() << "  [路径" << path.pathNum << "] 相对时延设置失败 - 错误码:" << retdelay;
        } else {
            qDebug() << "  [路径" << path.pathNum << "] 相对时延设置成功";
        }

        //频移
        qDebug() << "  [路径" << path.pathNum << "] 设置路径频移 - 通道:" << dacIndex << " 路径:" << path.pathNum << " 值:" << path.freShift << "Hz";
        int retshift = set_dpl_dfs(static_cast<RS_OUT_E>(dacIndex),static_cast<ALG_PATH_E>(path.pathNum-1), static_cast<float>(path.freShift));
        if (retshift != FPGA_OK) {
            qDebug() << "  [路径" << path.pathNum << "] 路径频移设置失败 - 错误码:" << retshift;
        } else {
            qDebug() << "  [路径" << path.pathNum << "] 路径频移设置成功";
        }

        //频扩
        qDebug() << "  [路径" << path.pathNum << "] 设置路径频扩 - 通道:" << dacIndex << " 路径:" << path.pathNum << " 值:" << path.freSpread << "Hz";
        int retspread = set_dpl_dfs(static_cast<RS_OUT_E>(dacIndex), static_cast<ALG_PATH_E>(path.pathNum-1), static_cast<float>(path.freSpread));
        if (retspread != FPGA_OK) {
            qDebug() << "  [路径" << path.pathNum << "] 路径频扩设置失败 - 错误码:" << retspread;
        } else {
            qDebug() << "  [路径" << path.pathNum << "] 路径频扩设置成功";
        }

        //衰减值
        qDebug() << "  [路径" << path.pathNum << "] 设置路径衰减功率 - 通道:" << dacIndex << " 路径:" << path.pathNum << " 值:" << path.antPower << "dB";
        int retgain = set_gain(static_cast<RS_OUT_E>(dacIndex), static_cast<ALG_PATH_E>(path.pathNum-1), static_cast<float>(path.antPower));
        if (retgain != FPGA_OK) {
            qDebug() << "  [路径" << path.pathNum << "] 路径衰减功率设置失败 - 错误码:" << retgain;
        } else {
            qDebug() << "  [路径" << path.pathNum << "] 路径衰减功率设置成功";
        }
    }

    //4、信道开关 —— 对应信道参数 20
    qDebug() << "[信道参数设置] 4、设置信道开关 - 通道:" << dacIndex << " 值:" << params.switchFlag;
    int retsw = set_chl_sw(static_cast<RS_OUT_E>(dacIndex), params.switchFlag);
    if (retsw != FPGA_OK) {
        qDebug() << "[信道参数设置] 4、信道开关设置失败 - 错误码:" << retsw;
    } else {
        qDebug() << "[信道参数设置] 4、信道开关设置成功";
    }

    //5、算法初始值 —— 暂未知如何取
    //qDebug() << "[信道参数设置] 5、算法初始值设置 - 暂未实现";

    //6、设置滤波器参数
    //qDebug() << "[信道参数设置] 6、设置滤波器参数 - 通道:" << dacIndex << " 滤波器编号:" << params.filterNum;

    //qDebug() << "[信道参数设置] 所有参数设置完成 - 通道:" << dacIndex;
    qDebug() << "-------------------------------信道参数设置------------------------------------";
}

UINT8 RadioChannelManager::getCurrentPtt() const
{
    return ptt_val_current;
}

QString RadioChannelManager::getStatusString() const
{
    QString status;
    status += QString("Current PTT: 0x%1\n").arg(ptt_val_current, 2, 16, QChar('0')).toUpper();
    status += "DAC Channels: ";
    for (int i = 0; i < 4; i++) {
        status += QString("%1 ").arg(dac_chl[i]);
    }
    status += "\nDAC Selections: ";
    for (int i = 0; i < 4; i++) {
        status += QString("%1 ").arg(dac_sel[i]);
    }
    return status;
}

bool RadioChannelManager::releaseFpgaChl(int dacNum,int chl)
{
    if(!IS_VALID_DAC_CHANNEL(dacNum)){
        qDebug() << "[FPGA通道释放] 通道号错误 - 通道:" << dacNum;
        return false;
    }
    qDebug() << "-----------------------------FPGA通道释放--------------------------------------";
    qDebug() << "[FPGA通道释放] 开始释放FPGA通道:" << dacNum << " 信道编号" << chl;

    // 设置DAC输出选择
    qDebug() << "[FPGA通道释放] 1、设置DAC输出选择 - 通道:" << dacNum << " 信道编号:" << DATA_SRC_NONE;
    int setChlRet=set_chl_out_sel(static_cast<RS_OUT_E>(dacNum),DATA_SRC_NONE);
    if (setChlRet != FPGA_OK) {
        qDebug() << "[FPGA通道释放] 1、DAC输出选择设置失败 - 错误码:" << setChlRet;
        return false;
    } else {
        qDebug() << "[FPGA通道释放] 1、DAC输出选择设置成功";
    }

    //设置通道开关
    qDebug() << "[FPGA通道释放] 4、设置信道开关 - 通道:" << dacNum << " 值:" << false;
    int retsw = set_chl_sw(static_cast<RS_OUT_E>(dacNum), false);
    if (retsw != FPGA_OK) {
        qDebug() << "[FPGA通道释放] 4、信道开关设置失败 - 错误码:" << retsw;
    } else {
        qDebug() << "[FPGA通道释放] 4、信道开关设置成功";
    }

    // 输出最终结果
    qDebug() << "[FPGA通道释放] 通道:" << dacNum << " 信道编号" << chl << "释放成功";
    qDebug() << "-----------------------------FPGA通道释放--------------------------------------";

    return true;
}

bool RadioChannelManager::resetFpgaChl(int dacNum,int chl){
    if(!IS_VALID_DAC_CHANNEL(dacNum)){
        qDebug() << "[FPGA通道设置] 通道号错误 - 通道:" << dacNum;
        return false;
    }
    if(!IS_VALID_DYNAMIC_CHANNEL(qAbs(chl))){
        qDebug() << "[FPGA通道设置] 缓存中的信道号错误 - 信道号:" << chl;
        return false;
    }
    qDebug() << "-----------------------------FPGA通道设置--------------------------------------";
    qDebug() << "[FPGA通道设置] 开始设置FPGA通道:" << dacNum << " 信道编号" << chl;

    // 设置DAC输出选择
    qDebug() << "[FPGA通道设置] 1、设置DAC输出选择 - 通道:" << dacNum << " 信道编号:" << chl;
    int setChlRet=set_chl_out_sel(static_cast<RS_OUT_E>(dacNum),static_cast<DATA_SRC>(chl));
    if (setChlRet != FPGA_OK) {
        qDebug() << "[FPGA通道设置] 1、DAC输出选择设置失败 - 错误码:" << setChlRet;
        return false;
    } else {
        qDebug() << "[FPGA通道设置] 1、DAC输出选择设置成功";
    }

    // 输出最终结果
    qDebug() << "[FPGA通道设置] 通道:" << dacNum << " 信道编号" << chl << "设置信道成功";
    qDebug() << "-----------------------------FPGA通道设置--------------------------------------";

    return true;
}

void RadioChannelManager::recvFreeDacChl(UINT8 rs_bits)
{
    if (rs_bits != 0) {
        for (UINT8 i = 0; i < 4; i++) {
            if (((rs_bits >> i) & 0x1) != 0) {
                // 电台i从接收变为了发送
                for (UINT8 j = 0; j < 4; j++) {
                    if (ifRecv(i, dac_chl[j]) == true) {
                        releaseFpgaChl(j,dac_chl[j]);
                        dac_chl[j] = 0; // 清除通道承载的信道号
                        dac_sel[j] = 0;
                    }
                }
            }
        }
    }
}

void RadioChannelManager::sendFreeDacChl(UINT8 rs_bits)
{
    if (rs_bits != 0) {
        for (UINT8 i = 0; i < 4; i++) {
            if (((rs_bits >> i) & 0x1) != 0) {
                // 电台i从发送变为了接收
                for (UINT8 j = 0; j < 4; j++) {
                    if (ifSend(i, dac_chl[j]) == true) {
                        releaseFpgaChl(j,dac_chl[j]);
                        dac_chl[j] = 0;
                        dac_sel[j] = 0;
                    }
                }
            }
        }
    }
}

bool RadioChannelManager::ifSend(UINT8 rs_idx, INT8 chl) const
{
    if (chl == 0) {
        return false;
    }

    for (int i = 0; i < 3; i++) {
        if (chl == chl_send_tab[rs_idx][i]) {
            return true;
        }
    }
    return false;
}

bool RadioChannelManager::ifRecv(UINT8 rs_idx, INT8 chl) const
{
    if (chl == 0) {
        return false;
    }

    for (int i = 0; i < 3; i++) {
        if (chl == chl_recv_tab[rs_idx][i]) {
            return true;
        }
    }
    return false;
}

bool RadioChannelManager::ifChannelRunning(INT8 chl) const
{
    for (int i = 0; i < 4; i++) {
        if (dac_chl[i] == chl) {
            return true;
        }
    }
    return false;
}

void RadioChannelManager::dacLoadChannel(INT8 chl)
{
    for (int i = 0; i < 4; i++) {
        if (dac_chl[i] == 0) {
            dac_chl[i] = chl;
            if (chl >= 0) {
                dac_sel[i] = chl_sel_p[chl];
            }
            else {
                dac_sel[i] = chl_sel_n[chl * -1];
            }
            return;
        }
    }
    qWarning() << "exception no free dac";
}

void RadioChannelManager::allocateDacChl(UINT8 ptt)
{
    for (int i = 0; i < 4; i++) {
        INT8 chl = ptt2chls[ptt][i];
        if (chl == 0) {
            // 0信道，不处理
            continue;
        }

        if (ifChannelRunning(chl) == true) {
            continue;
        }

        dacLoadChannel(chl);
    }
}
