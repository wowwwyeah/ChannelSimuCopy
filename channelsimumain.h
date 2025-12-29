#ifndef CHANNELSIMUMAIN_H
#define CHANNELSIMUMAIN_H

#include <QWidget>

namespace Ui{
class ChannelSimuMain;
}

class ChannelSimuMain : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelSimuMain(QWidget *parent = nullptr);
    ~ChannelSimuMain();

signals:

private slots:
    void on_exitBT_clicked();
    void on_backBT_clicked();
    void on_nextBT_clicked();
    void on_modelAddBT_clicked();

private:
    Ui::ChannelSimuMain *ui;
    void initValue();
    bool checkModelSelected();
    bool createDiyModel(QString diyName);

};

#endif // CHANNELSIMUMAIN_H
