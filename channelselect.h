#ifndef CHANNELSELECT_H
#define CHANNELSELECT_H

#include <QWidget>
#include "matrixwidget.h"

class ChannelSelect : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelSelect(QWidget *parent = nullptr);
    void initUI();

    int getSimuChannel();
    int getSwitchChannel();
signals:
    // 转发MatrixWidget的通道开关状态变化信号
    void channelSwitchChanged(int channelNum, bool switchFlag);

private:
    MatrixWidget *m_matrixWidget;
};

#endif // CHANNELSELECT_H
