// RadioChannelManager.h
#ifndef RADIOCHANNELMANAGER_H
#define RADIOCHANNELMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include "configmanager.h"
#include "channelcachemanager.h"

typedef signed char INT8;
typedef unsigned char UINT8;

// 电台信道管理器类，负责管理4个电台之间的信道分配
class RadioChannelManager : public QObject
{
    Q_OBJECT

public:
    explicit RadioChannelManager(ConfigManager* configManager, QObject *parent = nullptr);
    ~RadioChannelManager();

    // 处理PTT值变化，更新信道分配
    void processPttChange(UINT8 newPtt);

    // 获取当前所有DAC的信道状态
    QVector<INT8> getDacChannels() const;

    // 获取当前所有DAC的选择器状态
    QVector<INT8> getDacSelections() const;

    // 发送参数到硬件
    void sendToHardware(int dacIndex, const ModelParaSetting& params);
    // 发送ChannelSetting参数到硬件
    void sendToHardware(int dacIndex, const ChannelSetting& params);
    //重设 dacNum:通道号 [1-4] chl:信道号 [-6,6]
    bool resetFpgaChl(int dacNum,int chl);
private:
    //释放 dacNum:通道号 [1-4]    //chl:信道号 [-6,6]
    bool releaseFpgaChl(int dacNum,int chl);

    void initialize();

    // 获取信道的描述字符串
    QString getChannelDescription(INT8 channel) const;

    // 获取当前PTT值
    UINT8 getCurrentPtt() const;

    // 获取状态字符串（用于调试）
    QString getStatusString() const;

    // 从接收变为发送时的DAC信道释放
    void recvFreeDacChl(UINT8 rs_bits);

    // 从发送变为接收时的DAC信道释放
    void sendFreeDacChl(UINT8 rs_bits);

    // 判断信道是否为指定电台发送的信道
    bool ifSend(UINT8 rs_idx, INT8 chl) const;

    // 判断信道是否为发送给指定电台的信道
    bool ifRecv(UINT8 rs_idx, INT8 chl) const;

    // 判断信道是否已经在运行（已分配到DAC）
    bool ifChannelRunning(INT8 chl) const;

    // 将信道加载到空闲的DAC通道
    void dacLoadChannel(INT8 chl);

    // 根据PTT值分配DAC信道
    void allocateDacChl(UINT8 ptt);

private:
    // 静态查找表
    static const INT8 ptt2chls[0x10][4];    // PTT值(0x0-0xf)到信道列表的映射表，每个PTT值对应4个可能的信道
    static const INT8 chl_send_tab[4][3];    // 电台发送信道表，4个电台每个电台对应3个可能的发送信道
    static const INT8 chl_recv_tab[4][3];    // 电台接收信道表，4个电台每个电台对应3个可能的接收信道
    static const char* chl_dsp_p[7];         // 正信道号(1-6)的显示描述字符串
    static const char* chl_dsp_n[7];         // 负信道号(-1--6)的显示描述字符串
    static const INT8 chl_sel_p[7];          // 正信道号到选择器值的映射表
    static const INT8 chl_sel_n[7];          // 负信道号到选择器值的映射表

    // 状态变量
    INT8 dac_chl[4];        // DAC通道承载的信道号
    INT8 dac_sel[4];        // DAC通道的目的电台号
    UINT8 ptt_val_old;      // 上一次的PTT值
    UINT8 ptt_val_current;  // 当前的PTT值

    // 配置管理器指针
    ConfigManager* m_configManager;

    // 互斥锁，用于保护线程安全的数据访问
    QMutex m_mutex;
};

#endif // RADIOCHANNELMANAGER_H
