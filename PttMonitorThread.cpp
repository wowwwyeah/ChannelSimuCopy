// PttMonitorThread.cpp
#include "PttMonitorThread.h"
#include <QDebug>
#include <QMutexLocker>
#include <QDateTime>
#include "fpga_driver.h"
#include "channel_utils.h"
PttMonitorThread::PttMonitorThread(ConfigManager* configManager, QObject* parent)
    : QThread(parent)
    , m_stopFlag(0)
    , m_currentPtt(0)
    , m_configManager(configManager)
    , m_manager(new RadioChannelManager(configManager, this))
{
}

PttMonitorThread::~PttMonitorThread()
{
    stop();
    wait();
    if(m_manager){
        delete m_manager;
    }
}

void PttMonitorThread::stop()
{
    m_stopFlag.storeRelaxed(1);
    m_semaphore.release(); // 唤醒线程以确保它能正确退出
}

void PttMonitorThread::wakeUp()
{
    m_semaphore.release(); // 唤醒等待中的线程
}

void PttMonitorThread::run()
{
    UINT8 lastPtt = 0;
    
    qDebug("PTT监控线程启动"); 
    while (!m_stopFlag.loadRelaxed()) {
#if USE_FPGA_TEST
        bool semaphoreAcquired = m_semaphore.tryAcquire(1, 10000);
#else
        // 等待信号量，超时时间为1ms,线程会定期唤醒检查PTT值，也会在信号量被释放时立即唤醒
        bool semaphoreAcquired = m_semaphore.tryAcquire(1, 10000);
#endif
        // 如果停止标志已设置，退出循环
        if (m_stopFlag.loadRelaxed()) {
            break;
        }
        // 获取当前PTT值
        UINT8 currentPtt;

        // 正常模式：获取当前PTT值从硬件
        struct radios radio_data;
        qDebug() << "-----------------------------获取PTT状态start--------------------------------------";
        int ret = get_ptt_sta_power(&radio_data); //待填
        if (ret != FPGA_OK) {
            // 获取失败，保持上次的值
            currentPtt = lastPtt;
            qDebug() << "获取PTT状态失败,错误玛： " << ret;
            continue; // 获取失败，跳过后续处理，直接进入下一次循环
        }
        
        // 检查PTT值是否在有效范围内 (0x0到0xf)
        if (!IS_VALID_PTT(radio_data.radio_sta)) {
            // 值不在有效范围内，保持上次的值
            currentPtt = lastPtt;
            qDebug() << "获取到无效的PTT状态值: " << radio_data.radio_sta << "，保持上次值: " << lastPtt;
            continue; // PTT值无效，跳过后续处理，直接进入下一次循环
        }
        
        // PTT值有效，更新currentPtt
        currentPtt = radio_data.radio_sta;
        // 设置当前PTT值
        m_currentPtt.storeRelaxed(currentPtt);
        qDebug() << "-----------------------------获取PTT状态end--------------------------------------";

        bool pttChanged = (currentPtt != lastPtt);

        if (pttChanged) {
            qDebug()<<"执行配置信道操作原因：[PTT变化]";
            qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
                     << "[PTT值改变]: 从0x" << QString::number(lastPtt, 16).toUpper() <<lastPtt
                     << "到0x" << QString::number(currentPtt, 16).toUpper()<<currentPtt;
            // 通过管理器处理PTT变化
            m_manager->processPttChange(currentPtt);
            // 更新lastPtt
            lastPtt = currentPtt;
        }

        // 当信号量被释放（semaphoreAcquired为true）或者PTT改变时，执行配置下发操作
        if (semaphoreAcquired || pttChanged) {
            if(semaphoreAcquired){
                qDebug()<<"执行配置信道操作原因：[信道开关改变]";
            }

            // 从管理器获取当前所有DAC通道承载的信道编号列表
            QVector<INT8> dacChannels = m_manager->getDacChannels();

            // 提取当前正在使用的信道编号（非0值）
            QVector<INT8> currentChannelList;
            for (INT8 channel : dacChannels) {
                if (channel != 0) {
                    currentChannelList.append(channel);
                }
            }

            // 将INT8转换为int后输出，避免字符编码问题
            QVector<int> intChannelList;
            for (INT8 channel : currentChannelList) {
                intChannelList.append(static_cast<int>(channel));
            }
            qDebug() << "当前正在使用的信道编号列表:" << intChannelList;

            // 从ConfigManager获取配置信息并发送到硬件
            if (m_configManager && !currentChannelList.isEmpty()) {
                // 获取所有配置键
                QList<QString> keys = m_configManager->getAllConfigKeys();

                qDebug()<<"当前缓存信道配置总数"<<keys.size();

                // 遍历当前正在使用的信道编号列表
                for (INT8 channelNum : currentChannelList) {
                    // 遍历所有配置，找到匹配当前信道编号的配置
                    for (const QString& key : keys) {
                        // 获取配置信息
                        ModelParaSetting config = m_configManager->getConfigFromMap(key);
                        int dacChannelIndex = dacChannels.indexOf(channelNum);

                        // 如果配置的信道编号与当前信道编号匹配
                        if (config.channelNum == qAbs(channelNum)) {
                            // 找到该信道对应的DAC通道索引
                            int dacChannelIndex = dacChannels.indexOf(channelNum);
                            if (dacChannelIndex != -1) {
                                // 发送参数到硬件
                                m_manager->sendToHardware(dacChannelIndex, config);
                                if(pttChanged){
                                    m_manager->resetFpgaChl(dacChannelIndex,channelNum);
                                }
                            }
                            break; // 找到匹配的配置后跳出内层循环
                        }
                    }
                }
            }
        }
    }

    qDebug("PTT监控线程停止");
}
