#include "channelparaconifg.h"
#include "configmanager.h"

ChannelParaConifg::ChannelParaConifg(QObject *parent)
    : QObject{parent}
{

}


void ChannelParaConifg::getChannelModelList()
{

}

void ChannelParaConifg::getChannelConfig()
{
    bool flag;
#if 0

#endif
}

void ChannelParaConifg::setChannelConfig(const ModelParaSetting &paraInfo)
{
    bool flag;
    int i = 0;
#if 0

    set_channel_sub(paraData.channelNum, paraData.signalAnt);
    for(i = 0; i < paraData.multipathNum; i++)
    {
        set_path_delay(paraData.channelNum, paraData.multipathType[i].pathNum, paraData.multipathType[i].relativDelay);
        set_path_delay(paraData.channelNum, paraData.multipathType[i].pathNum, paraData.multipathType[i].filterNum);
        set_path_delay(paraData.channelNum, paraData.multipathType[i].pathNum, paraData.multipathType[i].antPower);
        set_path_delay(paraData.channelNum, paraData.multipathType[i].pathNum, paraData.multipathType[i].freSpread);
        set_path_delay(paraData.channelNum, paraData.multipathType[i].pathNum, paraData.multipathType[i].freShift);
    }

#endif
}

void ChannelParaConifg::getRadioConfig()
{
    bool flag;
    int i = 0;

    //打桩函数
    for(i = 1; i <= 4; i++)
    {
        RadioStatus radiostatus;
        radiostatus.radioState = RADIO_TRANSMIT;
        radiostatus.txPower = 12;
        {
            QMutexLocker locker(&globalMutex);
            globalStatusMap.insert(i, radiostatus);
        }
    }

#if 0

    flag = get_radio_status();
#endif
}

void ChannelParaConifg::setRadioConfig(const RadioConfig &radioConfigInfo)
{
    bool flag;
    int i = 0;
#if 0

    for(i = 0; i < radioConfigInfo.radioNum; i++)
    {

    }
#endif

}

// 初始化固定的滤波器参数
const FilterParameter ChannelParaConifg::m_filterParameters[5] = {
    // 滤波器1参数（索引0）
    {{100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900}},

    // 滤波器2参数（索引1）
    {{200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000}},

    // 滤波器3参数（索引2）
    {{300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2100}},

    // 滤波器4参数（索引3）
    {{400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2100, 2200}},

    // 滤波器5参数（索引4）
    {{500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2100, 2200, 2300}}
};

// 获取指定编号的滤波器参数
FilterParameter ChannelParaConifg::getFilterParameter(int filterNum)
{
    // 检查滤波器编号是否在有效范围内（1-5）
    if (filterNum < 1 || filterNum > 5)
    {
        // 如果编号无效，返回默认参数（全0）
        static const FilterParameter defaultParam = {{0}};
        return defaultParam;
    }

    // 使用滤波器编号-1作为索引（滤波器1对应索引0，滤波器5对应索引4）
    return m_filterParameters[filterNum - 1];
}
