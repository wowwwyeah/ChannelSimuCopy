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

private:

    MatrixWidget *m_matrixWidget;
};

#endif // CHANNELSELECT_H
