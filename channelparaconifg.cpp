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
        globalStatusMap.insert(i, radiostatus);
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
