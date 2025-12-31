// PttMonitorThread.h
#ifndef PTTMONITORTHREAD_H
#define PTTMONITORTHREAD_H

#include <QThread>
#include <QAtomicInt>
#include <QSemaphore>
#include "RadioChannelManager.h"
#include "configmanager.h"
#include <QMutex>
class PttMonitorThread : public QThread
{
    Q_OBJECT

public:
    explicit PttMonitorThread(ConfigManager* configManager, QObject* parent = nullptr);
    ~PttMonitorThread();

    // 设置停止标志
    void stop();

    // 唤醒线程的方法
    void wakeUp();

protected:
    void run() override;

private:
    RadioChannelManager* m_manager;
    ConfigManager* m_configManager;
    QAtomicInt m_stopFlag;
    QAtomicInt m_currentPtt;
    QMutex m_mutex;
    QSemaphore m_semaphore;  // 用于唤醒线程的信号量
};

#endif // PTTMONITORTHREAD_H
