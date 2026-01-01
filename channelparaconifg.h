#ifndef CHANNELPARACONIFG_H
#define CHANNELPARACONIFG_H

#include <QObject>

enum ChannelModel
{
    MODEL_AUTO,         //模板场景
    MODEL_DIY, 			//自定义场景
    MODEL_MAX,
};

enum RadioState
{
    RADIO_DISABLE,          //未启用
    RADIO_RECEIVE, 			//接收
    RADIO_TRANSMIT,         //发送
    RADIO_ALARM             //告警
};

typedef struct RadioControl
{
    int rxNum;              //接收信号数量
    int rxSignal[4];		//接收电台信号
}RadioControl;

typedef struct RadioStatus
{
    RadioState radioState;			//电台状态0：未启用，1：接收，2：发送
    int txPower;					//发送功率
}RadioStatus;

typedef struct RadioConfig
{
    int radioNum;							//电台数量
    RadioStatus	radioStatusConfig;			//电台状态
    RadioControl radioSignalConfig;			//电台信号接收
}RadioConfig;

typedef struct MultiPathType
{
    int pathNum;    			//路径编号
    int relativDelay;			//相对时延,单位ns
    int antPower;  				//衰减功率
    int freShift;  				//路径频移
    int freSpread; 				//路径频扩
    int dopplerType;			//多普勒谱类型
}MultiPathType;

typedef struct FilterParameter
{
    int params[19];                 //滤波器参数数组
}FilterParameter;

typedef struct ModelParaSetting
{
    int channelNum;                 //电台之间通道
    int modelType;                  //场景类型，自定义/模板
    QString modelName;			//场景名称
    double noisePower; 				//噪声功率
    double signalAnt;  				//信号衰减
    double comDistance;                //通信距离
    int multipathNum;               //多径数量
    int filterNum;                  //滤波器编号
    QList<MultiPathType> multipathType;    //多径设置
    bool switchFlag=false;                    //开关状态
    bool isChange=false; //配置是否改变
}ModelParaSetting;

class ChannelParaConifg : public QObject
{
    Q_OBJECT
public:
    ModelParaSetting m_channelPara;
    explicit ChannelParaConifg(QObject *parent = nullptr);
    void getChannelModelList();
    void getChannelConfig();
    void setChannelConfig(const ModelParaSetting &paraInfo);
    void getRadioConfig();
    void setRadioConfig(const RadioConfig &radioConfigInfo);

    // 获取指定编号的滤波器参数
    static FilterParameter getFilterParameter(int filterNum);

signals:

private:
    // 固定的滤波器参数（编号1-5）
    static const FilterParameter m_filterParameters[5]; // 索引0-4对应滤波器1-5（使用滤波器编号-1作为索引）
};

#endif // CHANNELPARACONIFG_H
